#include "firejail.h"
#include <string.h>

#define MAX_READ 4096	// maximum line length
#define MAX_ARGS 32		// maximum firejail arguments

void restricted_shell(const char *parent, const char *user) {
	assert(parent);
	assert(user);

	// open profile file:
	FILE *fp = fopen("/etc/firejail/users.conf", "r");
	if (fp == NULL)
		return;

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
		char *prog = ptr;
		char *usr = strchr(prog, ';');
		if (usr == NULL) {
			fprintf(stderr, "Error: users.conf line %d\n", lineno);
			exit(1);
		}
		*usr = '\0';
		usr++;
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
			
printf("#%s#%s#%s#\n", prog, usr, args);
		if (strcmp(parent, prog) == 0 &&
		    strcmp(user, usr) == 0) {
		    	// extract program arguments
		    	char *arg[MAX_ARGS + 1];
		    	arg[0] = "firejail";
		    	int i;
		    	ptr = args;
		    	for (i = 1; i < MAX_ARGS; i++) {
		    		arg[i] = ptr;
		    		while (*ptr != ' ' && *ptr != '\t' && *ptr != '\0')
		    			ptr++;
		    		if (*ptr != '\0') {
		    			*ptr ='\0';
		    			ptr++;
		    			continue;
		    		}
		    		// run the program
		    		arg[i + 1] = NULL;
				execvp("firejail", arg);
			}
			fprintf(stderr, "Error: too many arguments in users.conf line %d\n", lineno);
			exit(1);
		}
	}		    
}

