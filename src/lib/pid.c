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
#include <stdlib.h>
#include <stdio.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <string.h>
#include <dirent.h>
#include <pwd.h>
#include <termios.h>
#include <sys/ioctl.h>
#include "../include/pid.h"

#define PIDS_BUFLEN 4096
Process pids[MAX_PIDS];
#define errExit(msg)    do { char msgout[500]; sprintf(msgout, "Error %s %s %d", msg, __FUNCTION__, __LINE__); perror(msgout); exit(1);} while (0)
static unsigned long long sysuptime = 0;
unsigned clocktick = 0;
static unsigned pgs_rss = 0;
static unsigned pgs_shared = 0;

// get the memory associated with this pid
static void getmem(unsigned pid) {
	// open stat file
	char *file;
	if (asprintf(&file, "/proc/%u/statm", pid) == -1) {
		perror("asprintf");
		exit(1);
	}
	FILE *fp = fopen(file, "r");
	if (!fp) {
		free(file);
		return;
	}
	free(file);
	
	unsigned a, b, c;
	if (3 != fscanf(fp, "%u %u %u", &a, &b, &c))
		return;
	pgs_rss += b;
	pgs_shared += c;
	fclose(fp);
	
}


static void get_cpu_time(unsigned pid, unsigned *utime, unsigned *stime) {
	// open stat file
	char *file;
	if (asprintf(&file, "/proc/%u/stat", pid) == -1) {
		perror("asprintf");
		exit(1);
	}
	FILE *fp = fopen(file, "r");
	if (!fp) {
		free(file);
		return;
	}
	free(file);
	
	char line[PIDS_BUFLEN];
	if (fgets(line, PIDS_BUFLEN - 1, fp)) {
		char *ptr = line;
		// jump 13 fields
		int i;
		for (i = 0; i < 13; i++) {
			while (*ptr != ' ' && *ptr != '\t' && *ptr != '\0')
				ptr++;
			if (*ptr == '\0')
				return;
			ptr++;
		}
		if (2 != sscanf(ptr, "%u %u", utime, stime))
			return;
	}
	
	fclose(fp);
}

static unsigned long long get_start_time(unsigned pid) {
	// open stat file
	char *file;
	if (asprintf(&file, "/proc/%u/stat", pid) == -1) {
		perror("asprintf");
		exit(1);
	}
	FILE *fp = fopen(file, "r");
	if (!fp) {
		free(file);
		return 0;
	}
	free(file);
	
	char line[PIDS_BUFLEN];
	unsigned long long retval;
	if (fgets(line, PIDS_BUFLEN - 1, fp)) {
		char *ptr = line;
		// jump 13 fields
		int i;
		for (i = 0; i < 21; i++) {
			while (*ptr != ' ' && *ptr != '\t' && *ptr != '\0')
				ptr++;
			if (*ptr == '\0')
				return 0;
			ptr++;
		}
		if (1 != sscanf(ptr, "%llu", &retval))
			return 0;
	}
	
	fclose(fp);
	return retval;
}

char *pid_proc_cmdline(const pid_t pid) {
	// open /proc/pid/cmdline file
	char *fname;
	int fd;
	if (asprintf(&fname, "/proc/%d/cmdline", pid) == -1)
		return NULL;
	if ((fd = open(fname, O_RDONLY)) < 0) {
		free(fname);
		return NULL;
	}
	free(fname);

	// read file
	char buffer[PIDS_BUFLEN];
	ssize_t len;
	if ((len = read(fd, buffer, sizeof(buffer) - 1)) <= 0) {
		close(fd);
		return NULL;
	}
	buffer[len] = '\0';
	close(fd);

	// clean data
	int i;
	for (i = 0; i < len; i++)
		if (buffer[i] == '\0')
			buffer[i] = ' ';

	// return a malloc copy of the command line
	char *rv = strdup(buffer);
	if (strlen(rv) == 0) {
		free(rv);
		return NULL;
	}
	return rv;
}

char *pid_get_user_name(uid_t uid) {
	struct passwd *pw = getpwuid(uid);
	if (pw)
		return strdup(pw->pw_name);
	return NULL;
}

