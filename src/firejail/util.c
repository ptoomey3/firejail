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
#include <fcntl.h>
#include <syslog.h>
#include <errno.h>
#include <dirent.h>
#include <grp.h>

#define MAX_GROUPS 1024
// drop privileges
// - for root group or if nogroups is set, supplementary groups are not configured
void drop_privs(int nogroups) {
	gid_t gid = getgid();

	// configure supplementary groups
	if (gid == 0 || nogroups) {
		if (setgroups(0, NULL) < 0)
			errExit("setgroups");
	}
	else {
		assert(cfg.username);
		gid_t groups[MAX_GROUPS];
		int ngroups = MAX_GROUPS;
		int rv = getgrouplist(cfg.username, gid, groups, &ngroups);

		if (arg_debug && rv) {
			printf("username %s, groups ", cfg.username);
			int i;
			for (i = 0; i < ngroups; i++)
				printf("%u, ", groups[i]);
			printf("\n");
		}

		if (rv == -1) {
			fprintf(stderr, "Warning: cannot extract supplementary group list, dropping them\n");
			if (setgroups(0, NULL) < 0)
				errExit("setgroups");
		}
		else {
			rv = setgroups(ngroups, groups);
			if (rv) {
				fprintf(stderr, "Warning: cannot set supplementary group list, dropping them\n");
				if (setgroups(0, NULL) < 0)
					errExit("setgroups");
			}
		}
	}

	// set uid/gid
	if (setgid(getgid()) < 0)
		errExit("setgid/getgid");
	if (setuid(getuid()) < 0)
		errExit("setuid/getuid");
}



void logsignal(int s) {
	openlog("firejail", LOG_NDELAY | LOG_PID, LOG_USER);
	syslog(LOG_INFO, "Signal %d caught", s);
	closelog();
}

void logmsg(const char *msg) {
	openlog("firejail", LOG_NDELAY | LOG_PID, LOG_USER);
	syslog(LOG_INFO, "%s\n", msg);
	closelog();
}

void logargs(int argc, char **argv) {
	int i;
	int len = 0;

	// calculate message length
	for (i = 0; i < argc; i++)
		len += strlen(argv[i]) + 1; // + ' '

	// build message
	char msg[len + 1];
	char *ptr = msg;
	for (i = 0; i < argc; i++) {
		sprintf(ptr, "%s ", argv[i]);
		ptr += strlen(ptr);
	}

	// log message
	logmsg(msg);
}


void logerr(const char *msg) {
	openlog("firejail", LOG_NDELAY | LOG_PID, LOG_USER);
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
		fprintf(stderr, "Warning: cannot open %s, file not copied\n", srcname);
		return -1;
	}

	// open destination
	int dst = open(destname, O_CREAT|O_WRONLY|O_TRUNC, S_IRUSR | S_IWUSR | S_IRGRP | S_IROTH);
	if (dst < 0) {
		fprintf(stderr, "Warning: cannot open %s, file not copied\n", destname);
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
	if (r < 0) {
		free(linkname);
		return NULL;
	}
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

	// strip last blank character if any
	if (*(ptr2 - 1) == ' ')
		--ptr2;
	*ptr2 = '\0';
//	if (arg_debug)
//		printf("Processing line #%s#\n", rv);

	return rv;
}


char *split_comma(char *str) {
	if (str == NULL || *str == '\0')
		return NULL;
	char *ptr = strchr(str, ',');
	if (!ptr)
		return NULL;
	*ptr = '\0';
	ptr++;
	if (*ptr == '\0')
		return NULL;
	return ptr;
}


int not_unsigned(const char *str) {
	int rv = 0;
	const char *ptr = str;
	while (*ptr != ' ' && *ptr != '\t' && *ptr != '\0') {
		if (!isdigit(*ptr)) {
			rv = 1;
			break;
		}
		ptr++;
	}

	return rv;
}

