#define _GNU_SOURCE
#include <sys/utsname.h>
#include <sched.h>
#include <string.h>
#include <unistd.h>
#include <sys/mount.h>
#include <sys/wait.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <assert.h>
#include <fcntl.h>
#include <dirent.h>
 #include <sys/types.h>
#include <sys/stat.h>
#include <pwd.h>
#include <errno.h>
#include <limits.h>
#include "firejail.h"

#define STACK_SIZE (1024 * 1024)
static char child_stack[STACK_SIZE];	// space for child's stack
static char *username = NULL;		// current user
static char *chrootdir = NULL;
static char *homedir = NULL;
static char *bridgedev = NULL;
static char *hostname = NULL;
static char *command_line = NULL;
static char *command_name = NULL;
static uint32_t ipaddress= 0;
static uint32_t bridgeip = 0;
static uint32_t bridgemask = 0;
char **custom_profile = NULL;
static int arg_private = 0;		// mount private /home directoryu
int arg_debug = 0;		// print debug messages
int arg_nonetwork = 0;
int arg_command = 0;		// -c
int arg_overlay = 0;		// --overlay

// parent-child communication pipe
int fds[2];

#define BUFLEN 500 // generic read buffer

static void extract_user_data(void) {
	// check suid
	if (geteuid()) {
		fprintf(stderr, "Error: the sandbox is not setuid root, aborting...\n");
		exit(1);
	}

	struct passwd *pw = getpwuid(getuid());
	if (!pw)
		errExit("getpwuid");
	username = strdup(pw->pw_name);
	if (!username)
		errExit("strdup");

	// build home directory name
	homedir = NULL;
	if (pw->pw_dir != NULL) {
		homedir = strdup(pw->pw_dir);
		if (!homedir)
			errExit("strdup");
	}
	else {
		fprintf(stderr, "Error: user %s doesn't have a user directory assigned, aborting...\n", username);
		exit(1);
	}
}

//*******************************************
// Worker thread
//*******************************************
int worker(void* worker_arg) {
	if (arg_debug)
		printf("Initializing child process\n");	
	
	//****************************
	// wait for the parent to be initialized
	//****************************
	char childstr[BUFLEN + 1];
	FILE* stream;
	close(fds[1]);
	stream = fdopen(fds[0], "r");
	*childstr = '\0';
	if (fgets(childstr, BUFLEN, stream)) {
		// remove \n
		char *ptr = childstr;
		while(*ptr !='\0' && *ptr != '\n')
			ptr++;
		if (*ptr == '\0')
			errExit("fgets");
		*ptr = '\0';
	}
	else
		errExit("fgets");
	close(fds[0]);
	if (arg_debug && getpid() == 1)
			printf("PID namespace installed\n");

	//****************************
	// set hostname
	//****************************
	if (hostname) {
		if (sethostname(hostname, strlen(hostname)) < 0)
			errExit("sethostname");
	}

	//****************************
	// configure filesystem
	//****************************
	if (mount(NULL, "/", NULL, MS_SLAVE | MS_REC, NULL) < 0)
		errExit("mount slave");

	if (chrootdir) {
		mnt_chroot(chrootdir);
	}
	else if (arg_overlay)
		mnt_overlayfs();
	else
		mnt_basic_fs();
	
	if (arg_private)
		mnt_home(homedir);
		
	//****************************
	// apply the profile file
	//****************************
	assert(command_name);
	if (!custom_profile) {
		// look for a profile in ~/.config/firejail directory
		char *usercfg;
		if (asprintf(&usercfg, "%s/.config/firejail", homedir) == -1)
			errExit("asprintf");
		get_profile(command_name, usercfg);
	}
	if (!custom_profile)
		// look for a user profile in /etc/firejail directory
		get_profile(command_name, "/etc/firejail");
	if (custom_profile)
		mnt_blacklist(custom_profile, homedir);

	mnt_proc_sys();
	
	//****************************
	// networking
	//****************************
	if (arg_nonetwork) {
		sleep(1);
		net_if_up("lo");
	}
	else if (bridgedev && bridgeip && bridgemask) {
		sleep(1);
		
		// configure lo and eth0
		net_if_up("lo");
		net_if_up("eth0");
		sleep(2);
		if (!ipaddress)
			ipaddress = arp(bridgeip, bridgemask);
		if (ipaddress) {
			if (arg_debug)
				printf("Configuring %d.%d.%d.%d address on interface eth0\n", PRINT_IP(ipaddress));
			net_if_ip("eth0", ipaddress, bridgemask);
		}
		sleep(1);
		
		// add a default route
		if (net_add_route(0, 0, bridgeip))
			fprintf(stderr, "Warning: cannot configure default route\n");
			
		if (arg_debug)
			printf("Network namespace enabled\n");
	}
	net_ifprint();
	
	//****************************
	// start executable
	//****************************
	if (chdir("/") < 0)
		errExit("chdir");
	struct stat s;
	if (stat(homedir, &s) == 0) {
		if (chdir(homedir) < 0)
			errExit("chdir");
	}
	// fix qt 4.8
	if (setenv("QT_X11_NO_MITSHM", "1", 1) < 0)
		errExit("setenv");
	// drop privileges
	if (setuid(getuid()) < 0)
		errExit("setuid/getuid");
	// set prompt color to green
	//export PS1='\[\e[1;32m\][\u@\h \W]\$\[\e[0m\] '
	if (setenv("PROMPT_COMMAND", "export PS1=\"\\[\\e[1;32m\\][\\u@\\h \\W]\\$\\[\\e[0m\\] \"", 1) < 0)
		errExit("setenv");
	char *arg[4];
	arg[0] = "bash";
	arg[1] = "-c";
	assert(command_line);
	if (arg_debug)
		printf("Starting %s\n", command_line);
	arg[2] = command_line;
	arg[3] = NULL;

	if (!arg_command)
		printf("Child process initialized\n");
	if (arg_debug) {
		FILE *fp = fopen("/tmp/firejail.log", "a");
		if (fp) {			
			fprintf(fp, "child pid %u, execvp into %s\n\n", getpid(), command_line);
			fclose(fp);
		}
	}
	execvp("/bin/bash", arg); 

	perror("execvp");
	return 0;
}

