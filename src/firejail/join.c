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
#include <sys/stat.h>
#include <sys/wait.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/prctl.h>

static int apply_caps = 0;
static uint64_t caps = 0;
static int apply_seccomp = 0;
#define BUFLEN 4096

static void extract_command(int argc, char **argv, int index) {
	if (index >= argc)
		return;

	// first argv needs to be a valid command
	if (*argv[index] == '-') {
		fprintf(stderr, "Error: invalid option %s after --join\n", argv[index]);
		exit(1);
	}


	int len = 0;
	int i;
	// calculate command length
	for (i = index; i < argc; i++) {
		len += strlen(argv[i]) + 1;
	}
	assert(len > 0);

	// build command
	cfg.command_line = malloc(len + 1);
	*cfg.command_line = '\0';
	for (i = index; i < argc; i++) {
		strcat(cfg.command_line, argv[i]);
		strcat(cfg.command_line, " ");
	}
	if (arg_debug)
		printf("Extracted command #%s#\n", cfg.command_line);
}

static void extract_cpu(pid_t pid) {
	char *fname;
	if (asprintf(&fname, "/proc/%d/root%s/cpu", pid, MNT_DIR) == -1)
		errExit("asprintf");
		
	struct stat s;
	if (stat(fname, &s) == -1)
		return;
	
	// there is a cpu file in MNT_DIR; load the information from the file
	load_cpu(fname);
	free(fname);
}

static void extract_cgroup(pid_t pid) {
	char *fname;
	if (asprintf(&fname, "/proc/%d/root%s/cgroup", pid, MNT_DIR) == -1)
		errExit("asprintf");
		
	struct stat s;
	if (stat(fname, &s) == -1)
		return;
	
	// there is a cgroup file in MNT_DIR; load the information from the file
	load_cgroup(fname);
	free(fname);
}

static void extract_caps_seccomp(pid_t pid) {
	// open stat file
	char *file;
	if (asprintf(&file, "/proc/%u/status", pid) == -1) {
		perror("asprintf");
		exit(1);
	}
	FILE *fp = fopen(file, "r");
	if (!fp) {
		free(file);
		fprintf(stderr, "Error: cannot open stat file for process %u\n", pid);
		exit(1);
	}

	char buf[BUFLEN];
	while (fgets(buf, BUFLEN - 1, fp)) {
		if (strncmp(buf, "Seccomp:", 8) == 0) {
			char *ptr = buf + 8;
			int val;
			sscanf(ptr, "%d", &val);
			if (val == 2)
				apply_seccomp = 1;
			break;
		}
		else if (strncmp(buf, "CapBnd:", 7) == 0) {		
			char *ptr = buf + 8;
			unsigned long long val;
			sscanf(ptr, "%llx", &val);
			apply_caps = 1;
			caps = val;
		}
	}
	fclose(fp);
	free(file);
}

void join_name(const char *name, const char *homedir, int argc, char **argv, int index) {
	if (!name || strlen(name) == 0) {
		fprintf(stderr, "Error: invalid sandbox name\n");
		exit(1);
	}
	pid_t pid;
	if (name2pid(name, &pid)) {
		fprintf(stderr, "Error: cannot find sandbox %s\n", name);
		exit(1);
	}

	join(pid, homedir, argc, argv, index);
}

void join(pid_t pid, const char *homedir, int argc, char **argv, int index) {
	extract_command(argc, argv, index);

	// if the pid is that of a firejail  process, use the pid of a child process inside the sandbox
	char *comm = pid_proc_comm(pid);
	if (comm) {
		// remove \n
		char *ptr = strchr(comm, '\n');
		if (ptr)
			*ptr = '\0';
		if (strcmp(comm, "firejail") == 0) {
			pid_t child;
			if (find_child(pid, &child) == 0) {
				pid = child;
				printf("Switching to pid %u, the first child process inside the sandbox\n", (unsigned) pid);
			}
		}
		free(comm);
	}

	// check privileges for non-root users
	uid_t uid = getuid();
	if (uid != 0) {
		struct stat s;
		char *dir;
		if (asprintf(&dir, "/proc/%u/ns", pid) == -1)
			errExit("asprintf");
		if (stat(dir, &s) < 0)
			errExit("stat");
		if (s.st_uid != uid) {
			fprintf(stderr, "Error: permission is denied to join a sandbox created by a different user.\n");
			exit(1);
		}
	}

	// in user mode set caps seccomp, cpu, cgroup
	if (getuid() != 0) {
		extract_caps_seccomp(pid);
		extract_cpu(pid);
		extract_cgroup(pid);
	}
	
	// set cgroup
	if (cfg.cgroup)
		set_cgroup(cfg.cgroup);
		
	// join namespaces
	if (join_namespace(pid, "ipc"))
		exit(1);
	if (join_namespace(pid, "net"))
		exit(1);
	if (join_namespace(pid, "pid"))
		exit(1);
	if (join_namespace(pid, "uts"))
		exit(1);
	if (join_namespace(pid, "mnt"))
		exit(1);

	pid_t child = fork();
	if (child < 0)
		errExit("fork");
	if (child == 0) {
		// chroot into /proc/PID/root directory
		char *rootdir;
		if (asprintf(&rootdir, "/proc/%d/root", pid) == -1)
			errExit("asprintf");
			
		int rv = chroot(rootdir); // this will fail for processes in sandboxes not started with --chroot option
		if (rv == 0)
			printf("changing root to %s\n", rootdir);
		
		prctl(PR_SET_PDEATHSIG, SIGKILL, 0, 0, 0); // kill the child in case the parent died
		if (chdir("/") < 0)
			errExit("chdir");
		if (homedir) {
			struct stat s;
			if (stat(homedir, &s) == 0) {
				if (chdir(homedir) < 0)
					errExit("chdir");
			}
		}
		
		// set cpu affinity
		if (cfg.cpus)
			set_cpu_affinity();
					
		// set caps filter
		if (apply_caps == 1)
			caps_set(caps);
#ifdef HAVE_SECCOMP
		// set seccomp filter
		if (apply_seccomp == 1)
			seccomp_set();
#endif
		
		// fix qt 4.8
		if (setenv("QT_X11_NO_MITSHM", "1", 1) < 0)
			errExit("setenv");
		if (setenv("container", "firejail", 1) < 0) // LXC sets container=lxc,
			errExit("setenv");
		// drop privileges
		drop_privs();

		// set prompt color to green
		//export PS1='\[\e[1;32m\][\u@\h \W]\$\[\e[0m\] '
		if (setenv("PROMPT_COMMAND", "export PS1=\"\\[\\e[1;32m\\][\\u@\\h \\W]\\$\\[\\e[0m\\] \"", 1) < 0)
			errExit("setenv");

		// run icmdline trough /bin/bash
		if (cfg.command_line == NULL)
			// replace the process with a regular bash session
			execlp("/bin/bash", "/bin/bash", NULL);
		else {
			// run the command supplied by the user


			char *arg[4];
			arg[0] = "/bin/bash";
			arg[1] = "-c";
			if (arg_debug)
				printf("Starting %s\n", cfg.command_line);
			arg[2] = cfg.command_line;
			arg[3] = NULL;
			execvp("/bin/bash", arg);
		}

//		execlp("/bin/bash", "/bin/bash", NULL);
		// it will never get here!!!
	}

	// wait for the child to finish
	waitpid(child, NULL, 0);
	exit(0);
}



