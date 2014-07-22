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
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <fcntl.h>
#include <string.h>
#include <syslog.h>
#include <errno.h>
#include "firejail.h"

void logmsg(const char *msg) {
	openlog("firejail", LOG_NDELAY | LOG_PID, LOG_DAEMON);
	syslog(LOG_INFO, "%s\n", msg);
	closelog();
}


void logerr(const char *msg) {
	openlog("firejail", LOG_NDELAY | LOG_PID, LOG_DAEMON);
	syslog(LOG_ERR, "%s\n", msg);
	closelog();
}


int mkpath(char* file_path, mode_t mode) {
	assert(file_path && *file_path);
	char* p;
	for (p=strchr(file_path+1, '/'); p; p=strchr(p+1, '/')) {
		*p='\0';
		if (mkdir(file_path, mode)==-1) {
			if (errno!=EEXIST) { *p='/'; return -1; }
		}
		*p='/';
	}
	return 0;
}


// return -1 if error, 0 if no error
int copy_file(const char *srcname, const char *destname) {
	assert(srcname);
	assert(destname);

	// open source
	int src = open(srcname, O_RDONLY);
	if (src < 0) {
		fprintf(stderr, "Warning: cannot open %s\n", srcname);
		return -1;
	}

	// open destination
	int dst = open(destname, O_CREAT|O_WRONLY|O_TRUNC, S_IRUSR | S_IWUSR | S_IRGRP | S_IROTH);
	if (dst < 0) {
		fprintf(stderr, "Warning: cannot open %s\n", destname);
		return -1;
	}

	// copy
	ssize_t len;
	static const int BUFLEN = 1024;
	unsigned char buf[BUFLEN];
	while ((len = read(src, buf, BUFLEN)) > 0) {
		int done = 0;
		while (done != len) {
			int rv = write(dst, buf + done, len - done);
			if (rv == -1) {
				close(src);
				close(dst);
				return -1;
			}

			done += rv;
		}
	}

	close(src);
	close(dst);
	return 0;
}


char *get_link(const char *fname) {
	assert(fname);
	struct stat sb;
	char *linkname;
	ssize_t r;

	if (lstat(fname, &sb) == -1)
		return NULL;

	linkname = malloc(sb.st_size + 1);
	if (linkname == NULL)
		return NULL;
	memset(linkname, 0, sb.st_size + 1);

	r = readlink(fname, linkname, sb.st_size + 1);
	if (r < 0)
		return NULL;
	return linkname;
}


int is_dir(const char *fname) {
	assert(fname);
	struct stat s;
	if (lstat(fname, &s) == 0) {
		if (S_ISDIR(s.st_mode))
			return 1;
	}

	return 0;
}


int is_link(const char *fname) {
	assert(fname);
	struct stat s;
	if (lstat(fname, &s) == 0) {
		if (S_ISLNK(s.st_mode))
			return 1;
	}

	return 0;
}


char *line_remove_spaces(const char *buf) {
	assert(buf);
	if (strlen(buf) == 0)
		return NULL;

	// allocate memory for the new string
	char *rv = malloc(strlen(buf) + 1);
	if (rv == NULL)
		errExit("malloc");

	// remove space at start of line
	const char *ptr1 = buf;
	while (*ptr1 == ' ' || *ptr1 == '\t')
		ptr1++;

	// copy data and remove additional spaces
	char *ptr2 = rv;
	int state = 0;
	while (*ptr1 != '\0') {
		if (*ptr1 == '\n' || *ptr1 == '\r')
			break;

		if (state == 0) {
			if (*ptr1 != ' ' && *ptr1 != '\t')
				*ptr2++ = *ptr1++;
			else {
				*ptr2++ = ' ';
				ptr1++;
				state = 1;
			}
		}
		else {				  // state == 1
			while (*ptr1 == ' ' || *ptr1 == '\t')
				ptr1++;
			state = 0;
		}
	}
	*ptr2 = '\0';

	return rv;
}

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
	char buffer[4096];
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
		if (buffer[i] >= 0x80) // execve in progress
			return NULL;
	}

	// return a malloc copy of the command line
	char *rv = strdup(buffer);
	if (strlen(rv) == 0) {
		free(rv);
		return NULL;
	}
	return rv;
}
	