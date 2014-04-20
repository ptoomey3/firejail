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
#include <string.h>
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
	if (strcspn(ptr, "\\&!?\"'<>%^(){}[];, ") != len) {
		fprintf(stderr, "Error: line %d in the custom profile is invalid\n", lineno);
		exit(1);
	}
}


static void check_line(char *ptr, int lineno) {
	if (strncmp(ptr, "newdir ", 7) == 0)
		ptr += 7;
	else if (strncmp(ptr, "blacklist ", 10) == 0)
		ptr += 10;
	else if (strncmp(ptr, "preserve ", 9) == 0)
		ptr += 9;
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


static char *remove_spaces(const char *buf) {
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
		else { // state == 1
			while (*ptr1 == ' ' || *ptr1 == '\t')
				ptr1++;
			state = 0;
		}
	}
	*ptr2 = '\0';

	return rv;
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
		char *ptr = remove_spaces(buf);
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
	custom_profile  = malloc(sizeof(char *) * mylist_cnt);
	if (!custom_profile)
		errExit("malloc");
	mptr = &m;
	lineno = 0;
	while (mptr->next != NULL) {
		assert(mptr->line);
		custom_profile[lineno] = mptr->line;
		mptr = mptr->next;
		lineno++;
	}
	custom_profile[lineno] = NULL;
	
	// free the list
	mptr = &m;
	mptr = mptr->next;
	while (mptr) {
		struct mylist *next = mptr->next;
		free(mptr);
		mptr = next;
	}
}
