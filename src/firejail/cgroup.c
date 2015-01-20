#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <string.h>
#include "firejail.h"

#define MAXBUF 4096

void save_cgroup(void) {
	if (cfg.cgroup == NULL)
		return;
	
	char *fname;
	if (asprintf(&fname, "%s/cgroup", MNT_DIR) == -1)
		errExit(fname);
	
	FILE *fp = fopen(fname, "w");
	if (fp) {
		fprintf(fp, "%s", cfg.cgroup);
		fflush(0);
		fclose(fp);
		if (chown(fname, 0, 0) < 0)
			errExit("chown");
	}
	else {
		fprintf(stderr, "Error: cannot save cgroup\n");
		free(fname);
		exit(1);
	}
	
	free(fname);
}

void load_cgroup(const char *fname) {
	if (!fname)
		return;

	FILE *fp = fopen(fname, "r");
	if (fp) {
		char buf[MAXBUF];
		if (fgets(buf, MAXBUF, fp)) {
			cfg.cgroup = strdup(buf);
			if (!cfg.cgroup)
				errExit("strdup");
		}
		else
			goto errout;
		
		fclose(fp);
		return;
	}
errout:
	fprintf(stderr, "Warrning: cannot load control group\n");
}


void set_cgroup(const char *path) {
	// path starts with /sys/fs/cgroup
	if (strncmp(path, "/sys/fs/cgroup", 14) != 0)
		goto errout;
	
	// path ends in tasks
	char *ptr = strstr(path, "tasks");
	if (!ptr)
		goto errout;
	if (*(ptr + 5) != '\0')
		goto errout;
	
	// no .. traversal
	ptr = strstr(path, "..");
	if (ptr)
		goto errout;
	
	// tasks file exists
	struct stat s;
	if (stat(path, &s) == -1)
		goto errout;
	
	// task file belongs to root
	if (s.st_uid || s.st_gid)
		goto errout;
		
	// add the task to cgroup
	FILE *fp = fopen(path,	"a");
	if (!fp)
		goto errout;
	pid_t pid = getpid();
	int rv = fprintf(fp, "%d\n", pid);
	fclose(fp);
	return;

errout:		
	fprintf(stderr, "Error: invalid cgroup\n");
	exit(1);
}
