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
// utils
//***********************************************
// return -1 if error, 0 if no error
static int copy_file(const char *srcname, const char *destname) {
	assert(srcname);
	assert(destname);
	
	// open source
	int src = open(srcname, O_RDONLY);
	if (src < 0)
		return -1;
	
	// open destination
	int dst = open(destname, O_CREAT|O_WRONLY|O_TRUNC, S_IRUSR | S_IWUSR | S_IRGRP | S_IROTH);
	if (dst < 0)
		return -1;
	
	// copy
	ssize_t len;
	static const int BUFLEN = 1024;
	unsigned char buf[BUFLEN];
	while ((len = read(src, buf, BUFLEN)) > 0) {
		int done = 0;
		while (done != len) {
			int rv = write(dst, buf + done, len - done);
			if (rv == -1) {
				close(src);
				close(dst);
				return -1;
			}
			
			done += rv;
		}
	}
	
	close(src);
	close(dst);
	return 0;
}

char *get_link(const char *fname) {
	assert(fname);
	struct stat sb;
	char *linkname;
	ssize_t r;
	
	if (lstat(fname, &sb) == -1)
		return NULL;

	linkname = malloc(sb.st_size + 1);
	if (linkname == NULL)
		return NULL;
	memset(linkname, 0, sb.st_size + 1);

	r = readlink(fname, linkname, sb.st_size + 1);
	if (r < 0)
		return NULL;
	return linkname;
}

int is_dir(const char *fname) {
	assert(fname);
	struct stat s;
	if (lstat(fname, &s) == 0) {
		if (S_ISDIR(s.st_mode))
			return 1;
	}

	return 0;
}

int is_link(const char *fname) {
	assert(fname);
	struct stat s;
	if (lstat(fname, &s) == 0) {
		if (S_ISLNK(s.st_mode))
			return 1;
	}

	return 0;
}


//***********************************************
// atexit
//***********************************************
static char *tmpdir = NULL;
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
		errExit("Error asprintf");
	tmpdir = mkdtemp(name_template);
	if (tmpdir == NULL)
		errExit("Error mkdtemp");
	if (arg_debug)
		printf("Creating %s directory\n", tmpdir);
	mkdir(tmpdir, S_IRWXU);
	uid_t u = getuid();
	gid_t g = getgid();
	if (chown(tmpdir, u, g) < 0)
		errExit("Error chown");

	if (atexit(bye_parent))
		errExit("Error atexit");
}

//***********************************************
// chroot filesystem
//***********************************************
static void mnt_tmp(void) {
	if (arg_debug)
		printf("Mounting new /tmp and /var/tmp directories\n");
	if (mount("tmpfs", "/var/tmp", "tmpfs", MS_NOSUID | MS_NOEXEC | MS_NODEV | MS_STRICTATIME | MS_REC,  "mode=755,gid=0") < 0)
		errExit("Error mounting /var/tmp");

	if (mount("tmpfs", "/tmp", "tmpfs", MS_NOSUID | MS_NOEXEC | MS_NODEV | MS_STRICTATIME | MS_REC,  "mode=777,gid=0") < 0)
		errExit("Error mounting /tmp");
}