uid_t pid_get_uid(pid_t pid) {
	uid_t rv = 0;
	
	// open stat file
	char *file;
	if (asprintf(&file, "/proc/%u/status", pid) == -1) {
		perror("asprintf");
		exit(1);
	}
	FILE *fp = fopen(file, "r");
	if (!fp) {
		free(file);
		return 0;
	}

	// look for firejail executable name
	char buf[PIDS_BUFLEN];
	while (fgets(buf, PIDS_BUFLEN - 1, fp)) {
		if (strncmp(buf, "Uid:", 4) == 0) {
			char *ptr = buf + 5;
			while (*ptr != '\0' && (*ptr == ' ' || *ptr == '\t')) {
				ptr++;
			}
			if (*ptr == '\0')
				goto doexit;
				
			rv = atoi(ptr);
			break; // break regardless!
		}
	}
doexit:	
	fclose(fp);
	free(file);
	return rv;
}

int pid_is_firejail(pid_t pid) {
	uid_t rv = 0;
	
	// open stat file
	char *file;
	if (asprintf(&file, "/proc/%u/status", pid) == -1) {
		perror("asprintf");
		exit(1);
	}
	FILE *fp = fopen(file, "r");
	if (!fp) {
		free(file);
		return 0;
	}

	// look for firejail executable name
	char buf[PIDS_BUFLEN];
	while (fgets(buf, PIDS_BUFLEN - 1, fp)) {
		if (strncmp(buf, "Name:", 5) == 0) {
			char *ptr = buf + 5;
			while (*ptr != '\0' && (*ptr == ' ' || *ptr == '\t')) {
				ptr++;
			}
			if (*ptr == '\0')
				goto doexit;
			if (strncmp(ptr, "firejail", 8) == 0)
				rv = 1;
			break;
		}
	}
doexit:	
	fclose(fp);
	free(file);
	return rv;
}

static void print_elem(unsigned index, int nowrap) {
	// get terminal size
	struct winsize sz;
	int col = 0;
	if (isatty(STDIN_FILENO)) {
		if (!ioctl(0, TIOCGWINSZ, &sz))
			col  = sz.ws_col;
	}

	// indent
	char indent[(pids[index].level - 1) * 2 + 1];
	memset(indent, ' ', sizeof(indent));
	indent[(pids[index].level - 1) * 2] = '\0';

	// get data
	uid_t uid = pids[index].uid;
	char *cmd = pid_proc_cmdline(index);
	char *user = pid_get_user_name(uid);
	char *allocated = user;
	if (user ==NULL)
		user = "";
	if (cmd) {
		if (col < 4 || nowrap) 
			printf("%s%u:%s:%s\n", indent, index, user, cmd);
		else {
			char *out;
			if (asprintf(&out, "%s%u:%s:%s\n", indent, index, user, cmd) == -1)
				errExit("asprintf");
			int len = strlen(out);
			if (len > col) {
				out[col] = '\0';
				out[col - 1] = '\n';
			}
			printf("%s", out);
			free(out);
		}
				
		free(cmd);
	}
	else {
		if (pids[index].zombie)
			printf("%u: (zombie)\n", index);
		else
			printf("%u:\n", index);
	}
	if (allocated)
		free(allocated);
}

// recursivity!!!
void pid_print_tree(unsigned index, unsigned parent, int nowrap) {
	print_elem(index, nowrap);
	
	int i;
	for (i = index + 1; i < MAX_PIDS; i++) {
		if (pids[i].parent == index)
			pid_print_tree(i, index, nowrap);
	}
}


void pid_print_mem_header(void) {
	printf("%-55.55s  %-10.10s %s\n", "PID:user:command", "RSS (KiB)", "Shared (Kib)");
}


// recursivity!!!
void pid_print_mem(unsigned index, unsigned parent) {
	if (pids[index].level == 1) {
		pgs_rss = 0;
		pgs_shared = 0;
		
		char pidstr[10];
		snprintf(pidstr, 10, "%u", index);

		char *cmd = pid_proc_cmdline(index);
		char *ptrcmd;
		if (cmd == NULL) {
			if (pids[index].zombie)
				ptrcmd = "(zombie)";
			else
				ptrcmd = "";
		}
		else
			ptrcmd = cmd;
		
		char *user = pid_get_user_name(pids[index].uid);
		char *ptruser;
		if (user)
			ptruser = user;
		else
			ptruser = "";
			
		char entry[60];
		snprintf(entry, 60, "%s:%s:%s", pidstr, ptruser, ptrcmd);
			
		printf("%-55.55s  ", entry);
		if (cmd)
			free(cmd);
		if (user)
			free(user);
	}
	
	getmem(index);
	
	
	int i;
	for (i = index + 1; i < MAX_PIDS; i++) {
		if (pids[i].parent == index)
			pid_print_mem(i, index);
	}

	if (pids[index].level == 1) {
		int pgsz = getpagesize();
		char rss[10];
		snprintf(rss, 10, "%u", pgs_rss * pgsz / 1024);
		char shared[10];
		snprintf(shared, 10, "%u", pgs_shared * pgsz / 1024);
		printf("%-10.10s %-10.10s\n", rss, shared);
	}
}

