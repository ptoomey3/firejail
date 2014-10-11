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


// build /tmp/firejail directory
void fs_build_firejail_dir(void) {
	struct stat s;

	if (stat(FIREJAIL_DIR, &s)) {
		if (arg_debug)
			printf("Creating %s directory\n", FIREJAIL_DIR);
		mkdir(FIREJAIL_DIR, S_IRWXU | S_IRWXG | S_IRWXO);
		if (chown(FIREJAIL_DIR, 0, 0) < 0)
			errExit("chown");
		if (chmod(FIREJAIL_DIR, S_IRWXU  | S_IRGRP | S_IXGRP | S_IROTH | S_IXOTH) < 0)
			errExit("chmod");
	}
}


// build /tmp/firejail/mnt directory
void fs_build_mnt_dir(void) {
	struct stat s;
	fs_build_firejail_dir();
	
	// create /tmp/firejail directory
	if (stat(MNT_DIR, &s)) {
		if (arg_debug)
			printf("Creating %s directory\n", MNT_DIR);
		mkdir(MNT_DIR, S_IRWXU | S_IRWXG | S_IRWXO);
		if (chown(MNT_DIR, 0, 0) < 0)
			errExit("chown");
		if (chmod(MNT_DIR, S_IRWXU  | S_IRGRP | S_IXGRP | S_IROTH | S_IXOTH) < 0)
			errExit("chmod");
	}
}

// build /tmp/firejail/overlay directory
void fs_build_overlay_dir(void) {
	struct stat s;
	fs_build_firejail_dir();
	
	// create /tmp/firejail directory
	if (stat(OVERLAY_DIR, &s)) {
		if (arg_debug)
			printf("Creating %s directory\n", MNT_DIR);
		mkdir(OVERLAY_DIR, S_IRWXU | S_IRWXG | S_IRWXO);
		if (chown(OVERLAY_DIR, 0, 0) < 0)
			errExit("chown");
		if (chmod(OVERLAY_DIR, S_IRWXU  | S_IRGRP | S_IXGRP | S_IROTH | S_IXOTH) < 0)
			errExit("chmod");
	}
}



//***********************************************
// process profile file
//***********************************************
typedef enum {
	BLACKLIST_FILE,
	MOUNT_READONLY,
	MOUNT_TMPFS,
	OPERATION_MAX
} OPERATION;


static char *create_empty_dir(void) {
	struct stat s;
	fs_build_firejail_dir();
	
	if (stat(RO_DIR, &s)) {
		mkdir(RO_DIR, S_IRUSR | S_IXUSR);
		if (chown(RO_DIR, 0, 0) < 0)
			errExit("chown");
	}
	
	return RO_DIR;
}

static char *create_empty_file(void) {
	struct stat s;
	fs_build_firejail_dir();

	if (stat(RO_FILE, &s)) {
		FILE *fp = fopen(RO_FILE, "w");
		if (!fp)
			errExit("fopen");
		fclose(fp);
		if (chown(RO_FILE, 0, 0) < 0)
			errExit("chown");
		if (chmod(RO_FILE, S_IRUSR) < 0)
			errExit("chown");
	}
	
	return RO_FILE;
}

