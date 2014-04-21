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
#ifdef USELOCK
#include <sys/file.h>
#endif


#include "firejail.h"
#include "../include/pid.h"

#define STACK_SIZE (1024 * 1024)
static char child_stack[STACK_SIZE];	// space for child's stack
Config cfg;			// configuration
int arg_private = 0;		// mount private /home and /tmp directoryu
int arg_debug = 0;		// print debug messages
int arg_nonetwork = 0;		// --net=none
int arg_noip = 0;			// --ip=none
int arg_command = 0;		// -c
int arg_overlay = 0;		// --overlay
int fds[2];			// parent-child communication pipe

char *fullargv[MAX_ARGS]; // expanded argv for restricted shell
int fullargc = 0;


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
// Main program
//*******************************************
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
			
			logmsg(pid_proc_cmdline(mypid));
			join(pid, cfg.homedir);
			// it will never get here!!!
			exit(0);
		}
		
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
			if (strcmp(argv[i] + 5, "none") == 0)
				arg_noip = 1;
			else if (atoip(argv[i] + 5, &cfg.ipaddress)) {
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
	logmsg(pid_proc_cmdline(mypid));
	if (fullargc) {
		int i;
		int len = 0;
		for (i = 1; i < fullargc; i++)
			len += strlen(fullargv[i]) + 1; // + ' '
		char cmd[len + 50];
		strcpy(cmd, "expanded args: ");
		char *ptr = cmd + strlen(cmd);
		for (i = 1; i < fullargc; i++) {
			sprintf(ptr, "%s ", fullargv[i]);
			ptr += strlen(ptr);
		} 
		logmsg(cmd);
		
		char *msg;
		if (asprintf(&msg, "user %s entering restricted shell", cfg.username) == -1)
			errExit("asprintf");
		logmsg(msg);
		free(msg);
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
		if (cfg.defaultgw) {
			// check network range
			char *rv = in_netrange(cfg.defaultgw, cfg.bridgeip, cfg.bridgemask);
			if (rv) {
				fprintf(stderr, "%s", rv);
				exit(1);
			}
		}

		if (arg_noip);
		else if (cfg.ipaddress) {
			// check network range
			char *rv = in_netrange(cfg.ipaddress, cfg.bridgeip, cfg.bridgemask);
			if (rv) {
				fprintf(stderr, "%s", rv);
				exit(1);
			}
			// send an ARP request and check if there is anybody on this IP address
			if (arp_check(cfg.bridgedev, cfg.ipaddress, cfg.bridgeip)) {
				fprintf(stderr, "Error: IP address %d.%d.%d.%d is already in use\n", PRINT_IP(cfg.ipaddress));
				exit(1);
			}
		}
		else
			cfg.ipaddress = arp_assign(cfg.bridgedev, cfg.bridgeip, cfg.bridgemask);
		
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
	const pid_t child = clone(sandbox,
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
		
		char *msg;
		if (asprintf(&msg, "%d.%d.%d.%d address assigned to sandbox", PRINT_IP(cfg.ipaddress)) == -1)
			errExit("asprintf");
		logmsg(msg);
		fflush(0);
		free(msg);
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
		while (cnt < 5) { // arp_check has a 1s wait
			if (arp_check(cfg.bridgedev, cfg.ipaddress, cfg.bridgeip) == 0)
				break;
			cnt++;
		}
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
	// wait for the child to finish
	waitpid(child, NULL, 0);
	logmsg("exiting...");
	bye_parent();
	return 0;
}