void pid_print_uptime_header(void) {
	// open stat file
	FILE *fp = fopen("/proc/uptime", "r");
	if (fp) {
		float f;
		int rv = fscanf(fp, "%f", &f);
		sysuptime = (unsigned long long) f;
		fclose(fp);
	}

	printf("%-55.55s  %s\n", "PID:user:command", "Uptime");
}


// recursivity!!!
void pid_print_uptime(unsigned index, unsigned parent) {
	if (pids[index].level == 1) {
		char pidstr[10];
		snprintf(pidstr, 10, "%u", index);

		char *cmd = pid_proc_cmdline(index);
		char *ptrcmd;
		if (cmd == NULL) {
			if (pids[index].zombie)
				ptrcmd = "(zombie)";
			else
				ptrcmd = "";
		}
		else
			ptrcmd = cmd;
		
		char *user = pid_get_user_name(pids[index].uid);
		char *ptruser;
		if (user)
			ptruser = user;
		else
			ptruser = "";
			
		char entry[60];
		snprintf(entry, 60, "%s:%s:%s", pidstr, ptruser, ptrcmd);
			
		printf("%-55.55s  ", entry);
		if (cmd)
			free(cmd);
		if (user)
			free(user);
		unsigned long long uptime = get_start_time(index);
		if (clocktick == 0)
			clocktick = sysconf(_SC_CLK_TCK);
		uptime /= sysconf(_SC_CLK_TCK);
		uptime = sysuptime - uptime;

		unsigned sec = uptime % 60;
		uptime -= sec;
		uptime /= 60;
		unsigned min = uptime % 60;
		uptime -= min;
		uptime /= 60;
		unsigned hour = uptime % 24;
		uptime -= hour;
		unsigned day = uptime / 24;
		if (day == 1)
			printf("one day ");
		else if (day)
			printf("%u days ", day);
		printf("%02u:%02u:%02u\n", hour, min, sec);
	}
}	
	




void pid_print_cpu_header(void) {
	printf("%-55.55s   %s\n", "PID:user:command", "User   System   CPU");
}


// recursivity!!!
void pid_store_cpu(unsigned index, unsigned parent, unsigned *utime, unsigned *stime) {
	if (pids[index].level == 1) {
		*utime = 0;
		*stime = 0;
	}
	
	unsigned utmp;
	unsigned stmp;
	get_cpu_time(index, &utmp, &stmp);
	*utime += utmp;
	*stime += stmp;
	
	int i;
	for (i = index + 1; i < MAX_PIDS; i++) {
		if (pids[i].parent == index)
			pid_store_cpu(i, index, utime, stime);
	}

	if (pids[index].level == 1) {
		pids[index].utime = *utime;
		pids[index].stime = *stime;
	}
}



