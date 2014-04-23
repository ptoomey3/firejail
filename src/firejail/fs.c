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


//***********************************************
// chroot filesystem
//***********************************************
static void mnt_tmp(void) {
	if (arg_debug)
		printf("Mounting new /tmp and /var/tmp directories\n");
	if (mount("tmpfs", "/var/tmp", "tmpfs", MS_NOSUID | MS_NOEXEC | MS_NODEV | MS_STRICTATIME | MS_REC,  "mode=755,gid=0") < 0)
		errExit("mounting /var/tmp");

	if (mount("tmpfs", "/tmp", "tmpfs", MS_NOSUID | MS_NOEXEC | MS_NODEV | MS_STRICTATIME | MS_REC,  "mode=777,gid=0") < 0)
		errExit("mounting /tmp");
}

static void disable_file(const char *fname, const char *emptydir, const char *emptyfile) {
	assert(fname);
	assert(emptydir);
	assert(emptyfile);

	struct stat s;
	if (stat(fname, &s) == 0) {
		if (S_ISDIR(s.st_mode)) {
			if (mount(emptydir, fname, "none", MS_BIND, "mode=400,gid=0") < 0)
				errExit("disable file");
		}
		else {
			if (mount(emptyfile, fname, "none", MS_BIND, "mode=400,gid=0") < 0)
				errExit("disable file");
		}
		if (arg_debug)
			printf("Disabling %s\n", fname);
	}
}

static void globbing(const char *fname, const char *emptydir, const char *emptyfile) {
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
			disable_file(globbuf.gl_pathv[i], emptydir, emptyfile);
		}
	}
	else
		disable_file(fname, emptydir, emptyfile);
}

static void expand_path(const char *path, const char *fname, const char *emptydir, const char *emptyfile) {
	assert(path);
	assert(fname);
	assert(emptydir);
	assert(emptyfile);
	char newname[strlen(path) + strlen(fname) + 1];
	sprintf(newname, "%s%s", path, fname);

	globbing(newname, emptydir, emptyfile);
}

// blacklist files or directoies by mounting empty files on top of them
void fs_blacklist(char **blacklist, const char *homedir) {
	char *emptydir;
	char *emptyfile;
	assert(tmpdir);

	// create read-only root directory
	if (asprintf(&emptydir, "%s/firejail.ro.dir", tmpdir) == -1)
		errExit("asprintf");
	mkdir(emptydir, S_IRWXU);
	if (chown(emptydir, 0, 0) < 0)
		errExit("chown");

	// create read-only root file
	if (asprintf(&emptyfile, "%s/firejail.ro.file", tmpdir) == -1)
		errExit("asprintf");
	FILE *fp = fopen(emptyfile, "w");
	if (!fp)
		errExit("fopen");
	fclose(fp);
	if (chown(emptyfile, 0, 0) < 0)
		errExit("chown");
	if (chmod(emptyfile, S_IRUSR) < 0)
		errExit("chown");

	int i = 0;
	while (blacklist[i]) {
		// process blacklist command
		if (strncmp(blacklist[i], "blacklist", 9) != 0) {
			fprintf(stderr, "Error: invalid profile line %s\n", blacklist[i]);
			i++;
			continue;
		}

		char *ptr = blacklist[i] + 10;
		// replace home macro in blacklist array
		char *new_name = NULL;
		if (strncmp(ptr, "${HOME}", 7) == 0) {
			if (asprintf(&new_name, "%s%s", homedir, ptr + 7) == -1)
				errExit("asprintf");
			ptr = new_name;
		}

		// expand path macro - look for the file in /bin, /usr/bin, /sbin and  /usr/sbin directories
		if (strncmp(ptr, "${PATH}", 7) == 0) {
			expand_path("/bin", ptr + 7, emptydir, emptyfile);
			expand_path("/sbin", ptr + 7, emptydir, emptyfile);
			expand_path("/usr/bin", ptr + 7, emptydir, emptyfile);
			expand_path("/usr/sbin", ptr + 7, emptydir, emptyfile);
		}
		else
			globbing(ptr, emptydir, emptyfile);
		if (new_name)
			free(new_name);
		i++;
	}

	free(emptydir);
	free(emptyfile);
}

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

// mount /proc and /sys directories
void fs_proc_sys(void) {
	if (arg_debug)
		printf("Remounting /proc and /proc/sys filesystems\n");
	if (mount("proc", "/proc", "proc", MS_NOSUID | MS_NOEXEC | MS_NODEV | MS_REC, NULL) < 0)
		errExit("mounting /proc");

	if (mount("/proc/sys", "/proc/sys", NULL, MS_BIND | MS_REC, NULL) < 0)
		errExit("mounting /proc/sys");

	if (mount(NULL, "/proc/sys", NULL, MS_BIND | MS_REMOUNT | MS_RDONLY | MS_REC, NULL) < 0)
		errExit("mounting /proc/sys");

	//	if (mount("sysfs", "/sys", "sysfs", MS_RDONLY|MS_NOSUID|MS_NOEXEC|MS_NODEV|MS_REC, NULL) < 0)
	//		errExit("/sys");
}

