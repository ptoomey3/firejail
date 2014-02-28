#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <sys/syscall.h>
#include <errno.h>
#include <unistd.h>
#include "firejail.h"

void join_namespace(pid_t pid, char *type) {
	char *path;
	if (asprintf(&path, "/proc/%u/ns/%s", pid, type) == -1)
		errExit("asprintf");
	
	int fd = open(path, O_RDONLY);
	if (fd < 0) {
		fprintf(stderr, "Error: cannot open /proc/%u/ns/%s.\n", pid, type);
		exit(1);
	}

	if (syscall(__NR_setns, fd, 0) < 0) {
		fprintf(stderr, "Error: cannot join namespace.\n");
		exit(1);
	}

	close(fd);
	free(path);
}


void join(pid_t pid) {
	// check privileges for non-root users
	uid_t uid = getuid();
	if (uid != 0) {
		struct stat s;
		char *dir;
		if (asprintf(&dir, "/proc/%u/ns", pid) < 0)
			errExit("asprintf");
		if (stat(dir, &s) < 0)
			errExit("stat");
		if (s.st_uid != uid) {
			fprintf(stderr, "Error: permission is denied to join a sandbox created by a different user.\n");
			exit(1);
		}
	}

	// join namespaces
	join_namespace(pid, "ipc");
	join_namespace(pid, "net");
	join_namespace(pid, "pid");
	join_namespace(pid, "uts");
	join_namespace(pid, "mnt");
	
	if (chdir("/") < 0)
		errExit("chdir");
	// drop privileges
	if (setuid(getuid()) < 0)
		errExit("setuid/getuid");
	// fix qt 4.8
	if (setenv("QT_X11_NO_MITSHM", "1", 1) < 0)
		errExit("setenv");
	// set prompt for non-debian/ubuntu/mint systems
	if (setenv("PS1", "\\e[01;31m\\u@\\h::\\w \\$ \\e[00m", 1) < 0)
		errExit("setenv");
	if (setenv("color_prompt", "yes", 1) < 0)
		errExit("setenv");

	// replace the process with a regular bash session
	execlp("/bin/bash", "/bin/bash", NULL);
}



