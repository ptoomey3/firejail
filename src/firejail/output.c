#include "firejail.h"
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>

void check_output(int argc, char **argv) {
	int i;
	char *outfile = NULL;
	
	int found = 0;
	for (i = 1; i < argc; i++) {
		if (strncmp(argv[i], "--output=", 9) == 0) {
			found = 1;
			outfile = argv[i] + 9;
		
			struct stat s;
			int found = 0;
			if (stat(outfile, &s) == 0)
				found = 1;
		
			// try to open the file for writing
			FILE *fp = fopen(outfile, "a");
			if (!fp) {
				fprintf(stderr, "Error: cannot open output file %s\n", outfile);
				exit(1);
			}
			fclose(fp);
			if (found == 0)
				unlink(outfile);
			break;
		}
	}
	
	if (!found)
		return;
	
	// build the new command line
	int len = 0;
	for (i = 0; i < argc; i++) {
		len += strlen(argv[i]) + 1; // + ' '
	}
	len += 10 + strlen(outfile); // tee command
	
	char *cmd = malloc(len + 1); // + '\0'
	if (!cmd)
		errExit("malloc");
	
	char *ptr = cmd;
	for (i = 0; i < argc; i++) {
		if (strncmp(argv[i], "--output=", 9) == 0)
			continue;
		ptr += sprintf(ptr, "%s ", argv[i]);
	}
	sprintf(ptr, "| ftee %s", outfile);

	// run command
	char *a[4];
	a[0] = "/bin/bash";
	a[1] = "-c";
	a[2] = cmd;
	a[3] = NULL;

	execvp(a[0], a); 

	perror("execvp");
	exit(1);
}
