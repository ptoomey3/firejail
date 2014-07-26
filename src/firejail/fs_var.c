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

static void build_dirs(void) {
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
	
void fs_var_log(void) {
	build_list("/var/log");
	
	// create /var/log if it does't exit
	struct stat s;
	if (is_dir("/var/log")) {
		// mount a tmpfs on top of /var/log
		if (arg_debug)
			printf("Mounting tmpfs on /var/log\n");
		if (mount("tmpfs", "/var/log", "tmpfs", MS_NOSUID | MS_STRICTATIME | MS_REC,  "mode=755,gid=0") < 0)
			errExit("mounting /var/log");
		
		build_dirs();
		release_all();
	}
	else
		fprintf(stderr, "Warning: cannot mount tmpfs in top of /var/log\n");
}

void fs_var_lib(void) {
	struct stat s;
	if (stat("/var/lib/dhcp", &s) == 0) {
		if (arg_debug)
			printf("Mounting tmpfs on /var/lib/dhcp\n");
		if (mount("tmpfs", "/var/lib/dhcp", "tmpfs", MS_NOSUID | MS_STRICTATIME | MS_REC,  "mode=755,gid=0") < 0)
			errExit("mounting /var/lib/dhcp");
			
		// isc dhcp server requires a /var/lib/dhcp/dhcpd.leases file
		FILE *fp = fopen("/var/lib/dhcp/dhcpd.leases", "w");
		
		if (fp) {
			fprintf(fp, "\n");
			fclose(fp);
			if (chown("/var/lib/dhcp/dhcpd.leases", 0, 0) == -1)
				errExit("chown");
			if (chmod("/var/lib/dhcp/dhcpd.leases", S_IRUSR|S_IWUSR|S_IRGRP|S_IROTH))
				errExit("chmod");
		}
	}

	if (stat("/var/lib/nginx", &s) == 0) {
		if (arg_debug)
			printf("Mounting tmpfs on /var/lib/nginx\n");
		if (mount("tmpfs", "/var/lib/nginx", "tmpfs", MS_NOSUID | MS_STRICTATIME | MS_REC,  "mode=755,gid=0") < 0)
			errExit("mounting /var/lib/nginx");
	}			

	if (stat("/var/lib/snmp", &s) == 0) {
		if (arg_debug)
			printf("Mounting tmpfs on /var/lib/snmp\n");
		if (mount("tmpfs", "/var/lib/snmp", "tmpfs", MS_NOSUID | MS_STRICTATIME | MS_REC,  "mode=755,gid=0") < 0)
			errExit("mounting /var/lib/snmp");
	}			

	if (stat("/var/lib/sudo", &s) == 0) {
		if (arg_debug)
			printf("Mounting tmpfs on /var/lib/sudo\n");
		if (mount("tmpfs", "/var/lib/sudo", "tmpfs", MS_NOSUID | MS_STRICTATIME | MS_REC,  "mode=755,gid=0") < 0)
			errExit("mounting /var/lib/sudo");
	}			

}

void fs_var_cache(void) {
	struct stat s;

	if (stat("/var/cache/apache2", &s) == 0) {
		if (arg_debug)
			printf("Mounting tmpfs on /var/cache/apache2\n");
		if (mount("tmpfs", "/var/cache/apache2", "tmpfs", MS_NOSUID | MS_STRICTATIME | MS_REC,  "mode=755,gid=0") < 0)
			errExit("mounting /var/cahce/apache2");
	}			

	if (stat("/var/cache/lighttpd", &s) == 0) {
		if (arg_debug)
			printf("Mounting tmpfs on /var/cache/lighttpd\n");
		if (mount("tmpfs", "/var/cache/lighttpd", "tmpfs", MS_NOSUID | MS_STRICTATIME | MS_REC,  "mode=755,gid=0") < 0)
			errExit("mounting /var/cache/lighttpd");

		struct passwd *p = getpwnam("www-data");
		uid_t uid = 0;
		gid_t gid = 0;
		if (p) {
			uid = p->pw_uid;
			gid = p->pw_gid;
		}
		
		mkdir("/var/cache/lighttpd/compress", S_IRWXU | S_IRWXG | S_IRWXO);
		if (chown("/var/cache/lighttpd/compress", uid, gid) < 0)
			errExit("chown");

		mkdir("/var/cache/lighttpd/uploads", S_IRWXU | S_IRGRP | S_IXGRP | S_IROTH | S_IXOTH);
		if (chown("/var/cache/lighttpd/uploads", uid, gid) < 0)
			errExit("chown");
	}			
}

static void dbg_test_dir(const char *dir) {
	if (arg_debug) {
		if (is_dir(dir))
			printf("%s is a directory\n", dir);
		if (is_link("/dev/shm")) {
			char *lnk = get_link(dir);
			if (lnk) {
				printf("%s is a symbolic link to %s\n", dir, lnk);
				free(lnk);
			}
		}
	}
}

void fs_dev_shm(void) {
	
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

void fs_var_run(void) {
	// create a temporary resolv.conf file
	pid_t pid = getpid();
	char *resolv_fname;
	if (asprintf(&resolv_fname, "/tmp/resolv.conf-%u-XXXXXX", pid) == -1)
		errExit("asprintf");
	int h = mkstemp(resolv_fname);
	if (h == -1)
		errExit("mkstemp");
	// close the file and copy the content of resolv.conf into it
	close(h);	
	int resolv_err = copy_file("/etc/resolv.conf", resolv_fname);

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
					printf("Mounting tmpfs on %s directory on behalf of /var/run\n", lnk2);
				if (mount("tmpfs", lnk2, "tmpfs", MS_NOSUID | MS_NODEV | MS_STRICTATIME | MS_REC,  "mode=777,gid=0") < 0)
					errExit("mounting tmpfs");
			}
			free(lnk);
		}
		else {
			fprintf(stderr, "Warning: /var/run not mounted\n");
			dbg_test_dir("/var/run");
			unlink(resolv_fname);
			free(resolv_fname);
			return;
		}
	}
	
	// create /run directory where resolv.conf can reside
	if (resolv_err == 0) {
		mkdir("/run/resolvconf", S_IRWXU | S_IRGRP | S_IXGRP | S_IROTH | S_IXOTH);
		mkdir("/run/systemd", S_IRWXU | S_IRGRP | S_IXGRP | S_IROTH | S_IXOTH);
		mkdir("/run/systemd/resolve", S_IRWXU | S_IRGRP | S_IXGRP | S_IROTH | S_IXOTH);
		copy_file(resolv_fname, "/run/resolvconf/resolv.conf");
		copy_file(resolv_fname, "/run/systemd/resolve/resolv.conf");
	}
	unlink(resolv_fname);
	free(resolv_fname);
}