static void disable_file(const char *fname, const char *emptydir, const char *emptyfile) {
	assert(fname);
	assert(emptydir);
	assert(emptyfile);

	struct stat s;
	if (stat(fname, &s) == 0) {
		if (S_ISDIR(s.st_mode)) {
			if (mount(emptydir, fname, "none", MS_BIND, "mode=400,gid=0") < 0)
				errExit("Error disable file");
		}
		else {
			if (mount(emptyfile, fname, "none", MS_BIND, "mode=400,gid=0") < 0)
				errExit("Error disable file");
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
void mnt_blacklist(char **blacklist, const char *homedir) {
	char *emptydir;
	char *emptyfile;
	assert(tmpdir);

	// create read-only root directory
	if (asprintf(&emptydir, "%s/firejail.ro.dir", tmpdir) == -1)
		errExit("Error asprintf");
	mkdir(emptydir, S_IRWXU);
	if (chown(emptydir, 0, 0) < 0)
		errExit("Error chown");

	// create read-only root file
	if (asprintf(&emptyfile, "%s/firejail.ro.file", tmpdir) == -1)
		errExit("Error asprintf");
	FILE *fp = fopen(emptyfile, "w");
	if (!fp)
		errExit("Error fopen");
	fclose(fp);
	if (chown(emptyfile, 0, 0) < 0)
		errExit("Error chown");
	if (chmod(emptyfile, S_IRUSR) < 0)
		errExit("Error chown");

	int i = 0;
	while (blacklist[i]) {
		// process newtmp macro
		if (strncmp(blacklist[i], "newdir", 6) == 0) {
			if (strcmp(blacklist[i], "Error newdir /tmp") == 0)
				mnt_tmp();
			else
				fprintf(stderr, "Warning: %s not implemented yet\n", blacklist[i]);
			i++;
			continue;
		}

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
				errExit("Error asprintf");
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
void mnt_rdonly(const char *dir) {
	assert(dir);
	// check directory exists
	struct stat s;
	int rv = stat(dir, &s);
	if (rv == 0) {
		// mount --bind /bin /bin
		if (mount(dir, dir, NULL, MS_BIND|MS_REC, NULL) < 0)
			errExit("Error mount read-only");
		// mount --bind -o remount,ro /bin
		if (mount(NULL, dir, NULL, MS_BIND|MS_REMOUNT|MS_RDONLY|MS_REC, NULL) < 0)
			errExit("Error mount read-only");
	}
}

// mount /proc and /sys directories
void mnt_proc_sys(void) {
	if (arg_debug)
		printf("Remounting /proc and /proc/sys filesystems\n");
	if (mount("proc", "/proc", "proc", MS_NOSUID | MS_NOEXEC | MS_NODEV | MS_REC, NULL) < 0)
		errExit("Error mounting /proc");

	if (mount("/proc/sys", "/proc/sys", NULL, MS_BIND | MS_REC, NULL) < 0)
		errExit("Error mounting /proc/sys");

	if (mount(NULL, "/proc/sys", NULL, MS_BIND | MS_REMOUNT | MS_RDONLY | MS_REC, NULL) < 0)
		errExit("Error mounting /proc/sys");

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
			errExit("Error mounting /var/tmp");
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
					errExit("Error mounting tmpfs");
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
			errExit("Error mounting /dev/shm");
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
					errExit("Error mkdir");
				if (chown(lnk2, 0, 0))
					errExit("Error chown");
				if (chmod(lnk2, S_IRWXU|S_IRWXG|S_IRWXO))
					errExit("Error chmod");
			}
			if (arg_debug)
				printf("Mounting tmpfs on %s\n", lnk2);
			if (mount("tmpfs", lnk2, "tmpfs", MS_NOSUID | MS_STRICTATIME | MS_REC,  "mode=777,gid=0") < 0)
				errExit("Error mounting /var/tmp");
			free(lnk);
		}
		else
			fprintf(stderr, "Warning: /dev/shm not mounted\n");
	}
}

// build a basic read-only filesystem
void mnt_basic_fs(void) {
	if (arg_debug)
		printf("Mounting read-only /bin, /sbin, /lib, /lib64, /usr, /boot, /etc\n");
	mnt_rdonly("/bin");
	mnt_rdonly("/sbin");
	mnt_rdonly("/lib");
	mnt_rdonly("/lib64");
	mnt_rdonly("/usr");
	mnt_rdonly("/boot");
	mnt_rdonly("/etc");
	resolve_run_shm();
}




void mnt_home(const char *homedir) {
	assert(homedir);
	
	if (arg_debug)
		printf("Mounting a new /home directory\n");
	if (mount("tmpfs", homedir, "tmpfs", MS_NOSUID | MS_NODEV | MS_STRICTATIME | MS_REC,  "mode=755,gid=0") < 0)
		errExit("Error mounting home directory");

	uid_t u = getuid();
	gid_t g = getgid();

	// copy skel files
	char *fname;
	if (asprintf(&fname, "%s/.bashrc", homedir) == -1)
		errExit("Error asprintf");

	if (copy_file("/etc/skel/.bashrc", fname) == 0) {
		if (chown(fname, u, g) == -1)
			errExit("Error chown");
	}
	free(fname);
}

void mnt_overlayfs(void) {
	assert(tmpdir);

	// build overlay directory
	char *overlay;
	if (asprintf(&overlay, "%s/overlay", tmpdir) == -1)
		errExit("Error asprintf");
	if (mkdir(overlay, S_IRWXU|S_IRWXG|S_IRWXO))
		errExit("Error mkdir");
	if (chown(overlay, 0, 0) == -1)
		errExit("Error chown");
	if (chmod(overlay, S_IRWXU|S_IRWXG|S_IRWXO))
		errExit("Error chmod");

	// build new root directory
	char *root;
	if (asprintf(&root, "%s/root", tmpdir) == -1)
		errExit("Error asprintf");
	if (mkdir(root, S_IRWXU|S_IRWXG|S_IRWXO))
		errExit("Error mkdir");
	if (chown(root, 0, 0) == -1)
		errExit("Error chown");
	if (chmod(root, S_IRWXU|S_IRWXG|S_IRWXO))
		errExit("Error chmod");

	// mount overlayfs:
	//      mount -t overlayfs -o lowerdir=/,upperdir=$tmpdir/overlay overlayfs $tmpdir/root
	if (arg_debug)
		printf("Mounting OverlayFS\n");
	char *option;
	if (asprintf(&option, "lowerdir=/,upperdir=%s", overlay) == -1)
		errExit("Error asprintf");
	if (mount("overlayfs", root, "overlayfs", MS_MGC_VAL, option) < 0)
		errExit("Error mounting overlayfs");

	// mount-bind dev directory
	if (arg_debug)
		printf("Mounting /dev\n");
	char *dev;
	if (asprintf(&dev, "%s/dev", root) == -1)
		errExit("Error asprintf");
	if (mount("/dev", dev, NULL, MS_BIND|MS_REC, NULL) < 0)
		errExit("Error mounting /dev");

	resolve_run_shm();

	// chroot in the new filesystem
	if (chroot(root) == -1)
		errExit("Error chroot");

	// cleanup and exit
	free(option);
	free(root);
	free(overlay);
	free(dev);
}

void mnt_chroot(const char *rootdir) {
	assert(rootdir);
	
	// mount-bind a /dev in rootdir
	if (arg_debug)
		printf("Mounting /dev in chroot directory %s\n", rootdir);
	char *newdev;
	if (asprintf(&newdev, "%s/dev", rootdir) == -1)
		errExit("Error asprintf");
	if (mount("/dev", newdev, NULL, MS_BIND|MS_REC, NULL) < 0)
		errExit("Error mounting /dev");
	
	// copy /etc/resolv.conf in chroot directory
	if (arg_debug)
		printf("Updating /etc/resolv.conf\n");
	char *fname;
	if (asprintf(&fname, "%s/etc/resolv.conf", rootdir) == -1)
		errExit("Error asprintf");
	if (copy_file("/etc/resolv.conf", fname) == -1)
		fprintf(stderr, "Warning: /etc/resolv.conf not initialized\n");
		
	resolve_run_shm();

	// chroot into the new directory
	if (chroot(rootdir) < 0)
		errExit("Error chroot");
}