static void resolve_run_shm(void) {
	if (arg_debug) {
		char *lnk;
		if (is_dir("/var/run"))
			printf("/var/run is a directory\n");
		if (is_link("/var/run")) {
			lnk = get_link("/var/run");
			if (lnk) {
				printf("/var/run is a symbolic link to %s\n", lnk);
				free(lnk);
			}
		}
		if (is_dir("/dev/shm"))
			printf("/dev/shm is a directory\n");
		if (is_link("/dev/shm")) {
			lnk = get_link("/dev/shm");
			if (lnk) {
				printf("/dev/shm is a symbolic link to %s\n", lnk);
				free(lnk);
			}
		}
	}

	if (is_dir("/var/run")) {
		if (arg_debug)
			printf("Mounting tmpfs on /var/run\n");
		if (mount("tmpfs", "/var/run", "tmpfs", MS_NOSUID | MS_NODEV | MS_STRICTATIME | MS_REC,  "mode=777,gid=0") < 0)
			errExit("mounting /var/tmp");
	}
	else if (is_link("/var/run")) {
		char *lnk = get_link("/var/run");
		if (lnk) {
			// convert a link such as "../run" into "/run"
			char *lnk2 = lnk;
			int cnt = 0;
			while (strncmp(lnk2, "../", 3) == 0) {
				cnt++;
				lnk2 = lnk2 + 3;
			}
			if (cnt != 0)
				lnk2 = lnk + (cnt - 1) * 3 + 2;

			if (is_dir(lnk2)) {
				if (arg_debug)
					printf("Mounting tmpfs on %s directory\n", lnk2);
				if (mount("tmpfs", lnk2, "tmpfs", MS_NOSUID | MS_NODEV | MS_STRICTATIME | MS_REC,  "mode=777,gid=0") < 0)
					errExit("mounting tmpfs");
			}
			free(lnk);
		}
		else
			fprintf(stderr, "Warning: /var/run not mounted\n");
	}

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
				printf("Mounting tmpfs on %s\n", lnk2);
			if (mount("tmpfs", lnk2, "tmpfs", MS_NOSUID | MS_STRICTATIME | MS_REC,  "mode=777,gid=0") < 0)
				errExit("mounting /var/tmp");
			free(lnk);
		}
		else
			fprintf(stderr, "Warning: /dev/shm not mounted\n");
	}
}

// build a basic read-only filesystem
void fs_basic_fs(void) {
	if (arg_debug)
		printf("Mounting read-only /bin, /sbin, /lib, /lib64, /usr, /boot, /etc, /var\n");
	fs_rdonly("/bin");
	fs_rdonly("/sbin");
	fs_rdonly("/lib");
	fs_rdonly("/lib64");
	fs_rdonly("/usr");
	fs_rdonly("/boot");
	fs_rdonly("/etc");
	fs_rdonly("/var");
	resolve_run_shm();
}



// private mode: mount tmpfs over /home and /tmp
void fs_private(const char *homedir) {
	assert(homedir);
	
	if (arg_debug)
		printf("Mounting a new /home directory\n");
	if (mount("tmpfs", homedir, "tmpfs", MS_NOSUID | MS_NODEV | MS_STRICTATIME | MS_REC,  "mode=755,gid=0") < 0)
		errExit("mounting home directory");

	uid_t u = getuid();
	gid_t g = getgid();
	if (chown(homedir, u, g) == -1)
		errExit("chown");

	// copy skel files
	char *fname;
	if (asprintf(&fname, "%s/.bashrc", homedir) == -1)
		errExit("asprintf");

	if (copy_file("/etc/skel/.bashrc", fname) == 0) {
		if (chown(fname, u, g) == -1)
			errExit("chown");
	}
	free(fname);

	if (arg_debug)
		printf("Mounting a new /tmp directory\n");
	if (mount("tmpfs", "/tmp", "tmpfs", MS_NOSUID | MS_NODEV | MS_STRICTATIME | MS_REC,  "mode=777,gid=0") < 0)
		errExit("mounting tmp directory");

}

// mount overlayfs on top of / directory
void fs_overlayfs(void) {
	assert(tmpdir);

	// build overlay directory
	char *overlay;
	if (asprintf(&overlay, "%s/overlay", tmpdir) == -1)
		errExit("asprintf");
	if (mkdir(overlay, S_IRWXU|S_IRWXG|S_IRWXO))
		errExit("mkdir");
	if (chown(overlay, 0, 0) == -1)
		errExit("chown");
	if (chmod(overlay, S_IRWXU|S_IRWXG|S_IRWXO))
		errExit("chmod");

	// build new root directory
	char *root;
	if (asprintf(&root, "%s/root", tmpdir) == -1)
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
	if (asprintf(&option, "lowerdir=/,upperdir=%s", overlay) == -1)
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

	resolve_run_shm();

	// chroot in the new filesystem
	if (chroot(root) == -1)
		errExit("chroot");

	// cleanup and exit
	free(option);
	free(root);
	free(overlay);
	free(dev);
}

// chroot into an existing directory; mount exiting /dev and update /etc/resolv.conf
void fs_chroot(const char *rootdir) {
	assert(rootdir);
	
	// mount-bind a /dev in rootdir
	if (arg_debug)
		printf("Mounting /dev in chroot directory %s\n", rootdir);
	char *newdev;
	if (asprintf(&newdev, "%s/dev", rootdir) == -1)
		errExit("asprintf");
	if (mount("/dev", newdev, NULL, MS_BIND|MS_REC, NULL) < 0)
		errExit("mounting /dev");
	
	// copy /etc/resolv.conf in chroot directory
	if (arg_debug)
		printf("Updating /etc/resolv.conf\n");
	char *fname;
	if (asprintf(&fname, "%s/etc/resolv.conf", rootdir) == -1)
		errExit("asprintf");
	if (copy_file("/etc/resolv.conf", fname) == -1)
		fprintf(stderr, "Warning: /etc/resolv.conf not initialized\n");
		
	resolve_run_shm();

	// chroot into the new directory
	if (chroot(rootdir) < 0)
		errExit("chroot");
}
