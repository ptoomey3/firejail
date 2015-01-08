#define _GNU_SOURCE
#include <string.h>
#include <sched.h>
#include "firejail.h"

static void set_cpu(const char *str) {
	if (strlen(str) == 0)
		return;
	
	int val = atoi(str);
	if (val < 0 || val >= 32) {
		fprintf(stderr, "Error: invalid cpu number. Accepted values are between 0 and 31.\n");
		exit(1);
	}
	
	uint32_t mask = 1;
	int i;
	for (i = 0; i < val; i++, mask <<= 1);
	cfg.cpus |= mask;
}

void read_cpu_list(const char *str) {
	char *tmp = strdup(str);
	if (tmp == NULL)
		errExit("strdup");
	
	char *ptr = tmp;
	while (*ptr != '\0') {
		if (*ptr == ',' || isdigit(*ptr))
			;
		else {
			fprintf(stderr, "Error: invalid cpu list\n");
			exit(1);
		}
		ptr++;
	}
	
	char *start = tmp;
	ptr = tmp;
	while (*ptr != '\0') {
		if (*ptr == ',') {
			*ptr = '\0';
			set_cpu(start);
			start = ptr + 1;
		}
		ptr++;
	}
	set_cpu(start);
	free(tmp);
}

void set_cpu_affinity(void) {
	// set cpu affinity
	cpu_set_t mask;
	CPU_ZERO(&mask);
	
	int i;
	uint32_t m = 1;
	for (i = 0; i < 32; i++, m <<= 1) {
		if (cfg.cpus & m)
			CPU_SET(i, &mask);
	}
	
        	if (sched_setaffinity(0, sizeof(mask), &mask) == -1) {
        		fprintf(stderr, "Warning: cannot set cpu affinity\n");
        		fprintf(stderr, "  ");
        		perror("sched_setaffinity");
        	}
        	
        	// verify cpu affinity
	cpu_set_t mask2;
	CPU_ZERO(&mask2);
        	if (sched_getaffinity(0, sizeof(mask2), &mask2) == -1) {
        		fprintf(stderr, "Warning: cannot verify cpu affinity\n");
        		fprintf(stderr, "   ");
        		perror("sched_getaffinity");
        	}
        	else {
	        	if (CPU_EQUAL(&mask, &mask2))
        			printf("CPU affinity set\n");
		else
        			printf("CPU affinity not set\n");
        	}
}