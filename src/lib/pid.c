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
#include "../include/pid.h"

#define PIDS_BUFLEN 4096
Process pids[MAX_PIDS];

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
	char buffer[PIDS_BUFLEN];
	ssize_t len;
	if ((len = read(fd, buffer, sizeof(buffer) - 1)) <= 0) {
		close(fd);
		return NULL;
	}
	buffer[len] = '\0';
	close(fd);

	// clean data
	int i;
	for (i = 0; i < len; i++)
		if (buffer[i] == '\0')
			buffer[i] = ' ';

	// return a malloc copy of the command line
	char *rv = strdup(buffer);
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

int pid_is_firejail(pid_t pid) {
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
		if (strncmp(buf, "Name:", 5) == 0) {
			char *ptr = buf + 5;
			while (*ptr != '\0' && (*ptr == ' ' || *ptr == '\t')) {
				ptr++;
			}
			if (*ptr == '\0')
				goto doexit;
			if (strncmp(ptr, "firejail", 8) == 0)
				rv = 1;
			break;
		}
	}
doexit:	
	fclose(fp);
	free(file);
	return rv;
}


static void print_elem(unsigned index) {
	uid_t uid = pids[index].uid;
	int i;
	for (i = 0; i < pids[index].level - 1; i++)
		printf("  ");
	
	char *cmd = pid_proc_cmdline(index);
	char *user = pid_get_user_name(uid);
	char *allocated = user;
	if (user ==NULL)
		user = "";
	if (cmd) {
		if (strlen(cmd) > 55)
			printf("%u:%s:%-55.55s...\n", index, user, cmd);
		else
			printf("%u:%s:%s\n", index, user, cmd);
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
void pid_print_tree(unsigned index, unsigned parent) {
	print_elem(index);
	
	int i;
	for (i = index + 1; i < MAX_PIDS; i++) {
		if (pids[i].parent == index)
			pid_print_tree(i, index);
	}
}


void pid_read(void) {
	memset(pids, 0, sizeof(pids));
	pid_t mypid = getpid();

	DIR *dir;
	if (!(dir = opendir("/proc"))) {
		fprintf(stderr, "Error: cannot open /proc directory\n");
		exit(1);
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

				if (strncmp(ptr, "firejail", 8) == 0) {
					pids[pid].level = 1;
					break;
				}
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
				break; // break regardless!
			}
		}
		fclose(fp);
		free(file);
	}
	closedir(dir);
}