//*******************************************
// Main program
//*******************************************
char *fullargv[MAX_ARGS]; // expanded argv for restricted shell
int fullargc = 0;

int main(int argc, char **argv) {
	int i;
	int prog_index = -1;		// index in argv where the program command starts

	extract_user_data();
		
	// detect restricted shell calls from sshd
	pid_t ppid = getppid();	
	char *pcmd = proc_cmdline(ppid);

	if (pcmd) {
		printf("Parent %s, pid %u\n", pcmd, ppid);
		// sshd test
		if (strncmp(pcmd, "sshd", 4) == 0 /*&& strstr(pcmd, "notty") == NULL*/) {
			// test for restricted shell
			fullargc = restricted_shell(username);
			if (fullargc) {
				int j;
				for (i = 1, j = fullargc; i < argc && j < MAX_ARGS; i++, j++, fullargc++)
					fullargv[j] = argv[i];
				argv = fullargv;
				argc = j;
			}
		}
	}

	// parse arguments
	for (i = 1; i < argc; i++) {
		if (strcmp(argv[i], "--help") == 0 ||
		    strcmp(argv[i], "-?") == 0) {
			usage();
			return 0;
		}
		else if (strcmp(argv[i], "-c") == 0) {
			arg_command = 1;
			if (i == (argc -  1)) {
				fprintf(stderr, "Error: option -c requires an argument\n");
				return 1;
			}
		}
		else if (strcmp(argv[i], "--version") == 0) {
			printf("firejail version %s\n", VERSION);
			return 0;
		}
		else if (strcmp(argv[i], "--overlay") == 0)
			arg_overlay = 1;
		else if (strcmp(argv[i], "--private") == 0)
			arg_private = 1;
		else if (strcmp(argv[i], "--debug") == 0) {
			arg_debug = 1;
			FILE *fp = fopen("/tmp/firejail.log", "a");
			if (fp) {
				fprintf(fp, "parent pid %u, command %s\n", ppid, (pcmd)? pcmd: "unknown");
				if (restricted_user)
					fprintf(fp, "user %s entering restricted shell\n", restricted_user);
				fprintf(fp, "pid %u, extended argument list: ", getpid());
				int j;
				for (j = 0; j < argc; j++)
					fprintf(fp, "%s ", argv[j]);
				fprintf(fp, "\n");
				fclose(fp);
				chmod("/tmp/firejail.log", S_IRWXU|S_IRWXG|S_IRWXO);
				int rv = chown("/tmp/firejail.log", 0, 0);
				(void) rv;
			}
		}			
		else if (strncmp(argv[i], "--profile=",10) == 0)
			read_profile(argv[i] + 10);
		else if (strncmp(argv[i], "--name=", 7) == 0) {
			hostname = argv[i] + 7;
			if (strlen(hostname) == 0) {
				fprintf(stderr, "Error: please provide a name for sandbox\n");
				return 1;
			}
		}
		else if (strncmp(argv[i], "--join=", 7) == 0) {
			char *endptr;
			errno = 0;
			pid_t pid = strtol(argv[i] + 7, &endptr, 10);
			if ((errno == ERANGE && (pid == LONG_MAX || pid == LONG_MIN))
				|| (errno != 0 && pid == 0)) {
				fprintf(stderr, "Error: invalid process ID\n");
				return 1;
			}
			if (endptr == argv[i]) {
				fprintf(stderr, "Error: invalid process ID\n");
				return 1;
			}
			
			join(pid);
		}
		else if (strncmp(argv[i], "--chroot=", 9) == 0) {
			// extract chroot dirname
			chrootdir = argv[i] + 9;
			// check chroot dirname exists
			struct stat s;
			int rv = stat(chrootdir, &s);
			if (rv < 0) {
				fprintf(stderr, "Error: cannot find %s directory, aborting\n", chrootdir);
				return 1;
			}
		}
		else if (strncmp(argv[i], "--net=", 6) == 0) {
			bridgedev = argv[i] + 6;
			if (strcmp(bridgedev, "none") == 0) {
				arg_nonetwork = 1;
				continue;
			}
			// check the bridge device exists
			char sysbridge[24 + strlen(bridgedev)];
			sprintf(sysbridge, "/sys/class/net/%s/bridge", bridgedev);
			struct stat s;
			int rv = stat(sysbridge, &s);
			if (rv < 0) {
				fprintf(stderr, "Error: cannot find bridge device %s, aborting\n", bridgedev);
				return 1;
			}
			if (net_bridge_addr(bridgedev, &bridgeip, &bridgemask)) {
				fprintf(stderr, "Error: bridge device %s not configured, aborting\n", bridgedev);
				return 1;
			}
			
			if (arg_debug)
				printf("Bridge device %s at %d.%d.%d.%d/%d\n",
					bridgedev, PRINT_IP(bridgeip), mask2bits(bridgemask));
		}
		else if (strncmp(argv[i], "--ip=", 5) == 0) {
			if (atoip(argv[i] + 5, &ipaddress)) {
				fprintf(stderr, "Error: invalid IP address, aborting\n");
				return 1;
			}
		}
		else if (strcmp(argv[i], "--list") == 0) {
			list();
			return 0;
		}		
		else if (strncmp(argv[i], "--", 2) == 0) {
			fprintf(stderr, "Error: invalid argument, aborting\n\n");
			usage();
			return 1;
		}
		else if (strncmp(argv[i], "-", 1) == 0) {
			fprintf(stderr, "Error: invalid argument, aborting\n\n");
			usage();
			return 1;
		}
		else {
			// we have a program name coming
			if (asprintf(&command_name, "%s", argv[i]) == -1)
				errExit("asprintf");
			prog_index = i;
			break;		
		}
	}
	
	// build the sandbox command
	if (prog_index == -1) {
		command_line = "/bin/bash";
		command_name = "bash";
	}
	else {
		// calculate the length of the command
		int i;
		int len = 0;
		int argcnt = argc - prog_index;
		for (i = 0; i < argcnt; i++)
			len += strlen(argv[i + prog_index]) + 1; // + ' '
		
		// build the string
		command_line = malloc(len + 1); // + '\0'
		if (!command_line)
			errExit("malloc");
		char *ptr = command_line;
		for (i = 0; i < argcnt; i++) {
			sprintf(ptr, "%s ", argv[i + prog_index]);
			ptr += strlen(ptr);
		}
	}
	
	// if not a tty, create a new session
	if (!isatty(fileno(stdin))) {
		if (setsid() == -1)
			errExit("setsid");
	}
	
	// create the parrent-child communication pipe
	if (pipe(fds) < 0)
		errExit("pipe");
	
	set_exit_parent(getpid());

	// clone environment
	int flags = CLONE_NEWNS | CLONE_NEWIPC | CLONE_NEWPID | CLONE_NEWUTS | SIGCHLD;
	if (bridgedev) {
		flags |= CLONE_NEWNET;
	}
	const pid_t child = clone(worker,
		child_stack + STACK_SIZE,
		flags,
		NULL);
	if (child == -1)
		errExit("clone");

	const pid_t mypid = getpid();
	if (!arg_command)
		printf("Parent pid %u, child pid %u\n", mypid, child);
	
	// create veth pair
	if (bridgedev && !arg_nonetwork) {
		char cmd[200];
		sprintf(cmd, "/bin/ip link add veth%u type veth peer name eth0 netns %u", mypid, child);
		if (system(cmd) < 0)
			errExit("system");
		
		sprintf(cmd, "veth%u", mypid);
		net_if_up(cmd);

		sprintf(cmd, "/sbin/brctl addif %s veth%u", bridgedev, mypid);
		if (system(cmd) < 0)
			errExit("system");
	}

	// notify the child the initialization is done
	FILE* stream;
	close(fds[0]);
	stream = fdopen(fds[1], "w");
	fprintf(stream, "%u\n", child);
	fflush(stream);
	close(fds[1]);
	
	// wait for the child to finish
	waitpid(child, NULL, 0);
	bye_parent();
	return 0;
}