static void disable_file(OPERATION op, const char *fname, const char *emptydir, const char *emptyfile) {
	assert(fname);
	assert(emptydir);
	assert(emptyfile);
	assert(op <OPERATION_MAX);

	// if the file is a link, follow the link
	char *lnk = NULL;
	if (is_link(fname)) {
		lnk = get_link(fname);
		if (lnk)
			fname = lnk;
		else
			fprintf(stderr, "Warning: cannot follow link %s, skipping...\n", fname);
	}
	
	// if the file is not present, do nothing
	struct stat s;
	if (stat(fname, &s) == -1)
		return;

	// modify the file
	if (op == BLACKLIST_FILE) {
		if (arg_debug)
			printf("Disable %s\n", fname);
		if (S_ISDIR(s.st_mode)) {
			if (mount(emptydir, fname, "none", MS_BIND, "mode=400,gid=0") < 0)
				errExit("disable file");
		}
		else {
			if (mount(emptyfile, fname, "none", MS_BIND, "mode=400,gid=0") < 0)
				errExit("disable file");
		}
	}
	else if (op == MOUNT_READONLY) {
		if (arg_debug)
			printf("Mounting read-only %s\n", fname);
		fs_rdonly(fname);
	}
	else if (op == MOUNT_TMPFS) {
		if (S_ISDIR(s.st_mode)) {
			if (arg_debug)
				printf("Mounting tmpfs on %s\n", fname);
			// preserve owner and mode for the directory
			if (mount("tmpfs", fname, "tmpfs", MS_NOSUID | MS_NODEV | MS_STRICTATIME | MS_REC,  0) < 0)
				errExit("mounting tmpfs");
			if (chown(fname, s.st_uid, s.st_gid) == -1)
				errExit("mounting tmpfs chmod");
		}
		else
			printf("Warning: %s is not a directory; cannot mount a tmpfs on top of it.\n", fname);
	}
	else
		assert(0);

	if (lnk)
		free(lnk);
}

static void globbing(OPERATION op, const char *fname, const char *emptydir, const char *emptyfile) {
	assert(fname);
	assert(emptydir);
	assert(emptyfile);

	// filename globbing: expand * macro and continue processing for every single file
	if (strchr(fname, '*')) {
		glob_t globbuf;
		globbuf.gl_offs = 0;
		glob(fname, GLOB_DOOFFS, NULL, &globbuf);
		int i;
		for (i = 0; i < globbuf.gl_pathc; i++) {
			assert(globbuf.gl_pathv[i]);
			disable_file(op, globbuf.gl_pathv[i], emptydir, emptyfile);
		}
	}
	else
		disable_file(op, fname, emptydir, emptyfile);
}

static void expand_path(OPERATION op, const char *path, const char *fname, const char *emptydir, const char *emptyfile) {
	assert(path);
	assert(fname);
	assert(emptydir);
	assert(emptyfile);
	char newname[strlen(path) + strlen(fname) + 1];
	sprintf(newname, "%s%s", path, fname);

	globbing(op, newname, emptydir, emptyfile);
}

