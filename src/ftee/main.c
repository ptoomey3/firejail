#include "ftee.h"
#include <errno.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#define MAXBUF 512

static unsigned char buf[MAXBUF];

static FILE *out_fp = NULL;
static int out_cnt = 0;
static int out_max = 500 * 1024;

static void log_close(void) {
	if (out_fp) {
		fclose(out_fp);
		out_fp = NULL;
	}
}

static void log_rotate(const char *fname) {
	struct stat s;
	int index = strlen(fname);
	char *name1 = malloc(index + 2 + 1);
	char *name2 = malloc(index + 2 + 1);
	if (!name1 || !name2)
		errExit("malloc");
	strcpy(name1, fname);
	strcpy(name2, fname);
	fflush(0);
	
	// delete filename.5
	sprintf(name1 + index, ".5");
	if (stat(name1, &s) == 0) {
		int rv = unlink(name1);
		if (rv == -1)
			perror("unlink");
	}
	
	// move files 1 to 4 down one position
	sprintf(name2 + index, ".4");
	if (stat(name2, &s) == 0) {
		int rv = rename(name2, name1);
		if (rv == -1)
			perror("rename");
	}

	sprintf(name1 + index, ".3");
	if (stat(name1, &s) == 0) {
		int rv = rename(name1, name2);
		if (rv == -1)
			perror("rename");
	}

	sprintf(name2 + index, ".2");
	if (stat(name2, &s) == 0) {
		int rv = rename(name2, name1);
		if (rv == -1)
			perror("rename");
	}

	sprintf(name1 + index, ".1");
	if (stat(name1, &s) == 0) {
		int rv = rename(name1, name2);
		if (rv == -1)
			perror("rename");
	}

	// move the first file
	if (out_fp)
		fclose(out_fp);

	out_fp = NULL;
	if (stat(fname, &s) == 0) {
		int rv = rename(fname, name1);
		if (rv == -1)
			perror("rename");
	}
}

static void log_write(const unsigned char *str, int len, const char *fname) {
	assert(fname);
	
	if (out_fp == NULL) {
		out_fp = fopen(fname, "w");
		if (!out_fp) {
			fprintf(stderr, "Error: cannot open log file %s\n", fname);
			exit(1);
		}
		out_cnt = 0;
	}
	
	// rotate files
	out_cnt += len;
	if (out_cnt >= out_max) {
		log_rotate(fname);

		// reopen the first file
		out_fp = fopen(fname, "w");
		if (!out_fp) {
			fprintf(stderr, "Error: cannot open log file %s\n", fname);
			exit(1);
		}
		out_cnt = len;
	}				
		
	fwrite(str, len, 1, out_fp);	
	fflush(0);
}

static void usage(void) {
	printf("Usage: ftee filename\n");
}

int main(int argc, char **argv) {
	if (argc < 2) {
		fprintf(stderr, "Error: please provide a filename to store the program output\n");
		usage();
		exit(1);
	}
	char *fname = argv[1];

	// preserve the last log file
	log_rotate(fname);

	setvbuf (stdout, NULL, _IONBF, 0);
	
	while(1) {
		int n = read(0, buf, sizeof(buf));
		if (n < 0 && errno == EINTR)
			continue;
		if (n <= 0)
			break;
		
		fwrite(buf, n, 1, stdout);
		log_write(buf, n, fname);
	}
	
	log_close();
	return 0;
}
