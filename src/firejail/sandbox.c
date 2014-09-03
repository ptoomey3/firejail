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
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <sys/mount.h>
#include <sys/wait.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <assert.h>
#include <sys/prctl.h>
#include "firejail.h"

#define BUFLEN 500 // generic read buffer

int sandbox(void* sandbox_arg) {
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
	// mount events are not forwarded between the host the sandbox
	if (mount(NULL, "/", NULL, MS_SLAVE | MS_REC, NULL) < 0) {
		// if we are starting firejail inside firejail, we don't care about this
		char *mycont = getenv("container");
		if (mycont == NULL)
			errExit("mounting filesystem as slave");
		if (strcmp(mycont, "firejail") != 0)
			errExit("mounting filesystem as slave");
	}
	
	if (cfg.chrootdir) {
		fs_chroot(cfg.chrootdir);
	}
	else if (arg_overlay)
		fs_overlayfs();
	else
		fs_basic_fs();
	
	//****************************
	// apply the profile file
	//****************************
	// profiles are not handled in chroot mode; the profile file might be different in chroot than on host
	if (!cfg.chrootdir) {	
		assert(cfg.command_name);
		if (!cfg.custom_profile) {
			// look for a profile in ~/.config/firejail directory
			char *usercfgdir;
			if (asprintf(&usercfgdir, "%s/.config/firejail", cfg.homedir) == -1)
				errExit("asprintf");
			profile_find(cfg.command_name, usercfgdir);
		}
		if (!cfg.custom_profile)
			// look for a user profile in /etc/firejail directory
			profile_find(cfg.command_name, "/etc/firejail");
		if (cfg.custom_profile)
			fs_blacklist(cfg.custom_profile, cfg.homedir);
	}
	
	//****************************
	// private mode
	//****************************
	if (arg_private)
		fs_private(cfg.homedir);
		
	//****************************
	// update /proc, /dev, /boot directory
	//****************************
	fs_proc_sys_dev_boot();
	
	//****************************
	// networking
	//****************************
	if (arg_nonetwork) {
		net_if_up("lo");
	}
	else if (arg_noip) {
		net_if_up("lo");
		if (cfg.bridge0.configured)
			net_if_up("eth0");
		if (cfg.bridge1.configured)
			net_if_up("eth1");
		if (cfg.bridge2.configured)
			net_if_up("eth2");
		if (cfg.bridge3.configured)
			net_if_up("eth3");
	}
	else if (any_bridge_configured()) {
		// configure lo and eth0...eth3
		net_if_up("lo");
		if (cfg.bridge0.configured) {
			Bridge *br = &cfg.bridge0;
			net_if_up("eth0");
			assert(br->ipaddress);
			if (arg_debug)
				printf("Configuring %d.%d.%d.%d address on interface eth0\n", PRINT_IP(br->ipaddress));
			net_if_ip("eth0", br->ipaddress, br->mask);
			net_if_up("eth0");
		}
		if (cfg.bridge1.configured) {
			Bridge *br = &cfg.bridge1;
			net_if_up("eth1");
			assert(br->ipaddress);
			if (arg_debug)
				printf("Configuring %d.%d.%d.%d address on interface eth1\n", PRINT_IP(br->ipaddress));
			net_if_ip("eth1", br->ipaddress, br->mask);
			net_if_up("eth1");
		}
		if (cfg.bridge2.configured) {
			Bridge *br = &cfg.bridge2;
			net_if_up("eth2");
			assert(br->ipaddress);
			if (arg_debug)
				printf("Configuring %d.%d.%d.%d address on interface eth2\n", PRINT_IP(br->ipaddress));
			net_if_ip("eth2", br->ipaddress, br->mask);
			net_if_up("eth2");
		}
		if (cfg.bridge3.configured) {
			Bridge *br = &cfg.bridge3;
			net_if_up("eth3");
			assert(br->ipaddress);
			if (arg_debug)
				printf("Configuring %d.%d.%d.%d address on interface eth3\n", PRINT_IP(br->ipaddress));
			net_if_ip("eth3", br->ipaddress, br->mask);
			net_if_up("eth3");
		}
		
		// add a default route
		if (!cfg.defaultgw) {
			// set the default route as IP address of first bridge
			cfg.defaultgw = cfg.bridge0.ip;
			if (arg_debug)
				printf("Using first bridge address as default route\n");
		}
		if (net_add_route(0, 0, cfg.defaultgw))
			fprintf(stderr, "Warning: cannot configure default route\n");
			
		if (arg_debug)
			printf("Network namespace enabled\n");
	}
	net_ifprint();
	
	//****************************
	// start executable
	//****************************
	prctl(PR_SET_PDEATHSIG, SIGKILL, 0, 0, 0); // kill the child in case the parent died
	int cwd = 0;
	if (cfg.cwd) {
		if (chdir(cfg.cwd) == 0)
			cwd = 1;
	}
	
	if (!cwd) {
		if (chdir("/") < 0)
			errExit("chdir");
		if (cfg.homedir) {
			struct stat s;
			if (stat(cfg.homedir, &s) == 0) {
				if (chdir(cfg.homedir) < 0)
					errExit("chdir");
			}
		}
	}
	
	// fix qt 4.8
	if (setenv("QT_X11_NO_MITSHM", "1", 1) < 0)
		errExit("setenv");
	if (setenv("container", "firejail", 1) < 0) // LXC sets container=lxc,
		errExit("setenv");
	if (arg_zsh && setenv("SHELL", "/usr/bin/zsh", 1) < 0)
		errExit("setenv");
	if (arg_csh && setenv("SHELL", "/bin/csh", 1) < 0)
		errExit("setenv");

	// set capabilities
	caps_filter();

	// set seccomp
	if (arg_seccomp == 1)
		seccomp_filter();

	// drop privileges
	drop_privs();
	
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
		char *msg;
		if (asprintf(&msg, "child pid %s, execvp into %s", childstr, cfg.command_line) == -1)
			errExit("asprintf");
		logmsg(msg);
		free(msg);
	}
	execvp("/bin/bash", arg); 

	perror("execvp");
	return 0;
}
