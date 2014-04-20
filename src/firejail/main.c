/*
 * Copyright (C) 2014 netblue30 (netblue30@yahoo.com)
 *
 * This file is part of firejail project
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License along
 * with this program; if not, write to the Free Software Foundation, Inc.,
 * 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA.
*/
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
#include <pwd.h>
#include <errno.h>
#include <limits.h>
#include <linux/prctl.h>
#ifdef USELOCK
#include <sys/file.h>
#endif


#include "firejail.h"

#define STACK_SIZE (1024 * 1024)
static char child_stack[STACK_SIZE];	// space for child's stack
Config cfg;
int arg_private = 0;		// mount private /home directoryu
int arg_debug = 0;		// print debug messages
int arg_nonetwork = 0;		// --net=none
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
	cfg.username = strdup(pw->pw_name);
	if (!cfg.username)
		errExit("strdup");

	// build home directory name
	cfg.homedir = NULL;
	if (pw->pw_dir != NULL) {
		cfg.homedir = strdup(pw->pw_dir);
		if (!cfg.homedir)
			errExit("strdup");
	}
	else {
		fprintf(stderr, "Error: user %s doesn't have a user directory assigned, aborting...\n", cfg.username);
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
	else {
		fprintf(stderr, "Error: cannot establish communication with the parent, exiting...\n");
		exit(1);
	}
	close(fds[0]);
	if (arg_debug && getpid() == 1)
			printf("PID namespace installed\n");

	//****************************
	// set hostname
	//****************************
	if (cfg.hostname) {
		if (sethostname(cfg.hostname, strlen(cfg.hostname)) < 0)
			errExit("sethostname");
	}

	//****************************
	// configure filesystem
	//****************************
	if (mount(NULL, "/", NULL, MS_SLAVE | MS_REC, NULL) < 0)
		errExit("mounting filesystem as slave");

	if (cfg.chrootdir) {
		fs_chroot(cfg.chrootdir);
	}
	else if (arg_overlay)
		fs_overlayfs();
	else
		fs_basic_fs();
	
	if (arg_private)
		fs_private(cfg.homedir);
		
	//****************************
	// apply the profile file
	//****************************
	assert(cfg.command_name);
	if (!cfg.custom_profile) {
		// look for a profile in ~/.config/firejail directory
		char *usercfg;
		if (asprintf(&usercfg, "%s/.config/firejail", cfg.homedir) == -1)
			errExit("asprintf");
		profile_find(cfg.command_name, usercfg);
	}
	if (!cfg.custom_profile)
		// look for a user profile in /etc/firejail directory
		profile_find(cfg.command_name, "/etc/firejail");
	if (cfg.custom_profile)
		fs_blacklist(cfg.custom_profile, cfg.homedir);

	fs_proc_sys();
	
	//****************************
	// networking
	//****************************
	if (arg_nonetwork) {
		net_if_up("lo");
	}
	else if (cfg.bridgedev && cfg.bridgeip && cfg.bridgemask) {
		assert(cfg.ipaddress);
		
		// configure lo and eth0
		net_if_up("lo");
		net_if_up("eth0");
		if (cfg.ipaddress) {
			if (arg_debug)
				printf("Configuring %d.%d.%d.%d address on interface eth0\n", PRINT_IP(cfg.ipaddress));
			net_if_ip("eth0", cfg.ipaddress, cfg.bridgemask);
			net_if_up("eth0");
		}
		
		// add a default route
		if (net_add_route(0, 0, cfg.bridgeip))
			fprintf(stderr, "Warning: cannot configure default route\n");
			
		if (arg_debug)
			printf("Network namespace enabled\n");
	}
	net_ifprint();
	
	//****************************
	// start executable
	//****************************
	prctl(PR_SET_PDEATHSIG, SIGKILL, 0, 0, 0); // kill the child in case the parent died
	if (chdir("/") < 0)
		errExit("chdir");
	if (cfg.homedir) {
		struct stat s;
		if (stat(cfg.homedir, &s) == 0) {
			if (chdir(cfg.homedir) < 0)
				errExit("chdir");
		}
	}
	// fix qt 4.8
	if (setenv("QT_X11_NO_MITSHM", "1", 1) < 0)
		errExit("setenv");
	if (setenv("container", "firejail", 1) < 0) // LXC sets container=lxc,
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
	assert(cfg.command_line);
	if (arg_debug)
		printf("Starting %s\n", cfg.command_line);
	arg[2] = cfg.command_line;
	arg[3] = NULL;

	if (!arg_command)
		printf("Child process initialized\n");
	if (arg_debug) {
		FILE *fp = fopen("/tmp/firejail.dbg", "a");
		if (fp) {			
			fprintf(fp, "child pid %u, execvp into %s\n\n", getpid(), cfg.command_line);
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
	int set_exit = 0;
#ifdef USELOCK
	int lockfd = -1;
#endif		
	memset(&cfg, 0, sizeof(cfg));
	extract_user_data();
	const pid_t ppid = getppid();	
	const pid_t mypid = getpid();

	// is this a login shell?
	if (*argv[0] == '-') {
		fullargc = restricted_shell(cfg.username);
		if (fullargc) {
			int j;
			for (i = 1, j = fullargc; i < argc && j < MAX_ARGS; i++, j++, fullargc++)
				fullargv[j] = argv[i];
			argv = fullargv;
			argc = j;
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
		else if (strcmp(argv[i], "--overlay") == 0) {
			arg_overlay = 1;
			set_exit = 1;
		}
		else if (strcmp(argv[i], "--private") == 0)
			arg_private = 1;
		else if (strcmp(argv[i], "--debug") == 0) {
			arg_debug = 1;
			FILE *fp = fopen("/tmp/firejail.dbg", "a");
			if (fp) {
				fprintf(fp, "parent pid %u\n", ppid);
				if (restricted_user)
					fprintf(fp, "user %s entering restricted shell\n", restricted_user);
				fprintf(fp, "pid %u, extended argument list: ", getpid());
				int j;
				for (j = 0; j < argc; j++)
					fprintf(fp, "%s ", argv[j]);
				fprintf(fp, "\n");
				fclose(fp);
				chmod("/tmp/firejail.dbg", S_IRWXU|S_IRWXG|S_IRWXO);
				int rv = chown("/tmp/firejail.dbg", 0, 0);
				(void) rv;
			}
		}			
		else if (strncmp(argv[i], "--profile=",10) == 0) {
			// check file access as user, not as root (suid)
			if (access(argv[i] + 10, R_OK)) {
				fprintf(stderr, "Error: cannot access profile file\n");
				return 1;
			}
			profile_read(argv[i] + 10);
			set_exit = 1;
		}
		else if (strncmp(argv[i], "--name=", 7) == 0) {
			cfg.hostname = argv[i] + 7;
			if (strlen(cfg.hostname) == 0) {
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
			
			join(pid, cfg.homedir);
		}
		else if (strncmp(argv[i], "--chroot=", 9) == 0) {
			// extract chroot dirname
			cfg.chrootdir = argv[i] + 9;
			// check chroot dirname exists
			struct stat s;
			int rv = stat(cfg.chrootdir, &s);
			if (rv < 0) {
				fprintf(stderr, "Error: cannot find %s directory, aborting\n", cfg.chrootdir);
				return 1;
			}
		}
		else if (strncmp(argv[i], "--net=", 6) == 0) {
			cfg.bridgedev = argv[i] + 6;
			if (strcmp(cfg.bridgedev, "none") == 0) {
				arg_nonetwork = 1;
				continue;
			}
			// check the bridge device exists
			char sysbridge[24 + strlen(cfg.bridgedev)];
			sprintf(sysbridge, "/sys/class/net/%s/bridge", cfg.bridgedev);
			struct stat s;
			int rv = stat(sysbridge, &s);
			if (rv < 0) {
				fprintf(stderr, "Error: cannot find bridge device %s, aborting\n", cfg.bridgedev);
				return 1;
			}
			if (net_bridge_addr(cfg.bridgedev, &cfg.bridgeip, &cfg.bridgemask)) {
				fprintf(stderr, "Error: bridge device %s not configured, aborting\n", cfg.bridgedev);
				return 1;
			}
			if (arg_debug)
				printf("Bridge device %s at %d.%d.%d.%d/%d\n",
					cfg.bridgedev, PRINT_IP(cfg.bridgeip), mask2bits(cfg.bridgemask));
			
			uint32_t range = ~cfg.bridgemask + 1; // the number of potential addresses
			// this software is not supported for /31 networks
			if (range < 4) {
				fprintf(stderr, "Error: the software is not supported for /31 networks\n");
				return 1;
			}
		}
		else if (strncmp(argv[i], "--ip=", 5) == 0) {
			if (atoip(argv[i] + 5, &cfg.ipaddress)) {
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
			if (asprintf(&cfg.command_name, "%s", argv[i]) == -1)
				errExit("asprintf");
			prog_index = i;
			break;		
		}
	}
	
	// build the sandbox command
	if (prog_index == -1) {
		cfg.command_line = "/bin/bash";
		cfg.command_name = "bash";
	}
	else {
		set_exit = 1;

		// calculate the length of the command
		int i;
		int len = 0;
		int argcnt = argc - prog_index;
		for (i = 0; i < argcnt; i++)
			len += strlen(argv[i + prog_index]) + 1; // + ' '
		
		// build the string
		cfg.command_line = malloc(len + 1); // + '\0'
		if (!cfg.command_line)
			errExit("malloc");
		char *ptr = cfg.command_line;
		for (i = 0; i < argcnt; i++) {
			sprintf(ptr, "%s ", argv[i + prog_index]);
			ptr += strlen(ptr);
		}
	}



	// check and assign an IP address
	if (cfg.bridgedev && cfg.bridgeip && cfg.bridgemask) {
#ifdef USELOCK
		lockfd = open("/var/firejail.lock", O_WRONLY | O_CREAT, S_IRUSR | S_IWUSR);
		if (lockfd != -1)
			flock(lockfd, LOCK_EX);	
#endif
		// initialize random number generator
		time_t t = time(NULL);
//		srand(t);
		srand(t ^ mypid);
	
		cfg.ipaddress = arp_assign(cfg.bridgedev, cfg.bridgeip, cfg.bridgemask, cfg.ipaddress);
	}

	// create the parrent-child communication pipe
	if (pipe(fds) < 0)
		errExit("pipe");
	
	if (set_exit)
		set_exit_parent(getpid(), arg_overlay);
	
	// clone environment
	int flags = CLONE_NEWNS | CLONE_NEWIPC | CLONE_NEWPID | CLONE_NEWUTS | SIGCHLD;
	if (cfg.bridgedev) {
		flags |= CLONE_NEWNET;
	}
	const pid_t child = clone(worker,
		child_stack + STACK_SIZE,
		flags,
		NULL);
	if (child == -1)
		errExit("clone");

	if (!arg_command)
		printf("Parent pid %u, child pid %u\n", mypid, child);
	
	// create veth pair
	if (cfg.bridgedev && !arg_nonetwork) {
		// create a veth pair
		char *dev;
		if (asprintf(&dev, "veth%u", mypid) < 0)
			errExit("asprintf");
		net_create_veth(dev, "eth0", child);

		// bring up the interface
		net_if_up(dev);
 		
 		// add interface to the bridge
		net_bridge_add_interface(cfg.bridgedev, dev);
	}

	// notify the child the initialization is done
	FILE* stream;
	close(fds[0]);
	stream = fdopen(fds[1], "w");
	fprintf(stream, "%u\n", child);
	fflush(stream);
	close(fds[1]);
	
#ifdef USELOCK
	if (lockfd != -1) {
		assert(cfg.bridgedev);
		assert(cfg.bridgeip);
		assert(cfg.bridgemask);
		assert(cfg.ipaddress);
		
		// wait for the ip address to come up
		int cnt = 0;
		while (cnt < 5) {
			if (arp_check(cfg.bridgedev, cfg.ipaddress, cfg.bridgeip) == 0)
				break;
			cnt++;
		}
		flock(lockfd, LOCK_UN);	
	}
#endif
	// wait for the child to finish
	waitpid(child, NULL, 0);
	bye_parent();
	return 0;
}
