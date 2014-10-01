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
#include <stdio.h>
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
		while ((ep = readdir(dp)) != NULL) {
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
	if (strncmp(ptr, "${HOME}", 7) == 0)
		ptr += 7;
	else if (strncmp(ptr, "${PATH}", 7) == 0)
		ptr += 7;

	int len = strlen(ptr);
	// file globbing ('*') is allowed
	if (strcspn(ptr, "\\&!?\"'<>%^(){}[];, ") != len) {
		if (lineno == 0)
			fprintf(stderr, "Error: \"%s\" is an invalid filename\n", ptr);
		else
			fprintf(stderr, "Error: line %d in the custom profile is invalid\n", lineno);
		exit(1);
	}
}


// check profile line; if line == 0, this was generated from a command line option
void profile_check_line(char *ptr, int lineno) {
	if (strncmp(ptr, "bind ", 5) == 0) {
		// extract two directories
		char *dname1 = ptr + 5;
		char *dname2 = split_colon(dname1); // this inserts a '0 to separate the two dierctories
		if (dname2 == NULL) {
			fprintf(stderr, "Error: mising second directory for bind\n");
			exit(1);
		}
		
		// check directories
		check_file_name(dname1, lineno);
		check_file_name(dname2, lineno);
		
		// insert colon back
		*(dname2 - 1) = ':';
		return;
	}

	if (strncmp(ptr, "blacklist ", 10) == 0)
		ptr += 10;
	else if (strncmp(ptr, "read-only ", 10) == 0)
		ptr += 10;
	else if (strncmp(ptr, "tmpfs ", 6) == 0)
		ptr += 6;
	else {
		if (lineno == 0)
			fprintf(stderr, "Error: \"%s\" as a command line option is invalid\n", ptr);
		else
			fprintf(stderr, "Error: line %d in the custom profile is invalid\n", lineno);
		exit(1);
	}

	// some characters just don't belong in filenames
	check_file_name(ptr, lineno);
}

// add a profile entry in cfg.profile list; use str to populate the list
void profile_add(char *str) {
	ProfileEntry *prf = malloc(sizeof(ProfileEntry));
	if (!prf)
		errExit("malloc");
	prf->next = NULL;
	prf->data = str;	

	// add prf to the list
	if (cfg.profile == NULL) {
		cfg.profile = prf;
		return;
	}
	ProfileEntry *ptr = cfg.profile;
	while (ptr->next != NULL)
		ptr = ptr->next;
	ptr->next = prf;
}

// read a profile file
void profile_read(const char *fname) {
	if (strlen(fname) == 0) {
		fprintf(stderr, "Error: invalid profile file\n");
		exit(1);
	}
int a = fname;
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
		profile_check_line(ptr, lineno);

		profile_add(ptr);
	}
	fclose(fp);
}
