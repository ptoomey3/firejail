#include "firejail.h"
#include <string.h>

#define MAX_READ 4096	// maximum line length

int restricted_shell(const char *user) {
	assert(user);

	// open profile file:
	FILE *fp = fopen("/etc/firejail/sshd.users", "r");
	if (fp == NULL)
		return 0;

	int lineno = 0;
	char buf[MAX_READ];
	while (fgets(buf, MAX_READ, fp)) {
		lineno++;
		
		// remove empty spaces at the beginning of the line
		char *ptr = buf;
		while (*ptr == ' ' || *ptr == '\t') { 
			ptr++;
		}
		if (*ptr == '\n' || *ptr == '#')
			continue;
		
		// parse line	
		char *usr = ptr;
		char *args = strchr(usr, ';');
		if (args == NULL) {
			fprintf(stderr, "Error: users.conf line %d\n", lineno);
			exit(1);
		}
		*args = '\0';
		args++;
		ptr = strchr(args, '\n');
		if (ptr)
			*ptr = '\0';
			
		if (strcmp(user, usr) == 0) {
		    	// extract program arguments
		    	char *arg[MAX_ARGS + 1];
		    	fullargv[0] = "firejail";
		    	int i;
		    	ptr = args;
		    	for (i = 1; i < MAX_ARGS; i++) {
		    		fullargv[i] = ptr;
		    		while (*ptr != ' ' && *ptr != '\t' && *ptr != '\0')
		    			ptr++;
		    		if (*ptr != '\0') {
		    			*ptr ='\0';
		    			ptr++;
		    			while (*ptr == ' ' || *ptr == '\t')
		    				ptr++;
		    			if (*ptr != '\0')
			    			continue;
		    		}
		    		return i + 1;
			}
			fprintf(stderr, "Error: too many arguments in users.conf line %d\n", lineno);
			exit(1);
		}
	}		 
	return 0;   
}