// blacklist files or directoies by mounting empty files on top of them
void fs_blacklist(const char *homedir) {
	ProfileEntry *entry = cfg.profile;
	if (!entry)
		return;
		
	char *emptydir = create_empty_dir();
	char *emptyfile = create_empty_file();

	while (entry) {
		OPERATION op = OPERATION_MAX;
		char *ptr;

		// process blacklist command
		if (strncmp(entry->data, "bind", 4) == 0)  {
			char *dname1 = entry->data + 5;
			char *dname2 = split_comma(dname1);
			if (dname2 == NULL) {
				fprintf(stderr, "Error: second directory missing in bind command\n");
				entry = entry->next;
				continue;
			}
			struct stat s;
			if (stat(dname1, &s) == -1) {
				fprintf(stderr, "Error: cannot find directories for bind command\n");
				entry = entry->next;
				continue;
			}
			if (stat(dname2, &s) == -1) {
				fprintf(stderr, "Error: cannot find directories for bind command\n");
				entry = entry->next;
				continue;
			}
			
			// mount --bind olddir newdir
			if (arg_debug)
				printf("Mount-bind %s on top of %s\n", dname1, dname2);
			// preserve dname2 mode and ownership
			if (mount(dname1, dname2, NULL, MS_BIND|MS_REC, NULL) < 0)
				errExit("mount bind");
			if (chown(dname2, s.st_uid, s.st_gid) == -1)
				errExit("mount-bind chown");
			if (chmod(dname2, s.st_mode) == -1)
				errExit("mount-bind chmod");
				
			entry = entry->next;
			continue;
		}


		// rlimit
		if (strncmp(entry->data, "rlimit", 6) == 0) {
			if (strncmp(entry->data, "rlimit-nofile ", 14) == 0) {
				sscanf(entry->data + 14, "%u", &cfg.rlimit_nofile);
				arg_rlimit_nofile = 1;
			}				
			else if (strncmp(entry->data, "rlimit-nproc ", 13) == 0) {
				sscanf(entry->data + 13, "%u", &cfg.rlimit_nproc);
				arg_rlimit_nproc = 1;
			}				
			else if (strncmp(entry->data, "rlimit-fsize ", 13) == 0) {
				sscanf(entry->data + 13, "%u", &cfg.rlimit_fsize);
				arg_rlimit_fsize = 1;
			}				
			else if (strncmp(entry->data, "rlimit-sigpending ", 18) == 0) {
				sscanf(entry->data + 18, "%u", &cfg.rlimit_sigpending);
				arg_rlimit_sigpending = 1;
			}				
			entry = entry->next;
			continue;
		}

		// process blacklist command
		if (strncmp(entry->data, "blacklist", 9) == 0)  {
			ptr = entry->data + 10;
			op = BLACKLIST_FILE;
		}
		else if (strncmp(entry->data, "read-only", 9) == 0) {
			ptr = entry->data + 10;
			op = MOUNT_READONLY;
		}			
		else if (strncmp(entry->data, "tmpfs", 5) == 0) {
			ptr = entry->data + 6;
			op = MOUNT_TMPFS;
		}			
		else {
			fprintf(stderr, "Error: invalid profile line %s\n", entry->data);
			entry = entry->next;
			continue;
		}

		// replace home macro in blacklist array
		char *new_name = NULL;
		if (strncmp(ptr, "${HOME}", 7) == 0) {
			if (asprintf(&new_name, "%s%s", homedir, ptr + 7) == -1)
				errExit("asprintf");
			ptr = new_name;
		}

		// expand path macro - look for the file in /bin, /usr/bin, /sbin and  /usr/sbin directories
		if (strncmp(ptr, "${PATH}", 7) == 0) {
			expand_path(op, "/bin", ptr + 7, emptydir, emptyfile);
			expand_path(op, "/sbin", ptr + 7, emptydir, emptyfile);
			expand_path(op, "/usr/bin", ptr + 7, emptydir, emptyfile);
			expand_path(op, "/usr/sbin", ptr + 7, emptydir, emptyfile);
		}
		else
			globbing(op, ptr, emptydir, emptyfile);

		if (new_name)
			free(new_name);
		entry = entry->next;
	}
}

//***********************************************
// mount namespace
//***********************************************

// remount a directory read-only
void fs_rdonly(const char *dir) {
	assert(dir);
	// check directory exists
	struct stat s;
	int rv = stat(dir, &s);
	if (rv == 0) {
		// mount --bind /bin /bin
		if (mount(dir, dir, NULL, MS_BIND|MS_REC, NULL) < 0)
			errExit("mount read-only");
		// mount --bind -o remount,ro /bin
		if (mount(NULL, dir, NULL, MS_BIND|MS_REMOUNT|MS_RDONLY|MS_REC, NULL) < 0)
			errExit("mount read-only");
	}
}
void fs_rdonly_noexit(const char *dir) {
	assert(dir);
	// check directory exists
	struct stat s;
	int rv = stat(dir, &s);
	if (rv == 0) {
		int merr = 0;
		// mount --bind /bin /bin
		if (mount(dir, dir, NULL, MS_BIND|MS_REC, NULL) < 0)
			merr = 1;
		// mount --bind -o remount,ro /bin
		if (mount(NULL, dir, NULL, MS_BIND|MS_REMOUNT|MS_RDONLY|MS_REC, NULL) < 0)
			merr = 1;
		if (merr)
			fprintf(stderr, "Warning: cannot mount %s read-only\n", dir); 
	}
}

