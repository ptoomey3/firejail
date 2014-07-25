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
#include <stdlib.h>
#include <stdio.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <string.h>
#include <dirent.h>
#include <pwd.h>
#include <termios.h>
#include <sys/ioctl.h>
#include "../include/pid.h"

#define PIDS_BUFLEN 4096
Process pids[MAX_PIDS];
#define errExit(msg)    do { char msgout[500]; sprintf(msgout, "Error %s %s %d", msg, __FUNCTION__, __LINE__); perror(msgout); exit(1);} while (0)
static unsigned long long sysuptime = 0;
static unsigned clocktick = 0;
static unsigned pgs_rss = 0;
static unsigned pgs_shared = 0;

// get the memory associated with this pid
void pid_getmem(unsigned pid, unsigned *rss, unsigned *shared) {
	// open stat file
	char *file;
	if (asprintf(&file, "/proc/%u/statm", pid) == -1) {
		perror("asprintf");
		exit(1);
	}
	FILE *fp = fopen(file, "r");
	if (!fp) {
		free(file);
		return;
	}
	free(file);
	
	unsigned a, b, c;
	if (3 != fscanf(fp, "%u %u %u", &a, &b, &c)) {
		fclose(fp);
		return;
	}
	*rss += b;
	*shared += c;
	fclose(fp);
}


void pid_get_cpu_time(unsigned pid, unsigned *utime, unsigned *stime) {
	// open stat file
	char *file;
	if (asprintf(&file, "/proc/%u/stat", pid) == -1) {
		perror("asprintf");
		exit(1);
	}
	FILE *fp = fopen(file, "r");
	if (!fp) {
		free(file);
		return;
	}
	free(file);
	
	char line[PIDS_BUFLEN];
	if (fgets(line, PIDS_BUFLEN - 1, fp)) {
		char *ptr = line;
		// jump 13 fields
		int i;
		for (i = 0; i < 13; i++) {
			while (*ptr != ' ' && *ptr != '\t' && *ptr != '\0')
				ptr++;
			if (*ptr == '\0')
				goto myexit;
			ptr++;
		}
		if (2 != sscanf(ptr, "%u %u", utime, stime))
			goto myexit;
	}

myexit:	
	fclose(fp);
}

unsigned long long pid_get_start_time(unsigned pid) {
	// open stat file
	char *file;
	if (asprintf(&file, "/proc/%u/stat", pid) == -1) {
		perror("asprintf");
		exit(1);
	}
	FILE *fp = fopen(file, "r");
	if (!fp) {
		free(file);
		return 0;
	}
	free(file);
	
	char line[PIDS_BUFLEN];
	unsigned long long retval = 0;
	if (fgets(line, PIDS_BUFLEN - 1, fp)) {
		char *ptr = line;
		// jump 21 fields
		int i;
		for (i = 0; i < 21; i++) {
			while (*ptr != ' ' && *ptr != '\t' && *ptr != '\0')
				ptr++;
			if (*ptr == '\0')
				goto myexit;
			ptr++;
		}
		if (1 != sscanf(ptr, "%llu", &retval))
			goto myexit;
	}
	
myexit:
	fclose(fp);
	return retval;
}

char *pid_proc_cmdline(const pid_t pid) {
	// open /proc/pid/cmdline file
	char *fname;
	int fd;
	if (asprintf(&fname, "/proc/%d/cmdline", pid) == -1)
		return NULL;
	if ((fd = open(fname, O_RDONLY)) < 0) {
		free(fname);
		return NULL;
	}
	free(fname);

	// read file
	unsigned char buffer[PIDS_BUFLEN];
	ssize_t len;
	if ((len = read(fd, buffer, sizeof(buffer) - 1)) <= 0) {
		close(fd);
		return NULL;
	}
	buffer[len] = '\0';
	close(fd);

	// clean data
	int i;
	for (i = 0; i < len; i++) {
		if (buffer[i] == '\0')
			buffer[i] = ' ';
		if (buffer[i] >= 0x80) // execv in progress!!!
			return NULL;
	}

	// return a malloc copy of the command line
	char *rv = strdup((char *) buffer);
	if (strlen(rv) == 0) {
		free(rv);
		return NULL;
	}
	return rv;
}

char *pid_get_user_name(uid_t uid) {
	struct passwd *pw = getpwuid(uid);
	if (pw)
		return strdup(pw->pw_name);
	return NULL;
}

uid_t pid_get_uid(pid_t pid) {
	uid_t rv = 0;
	
	// open stat file
	char *file;
	if (asprintf(&file, "/proc/%u/status", pid) == -1) {
		perror("asprintf");
		exit(1);
	}
	FILE *fp = fopen(file, "r");
	if (!fp) {
		free(file);
		return 0;
	}

	// look for firejail executable name
	char buf[PIDS_BUFLEN];
	while (fgets(buf, PIDS_BUFLEN - 1, fp)) {
		if (strncmp(buf, "Uid:", 4) == 0) {
			char *ptr = buf + 5;
			while (*ptr != '\0' && (*ptr == ' ' || *ptr == '\t')) {
				ptr++;
			}
			if (*ptr == '\0')
				goto doexit;
				
			rv = atoi(ptr);
			break; // break regardless!
		}
	}
doexit:	
	fclose(fp);
	free(file);
	return rv;
}