#define BUFLEN 4096
// find the first child for this parent; return 1 if error
int find_child(pid_t parent, pid_t *child) {
	*child = 0;	// use it to flag a found child

	DIR *dir;
	if (!(dir = opendir("/proc"))) {
		// sleep 2 seconds and try again
		sleep(2);
		if (!(dir = opendir("/proc"))) {
			fprintf(stderr, "Error: cannot open /proc directory\n");
			exit(1);
		}
	}

	struct dirent *entry;
	char *end;
	while (*child == 0 && (entry = readdir(dir))) {
		pid_t pid = strtol(entry->d_name, &end, 10);
		if (end == entry->d_name || *end)
			continue;
		if (pid == parent)
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
		char buf[BUFLEN];
		while (fgets(buf, BUFLEN - 1, fp)) {
			if (strncmp(buf, "PPid:", 5) == 0) {
				char *ptr = buf + 5;
				while (*ptr != '\0' && (*ptr == ' ' || *ptr == '\t')) {
					ptr++;
				}
				if (*ptr == '\0') {
					fprintf(stderr, "Error: cannot read /proc file\n");
					exit(1);
				}
				if (parent == atoi(ptr))
					*child = pid;
				break;	// stop reading the file
			}
		}
		fclose(fp);
		free(file);
	}
	closedir(dir);

	return (*child)? 0:1;	// 0 = found, 1 = not found
}

void check_private_dir(void) {
	// if the directory starts with ~, expand the home directory
	if (*cfg.home_private == '~') {
		char *tmp;
		if (asprintf(&tmp, "%s%s", cfg.homedir, cfg.home_private + 1) == -1)
			errExit("asprintf");
		cfg.home_private = tmp;
	}
	// check chroot dirname exists
	struct stat s2;
	int rv = stat(cfg.home_private, &s2);
	if (rv < 0) {
		fprintf(stderr, "Error: cannot find %s directory, aborting\n", cfg.home_private);
		exit(1);
	}

	// check home directory and chroot home directory have the same owner
	struct stat s1;
	rv = stat(cfg.homedir, &s1);
	if (rv < 0) {
		fprintf(stderr, "Error: cannot find %s directory, aborting\n", cfg.homedir);
		exit(1);
	}
	if (s1.st_uid != s2.st_uid || s1.st_gid != s2.st_gid) {
		printf("Error: the two home directories must have the same owner\n");
		exit(1);
	}
}

void extract_command_name(const char *str) {
	assert(str);
	cfg.command_name = strdup(str);
	if (!cfg.command_name)
		errExit("strdup");

	// restrict the command name to the first word
	char *ptr = cfg.command_name;
	while (*ptr != ' ' && *ptr != '\t' && *ptr != '\0')
		ptr++;
	*ptr = '\0';

	// remove the path: /usr/bin/firefox becomes firefox
	ptr = strrchr(cfg.command_name, '/');
	if (ptr) {
		ptr++;
		if (*ptr == '\0') {
			fprintf(stderr, "Error: invalid command name\n");
			exit(1);
		}

		char *tmp = strdup(ptr);
		if (!tmp)
			errExit("strdup");
		free(cfg.command_name);
		cfg.command_name = tmp;
	}
}

void update_map(char *mapping, char *map_file)
{
    int fd, j;
    size_t map_len;     /* Length of 'mapping' */

    /* Replace commas in mapping string with newlines */

    map_len = strlen(mapping);
    for (j = 0; j < map_len; j++)
        if (mapping[j] == ',')
            mapping[j] = '\n';

    fd = open(map_file, O_RDWR);
    if (fd == -1) {
        fprintf(stderr, "open %s: %s\n", map_file, strerror(errno));
        exit(EXIT_FAILURE);
    }

    if (write(fd, mapping, map_len) != map_len) {
        fprintf(stderr, "write %s: %s\n", map_file, strerror(errno));
        exit(EXIT_FAILURE);
    }

    close(fd);
}

void wait_for_other(int fd) {
	//****************************
	// wait for the parent to be initialized
	//****************************
	char childstr[BUFLEN + 1];
	FILE* stream;
	stream = fdopen(dup(fd), "r");
	*childstr = '\0';
	if (fgets(childstr, BUFLEN, stream)) {
		// remove \n)
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
	fclose(stream);
}

void notify_other(int fd) {
	FILE* stream;
	stream = fdopen(dup(fd), "w");
	fprintf(stream, "%u\n", getpid());
	fflush(stream);
	fclose(stream);
}
