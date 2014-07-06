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
#include <unistd.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <dirent.h>
#include <fcntl.h>
#include "firejail.h"

//***********************************************
// atexit
//***********************************************
char *tmpdir = NULL;
static int no_cleanup;

static void unlink_walker(void) {
	struct dirent *dir;
	DIR *d = opendir( "." );
	if (d == NULL)
		return;

	while ((dir = readdir(d))) {
		if(strcmp( dir->d_name, "." ) == 0 || strcmp( dir->d_name, ".." ) == 0 )
			continue;

		if (dir->d_type == DT_DIR ) {
			if (chdir(dir->d_name) == 0)
				unlink_walker();
			else
				return;
			if (chdir( ".." ) != 0)
				return;
			if (rmdir(dir->d_name) != 0)
				return;
		}
		else {
			if (dir->d_type == DT_UNKNOWN) {
				fprintf(stderr, "Error: cannot remove temporary directory %s - unknown filesystem type\n", tmpdir);
				return;
			}
			if (unlink(dir->d_name) != 0)
				return;
		}
	}

	closedir(d);
}

void bye_parent(void) {
	// the child is just inheriting it
	if (getpid() == 1)
		return;
	if (no_cleanup)
		return;
	if (!tmpdir)
		return;

	char *storage = tmpdir;
	tmpdir = NULL;
	
	if (chdir(storage) == 0) {
		if (!arg_command)
			printf("\nparent is shutting down, bye...\n");
		if (!arg_command && arg_debug)
			printf("Removing %s directory\n", storage);
		unlink_walker();
		if (chdir("..") == 0)
			rmdir(storage);
	}
}

void set_exit_parent(pid_t pid, int nocleanup) {
	no_cleanup = nocleanup;
	// create tmp directory
	char *name_template;
	if (asprintf(&name_template, "/tmp/firejail-%u-XXXXXX", pid) == -1)
		errExit("asprintf");
	tmpdir = mkdtemp(name_template);
	if (tmpdir == NULL)
		errExit("mkdtemp");
	if (arg_debug)
		printf("Creating %s directory\n", tmpdir);
	mkdir(tmpdir, S_IRWXU);
	uid_t u = getuid();
	gid_t g = getgid();
	if (chown(tmpdir, u, g) < 0)
		errExit("chown");

	if (atexit(bye_parent))
		errExit("atexit");
}
