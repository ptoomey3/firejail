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
#include <sys/mount.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <linux/limits.h>
#include <glob.h>
#include <dirent.h>
#include <fcntl.h>
#include "firejail.h"

// private mode: mount tmpfs over /home and /tmp
void fs_private(const char *homedir) {
	assert(homedir);
	uid_t u = getuid();
	gid_t g = getgid();

	if (arg_debug)
		printf("Mounting a new /home directory\n");
	if (mount("tmpfs", "/home", "tmpfs", MS_NOSUID | MS_NODEV | MS_STRICTATIME | MS_REC,  "mode=755,gid=0") < 0)
		errExit("mounting home directory");

	if (arg_debug)
		printf("Mounting a new /root directory\n");
	if (mount("tmpfs", "/root", "tmpfs", MS_NOSUID | MS_NODEV | MS_STRICTATIME | MS_REC,  "mode=700,gid=0") < 0)
		errExit("mounting home directory");

	// mounting /home/user
	if (u != 0) {
		mkdir(homedir, S_IRWXU);
		if (chown(homedir, u, g) < 0)
			errExit("chown");

	}
	
	char *fname;
	// zsh
	if (arg_zsh) {
		// copy skel files
		if (asprintf(&fname, "%s/.zshrc", homedir) == -1)
			errExit("asprintf");
		struct stat s;
		if (stat("/etc/skel/.zshrc", &s) == 0) {
			if (copy_file("/etc/skel/.zshrc", fname) == 0) {
				if (chown(fname, u, g) == -1)
					errExit("chown");
			}
		}
		else { // 
			FILE *fp = fopen(fname, "w");
			if (fp) {
				fprintf(fp, "\n");
				fclose(fp);
				if (chown(fname, u, g) == -1)
					errExit("chown");
				if (chmod(fname, S_IRUSR | S_IWUSR) < 0)
					errExit("chown");
			}
		}
		free(fname);
	}
	// csh
	else if (arg_csh) {
		// copy skel files
		if (asprintf(&fname, "%s/.cshrc", homedir) == -1)
			errExit("asprintf");
		struct stat s;
		if (stat("/etc/skel/.cshrc", &s) == 0) {
			if (copy_file("/etc/skel/.cshrc", fname) == 0) {
				if (chown(fname, u, g) == -1)
					errExit("chown");
			}
		}
		else { // 
			FILE *fp = fopen(fname, "w");
			if (fp) {
				fprintf(fp, "\n");
				fclose(fp);
				if (chown(fname, u, g) == -1)
					errExit("chown");
				if (chmod(fname, S_IRUSR | S_IWUSR) < 0)
					errExit("chown");
			}
		}
		free(fname);
	}
	// bash etc.
	else {
		// copy skel files
		if (asprintf(&fname, "%s/.bashrc", homedir) == -1)
			errExit("asprintf");
		struct stat s;
		if (stat("/etc/skel/.bashrc", &s) == 0) {
			if (copy_file("/etc/skel/.bashrc", fname) == 0) {
				if (chown(fname, u, g) == -1)
					errExit("chown");
			}
		}
		free(fname);
	}
	
	if (arg_debug)
		printf("Mounting a new /tmp directory\n");
	if (mount("tmpfs", "/tmp", "tmpfs", MS_NOSUID | MS_NODEV | MS_STRICTATIME | MS_REC,  "mode=777,gid=0") < 0)
		errExit("mounting tmp directory");
}
