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

#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>

// break recursivity on fopen call
typedef FILE *(*orig_fopen_t)(const char *pathname, const char *mode);
static orig_fopen_t orig_fopen = NULL;
typedef FILE *(*orig_fopen64_t)(const char *pathname, const char *mode);
static orig_fopen64_t orig_fopen64 = NULL;



//
// pid
//
static pid_t mypid = 0;
static inline pid_t pid(void) {
	if (!mypid)
		mypid = getpid();
	return mypid;
}

//
// process name
//
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
		if (!orig_fopen)
			orig_fopen = (orig_fopen_t)dlsym(RTLD_NEXT, "fopen");
		FILE *fp  = orig_fopen(fname, "r");
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

// open
typedef int (*orig_open_t)(const char *pathname, int flags, mode_t mode);
static orig_open_t orig_open = NULL;
int open(const char *pathname, int flags, mode_t mode) {
	if (!orig_open)
		orig_open = (orig_open_t)dlsym(RTLD_NEXT, "open");
		
	int rv = orig_open(pathname, flags, mode);
	printf("%u:%s:open %s\n", pid(), name(), pathname);
	return rv;
}
typedef int (*orig_open64_t)(const char *pathname, int flags, mode_t mode);
static orig_open64_t orig_open64 = NULL;
int open64(const char *pathname, int flags, mode_t mode) {
	if (!orig_open64)
		orig_open64 = (orig_open64_t)dlsym(RTLD_NEXT, "open64");
		
	int rv = orig_open64(pathname, flags, mode);
	printf("%u:%s:open64 %s\n", pid(), name(), pathname);
	return rv;
}

// openat
typedef int (*orig_openat_t)(int dirfd, const char *pathname, int flags, mode_t mode);
static orig_openat_t orig_openat = NULL;
int openat(int dirfd, const char *pathname, int flags, mode_t mode) {
	if (!orig_openat)
		orig_openat = (orig_openat_t)dlsym(RTLD_NEXT, "openat");
		
	int rv = orig_openat(dirfd, pathname, flags, mode);
	printf("%u:%s:openat %s\n", pid(), name(), pathname);
	return rv;
}
typedef int (*orig_openat64_t)(int dirfd, const char *pathname, int flags, mode_t mode);
static orig_openat64_t orig_openat64 = NULL;
int openat64(int dirfd, const char *pathname, int flags, mode_t mode) {
	if (!orig_openat64)
		orig_openat64 = (orig_openat64_t)dlsym(RTLD_NEXT, "openat64");
		
	int rv = orig_openat64(dirfd, pathname, flags, mode);
	printf("%u:%s:openat64 %s\n", pid(), name(), pathname);
	return rv;
}


// fopen
FILE *fopen(const char *pathname, const char *mode) {
	if (!orig_fopen)
		orig_fopen = (orig_fopen_t)dlsym(RTLD_NEXT, "fopen");
		
	FILE *rv = orig_fopen(pathname, mode);
	printf("%u:%s:fopen %s\n", pid(), name(), pathname);
	return rv;
}

FILE *fopen64(const char *pathname, const char *mode) {
	if (!orig_fopen64)
		orig_fopen64 = (orig_fopen_t)dlsym(RTLD_NEXT, "fopen64");
		
	FILE *rv = orig_fopen64(pathname, mode);
	printf("%u:%s:fopen64 %s\n", pid(), name(), pathname);
	return rv;
}


// freopen
typedef FILE *(*orig_freopen_t)(const char *pathname, const char *mode, FILE *stream);
static orig_freopen_t orig_freopen = NULL;
FILE *freopen(const char *pathname, const char *mode, FILE *stream) {
	if (!orig_freopen)
		orig_freopen = (orig_freopen_t)dlsym(RTLD_NEXT, "freopen");
		
	FILE *rv = orig_freopen(pathname, mode, stream);
	printf("%u:%s:freopen %s\n", pid(), name(), pathname);
	return rv;
}

typedef FILE *(*orig_freopen64_t)(const char *pathname, const char *mode, FILE *stream);
static orig_freopen64_t orig_freopen64 = NULL;
FILE *freopen64(const char *pathname, const char *mode, FILE *stream) {
	if (!orig_freopen64)
		orig_freopen64 = (orig_freopen64_t)dlsym(RTLD_NEXT, "freopen64");
		
	FILE *rv = orig_freopen64(pathname, mode, stream);
	printf("%u:%s:freopen64 %s\n", pid(), name(), pathname);
	return rv;
}

