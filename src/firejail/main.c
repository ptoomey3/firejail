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
#include <sys/file.h>
#include <sys/prctl.h>
#include <signal.h>
#include <time.h>

#include "firejail.h"
#include "../include/pid.h"

#define STACK_SIZE (1024 * 1024)
static char child_stack[STACK_SIZE];		  // space for child's stack
Config cfg;					  // configuration
int arg_private = 0;				  // mount private /home and /tmp directoryu
int arg_debug = 0;				  // print debug messages
int arg_nonetwork = 0;				  // --net=none
int arg_noip = 0;				  	// --ip=none
int arg_command = 0;				  // -c
int arg_overlay = 0;				  // --overlay
int arg_zsh = 0;					// use zsh as default shell
int arg_csh = 0;					// use csh as default shell
int arg_seccomp = 0;				 // enable seccomp filter

int fds[2];					  // parent-child communication pipe
char *fullargv[MAX_ARGS];			  // expanded argv for restricted shell
int fullargc = 0;
static pid_t child = 0;

static void my_handler(int s){
	printf("\nSignal %d caught, shutting down the child process\n", s);
	kill(child, SIGKILL);
	exit(1); 
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


//*******************************************
// Main program
//*******************************************
int main(int argc, char **argv) {
	int i;
	int prog_index = -1;			  // index in argv where the program command starts
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
		else if (strcmp(argv[i], "--list") == 0) {
			list();
			exit(0);
		}
		else if (strcmp(argv[i], "--top") == 0) {
			top();
			exit(0);
		}
		else if (strncmp(argv[i], "--join=", 7) == 0) {
			char *endptr;
			errno = 0;
			pid_t pid = strtol(argv[i] + 7, &endptr, 10);
			if ((errno == ERANGE && (pid == LONG_MAX || pid == LONG_MIN))
			|| (errno != 0 && pid == 0)) {
				fprintf(stderr, "Error: invalid process ID\n");
				exit(1);
			}
			if (endptr == argv[i]) {
				fprintf(stderr, "Error: invalid process ID\n");
				exit(1);
			}
			logargs(argc, argv);
			join(pid, cfg.homedir);
			// it will never get here!!!
			exit(0);
		}
		
		//*************************************
		// misc features
		//*************************************
		else if (strcmp(argv[i], "--seccomp") == 0)
			arg_seccomp = 1;
		
		//*************************************
		// filesystem
		//*************************************
		else if (strcmp(argv[i], "--overlay") == 0) {
			arg_overlay = 1;
			set_exit = 1;
		}
		else if (strcmp(argv[i], "--private") == 0)
			arg_private = 1;
		else if (strncmp(argv[i], "--profile=",10) == 0) {
			// check file access as user, not as root (suid)
			if (access(argv[i] + 10, R_OK)) {
				fprintf(stderr, "Error: cannot access profile file\n");
				return 1;
			}
			profile_read(argv[i] + 10);
			set_exit = 1;
		}
		else if (strncmp(argv[i], "--chroot=", 9) == 0) {
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

		//*************************************
		// network
		//*************************************
		else if (strncmp(argv[i], "--name=", 7) == 0) {
			cfg.hostname = argv[i] + 7;
			if (strlen(cfg.hostname) == 0) {
				fprintf(stderr, "Error: please provide a name for sandbox\n");
				return 1;
			}
		}
		else if (strncmp(argv[i], "--net=", 6) == 0) {
			if (strcmp(argv[i] + 6, "none") == 0) {
				arg_nonetwork = 1;
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
		else if (strcmp(argv[i], "--csh") == 0)
			arg_csh = 1;
		else if (strcmp(argv[i], "--zsh") == 0)
			arg_zsh = 1;
		else if (strcmp(argv[i], "-c") == 0) {
			arg_command = 1;
			if (i == (argc -  1)) {
				fprintf(stderr, "Error: option -c requires an argument\n");
				return 1;
			}
		}
		else {
			// we have a program name coming
			if (asprintf(&cfg.command_name, "%s", argv[i]) == -1)
				errExit("asprintf");
			prog_index = i;
			break;
		}
	}

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
	else if (prog_index == -1) {
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

	if (set_exit)
		set_exit_parent(getpid(), arg_overlay);

	// clone environment
	int flags = CLONE_NEWNS | CLONE_NEWIPC | CLONE_NEWPID | CLONE_NEWUTS | SIGCHLD;
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
	logmsg("exiting...");
	bye_parent();
	return 0;
}