static void print_elem(unsigned index, int nowrap) {
	// get terminal size
	struct winsize sz;
	int col = 0;
	if (isatty(STDIN_FILENO)) {
		if (!ioctl(0, TIOCGWINSZ, &sz))
			col  = sz.ws_col;
	}

	// indent
	char indent[(pids[index].level - 1) * 2 + 1];
	memset(indent, ' ', sizeof(indent));
	indent[(pids[index].level - 1) * 2] = '\0';

	// get data
	uid_t uid = pids[index].uid;
	char *cmd = pid_proc_cmdline(index);
	char *user = pid_get_user_name(uid);
	char *allocated = user;
	if (user ==NULL)
		user = "";
	if (cmd) {
		if (col < 4 || nowrap) 
			printf("%s%u:%s:%s\n", indent, index, user, cmd);
		else {
			char *out;
			if (asprintf(&out, "%s%u:%s:%s\n", indent, index, user, cmd) == -1)
				errExit("asprintf");
			int len = strlen(out);
			if (len > col) {
				out[col] = '\0';
				out[col - 1] = '\n';
			}
			printf("%s", out);
			free(out);
		}
				
		free(cmd);
	}
	else {
		if (pids[index].zombie)
			printf("%u: (zombie)\n", index);
		else
			printf("%u:\n", index);
	}
	if (allocated)
		free(allocated);
}

// recursivity!!!
void pid_print_tree(unsigned index, unsigned parent, int nowrap) {
	print_elem(index, nowrap);
	
	int i;
	for (i = index + 1; i < MAX_PIDS; i++) {
		if (pids[i].parent == index)
			pid_print_tree(i, index, nowrap);
	}
}


// recursivity!!!
void pid_store_cpu(unsigned index, unsigned parent, unsigned *utime, unsigned *stime) {
	if (pids[index].level == 1) {
		*utime = 0;
		*stime = 0;
	}
	
	unsigned utmp;
	unsigned stmp;
	pid_get_cpu_time(index, &utmp, &stmp);
	*utime += utmp;
	*stime += stmp;
	
	int i;
	for (i = index + 1; i < MAX_PIDS; i++) {
		if (pids[i].parent == index)
			pid_store_cpu(i, index, utime, stime);
	}

	if (pids[index].level == 1) {
		pids[index].utime = *utime;
		pids[index].stime = *stime;
	}
}

// mon_pid: pid of sandbox to be monitored, 0 if all sandboxes are included
void pid_read(pid_t mon_pid) {
	memset(pids, 0, sizeof(pids));
	pid_t mypid = getpid();

	DIR *dir;
	if (!(dir = opendir("/proc"))) {
		// sleep 2 seconds and try again
		sleep(2);
		if (!(dir = opendir("/proc"))) {
			fprintf(stderr, "Error: cannot open /proc directory\n");
			exit(1);
		}
	}
	
	pid_t child = -1;
	struct dirent *entry;
	char *end;
	while (child < 0 && (entry = readdir(dir))) {
		pid_t pid = strtol(entry->d_name, &end, 10);
		pid %= MAX_PIDS;
		if (end == entry->d_name || *end)
			continue;
		if (pid == mypid)
			continue;
		
		// open stat file
		char *file;
		if (asprintf(&file, "/proc/%u/status", pid) == -1) {
			perror("asprintf");
			exit(1);
		}
		FILE *fp = fopen(file, "r");
		if (!fp) {
			free(file);
			continue;
		}

		// look for firejail executable name
		char buf[PIDS_BUFLEN];
		while (fgets(buf, PIDS_BUFLEN - 1, fp)) {
			if (strncmp(buf, "Name:", 5) == 0) {
				char *ptr = buf + 5;
				while (*ptr != '\0' && (*ptr == ' ' || *ptr == '\t')) {
					ptr++;
				}
				if (*ptr == '\0') {
					fprintf(stderr, "Error: cannot read /proc file\n");
					exit(1);
				}

				if (mon_pid == 0 && strncmp(ptr, "firejail", 8) == 0) {
					pids[pid].level = 1;
				}
				else if (mon_pid == pid && strncmp(ptr, "firejail", 8) == 0) {
					pids[pid].level = 1;
				}
//				else if (mon_pid == 0 && strncmp(ptr, "lxc-execute", 11) == 0) {
//					pids[pid].level = 1;
//				}
//				else if (mon_pid == pid && strncmp(ptr, "lxc-execute", 11) == 0) {
//					pids[pid].level = 1;
//				}
				else
					pids[pid].level = -1;
			}
			if (strncmp(buf, "State:", 6) == 0) {
				if (strstr(buf, "(zombie)"))
					pids[pid].zombie = 1;
			}
			else if (strncmp(buf, "PPid:", 5) == 0) {
				char *ptr = buf + 5;
				while (*ptr != '\0' && (*ptr == ' ' || *ptr == '\t')) {
					ptr++;
				}
				if (*ptr == '\0') {
					fprintf(stderr, "Error: cannot read /proc file\n");
					exit(1);
				}
				unsigned parent = atoi(ptr);
				parent %= MAX_PIDS;
				if (pids[parent].level > 0) {
					pids[pid].level = pids[parent].level + 1;
					pids[pid].parent = parent;
				}
			}
			else if (strncmp(buf, "Uid:", 4) == 0) {
				if (pids[pid].level > 0) {
					char *ptr = buf + 5;
					while (*ptr != '\0' && (*ptr == ' ' || *ptr == '\t')) {
						ptr++;
					}
					if (*ptr == '\0') {
						fprintf(stderr, "Error: cannot read /proc file\n");
						exit(1);
					}
					pids[pid].uid = atoi(ptr);
				}
				break;
			}
		}
		fclose(fp);
		free(file);
	}
	closedir(dir);
}
