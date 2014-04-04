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
#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <dirent.h>
#include <string.h>
#include <fcntl.h>
#include "firejail.h"
#define BUFLEN 4096
#define MAX_PIDS 32769

typedef struct {
	unsigned char level;
	pid_t parent;
} Task;
Task pids[MAX_PIDS];

char *proc_cmdline(const pid_t pid) {
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
	char buffer[BUFLEN];
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

// recursivity!!!
void print_elem(unsigned index) {
	int i;
	for (i = 0; i < pids[index].level - 1; i++)
		printf("  ");
	
	char *cmd = proc_cmdline(index);
	if (cmd) {
		if (strlen(cmd) > 60)
			printf("%u:%-60.60s...\n", index, cmd);
		else
			printf("%u:%-60.60s\n", index, cmd);
		free(cmd);
	}
	else
		printf("%u", index);
}

void print_tree(unsigned index, unsigned parent) {
	print_elem(index);
	
	int i;
	for (i = index + 1; i < MAX_PIDS; i++) {
		if (pids[i].parent == index)
			print_tree(i, index);
	}
}

void list(void) {
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
		if (end == entry->d_name || *end)
			continue;

		if (pid == mypid)
			continue;
		
		// open stat file
		char *file;
		if (asprintf(&file, "/proc/%u/status", pid) == -1)
			errExit("Error asprintf");
		FILE *fp = fopen(file, "r");
		if (!fp) {
			free(file);
			continue;
		}

		// look for firejail executable name
		char buf[BUFLEN];
		while (fgets(buf, BUFLEN - 1, fp)) {
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
					pid %= MAX_PIDS;
					pids[pid].level = 1;
					break;
				}
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
				if (pids[parent].level) {
					pid %= MAX_PIDS;
					pids[pid].level = pids[parent].level + 1;
					pids[pid].parent = parent;
				}
				break; // break regardless!
			}
		}
		fclose(fp);
		free(file);
	}
	closedir(dir);
	
	// print files
	int i;
	for (i = 0; i < MAX_PIDS; i++) {
		if (pids[i].level == 1)
			print_tree(i, 0);
	}
}