void fs_var_lock(void) {

	if (is_dir("/var/lock")) {
		if (arg_debug)
			printf("Mounting tmpfs on /var/lock\n");
		if (mount("tmpfs", "/var/lock", "tmpfs", MS_NOSUID | MS_STRICTATIME | MS_REC,  "mode=777,gid=0") < 0)
			errExit("mounting /lock");
	}
	else {
		char *lnk = get_link("/var/lock");
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
				printf("Mounting tmpfs on %s on behalf of /var/lock\n", lnk2);
			if (mount("tmpfs", lnk2, "tmpfs", MS_NOSUID | MS_STRICTATIME | MS_REC,  "mode=777,gid=0") < 0)
				errExit("mounting /var/lock");
			free(lnk);
		}
		else {
			fprintf(stderr, "Warning: /dev/lock not mounted\n");
			dbg_test_dir("/var/lock");
		}
	}
}

void fs_var_tmp(void) {

	if (!is_link("/var/tmp")) {
		if (arg_debug)
			printf("Mounting tmpfs on /var/tmp\n");
		if (mount("tmpfs", "/var/tmp", "tmpfs", MS_NOSUID | MS_STRICTATIME | MS_REC,  "mode=777,gid=0") < 0)
			errExit("mounting /var/tmp");
	}
	else {
		fprintf(stderr, "Warning: /var/tmp not mounted\n");
		dbg_test_dir("/var/tmp");
	}
}


#if 0
Testing servers:

brctl addbr br0
ifconfig br0 10.10.20.1/24

apt-get install snmpd
insserv -r snmpd
sudo firejail --net=br0 --ip=10.10.20.10 "/etc/init.d/rsyslog start; /etc/init.d/ssh start; /etc/init.d/snmpd start; sleep inf"

apt-get install apache2
insserv -r apache2
sudo firejail --net=br0 --ip=10.10.20.10 "/etc/init.d/rsyslog start; /etc/init.d/ssh start; /etc/init.d/apache2 start; sleep inf"

apt-get install nginx
insserv -r nginx
sudo firejail --net=br0 --ip=10.10.20.10 "/etc/init.d/rsyslog start; /etc/init.d/ssh start; /etc/init.d/nginx start; sleep inf"

apt-get install lighttpd
insserv -r lighttpd
sudo firejail --net=br0 --ip=10.10.20.10 "/etc/init.d/rsyslog start; /etc/init.d/ssh start; /etc/init.d/lighttpd start; sleep inf"

apt-get install isc-dhcp-server
insserv -r isc-dhcp-server
sudo firejail --net=br0 --ip=10.10.20.10 "/etc/init.d/rsyslog start; /etc/init.d/ssh start; /etc/init.d/isc-dhcp-server start; sleep inf"
#endif
