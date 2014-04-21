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

// return -1 if error, 0 if no error
int copy_file(const char *srcname, const char *destname) {
	assert(srcname);
	assert(destname);
	
	// open source
	int src = open(srcname, O_RDONLY);
	if (src < 0)
		return -1;
	
	// open destination
	int dst = open(destname, O_CREAT|O_WRONLY|O_TRUNC, S_IRUSR | S_IWUSR | S_IRGRP | S_IROTH);
	if (dst < 0)
		return -1;
	
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