// unlink
typedef int (*orig_unlink_t)(const char *pathname);
static orig_unlink_t orig_unlink = NULL;
int unlink(const char *pathname) {
	if (!orig_unlink)
		orig_unlink = (orig_unlink_t)dlsym(RTLD_NEXT, "unlink");
		
	int rv = orig_unlink(pathname);
	printf("%u:%s:unlink %s\n", pid(), name(), pathname);
	return rv;
}

typedef int (*orig_unlinkat_t)(int dirfd, const char *pathname, int flags);
static orig_unlinkat_t orig_unlinkat = NULL;
int unlinkat(int dirfd, const char *pathname, int flags) {
	if (!orig_unlinkat)
		orig_unlinkat = (orig_unlinkat_t)dlsym(RTLD_NEXT, "unlinkat");
		
	int rv = orig_unlinkat(dirfd, pathname, flags);
	printf("%u:%s:unlinkat %s\n", pid(), name(), pathname);
	return rv;
}

// mkdir/mkdirat/rmdir
typedef int (*orig_mkdir_t)(const char *pathname, mode_t mode);
static orig_mkdir_t orig_mkdir = NULL;
int mkdir(const char *pathname, mode_t mode) {
	if (!orig_mkdir)
		orig_mkdir = (orig_mkdir_t)dlsym(RTLD_NEXT, "mkdir");
		
	int rv = orig_mkdir(pathname, mode);
	printf("%u:%s:mkdir %s\n", pid(), name(), pathname);
	return rv;
}

typedef int (*orig_mkdirat_t)(int dirfd, const char *pathname, mode_t mode);
static orig_mkdirat_t orig_mkdirat = NULL;
int mkdirat(int dirfd, const char *pathname, mode_t mode) {
	if (!orig_mkdirat)
		orig_mkdirat = (orig_mkdirat_t)dlsym(RTLD_NEXT, "mkdirat");
		
	int rv = orig_mkdirat(dirfd, pathname, mode);
	printf("%u:%s:mkdirat %s\n", pid(), name(), pathname);
	return rv;
}

typedef int (*orig_rmdir_t)(const char *pathname);
static orig_rmdir_t orig_rmdir = NULL;
int rmdir(const char *pathname) {
	if (!orig_rmdir)
		orig_rmdir = (orig_rmdir_t)dlsym(RTLD_NEXT, "rmdir");
		
	int rv = orig_rmdir(pathname);
	printf("%u:%s:rmdir %s\n", pid(), name(), pathname);
	return rv;
}

// stat
typedef int (*orig_stat_t)(const char *pathname, struct stat *buf);
static orig_stat_t orig_stat = NULL;
int stat(const char *pathname, struct stat *buf) {
	if (!orig_stat)
		orig_stat = (orig_stat_t)dlsym(RTLD_NEXT, "stat");
			
	int rv = orig_stat(pathname, buf);
	printf("%u:%s:stat %s\n", pid(), name(), pathname);
	return rv;
}

typedef int (*orig_stat64_t)(const char *pathname, struct stat64 *buf);
static orig_stat64_t orig_stat64 = NULL;
int stat64(const char *pathname, struct stat64 *buf) {
	if (!orig_stat)
		orig_stat64 = (orig_stat64_t)dlsym(RTLD_NEXT, "stat");
			
	int rv = orig_stat64(pathname, buf);
	printf("%u:%s:stat %s\n", pid(), name(), pathname);
	return rv;
}


// access
typedef int (*orig_access_t)(const char *pathname, int mode);
static orig_access_t orig_access = NULL;
int access(const char *pathname, int mode) {
	if (!orig_access)
		orig_access = (orig_access_t)dlsym(RTLD_NEXT, "access");
			
	int rv = orig_access(pathname, mode);
	printf("%u:%s:access %s\n", pid(), name(), pathname);
	return rv;
}


// connect
typedef int (*orig_connect_t)(int sockfd, const struct sockaddr *addr, socklen_t addrlen);
static orig_connect_t orig_connect = NULL;
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
			printf("%u:%s:connect &%s\n", pid(), name(), a->sun_path + 1);
	}
	else {
		printf("%u:%s:connect family %d\n", pid(), name(), addr->sa_family);
	}

	return rv;
}