// mount /proc and /sys directories
void fs_proc_sys_dev_boot(void) {
	if (arg_debug)
		printf("Remounting /proc and /proc/sys filesystems\n");
	if (mount("proc", "/proc", "proc", MS_NOSUID | MS_NOEXEC | MS_NODEV | MS_REC, NULL) < 0)
		errExit("mounting /proc");

	if (mount("/proc/sys", "/proc/sys", NULL, MS_BIND | MS_REC, NULL) < 0)
		errExit("mounting /proc/sys");

	if (mount(NULL, "/proc/sys", NULL, MS_BIND | MS_REMOUNT | MS_RDONLY | MS_REC, NULL) < 0)
		errExit("mounting /proc/sys");

//	if (mount("sysfs", "/sys", "sysfs", MS_RDONLY|MS_NOSUID|MS_NOEXEC|MS_NODEV|MS_REC, NULL) < 0)
//		errExit("mounting /sys");

	// Disable SysRq
	// a linux box can be shut down easilliy using the following commands (as root):
	// # echo 1 > /proc/sys/kernel/sysrq
	// #echo b > /proc/sysrq-trigger
	// for more information see https://www.kernel.org/doc/Documentation/sysrq.txt
	if (arg_debug)
		printf("Disable /proc/sysrq-trigger\n");
	fs_rdonly_noexit("/proc/sysrq-trigger");
	
	// disable hotplug and uevent_helper
	if (arg_debug)
		printf("Disable /proc/sys/kernel/hotplug\n");
	fs_rdonly_noexit("/proc/sys/kernel/hotplug");
	if (arg_debug)
		printf("Disable /sys/kernel/uevent_helper\n");
	fs_rdonly_noexit("/sys/kernel/uevent_helper");
	
	// read-only /proc/irq and /proc/bus
	if (arg_debug)
		printf("Disable /proc/irq\n");
	fs_rdonly_noexit("/proc/irq");
	if (arg_debug)
		printf("Disable /proc/bus\n");
	fs_rdonly_noexit("/proc/bus");
	
	// disable /proc/kcore
	disable_file(BLACKLIST_FILE, "/proc/kcore", "not used", "/dev/null");

	// disable /proc/kallsyms
	disable_file(BLACKLIST_FILE, "/proc/kallsyms", "not used", "/dev/null");
	
	// disable /boot
	struct stat s;
	if (stat("/boot", &s) == 0) {
		if (arg_debug)
			printf("Mounting a new /boot directory\n");
		if (mount("tmpfs", "/boot", "tmpfs", MS_NOSUID | MS_NODEV | MS_STRICTATIME | MS_REC,  "mode=777,gid=0") < 0)
			errExit("mounting /boot directory");
	}
	
	// disable /dev/port
	if (stat("/dev/port", &s) == 0) {
		disable_file(BLACKLIST_FILE, "/dev/port", "not used", "/dev/null");
	}
}


// build a basic read-only filesystem
void fs_basic_fs(void) {
	if (arg_debug)
		printf("Mounting read-only /bin, /sbin, /lib, /lib64, /usr, /etc, /var\n");
	fs_rdonly("/bin");
	fs_rdonly("/sbin");
	fs_rdonly("/lib");
	fs_rdonly("/lib64");
	fs_rdonly("/usr");
	fs_rdonly("/etc");
	fs_rdonly("/var");

	// update /var directory in order to support multiple sandboxes running on the same root directory
	fs_dev_shm();
	fs_var_lock();
	fs_var_tmp();
	fs_var_log();
	fs_var_lib();
	fs_var_cache();
}


