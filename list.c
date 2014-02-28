#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <dirent.h>
#include <string.h>
#include "firejail.h"
#define BUFLEN 1023

typedef struct mylist_t {
	struct mylist_t *next;
	pid_t pid;
} MyList;
MyList *mylist = NULL;

static void add(pid_t pid) {
	MyList *e = malloc(sizeof(MyList));
	if (!e)
		errExit("malloc");
	e->pid = pid;
	e->next = mylist;
	mylist = e;
}

static int is_listed(pid_t pid) {
	MyList *ptr = mylist;
	while (ptr) {
		if (ptr->pid == pid)
			return 1;
		ptr = ptr->next;
	}
	return 0;
}

static void clean(void) {
	MyList *ptr = mylist;
	while (ptr) {
		MyList *next = ptr->next;
		free(ptr);
		ptr = next;
	}
}

static void print_args(pid_t pid) {
	char *name;
	if (asprintf(&name, "/proc/%u/cmdline", pid) == -1)
		errExit("asprintf");
	FILE *fp = fopen(name, "r");
	if (!fp) {
		fprintf(stderr, "Error: cannot open /proc directory\n");
		exit(1);
	}

	char buf[BUFLEN + 1];
	memset(buf, 0, sizeof(buf));
	int cnt = fread(buf, 1, BUFLEN, fp);
	if (cnt > 0) {
		int len = strlen(buf) + 1;	// first string is the name of the program
		char *ptr = buf + len;
		cnt -= len;
		while (cnt > 0) {
			printf("%s ", ptr);
			len = strlen(ptr) + 1;
			ptr += len;
			cnt -= len;
		}
	}
	printf("\n");
}

void list(void) {
	pid_t mypid = getpid();

	DIR *dir;
	if (!(dir = opendir("/proc"))) {
		fprintf(stderr, "Error: cannot open /proc directory\n");
		exit(1);
	}
	
	pid_t child = -1;
	struct dirent *entry;
	char *end;
	while (child < 0 && (entry = readdir(dir))) {
		pid_t pid = strtol(entry->d_name, &end, 10);
		if (end == entry->d_name || *end)
			continue;

		if (pid == mypid)
			continue;
		
		// open stat file
		char *file;
		if (asprintf(&file, "/proc/%u/status", pid) == -1)
			errExit("asprintf");
		FILE *fp = fopen(file, "r");
		if (!fp) {
			free(file);
			continue;
		}
		
		// look for firejail executable name
		char buf[BUFLEN + 1];
		char *name = NULL;
		while (fgets(buf, BUFLEN, fp)) {
			if (strncmp(buf, "Name:", 5) == 0) {
				char *ptr = buf + 5;
				while (*ptr != '\0' && (*ptr == ' ' || *ptr == '\t')) {
					ptr++;
				}
				if (*ptr == '\0') {
					fprintf(stderr, "Error: cannot read /proc file\n");
					exit(1);
				}

				if (strncmp(ptr, "firejail", 8) == 0) {
					printf("%u - firejail ", pid);
					add(pid);	// place the pid in the container list
					print_args(pid);
					break;
				}
				if (mylist == NULL)
					break;
				else {
					if (asprintf(&name, "%s", ptr) == -1)
						errExit("asprintf");
				}
			}
			else if (strncmp(buf, "PPid:", 5) == 0) {
				pid_t parent;
				if (sscanf(buf, "PPid:\t%u", &parent) == 1) {
					if (is_listed(parent)) {
						printf("\t%u - %s", pid, name);
//						print_args(pid);
					}
				}
				break;
			}
		}
		if (name)
			free(name);
		fclose(fp);
		free(file);
	}
	clean();
	closedir(dir);
}