// recursivity!!!
// itv - time interval in seconds
void pid_print_cpu(unsigned index, unsigned parent, unsigned *utime, unsigned *stime, unsigned itv) {

	if (pids[index].level == 1) {
		*utime = 0;
		*stime = 0;

		char pidstr[10];
		snprintf(pidstr, 10, "%u", index);

		char *cmd = pid_proc_cmdline(index);
		char *ptrcmd;
		if (cmd == NULL) {
			if (pids[index].zombie)
				ptrcmd = "(zombie)";
			else
				ptrcmd = "";
		}
		else
			ptrcmd = cmd;
		
		char *user = pid_get_user_name(pids[index].uid);
		char *ptruser;
		if (user)
			ptruser = user;
		else
			ptruser = "";
			
		char entry[60];
		snprintf(entry, 60, "%s:%s:%s", pidstr, ptruser, ptrcmd);
			
		printf("%-55.55s  ", entry);
		if (cmd)
			free(cmd);
		if (user)
			free(user);
	}
	
	unsigned utmp;
	unsigned stmp;
	get_cpu_time(index, &utmp, &stmp);
	*utime += utmp;
	*stime += stmp;
//printf("%u, %u\n", utmp, stmp);	
	int i;
	for (i = index + 1; i < MAX_PIDS; i++) {
		if (pids[i].parent == index)
			pid_print_cpu(i, index, utime, stime, itv);
	}

	if (pids[index].level == 1) {
		if (clocktick == 0)
			clocktick = sysconf(_SC_CLK_TCK);

		itv *= clocktick;
		double ud = (double) (*utime - pids[index].utime) / itv * 100;
		double sd = (double) (*stime - pids[index].stime) / itv * 100;
		double cd = ud + sd;
		
		if (ud < 10)
			printf(" %2.2f   ", ud);
		else
			printf("%2.2f   ", ud);
		if (sd < 10)
			printf(" %2.2f   ", sd);
		else
			printf("%2.2f   ", sd);
		if (cd < 10)
			printf(" %2.2f\n", cd);
		else
			printf("%2.2f\n", cd);
	}
}





// mon_pid: pid of sandbox to be monitored, 0 if all sandboxes are included
void pid_read(pid_t mon_pid) {
	memset(pids, 0, sizeof(pids));
	pid_t mypid = getpid();

	DIR *dir;
	if (!(dir = opendir("/proc"))) {
		// sleep 2 seconds and try again
		sleep(2);
		if (!(dir = opendir("/proc"))) {
			fprintf(stderr, "Error: cannot open /proc directory\n");
			exit(1);
		}
	}
	
	pid_t child = -1;
	struct dirent *entry;
	char *end;
	while (child < 0 && (entry = readdir(dir))) {
		pid_t pid = strtol(entry->d_name, &end, 10);
		pid %= MAX_PIDS;
		if (end == entry->d_name || *end)
			continue;
		if (pid == mypid)
			continue;
		
		// open stat file
		char *file;
		if (asprintf(&file, "/proc/%u/status", pid) == -1) {
			perror("asprintf");
			exit(1);
		}
		FILE *fp = fopen(file, "r");
		if (!fp) {
			free(file);
			continue;
		}

		// look for firejail executable name
		char buf[PIDS_BUFLEN];
		while (fgets(buf, PIDS_BUFLEN - 1, fp)) {
			if (strncmp(buf, "Name:", 5) == 0) {
				char *ptr = buf + 5;
				while (*ptr != '\0' && (*ptr == ' ' || *ptr == '\t')) {
					ptr++;
				}
				if (*ptr == '\0') {
					fprintf(stderr, "Error: cannot read /proc file\n");
					exit(1);
				}
				
				if (mon_pid == 0 && strncmp(ptr, "firejail", 8) == 0) {
					pids[pid].level = 1;
				}
				else if (mon_pid == pid && strncmp(ptr, "firejail", 8) == 0) {
					pids[pid].level = 1;
				}
				else
					pids[pid].level = -1;
			}
			if (strncmp(buf, "State:", 6) == 0) {
				if (strstr(buf, "(zombie)"))
					pids[pid].zombie = 1;
			}
			else if (strncmp(buf, "PPid:", 5) == 0) {
				char *ptr = buf + 5;
				while (*ptr != '\0' && (*ptr == ' ' || *ptr == '\t')) {
					ptr++;
				}
				if (*ptr == '\0') {
					fprintf(stderr, "Error: cannot read /proc file\n");
					exit(1);
				}
				unsigned parent = atoi(ptr);
				parent %= MAX_PIDS;
				if (pids[parent].level > 0) {
					pids[pid].level = pids[parent].level + 1;
					pids[pid].parent = parent;
				}
			}
			else if (strncmp(buf, "Uid:", 4) == 0) {
				if (pids[pid].level > 0) {
					char *ptr = buf + 5;
					while (*ptr != '\0' && (*ptr == ' ' || *ptr == '\t')) {
						ptr++;
					}
					if (*ptr == '\0') {
						fprintf(stderr, "Error: cannot read /proc file\n");
						exit(1);
					}
					pids[pid].uid = atoi(ptr);
				}
				break;
			}
		}
		fclose(fp);
		free(file);
	}
	closedir(dir);
}
