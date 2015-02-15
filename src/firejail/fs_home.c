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
#include <sys/mount.h>
#include <linux/limits.h>
#include <glob.h>
#include <dirent.h>
#include <fcntl.h>
 #include <sys/stat.h>

static void skel(const char *homedir, uid_t u, gid_t g) {
	char *fname;
	// zsh
	if (arg_zsh) {
		// copy skel files
		if (asprintf(&fname, "%s/.zshrc", homedir) == -1)
			errExit("asprintf");
		struct stat s;
		// don't copy it if we already have the file
		if (stat(fname, &s) == 0)
			return;
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
		// don't copy it if we already have the file
		if (stat(fname, &s) == 0)
			return;
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
		// don't copy it if we already have the file
		if (stat(fname, &s) == 0)
			return;
		if (stat("/etc/skel/.bashrc", &s) == 0) {
			if (copy_file("/etc/skel/.bashrc", fname) == 0) {
				if (chown(fname, u, g) == -1)
					errExit("chown");
			}
		}
		free(fname);
	}
}

static int store_xauthority(void) {
	// put a copy of .Xauthority in MNT_DIR
	fs_build_mnt_dir();

	char *src;
	char *dest;
	if (asprintf(&src, "%s/.Xauthority", cfg.homedir) == -1)
		errExit("asprintf");
	if (asprintf(&dest, "%s/.Xauthority", MNT_DIR) == -1)
		errExit("asprintf");
	
	struct stat s;
	if (stat(src, &s) == 0) {	
		int rv = copy_file(src, dest);
		if (rv) {
			fprintf(stderr, "Warning: cannot transfer .Xauthority in private home directory\n");
			return 0;
		}
		return 1; // file copied
	}
	
	return 0;
}

static void copy_xauthority(void) {
	// put a copy of .Xauthority in MNT_DIR
	fs_build_mnt_dir();

	char *src;
	char *dest;
	if (asprintf(&dest, "%s/.Xauthority", cfg.homedir) == -1)
		errExit("asprintf");
	if (asprintf(&src, "%s/.Xauthority", MNT_DIR) == -1)
		errExit("asprintf");
	int rv = copy_file(src, dest);
	if (rv)
		fprintf(stderr, "Warning: cannot transfer .Xauthority in private home directory\n");

	// set permissions and ownership
	if (chown(dest, getuid(), getgid()) < 0)
		errExit("chown");
	if (chmod(dest, S_IRUSR | S_IWUSR) < 0)
		errExit("chmod");

	// delete the temporary file
	unlink(src);
}


// private mode: mount tmpfs over /home and /tmp
void fs_private_home(void) {
	char *homedir = cfg.homedir;
	char *private_homedir = cfg.home_private;
	assert(homedir);
	assert(private_homedir);
	
	int xflag = store_xauthority();
	
	uid_t u = getuid();
	gid_t g = getgid();
	struct stat s;
	if (stat(homedir, &s) == -1) {
		fprintf(stderr, "Error: cannot find user home directory, aborting\n");
		exit(1);
	}
	

	// mount bind private_homedir on top of homedir
	if (arg_debug)
		printf("Mount-bind %s on top of %s\n", private_homedir, homedir);
	if (mount(private_homedir, homedir, NULL, MS_BIND|MS_REC, NULL) < 0)
		errExit("mount bind");
// preserve mode and ownership
//	if (chown(homedir, s.st_uid, s.st_gid) == -1)
//		errExit("mount-bind chown");
//	if (chmod(homedir, s.st_mode) == -1)
//		errExit("mount-bind chmod");

	if (u != 0) {
		// mask /root
		if (arg_debug)
			printf("Mounting a new /root directory\n");
		if (mount("tmpfs", "/root", "tmpfs", MS_NOSUID | MS_NODEV | MS_STRICTATIME | MS_REC,  "mode=700,gid=0") < 0)
			errExit("mounting home directory");
	}
	else {
		// mask /home
		if (arg_debug)
			printf("Mounting a new /home directory\n");
		if (mount("tmpfs", "/home", "tmpfs", MS_NOSUID | MS_NODEV | MS_STRICTATIME | MS_REC,  "mode=755,gid=0") < 0)
			errExit("mounting home directory");

		// mask /tmp only in root mode; KDE keeps all kind of sockets in /tmp!
		if (arg_debug)
			printf("Mounting a new /tmp directory\n");
		if (mount("tmpfs", "/tmp", "tmpfs", MS_NOSUID | MS_NODEV | MS_STRICTATIME | MS_REC,  "mode=777,gid=0") < 0)
			errExit("mounting tmp directory");
	}
	

	skel(homedir, u, g);
	if (xflag)
		copy_xauthority();
}






// private mode: mount tmpfs over /home
void fs_private(void) {
	char *homedir = cfg.homedir;
	assert(homedir);
	uid_t u = getuid();
	gid_t g = getgid();

	int xflag = store_xauthority();

	// mask /home
	if (arg_debug)
		printf("Mounting a new /home directory\n");
	if (mount("tmpfs", "/home", "tmpfs", MS_NOSUID | MS_NODEV | MS_STRICTATIME | MS_REC,  "mode=755,gid=0") < 0)
		errExit("mounting home directory");

	// mask /root
	if (arg_debug)
		printf("Mounting a new /root directory\n");
	if (mount("tmpfs", "/root", "tmpfs", MS_NOSUID | MS_NODEV | MS_STRICTATIME | MS_REC,  "mode=700,gid=0") < 0)
		errExit("mounting home directory");

	if (u != 0) {
		// create /home/user
		if (arg_debug)
			printf("Create a new user directory\n");
		mkdir(homedir, S_IRWXU);
		if (chown(homedir, u, g) < 0)
			errExit("chown");
	}
	else {
		// mask tmp only in root mode; KDE keeps all kind of sockets in /tmp!
		if (arg_debug)
			printf("Mounting a new /tmp directory\n");
		if (mount("tmpfs", "/tmp", "tmpfs", MS_NOSUID | MS_NODEV | MS_STRICTATIME | MS_REC,  "mode=777,gid=0") < 0)
			errExit("mounting tmp directory");
	}
	
	skel(homedir, u, g);
	if (xflag)
		copy_xauthority();
}
