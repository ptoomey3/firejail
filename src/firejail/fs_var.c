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
	
void fs_var_log(void) {
	build_list("/var/log");
	
	// mount a tmpfs on top of /var/log
	if (arg_debug)
		printf("Mounting tmpfs on /var/log\n");
	if (mount("tmpfs", "/var/log", "tmpfs", MS_NOSUID | MS_STRICTATIME | MS_REC,  "mode=755,gid=0") < 0)
		errExit("mounting /var/log");
	
	build_dirs();
	release_all();
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

	if (stat("/var/lib/ngix", &s) == 0) {
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

void fs_var_run_shm(void) {
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
		
		if (is_dir("/var/lock"))
			printf("/var/lock is a directory\n");
		if (is_link("/var/lock")) {
			lnk = get_link("/var/lock");
			if (lnk) {
				printf("/var/lock is a symbolic link to %s\n", lnk);
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
	
	int run_resolv_conf = 0; // resolv.conf detected under /run/resolvconf directory
	// grab a copy of resolv.conf in case it is a symbolic link into /run directory
	// - setting found in Ubuntu
	// - setting found in Debian when resolvconf package is installed
	if (is_link("/etc/resolv.conf")) {
		char *lnk = get_link("/etc/resolv.conf");
		if (lnk) {
			if (arg_debug)
				printf("/etc/resolv.conf is a symbolic link to %s\n", lnk);
			struct stat s;
			if (stat("/run/resolvconf/resolv.conf", &s) == 0) {
				if (arg_debug)
					printf("Found /run/resolvconf/resolv.conf\n");
				int rv = copy_file("/run/resolvconf/resolv.conf", "/tmp/resolv.conf");
				if (rv == -1)
					fprintf(stderr,"Warning: /etc/resolv.conf not initialized\n");
				else
					run_resolv_conf = 1;
			}
			free(lnk);
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
				printf("Mounting tmpfs on %s\n", lnk2);
			if (mount("tmpfs", lnk2, "tmpfs", MS_NOSUID | MS_STRICTATIME | MS_REC,  "mode=777,gid=0") < 0)
				errExit("mounting /var/lock");
			free(lnk);
		}
		else
			fprintf(stderr, "Warning: /dev/lock not mounted\n");
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
	
	// restore resolv.conf
	if (run_resolv_conf) {
		// create directory
		if (mkdir("/run/resolvconf", S_IRWXU|S_IRWXG|S_IRWXO))
			errExit("mkdir");
		if (chown("/run/resolvconf", 0, 0))
			errExit("chown");
		
		// copy file
		int rv = copy_file("/tmp/resolv.conf", "/run/resolvconf/resolv.conf");
		if (rv == -1)
			fprintf(stderr, "Warning: /etc/resolv.conf not initialized\n");
		if (chown("/run/resolvconf/resolv.conf", 0, 0))
			errExit("chown");
		unlink("/tmp/resolv.conf");
		if (arg_debug)
			printf("Updated /run/resolvconf/resolv.conf\n");
	}
	
	if (!is_link("/var/tmp")) {
		if (arg_debug)
			printf("Mounting tmpfs on /var/tmp\n");
		if (mount("tmpfs", "/var/tmp", "tmpfs", MS_NOSUID | MS_STRICTATIME | MS_REC,  "mode=777,gid=0") < 0)
			errExit("mounting /var/tmp");
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