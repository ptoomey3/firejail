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
#include <pwd.h>
#include "firejail.h"

void fs_dev_shm(void) {
	uid_t uid = getuid(); // set a new shm only if we started as root
	if (uid)
		return;

	if (is_dir("/dev/shm")) {
		if (arg_debug)
			printf("Mounting tmpfs on /dev/shm\n");
		if (mount("tmpfs", "/dev/shm", "tmpfs", MS_NOSUID | MS_STRICTATIME | MS_REC,  "mode=777,gid=0") < 0)
			errExit("mounting /dev/shm");
	}
	else {
		char *lnk = get_link("/dev/shm");
		if (lnk) {
			// convert a link such as "../shm" into "/shm"
			char *lnk2 = lnk;
			int cnt = 0;
			while (strncmp(lnk2, "../", 3) == 0) {
				cnt++;
				lnk2 = lnk2 + 3;
			}
			if (cnt != 0)
				lnk2 = lnk + (cnt - 1) * 3 + 2;

			if (!is_dir(lnk2)) {
				// create directory
				if (mkdir(lnk2, S_IRWXU|S_IRWXG|S_IRWXO))
					errExit("mkdir");
				if (chown(lnk2, 0, 0))
					errExit("chown");
				if (chmod(lnk2, S_IRWXU|S_IRWXG|S_IRWXO))
					errExit("chmod");
			}
			if (arg_debug)
				printf("Mounting tmpfs on %s on behalf of /dev/shm\n", lnk2);
			if (mount("tmpfs", lnk2, "tmpfs", MS_NOSUID | MS_STRICTATIME | MS_REC,  "mode=777,gid=0") < 0)
				errExit("mounting /var/tmp");
			free(lnk);
		}
		else {
			fprintf(stderr, "Warning: /dev/shm not mounted\n");
			dbg_test_dir("/dev/shm");
		}
			
	}
}
