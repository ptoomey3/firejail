#include "firejail.h"

int sargc;
char *sargv[SARG_MAX];


void split_command(char *cmd) {
	int i;
	char *ptr = cmd;
	
	sargc = 0;
	memset(sargv, 0, sizeof(sargv));
	
	if (!ptr || *ptr == '\0')
		return;
	
	for (i = 0; i < SARG_MAX; i++) {
		char *start;
		
		// skip space
		while (*ptr == ' ' || *ptr == '\t')
			ptr++;
		if (*ptr == '\n' || *ptr == '\r' || *ptr == '\0')
			break;
		start = ptr;
		
		// advance
		while (*ptr != '\0' && *ptr != ' ' && *ptr != '\t' && *ptr != '\n' && *ptr != '\r')
			ptr++;
		if (*ptr == '\0') {
			sargv[sargc++] = start;
			break;
		}
		*ptr++ = '\0';
		sargv[sargc++] = start;
	}
}
