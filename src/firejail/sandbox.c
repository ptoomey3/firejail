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
	
#include "firejail.h"
#include <sys/mount.h>
#include <sys/wait.h>
#include <sys/stat.h>
#include <sys/prctl.h>
#include <sys/time.h>
#include <sys/resource.h>

#define BUFLEN 500 // generic read buffer


void save_nogroups(void) {
	if (arg_nogroups == 0)
		return;

	char *fname;
	if (asprintf(&fname, "%s/groups", MNT_DIR) == -1)
		errExit("asprintf");
	FILE *fp = fopen(fname, "w");
	if (fp) {
		fprintf(fp, "\n");
		fclose(fp);
		if (chown(fname, 0, 0) < 0)
			errExit("chown");
	}
	else {
		fprintf(stderr, "Error: cannot save nogroups state\n");
		free(fname);
		exit(1);
	}
	
	free(fname);
}

static void sandbox_if_up(Bridge *br) {
	assert(br);
	if (!br->configured)
		return;
		
	char *dev = br->devsandbox;
	net_if_up(dev);
	if (br->arg_ip_none == 0) {
		assert(br->ipsandbox);
		if (arg_debug)
			printf("Configuring %d.%d.%d.%d address on interface %s\n", PRINT_IP(br->ipsandbox), dev);
		net_if_ip(dev, br->ipsandbox, br->mask);
		net_if_up(dev);
	}
}

static char *client_filter = 
"*filter\n"
":INPUT DROP [0:0]\n"
":FORWARD DROP [0:0]\n"
":OUTPUT ACCEPT [0:0]\n"
"-A INPUT -i lo -j ACCEPT\n"
"-A INPUT -m state --state RELATED,ESTABLISHED -j ACCEPT\n"
"-A INPUT -p icmp -j ACCEPT\n"
"COMMIT\n";