// mount overlayfs on top of / directory
void fs_overlayfs(void) {
	fs_build_overlay_dir();

	// mount a tmpfs on top of overlay directory in order to hide it from the main filesystem
	struct stat s;
	if (stat(OVERLAY_DIR, &s) == -1)
		errExit("overlay directory");
	// preserve owner and mode for the directory
	if (mount("tmpfs", OVERLAY_DIR, "tmpfs", MS_NOSUID | MS_NODEV | MS_STRICTATIME | MS_REC,  0) < 0)
		errExit("mounting tmpfs");
	if (chown(OVERLAY_DIR, s.st_uid, s.st_gid) == -1)
		errExit("mounting tmpfs chmod");

	// build new root directory
	char *root;
	if (asprintf(&root, "%s/root", OVERLAY_DIR) == -1)
		errExit("asprintf");
	if (mkdir(root, S_IRWXU|S_IRWXG|S_IRWXO))
		errExit("mkdir");
	if (chown(root, 0, 0) == -1)
		errExit("chown");
	if (chmod(root, S_IRWXU|S_IRWXG|S_IRWXO))
		errExit("chmod");

	// mount overlayfs:
	//      mount -t overlayfs -o lowerdir=/,upperdir=$tmpdir/overlay overlayfs $tmpdir/root
	if (arg_debug)
		printf("Mounting OverlayFS\n");
	char *option;
	if (asprintf(&option, "lowerdir=/,upperdir=%s", OVERLAY_DIR) == -1)
		errExit("asprintf");
	if (mount("overlayfs", root, "overlayfs", MS_MGC_VAL, option) < 0)
		errExit("mounting overlayfs");

	// mount-bind dev directory
	if (arg_debug)
		printf("Mounting /dev\n");
	char *dev;
	if (asprintf(&dev, "%s/dev", root) == -1)
		errExit("asprintf");
	if (mount("/dev", dev, NULL, MS_BIND|MS_REC, NULL) < 0)
		errExit("mounting /dev");

	// chroot in the new filesystem
	if (chroot(root) == -1)
		errExit("chroot");

	// update /var directory in order to support multiple sandboxes running on the same root directory
	fs_dev_shm();
	fs_var_lock();
	fs_var_tmp();
	fs_var_log();
	fs_var_lib();
	fs_var_cache();

	// cleanup and exit
	free(option);
	free(root);
	free(dev);
}

// chroot into an existing directory; mount exiting /dev and update /etc/resolv.conf
void fs_chroot(const char *rootdir) {
	assert(rootdir);
	
	//***********************************
	// mount-bind a /dev in rootdir
	//***********************************
	// mount /dev
	char *newdev;
	if (asprintf(&newdev, "%s/dev", rootdir) == -1)
		errExit("asprintf");
	if (arg_debug)
		printf("Mounting /dev on %s\n", newdev);
	if (mount("/dev", newdev, NULL, MS_BIND|MS_REC, NULL) < 0)
		errExit("mounting /dev");
	
	// some older distros don't have a /run directory
	// create one by default
	// no exit on error, let the user deal with any problems
	char *rundir;
	if (asprintf(&rundir, "%s/run", rootdir) == -1)
		errExit("asprintf");
	if (!is_dir(rundir)) {
		int rv = mkdir(rundir, S_IRWXU | S_IRWXG | S_IRWXO);
		(void) rv;
		rv = chown(rundir, 0, 0);
		(void) rv;
	}
	
	// copy /etc/resolv.conf in chroot directory
	// if resolv.conf in chroot is a symbolic link, this will fail
	// no exit on error, let the user deal with the problem
	char *fname;
	if (asprintf(&fname, "%s/etc/resolv.conf", rootdir) == -1)
		errExit("asprintf");
	if (arg_debug)
		printf("Updating /etc/resolv.conf in %s\n", fname);
	if (copy_file("/etc/resolv.conf", fname) == -1)
		fprintf(stderr, "Warning: /etc/resolv.conf not initialized\n");
		
	// chroot into the new directory
	if (arg_debug)
		printf("Chrooting into %s\n", rootdir);
	if (chroot(rootdir) < 0)
		errExit("chroot");

	// update /var directory in order to support multiple sandboxes running on the same root directory
	fs_dev_shm();
	fs_var_lock();
	fs_var_tmp();
	fs_var_log();
	fs_var_lib();
	fs_var_cache();
}

