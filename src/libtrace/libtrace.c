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
#include <sys/un.h>

//
// file
//
static pid_t mypid = 0;
static inline pid_t pid(void) {
	if (!mypid)
		mypid = getpid();
	return mypid;
}

#define MAXNAME 16
static char myname[MAXNAME];
static int nameinit = 0;
static char *name(void) {
	if (!nameinit) {
		// initialize the name of the process based on /proc/PID/comm
		memset(myname, 0, MAXNAME);
		
		pid_t p = pid();
		char *fname;
		if (asprintf(&fname, "/proc/%u/comm", p) == -1)
			return "unknown";

		// read file
		FILE *fp = fopen(fname, "r");
		if (!fp)
			return "unknown";
		if (fgets(myname, MAXNAME, fp) == NULL) {
			fclose(fp);
			free(fname);
			return "unknown";
		}
		
		// clean '\n'
		char *ptr = strchr(myname, '\n');
		if (ptr)
			*ptr = '\0';
			
		fclose(fp);
		free(fname);
		nameinit = 1;
	}
	
	return myname;
}

//
// syscalls
//

typedef int (*orig_open_t)(const char *pathname, int flags);
orig_open_t orig_open = NULL;
int open(const char *pathname, int flags) {
	if (!orig_open)
		orig_open = (orig_open_t)dlsym(RTLD_NEXT, "open");
		
	int rv = orig_open(pathname, flags);
	printf("%u:%s:open %s\n", pid(), name(), pathname);
	return rv;
}

typedef int (*orig_access_t)(const char *pathname, int mode);
orig_access_t orig_access = NULL;
int access(const char *pathname, int mode) {
	if (!orig_access)
		orig_access = (orig_access_t)dlsym(RTLD_NEXT, "access");
			
	int rv = orig_access(pathname, mode);
	printf("%u:%s:access %s\n", pid(), name(), pathname);
	return rv;
}

typedef int (*orig_connect_t)(int sockfd, const struct sockaddr *addr, socklen_t addrlen);
orig_connect_t orig_connect = NULL;
int connect(int sockfd, const struct sockaddr *addr, socklen_t addrlen) {
	if (!orig_connect)
		orig_connect = (orig_connect_t)dlsym(RTLD_NEXT, "connect");
			
	int rv = orig_connect(sockfd, addr, addrlen);
	if (addr->sa_family == AF_INET) {
		struct sockaddr_in *a = (struct sockaddr_in *) addr;
		printf("%u:%s:connect %s:%u\n", pid(), name(), inet_ntoa(a->sin_addr), ntohs(a->sin_port));
	}
	else if (addr->sa_family == AF_INET6) {
		struct sockaddr_in6 *a = (struct sockaddr_in6 *) addr;
		char str[INET6_ADDRSTRLEN];
		inet_ntop(AF_INET6, &(a->sin6_addr), str, INET6_ADDRSTRLEN);
		printf("%u:%s:connect %s\n", pid(), name(), str);
	}
	else if (addr->sa_family == AF_UNIX) {
		struct sockaddr_un *a = (struct sockaddr_un *) addr;
		if (a->sun_path[0])
			printf("%u:%s:connect %s\n", pid(), name(), a->sun_path);
		else
			printf("%u:%sLconnect &%s\n", pid(), name(), a->sun_path + 1);
	}
	else {
		printf("%u:%s:connect family %d\n", pid(), name(), addr->sa_family);
	}

	return rv;
}