static void netfilter(const char *filter) {
	assert(filter);
	if (strcmp(filter, "client") == 0) {
		// mount a tempfs on top of /tmp directory
		if (mount("tmpfs", "/tmp", "tmpfs", MS_NOSUID | MS_STRICTATIME | MS_REC,  "mode=755,gid=0") < 0)
			errExit("mounting /tmp");
	
		// create the filter file
		FILE *fp = fopen("/tmp/client", "w");
		if (!fp) {
			fprintf(stderr, "Error: cannot open /tmp/client file\n");
			exit(1);
		}
		fprintf(fp, "%s\n", client_filter);
		fclose(fp);
		
		// push filter
		int rv;
		if (arg_debug) {
			printf("Installing network filter:\n");
			rv = system("cat /tmp/client");
		}
		rv = system("/sbin/iptables-restore < /tmp/client");
		if (rv == -1) {
			fprintf(stderr, "Error: failed to configure netfilter. Probablym iptables-restore command is missing.\n");
			exit(1);
		}
		if (arg_debug)
			rv = system("/sbin/iptables -vL");
		
		// unmount /tmp
		umount("/tmp");
	}
}



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
	// mount namespace
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
	
	//****************************
	// netfilter
	//****************************
	if (arg_netfilter && any_bridge_configured()) { // assuming by default the client filter
		netfilter("client");
	}

	//****************************
	// trace pre-install
	//****************************
	if (arg_trace)
		fs_trace_preload();

	//****************************
	// configure filesystem
	//****************************
	if (cfg.chrootdir) {
		fs_chroot(cfg.chrootdir);
		// force caps and seccomp if not started as root
		if (getuid() != 0) {
			// force seccomp inside the chroot
			arg_seccomp = 1;
			arg_seccomp_empty = 0; // force the default syscall list in case the user disabled it
			
			// disable all capabilities
			if (arg_caps_default_filter || arg_caps_custom_filter)
				fprintf(stderr, "Warning: all capabilities disabled for a regular user during chroot\n");
			arg_caps_default_filter = 0;
			arg_caps_custom_filter = 0;
			arg_caps_drop_all = 1;
			
			// drop all supplementary groups; /etc/group file inside chroot
			// is controlled by a regular usr
			arg_nogroups = 1;
			printf("Dropping all Linux capabilities and enforcing default seccomp filter\n");
		}
						
		//****************************
		// trace pre-install, this time inside chroot
		//****************************
		if (arg_trace)
			fs_trace_preload();
	}
	else if (arg_overlay)
		fs_overlayfs();
	else
		fs_basic_fs();
	

	//****************************
	// set hostname in /etc/hostname
	//****************************
	if (cfg.hostname) {
		fs_hostname(cfg.hostname);
	}
	
	//****************************
	// apply the profile file
	//****************************
	if (cfg.profile)
		fs_blacklist(cfg.homedir);
	
	//****************************
	// private mode
	//****************************
	if (arg_private) {
		if (cfg.home_private)
			fs_private_home();
		else
			fs_private();
	}
	
	//****************************
	// install trace
	//****************************
	if (arg_trace)
		fs_trace();
		
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
/*
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
*/
	else if (any_bridge_configured()) {
		// configure lo and eth0...eth3
		net_if_up("lo");
		sandbox_if_up(&cfg.bridge0);
		sandbox_if_up(&cfg.bridge1);
		sandbox_if_up(&cfg.bridge2);
		sandbox_if_up(&cfg.bridge3);
		
		// add a default route
		if (cfg.defaultgw) {
			// set the default route
			if (net_add_route(0, 0, cfg.defaultgw))
				fprintf(stderr, "Warning: cannot configure default route\n");
		}
			
		if (arg_debug)
			printf("Network namespace enabled\n");
	}
	net_ifprint();
	
	// if any dns server is configured, it is time to set it now
	fs_resolvconf();
	
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
	
	// set environment
	// fix qt 4.8
	if (setenv("QT_X11_NO_MITSHM", "1", 1) < 0)
		errExit("setenv");
	if (setenv("container", "firejail", 1) < 0) // LXC sets container=lxc,
		errExit("setenv");
	if (arg_zsh && setenv("SHELL", "/usr/bin/zsh", 1) < 0)
		errExit("setenv");
	if (arg_csh && setenv("SHELL", "/bin/csh", 1) < 0)
		errExit("setenv");
	if (cfg.shell && setenv("SHELL", cfg.shell, 1) < 0)
		errExit("setenv");
	// set prompt color to green
	//export PS1='\[\e[1;32m\][\u@\h \W]\$\[\e[0m\] '
	if (setenv("PROMPT_COMMAND", "export PS1=\"\\[\\e[1;32m\\][\\u@\\h \\W]\\$\\[\\e[0m\\] \"", 1) < 0)
		errExit("setenv");
		

	// set capabilities
	if (arg_caps_drop_all)
		caps_drop_all();
	else if (arg_caps_custom_filter)
		caps_set(arg_caps_custom_filter);
	else if (arg_caps_default_filter)
		caps_default_filter();

	// set rlimits
	set_rlimits();

	// set seccomp
#ifdef HAVE_SECCOMP
	if (arg_seccomp == 1)
		seccomp_filter(); // this will also save the filter to MNT_DIR/seccomp file
#endif

	// set cpu affinity
	if (cfg.cpus) {
		save_cpu(); // save cpu affinity mask to MNT_DIR/cpu file
		set_cpu_affinity();
	}
	
	// save cgroup in MNT_DIR/cgroup file
	if (cfg.cgroup)
		save_cgroup();
		
	// drop privileges
	save_nogroups();
	drop_privs(arg_nogroups);

	// set the shell
	char *sh;
	if (cfg.shell)
 		sh = cfg.shell;
	else if (arg_zsh)
		sh = "/usr/bin/zsh";
	else if (arg_csh)
		sh = "/bin/csh";
	else
		sh = "/bin/bash";
		
	char *arg[4];
	arg[0] = sh;
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
	execvp(sh, arg); 

	perror("execvp");
	return 0;
}
