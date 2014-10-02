// based on an idea from http://rafalcieslak.wordpress.com/2013/04/02/dynamic-linker-tricks-using-ld_preload-to-cheat-inject-features-and-investigate-programs/
// gcc -shared -fPIC -ldl traceopen.c -o traceopen.so
#define _GNU_SOURCE
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <dlfcn.h>
#include <sys/types.h>
#include <unistd.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>

//
// buffer
//
#define MAXBUF (16 * 1024)
int bufsize = 0;
char buffer[MAXBUF];

void bufinit(void) {
	memset(buffer, 0, MAXBUF);
}

void bufdump(void) {
	// dump the contents of the buffer
	pid_t pid = getpid();
	char *fname;
	if (asprintf(&fname, "/tmp/firejail/%u", pid) == -1) {
		fprintf(stderr, "Error: cannot allocate memory\n");
		exit(1);
	}
	
	FILE *fp = fopen(fname, "a");
	if (!fp) {
		fprintf(stderr, "Error: cannot open %s file\n", fname);
		free(fname);
		exit(1);
	}
	printf("Updating trace buffer\n");
	fprintf(fp, "%s", buffer);
	fclose(fp);
}
char *bufrequest(int size) {
	if ((size + bufsize + 1024) > MAXBUF) {
		bufdump();
		bufsize = 0;
	}

	return buffer + bufsize;
}

void bufset(int size) {
	bufsize += size;
}

//
// exit
//
static int exit_flag = 0;
static void myexit(void) {
	bufdump();
}
static void init_exit(void) {
	atexit(myexit);
	bufinit();
	exit_flag = 1;
}

//
// syscalls
//
typedef int (*orig_open_t)(const char *pathname, int flags);
orig_open_t orig_open = NULL;
int open(const char *pathname, int flags) {
	if (!exit_flag)
		init_exit();
	if (!orig_open)
		orig_open = (orig_open_t)dlsym(RTLD_NEXT, "open");
		
	int rv = orig_open(pathname, flags);

	char *ptr = bufrequest(strlen(pathname));
	if (ptr) {
		int len = sprintf(ptr, "open %s return %d\n", pathname, rv);
		bufset(len);
	}

	return rv;
}

typedef int (*orig_access_t)(const char *pathname, int mode);
orig_access_t orig_access = NULL;
int access(const char *pathname, int mode) {
	if (!exit_flag)
		init_exit();
	if (!orig_access)
		orig_access = (orig_access_t)dlsym(RTLD_NEXT, "access");
			
	int rv = orig_access(pathname, mode);

	char *ptr = bufrequest(strlen(pathname));
	if (ptr) {
		int len = sprintf(ptr, "access %s return %d\n", pathname, rv);
		bufset(len);
	}

	return rv;
}

typedef int (*orig_connect_t)(int sockfd, const struct sockaddr *addr, socklen_t addrlen);
orig_connect_t orig_connect = NULL;
int connect(int sockfd, const struct sockaddr *addr, socklen_t addrlen) {
	if (!exit_flag)
		init_exit();
	if (!orig_connect)
		orig_connect = (orig_connect_t)dlsym(RTLD_NEXT, "connect");
			
	int rv = orig_connect(sockfd, addr, addrlen);

	char *ptr = bufrequest(30);
	if (ptr) {
		if (addr->sa_family == AF_INET) {
			struct sockaddr_in *a = (struct sockaddr_in *) addr;
			int len = sprintf(ptr, "connect %s return %d\n", inet_ntoa(a->sin_addr), rv);
			bufset(len);
		}
		else {
			int len = sprintf(ptr, "connect family %d return %d\n", addr->sa_family, rv);
			bufset(len);
		}
	}

	return rv;
}
