#include <string.h>
#include <sys/types.h>
#include <dirent.h>
#include "firejail.h"

#define MAX_READ 1024				  // line buffer for profile files

void get_profile(const char *name, const char *dir) {
	assert(name);
	assert(dir);
	
	DIR *dp;
	char *pname;
	if (asprintf(&pname, "%s.profile", name) == -1)
		errExit("asprintf");

	dp = opendir (dir);
	if (dp != NULL) {
		struct dirent *ep;
		while (ep = readdir (dp)) {
			if (strcmp(ep->d_name, pname) == 0) {
				if (arg_debug)
					printf("Found %s profile in %s directory\n", name, dir);
				char *etcpname;
				if (asprintf(&etcpname, "%s/%s", dir, pname) == -1)
					errExit("asprintf");
				read_profile(etcpname);
				free(etcpname);
				break;
			}
		}
		(void) closedir (dp);
	}

	free(pname);
}


//***************************************************
// run-time profiles
//***************************************************
static void check_file_name(char *ptr, int lineno) {
	int len = strlen(ptr);
	if (strcspn(ptr, "\\&!?\"'<>%^(){}[];, ") != len) {
		fprintf(stderr, "Error: line %d in the custom profile is invalid\n", lineno);
		exit(1);
	}
}


static void check_line(char *ptr, int lineno) {
	if (strcmp(ptr, "${NEWTMP}") == 0)
		return;
	else if (strncmp(ptr, "blacklist", 9) == 0)
		ptr += 9;
	else if (strncmp(ptr, "preserve", 8) == 0)
		ptr += 8;
	else {
		fprintf(stderr, "Error: line %d in the custom profile is invalid\n", lineno);
		exit(1);
	}

	if (*ptr != ' ' && *ptr != '\t') {
		fprintf(stderr, "Error: line %d in the custom profile is invalid\n", lineno);
		exit(1);
	}
	while (*ptr == ' ' || *ptr == '\t')
		ptr++;
	
	// some characters just don't appear in filenames
	if (strncmp(ptr, "${HOME}", 7) == 0)
		check_file_name(ptr + 7, lineno);
	else if (strncmp(ptr, "${PATH}", 7) == 0)
		check_file_name(ptr + 7, lineno);
	else
		check_file_name(ptr, lineno);
}


void read_profile(const char *fname) {
	if (strlen(fname) == 0) {
		fprintf(stderr, "Error: invalid profile file\n");
		exit(1);
	}

	// open profile file:
	FILE *fp = fopen(fname, "r");
	if (fp == NULL) {
		fprintf(stderr, "Error: cannot open profile file\n");
		exit(1);
	}

	// linked list of lines
	struct mylist
	{
		char *line;
		struct mylist *next;
	}
	m = {
		NULL, NULL
	};
	struct mylist *mptr = &m;
	int mylist_cnt = 1;

	// read the file line by line
	char buf[MAX_READ + 1];
	int lineno = 0;
	while (fgets(buf, MAX_READ, fp)) {
		++lineno;
		// remove empty space
		char *ptr = buf;
		while (*ptr == ' ' || *ptr == '\t')
			ptr++;

		// empty line
		if (*ptr == '\n' || *ptr == '\r')
			continue;

		// comments
		if (*ptr == '#')
			continue;

		// strip end of line
		char *end = ptr;
		while (*end != '\0' && *end != '\n' && *end != '\r')
			end++;
		if (*end == '\0')
			continue;
		*end = '\0';
		char *ptr1 = end;
		while (*ptr1 == ' ' || *ptr1 == '\t')
			ptr1--;
		*ptr1 = '\0';

		// verify syntax, exit in case of error
		check_line(ptr, lineno);

		// populate the linked list
		assert(mptr);
		int len =strlen(ptr);
		mptr->line = malloc(len + 1);
		if (mptr->line == NULL)
			errExit("malloc");
		strcpy(mptr->line, ptr);
		mptr->next = malloc(sizeof(struct mylist));
		if (mptr->next == NULL)
			errExit("malloc");
		mptr = mptr->next;
		mptr->line = NULL;
		mptr->next = NULL;
		mylist_cnt++;
	}

	// build blacklist array
	custom_profile  = malloc(sizeof(struct mylist *) * mylist_cnt);
	if (custom_profile == NULL) {
		fprintf(stderr, "Error: cannot allocate memory");
		exit(1);
	}
	mptr = &m;
	lineno = 0;
	while (mptr->next != NULL) {
		assert(mptr->line);
		custom_profile[lineno] = mptr->line;
		mptr = mptr->next;
		lineno++;
	}
	custom_profile[lineno] = NULL;
}
