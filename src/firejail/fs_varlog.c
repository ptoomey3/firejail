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
#include <unistd.h>
#include <sys/mount.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <linux/limits.h>
#include <glob.h>
#include <dirent.h>
#include <fcntl.h>
#include "firejail.h"

typedef struct dirdata_t{
	struct dirdata_t *next;
	char *name;
	mode_t st_mode;
	uid_t st_uid;
	gid_t st_gid;
} DirData;

static DirData *dirlist = NULL;

static void release_all(void) {
	DirData *ptr = dirlist;
	while (ptr) {
		DirData *next = ptr->next;
		free(ptr->name);
		free(ptr);
		ptr = next;
	}
	dirlist = NULL;
}
	
static void build_list(const char *srcdir) {
	// extract current /var/log directory data
	struct dirent *dir;
	DIR *d = opendir(srcdir);
	if (d == NULL)
		return;

	while ((dir = readdir(d))) {
		if(strcmp(dir->d_name, "." ) == 0 || strcmp(dir->d_name, ".." ) == 0)
			continue;

		if (dir->d_type == DT_DIR ) {
			// get properties
			struct stat s;
			char *name;
			if (asprintf(&name, "%s/%s", srcdir, dir->d_name) == -1)
				continue;
			if (stat(name, &s) == -1)
				continue;
			if (S_ISLNK(s.st_mode)) {
				free(name);
				continue;
			}

//			printf("directory %u %u:%u %s\n",
//				s.st_mode,
//				s.st_uid,
//				s.st_gid,
//				dir->d_name);
			
			DirData *ptr = malloc(sizeof(DirData));
			if (ptr == NULL)
				errExit("malloc");
			memset(ptr, 0, sizeof(DirData));
			ptr->name = name;
			ptr->st_mode = s.st_mode;
			ptr->st_uid = s.st_uid;
			ptr->st_gid = s.st_gid;
			ptr->next = dirlist;
			dirlist = ptr;		
		}			
	}
	closedir(d);
}

static void build_dirs(viod) {
	// create directories under /var/log
	DirData *ptr = dirlist;
	while (ptr) {
		if (mkdir(ptr->name, ptr->st_mode))
			errExit("mkdir");
		if (chown(ptr->name, ptr->st_uid, ptr->st_gid))
			errExit("chown");
		ptr = ptr->next;
	}
}
	
void fs_varlog(void) {
	build_list("/var/log");
	
	// mount a tmpfs on top of /var/log
	if (arg_debug)
		printf("Mounting tmpfs on /var/log\n");
	if (mount("tmpfs", "/var/log", "tmpfs", MS_NOSUID | MS_STRICTATIME | MS_REC,  "mode=755,gid=0") < 0)
		errExit("mounting /var/log");
	
	build_dirs();
	release_all();
}

void fs_varlib(void) {
	build_list("/var/lib");
	
	// mount a tmpfs on top of /var/log
	if (arg_debug)
		printf("Mounting tmpfs on /var/lib\n");
	if (mount("tmpfs", "/var/lib", "tmpfs", MS_NOSUID | MS_STRICTATIME | MS_REC,  "mode=755,gid=0") < 0)
		errExit("mounting /var/lib");
	
	build_dirs();
	release_all();
}
