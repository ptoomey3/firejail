/*
 * Copyright (C) 2014, 2015 netblue30 (netblue30@yahoo.com)
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
#include <sys/file.h>
#include <sys/prctl.h>
#include <signal.h>
#include <time.h>

#include "firejail.h"
#include "../include/pid.h"

#define STACK_SIZE (1024 * 1024)
static char child_stack[STACK_SIZE];		// space for child's stack
Config cfg;					// configuration
int arg_private = 0;				// mount private /home and /tmp directoryu
int arg_debug = 0;				// print debug messages
int arg_nonetwork = 0;				// --net=none
int arg_noip = 0;				// --ip=none
int arg_command = 0;				// -c
int arg_overlay = 0;				// --overlay
int arg_zsh = 0;				// use zsh as default shell
int arg_csh = 0;				// use csh as default shell
int arg_seccomp = 0;				// enable seccomp filter
char *arg_seccomp_list = NULL;		// optional seccomp list
int arg_caps = 0;				// enable capabilities filter
int arg_trace = 0;				// syscall tracing support
int arg_rlimit_nofile = 0;			// rlimit nofile
int arg_rlimit_nproc = 0;			// rlimit nproc
int arg_rlimit_fsize = 0;				// rlimit fsize
int arg_rlimit_sigpending = 0;			// rlimit fsize
int arg_nox11 = 0;				// kill the program if x11 unix domain socket is accessed
int arg_nodbus = 0;				// kill the program if D-Bus is accessed

int fds[2];					// parent-child communication pipe
char *fullargv[MAX_ARGS];			// expanded argv for restricted shell
int fullargc = 0;
static pid_t child = 0;

static void myexit(int rv) {
	logmsg("exiting...");
	if (!arg_command)
		printf("\nparent is shutting down, bye...\n");
	exit(rv); 
}

static void my_handler(int s){
	printf("\nSignal %d caught, shutting down the child process\n", s);
	logsignal(s);
	kill(child, SIGKILL);
	myexit(1);
}

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
	
	cfg.cwd = getcwd(NULL, 0);
}


static void configure_bridge(Bridge *br, char *name) {
	assert(br);
	assert(name);

	br->dev = name;

	// check the bridge device exists
	char sysbridge[24 + strlen(br->dev)];
	sprintf(sysbridge, "/sys/class/net/%s/bridge", br->dev);
	struct stat s;
	int rv = stat(sysbridge, &s);
	if (rv < 0) {
		fprintf(stderr, "Error: cannot find bridge device %s, aborting\n", br->dev);
		exit(1);
	}
	if (net_bridge_addr(br->dev, &br->ip, &br->mask)) {
		fprintf(stderr, "Error: bridge device %s not configured, aborting\n",br->dev);
		exit(1);
	}
	if (arg_debug)
		printf("Bridge device %s at %d.%d.%d.%d/%d\n",
			br->dev, PRINT_IP(br->ip), mask2bits(br->mask));

	uint32_t range = ~br->mask + 1;		  // the number of potential addresses
	// this software is not supported for /31 networks
	if (range < 4) {
		fprintf(stderr, "Error: the software is not supported for /31 networks\n");
		exit(1);
	}
	br->configured = 1;
}


static void configure_ip(Bridge *br) {
	assert(br);
	if (br->configured == 0)
		return;

	if (arg_noip);
	else if (br->ipaddress) {
		// check network range
		char *rv = in_netrange(br->ipaddress, br->ip, br->mask);
		if (rv) {
			fprintf(stderr, "%s", rv);
			exit(1);
		}
		// send an ARP request and check if there is anybody on this IP address
		if (arp_check(br->dev, br->ipaddress, br->ip)) {
			fprintf(stderr, "Error: IP address %d.%d.%d.%d is already in use\n", PRINT_IP(br->ipaddress));
			exit(1);
		}
	}
	else
		br->ipaddress = arp_assign(br->dev, br->ip, br->mask);
}


static void configure_veth_pair(Bridge *br, const char *ifname, pid_t child) {
	assert(br);
	if (br->configured == 0)
		return;

	// create a veth pair
	char *dev;
	if (asprintf(&dev, "veth%u%s", getpid(), ifname) < 0)
		errExit("asprintf");
	net_create_veth(dev, ifname, child);

	// bring up the interface
	net_if_up(dev);

	// add interface to the bridge
	net_bridge_add_interface(br->dev, dev);

	char *msg;
	if (asprintf(&msg, "%d.%d.%d.%d address assigned to sandbox", PRINT_IP(br->ipaddress)) == -1)
		errExit("asprintf");
	logmsg(msg);
	fflush(0);
	free(msg);
}


static void bridge_wait_ip(Bridge *br) {
	assert(br);
	if (br->configured == 0)
		return;

	// wait for the ip address to come up
	int cnt = 0;
	while (cnt < 5) {			  // arp_check has a 1s wait
		if (arp_check(br->dev, br->ipaddress, br->ip) == 0)
			break;
		cnt++;
	}
}


static inline Bridge *last_bridge_configured(void) {
	if (cfg.bridge3.configured)
		return &cfg.bridge3;
	else if (cfg.bridge2.configured)
		return &cfg.bridge2;
	else if (cfg.bridge1.configured)
		return &cfg.bridge1;
	else if (cfg.bridge0.configured)
		return &cfg.bridge0;
	else
		return NULL;
}


void check_default_gw(uint32_t defaultgw) {
	assert(defaultgw);

	if (cfg.bridge0.configured) {
		char *rv = in_netrange(defaultgw, cfg.bridge0.ip, cfg.bridge0.mask);
		if (rv == 0)
			return;
	}
	if (cfg.bridge1.configured) {
		char *rv = in_netrange(defaultgw, cfg.bridge1.ip, cfg.bridge1.mask);
		if (rv == 0)
			return;
	}
	if (cfg.bridge2.configured) {
		char *rv = in_netrange(defaultgw, cfg.bridge2.ip, cfg.bridge2.mask);
		if (rv == 0)
			return;
	}
	if (cfg.bridge3.configured) {
		char *rv = in_netrange(defaultgw, cfg.bridge3.ip, cfg.bridge3.mask);
		if (rv == 0)
			return;
	}

	fprintf(stderr, "Error: default gateway %d.%d.%d.%d is not in the range of any network\n", PRINT_IP(defaultgw));
	exit(1);
}

static void network_check_cfg(void) {
	int net_configured = 0;
	if (cfg.bridge0.configured)
		net_configured++;
	if (cfg.bridge1.configured)
		net_configured++;
	if (cfg.bridge2.configured)
		net_configured++;
	if (cfg.bridge3.configured)
		net_configured++;

	// --net=none
	if (arg_nonetwork && net_configured) {
		fprintf(stderr, "Error: --net and --net=none are mutually exclusive\n");
		exit(1);
	}

	// --noip requires a network
	if (arg_noip && net_configured == 0) {
		fprintf(stderr, "Error: option --noip requires at least one network to be configured\n");
		exit(1);
	}

	// --defaultgw requires a network
	if (cfg.defaultgw && net_configured == 0) {
		fprintf(stderr, "Error: option --defaultgw requires at least one network to be configured\n");
		exit(1);
	}
}

// return 1 if error, 0 if a valid pid was found
static int read_pid(char *str, pid_t *pid) {
	char *endptr;
	errno = 0;
	pid_t pidtmp = strtol(str, &endptr, 10);
	if ((errno == ERANGE && (pidtmp == LONG_MAX || pidtmp == LONG_MIN))
		|| (errno != 0 && pidtmp == 0)) {
		return 1;
	}
	if (endptr == str) {
		return 1;
	}
	*pid = pidtmp;
	return 0;
}



//*******************************************
// Main program
//*******************************************
int main(int argc, char **argv) {
	int i;
	int prog_index = -1;			  // index in argv where the program command starts
#ifdef USELOCK
	int lockfd = -1;
#endif
	int arg_ipc = 0;
	int custom_profile = 0;	// custom profile loaded
	int arg_cgroup = 0;
	
	memset(&cfg, 0, sizeof(cfg));
	extract_user_data();
//	const pid_t ppid = getppid();
	const pid_t mypid = getpid();

	// is this a login shell?
	if (*argv[0] == '-') {
		fullargc = restricted_shell(cfg.username);
		if (fullargc) {
			int j;
			for (i = 1, j = fullargc; i < argc && j < MAX_ARGS; i++, j++, fullargc++)
				fullargv[j] = argv[i];

			// replace argc/argv with fullargc/fullargv
			argv = fullargv;
			argc = j;
		}
	}

	// parse arguments
	for (i = 1; i < argc; i++) {
		//*************************************
		// basic arguments
		//*************************************
		if (strcmp(argv[i], "--help") == 0 ||
		strcmp(argv[i], "-?") == 0) {
			usage();
			exit(0);
		}
		else if (strcmp(argv[i], "--version") == 0) {
			printf("firejail version %s\n", VERSION);
			exit(0);
		}
		else if (strcmp(argv[i], "--debug") == 0)
			arg_debug = 1;

		//*************************************
		// independent commands - the program will exit!
		//*************************************
		else if (strcmp(argv[i], "--debug-caps") == 0) {
			caps_print();
			exit(0);
		}
#ifdef HAVE_SECCOMP
		else if (strcmp(argv[i], "--debug-syscalls") == 0) {
			syscall_print();
			exit(0);
		}
#endif
		else if (strcmp(argv[i], "--list") == 0) {
			list();
			exit(0);
		}
		else if (strcmp(argv[i], "--tree") == 0) {
			tree();
			exit(0);
		}
		else if (strcmp(argv[i], "--top") == 0) {
			top();
			exit(0);
		}
		else if (strncmp(argv[i], "--join=", 7) == 0) {
			logargs(argc, argv);

			pid_t pid;
			if (read_pid(argv[i] + 7, &pid) == 0)		
				join(pid, cfg.homedir);
			else
				join_name(argv[i] + 7, cfg.homedir);
			// it will never get here!!!
			exit(0);
		}
		else if (strncmp(argv[i], "--shutdown=", 11) == 0) {
			logargs(argc, argv);
			
			pid_t pid;
			if (read_pid(argv[i] + 11, &pid) == 0)
				shut(pid);
			else
				shut_name(argv[i] + 11);
			exit(0);
		}
		
		//*************************************
		// filtering
		//*************************************
#ifdef HAVE_SECCOMP
		else if (strcmp(argv[i], "--seccomp") == 0)
			arg_seccomp = 1;
		else if (strncmp(argv[i], "--seccomp=", 10) == 0) {
			arg_seccomp = 1;
			arg_seccomp_list = strdup(argv[i] + 10);
			if (!arg_seccomp_list)
				errExit("strdup");
			// verify seccomp list and exit if problems
			if (syscall_check_list(arg_seccomp_list, NULL))
				return 1;
		}
#endif		
		else if (strcmp(argv[i], "--caps") == 0)
			arg_caps = 1;
		else if (strcmp(argv[i], "--trace") == 0)
			arg_trace = 1;
		else if (strncmp(argv[i], "--rlimit-nofile=", 16) == 0) {
			if (not_unsigned(argv[i] + 16)) {
				fprintf(stderr, "Error: invalid rlimt nofile\n");
				exit(1);
			}
			sscanf(argv[i] + 16, "%u", &cfg.rlimit_nofile);
			arg_rlimit_nofile = 1;
		}		
		else if (strncmp(argv[i], "--rlimit-nproc=", 15) == 0) {
			if (not_unsigned(argv[i] + 15)) {
				fprintf(stderr, "Error: invalid rlimt nproc\n");
				exit(1);
			}
			sscanf(argv[i] + 15, "%u", &cfg.rlimit_nproc);
			arg_rlimit_nproc = 1;
		}	
		else if (strncmp(argv[i], "--rlimit-fsize=", 15) == 0) {
			if (not_unsigned(argv[i] + 15)) {
				fprintf(stderr, "Error: invalid rlimt fsize\n");
				exit(1);
			}
			sscanf(argv[i] + 15, "%u", &cfg.rlimit_fsize);
			arg_rlimit_fsize = 1;
		}	
		else if (strncmp(argv[i], "--rlimit-sigpending=", 20) == 0) {
			if (not_unsigned(argv[i] + 20)) {
				fprintf(stderr, "Error: invalid rlimt sigpending\n");
				exit(1);
			}
			sscanf(argv[i] + 20, "%u", &cfg.rlimit_sigpending);
			arg_rlimit_sigpending = 1;
		}	
		else if (strncmp(argv[i], "--ipc-namespace", 15) == 0)
			arg_ipc = 1;
		else if (strncmp(argv[i], "--cpu=", 6) == 0)
			read_cpu_list(argv[i] + 6);
		else if (strcmp(argv[i], "--nox11") == 0) {
			// check if firejail lkm is present
			struct stat s;
			if (stat("/proc/firejail", &s) < 0) {
				fprintf(stderr, "Error: firejail Linux kernel module not found. The module"
					" is required for --nox11 option to work.\n");
				exit(1);
			}
			arg_nox11 = 1;
		}
		else if (strcmp(argv[i], "--nodbus") == 0) {
			// check if firejail lkm is present
			struct stat s;
			if (stat("/proc/firejail", &s) < 0) {
				fprintf(stderr, "Error: firejail Linux kernel module not found. The module"
					" is required for --nodbus option to work.\n");
				exit(1);
			}
			arg_nodbus = 1;
		}
		else if (strncmp(argv[i], "--cgroup=", 9) == 0) {
			if (arg_cgroup) {
				fprintf(stderr, "Error: only a cgroup can be defined\n");
				exit(1);
			}
			arg_cgroup = 1;
			set_cgroup(argv[i] + 9);
		}
		
		//*************************************
		// filesystem
		//*************************************
		else if (strncmp(argv[i], "--bind=", 7) == 0) {
			char *line;
			if (asprintf(&line, "bind %s", argv[i] + 7) == -1)
				errExit("asprintf");

			profile_check_line(line, 0);	// will exit if something wrong
			profile_add(line);
		}
		else if (strncmp(argv[i], "--tmpfs=", 8) == 0) {
			char *line;
			if (asprintf(&line, "tmpfs %s", argv[i] + 8) == -1)
				errExit("asprintf");
			
			profile_check_line(line, 0);	// will exit if something wrong
			profile_add(line);
		}
		else if (strncmp(argv[i], "--blacklist=", 12) == 0) {
			char *line;
			if (asprintf(&line, "blacklist %s", argv[i] + 12) == -1)
				errExit("asprintf");
			
			profile_check_line(line, 0);	// will exit if something wrong
			profile_add(line);
		}
		else if (strncmp(argv[i], "--read-only=", 12) == 0) {
			char *line;
			if (asprintf(&line, "read-only %s", argv[i] + 12) == -1)
				errExit("asprintf");
			
			profile_check_line(line, 0);	// will exit if something wrong
			profile_add(line);
		}
		else if (strcmp(argv[i], "--overlay") == 0) {
			if (cfg.chrootdir) {
				fprintf(stderr, "Error: --overlay and --chroot options are mutually exclusive\n");
				exit(1);
			}
			arg_overlay = 1;
		}
		else if (strncmp(argv[i], "--profile=", 10) == 0) {
			// multiple profile files are allowed!
			// check file access as user, not as root (suid)
			if (access(argv[i] + 10, R_OK)) {
				fprintf(stderr, "Error: cannot access profile file\n");
				return 1;
			}
			profile_read(argv[i] + 10);
			custom_profile = 1;
		}
		else if (strncmp(argv[i], "--chroot=", 9) == 0) {
			if (arg_overlay) {
				fprintf(stderr, "Error: --overlay and --chroot options are mutually exclusive\n");
				exit(1);
			}
			
			// extract chroot dirname
			cfg.chrootdir = argv[i] + 9;
			// if the directory starts with ~, expand the home directory
			if (*cfg.chrootdir == '~') {
				char *tmp;
				if (asprintf(&tmp, "%s%s", cfg.homedir, cfg.chrootdir + 1) == -1)
					errExit("asprintf");
				cfg.chrootdir = tmp;
			}
			// check chroot dirname exists
			struct stat s;
			int rv = stat(cfg.chrootdir, &s);
			if (rv < 0) {
				fprintf(stderr, "Error: cannot find %s directory, aborting\n", cfg.chrootdir);
				return 1;
			}
		}
		else if (strcmp(argv[i], "--private") == 0)
			arg_private = 1;
		else if (strncmp(argv[i], "--private=", 10) == 0) {
			// extract private home dirname
			cfg.home_private = argv[i] + 10;
			check_private_dir();
			arg_private = 1;
		}


		//*************************************
		// hostname
		//*************************************
		else if (strncmp(argv[i], "--name=", 7) == 0) {
			cfg.hostname = argv[i] + 7;
			if (strlen(cfg.hostname) == 0) {
				fprintf(stderr, "Error: please provide a name for sandbox\n");
				return 1;
			}
		}
		
		//*************************************
		// network
		//*************************************
		else if (strncmp(argv[i], "--net=", 6) == 0) {
			if (strcmp(argv[i] + 6, "none") == 0) {
				arg_nonetwork  = 1;
				cfg.bridge0.configured = 0;
				cfg.bridge1.configured = 0;
				cfg.bridge2.configured = 0;
				cfg.bridge3.configured = 0;
				continue;
			}

			Bridge *br;
			if (cfg.bridge0.configured == 0)
				br = &cfg.bridge0;
			else if (cfg.bridge1.configured == 0)
				br = &cfg.bridge1;
			else if (cfg.bridge2.configured == 0)
				br = &cfg.bridge2;
			else if (cfg.bridge3.configured == 0)
				br = &cfg.bridge3;
			else {
				fprintf(stderr, "Error: maximum 4 bridge devices allowed\n");
				return 1;
			}
			configure_bridge(br, argv[i] + 6);
		}
		else if (strcmp(argv[i], "--noip") == 0)
			arg_noip = 1;
		else if (strncmp(argv[i], "--ip=", 5) == 0) {
			Bridge *br = last_bridge_configured();
			if (br == NULL) {
				fprintf(stderr, "Error: no bridge device configured\n");
				return 1;
			}
			// configure this IP address for the last bridge defined
			if (atoip(argv[i] + 5, &br->ipaddress)) {
				fprintf(stderr, "Error: invalid IP address, aborting\n");
				return 1;
			}
		}
		else if (strncmp(argv[i], "--defaultgw=", 12) == 0) {
			if (atoip(argv[i] + 12, &cfg.defaultgw)) {
				fprintf(stderr, "Error: invalid IP address, aborting\n");
				return 1;
			}
		}

		//*************************************
		// command
		//*************************************
		else if (strcmp(argv[i], "--csh") == 0) {
			if (arg_zsh || cfg.shell ) {
				fprintf(stderr, "Error: only one default user shell can be specified\n");
				return 1;
			}
			arg_csh = 1;
		}
		else if (strcmp(argv[i], "--zsh") == 0) {
			if (arg_csh || cfg.shell ) {
				fprintf(stderr, "Error: only one default user shell can be specified\n");
				return 1;
			}
			arg_zsh = 1;
		}
		else if (strncmp(argv[i], "--shell=", 8) == 0) {
			if (arg_csh || arg_zsh) {
				fprintf(stderr, "Error: only one default user shell can be specified\n");
				return 1;
			}
			cfg.shell = argv[i] + 8;
			// check if the file exists
			struct stat s;
			if (stat(cfg.shell, &s) == -1) {
				fprintf(stderr, "Error: cannot find shell %s\n", cfg.shell);
				return 1;
			}
		}
		else if (strcmp(argv[i], "-c") == 0) {
			arg_command = 1;
			if (i == (argc -  1)) {
				fprintf(stderr, "Error: option -c requires an argument\n");
				return 1;
			}
		}
		else {
			// is this an invalid option?
			if (*argv[i] == '-') {
				fprintf(stderr, "Error: invalid %s command line option\n", argv[i]);
				return 1;
			}
		
			// we have a program name coming
			if (asprintf(&cfg.command_name, "%s", argv[i]) == -1)
				errExit("asprintf");
			// restrict the command name to the first word
			char *ptr = cfg.command_name;
			while (*ptr != ' ' && *ptr != '\t' && *ptr != '\0')
				ptr++;
			*ptr = '\0';
			prog_index = i;
			break;
		}
	}

	// check network configuration options - it will exit if anything wrong
	network_check_cfg();

	// log command
	logargs(argc, argv);
	if (fullargc) {
		char *msg;
		if (asprintf(&msg, "user %s entering restricted shell", cfg.username) == -1)
			errExit("asprintf");
		logmsg(msg);
		free(msg);
	}

	// build the sandbox command
	if (prog_index == -1 && arg_zsh) {
		cfg.command_line = "/usr/bin/zsh";
		cfg.command_name = "zsh";
	}
	else if (prog_index == -1 && arg_csh) {
		cfg.command_line = "/bin/csh";
		cfg.command_name = "csh";
	}
	else if (prog_index == -1 && cfg.shell) {
		cfg.command_line = cfg.shell;
		cfg.command_name = cfg.shell;
	}
	else if (prog_index == -1) {
		cfg.command_line = "/bin/bash";
		cfg.command_name = "bash";
	}
	else {
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
	
	// load the profile
	{
		assert(cfg.command_name);
		if (arg_debug)
			printf("Command name #%s#\n", cfg.command_name);		
		if (!custom_profile) {
			// look for a profile in ~/.config/firejail directory
			char *usercfgdir;
			if (asprintf(&usercfgdir, "%s/.config/firejail", cfg.homedir) == -1)
				errExit("asprintf");
			profile_find(cfg.command_name, usercfgdir);
			free(usercfgdir);
			custom_profile = 1;
		}
		if (!custom_profile) {
			// look for a user profile in /etc/firejail directory
			profile_find(cfg.command_name, "/etc/firejail");
			custom_profile = 1;
		}
	}

	// check and assign an IP address
	if (any_bridge_configured()) {
#ifdef USELOCK
		if (!arg_noip) {
			lockfd = open("/var/lock/firejail.lock", O_WRONLY | O_CREAT, S_IRUSR | S_IWUSR);
			if (lockfd != -1)
				flock(lockfd, LOCK_EX);
		}
#endif
		// initialize random number generator
		time_t t = time(NULL);
		srand(t ^ mypid);

		// check default gateway address
		if (cfg.defaultgw)
			check_default_gw(cfg.defaultgw);

		configure_ip(&cfg.bridge0);
		configure_ip(&cfg.bridge1);
		configure_ip(&cfg.bridge2);
		configure_ip(&cfg.bridge3);
	}

	// create the parrent-child communication pipe
	if (pipe(fds) < 0)
		errExit("pipe");

	// clone environment
	int flags = CLONE_NEWNS | CLONE_NEWPID | CLONE_NEWUTS | SIGCHLD;
	
	// in root mode also enable CLONE_NEWIPC
	// in user mode CLONE_NEWIPC will break MIT Shared Memory Extension (MIT-SHM)
	if (getuid() == 0 || arg_ipc)
		flags |= CLONE_NEWIPC;
	
	if (any_bridge_configured() || arg_nonetwork) {
		flags |= CLONE_NEWNET;
	}
	else if (arg_debug)
		printf("Using the local network stack\n");

	child = clone(sandbox,
		child_stack + STACK_SIZE,
		flags,
		NULL);
	if (child == -1)
		errExit("clone");

	if (!arg_command)
		printf("Parent pid %u, child pid %u\n", mypid, child);

	// create veth pair
	if (any_bridge_configured() && !arg_nonetwork) {
		configure_veth_pair(&cfg.bridge0, "eth0", child);
		configure_veth_pair(&cfg.bridge1, "eth1", child);
		configure_veth_pair(&cfg.bridge2, "eth2", child);
		configure_veth_pair(&cfg.bridge3, "eth3", child);
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
		bridge_wait_ip(&cfg.bridge0);
		bridge_wait_ip(&cfg.bridge1);
		bridge_wait_ip(&cfg.bridge2);
		bridge_wait_ip(&cfg.bridge3);
		flock(lockfd, LOCK_UN);
	}
#endif

	{
		char *msg;
		if (asprintf(&msg, "child %u started", child) == -1)
			errExit("asprintf");
		logmsg(msg);
		free(msg);
	}
	
	// handle CTRL-C in parent
	signal (SIGINT, my_handler);
	signal (SIGTERM, my_handler);
	
	// wait for the child to finish
	waitpid(child, NULL, 0);
	myexit(0);
	return 0;
}
