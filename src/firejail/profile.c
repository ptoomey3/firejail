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
#include <string.h>
#include <unistd.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <dirent.h>
#include "firejail.h"

#define MAX_READ 1024				  // line buffer for profile files

// find and read the profile specified by name from dir directory
void profile_find(const char *name, const char *dir) {
	assert(name);
	assert(dir);
	
	DIR *dp;
	char *pname;
	if (asprintf(&pname, "%s.profile", name) == -1)
		errExit("asprintf");

	dp = opendir (dir);
	if (dp != NULL) {
		struct dirent *ep;
		while (ep = readdir (dp)) {
			if (strcmp(ep->d_name, pname) == 0) {
				if (arg_debug)
					printf("Found %s profile in %s directory\n", name, dir);
				char *etcpname;
				if (asprintf(&etcpname, "%s/%s", dir, pname) == -1)
					errExit("asprintf");
				profile_read(etcpname);
				free(etcpname);
				break;
			}
		}
		(void) closedir (dp);
	}

	free(pname);
}


//***************************************************
// run-time profiles
//***************************************************
static void check_file_name(char *ptr, int lineno) {
	int len = strlen(ptr);
	// file globbing ('*') is allowed
	if (strcspn(ptr, "\\&!?\"'<>%^(){}[];, ") != len) {
		fprintf(stderr, "Error: line %d in the custom profile is invalid\n", lineno);
		exit(1);
	}
}


static void check_line(char *ptr, int lineno) {
	if (strncmp(ptr, "blacklist ", 10) == 0)
		ptr += 10;
	else if (strncmp(ptr, "read-only ", 10) == 0)
		ptr += 10;
	else if (strncmp(ptr, "tmpfs ", 6) == 0)
		ptr += 6;
	else {
		fprintf(stderr, "Error: line %d in the custom profile is invalid\n", lineno);
		exit(1);
	}

	// some characters just don't belong in filenames
	if (strncmp(ptr, "${HOME}", 7) == 0)
		check_file_name(ptr + 7, lineno);
	else if (strncmp(ptr, "${PATH}", 7) == 0)
		check_file_name(ptr + 7, lineno);
	else
		check_file_name(ptr, lineno);
}


// read a profile file
void profile_read(const char *fname) {
	if (strlen(fname) == 0) {
		fprintf(stderr, "Error: invalid profile file\n");
		exit(1);
	}

	// open profile file:
	FILE *fp = fopen(fname, "r");
	if (fp == NULL) {
		fprintf(stderr, "Error: cannot open profile file\n");
		exit(1);
	}

	printf("Reading %s\n", fname);

	// linked list of lines
	struct mylist {
		char *line;
		struct mylist *next;
	}
	m = {
		NULL, NULL
	};
	struct mylist *mptr = &m;
	int mylist_cnt = 1;

	// read the file line by line
	char buf[MAX_READ + 1];
	int lineno = 0;
	while (fgets(buf, MAX_READ, fp)) {
		++lineno;
		// remove empty space
		char *ptr = line_remove_spaces(buf);
		if (ptr == NULL || *ptr == '\0')
			continue;
		
		// comments
		if (*ptr == '#')
			continue;

		// verify syntax, exit in case of error
		check_line(ptr, lineno);

		// populate the linked list
		assert(mptr);
		mptr->line = ptr;
		mptr->next = malloc(sizeof(struct mylist));
		if (mptr->next == NULL)
			errExit("malloc");
		mptr = mptr->next;
		mptr->line = NULL;
		mptr->next = NULL;
		mylist_cnt++;
	}

	// build blacklist array
	cfg.custom_profile  = malloc(sizeof(char *) * mylist_cnt);
	if (!cfg.custom_profile)
		errExit("malloc");
	mptr = &m;
	lineno = 0;
	while (mptr->next != NULL) {
		assert(mptr->line);
		cfg.custom_profile[lineno] = mptr->line;
		mptr = mptr->next;
		lineno++;
	}
	cfg.custom_profile[lineno] = NULL;
	
	// free the list
	mptr = &m;
	mptr = mptr->next;
	while (mptr) {
		struct mylist *next = mptr->next;
		free(mptr);
		mptr = next;
	}
}
