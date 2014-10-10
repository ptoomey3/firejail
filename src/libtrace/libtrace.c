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
#include <sys/stat.h>

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
// network
//
typedef struct {
	int val;
	char *name;
} XTable;

static XTable socket_type[] = {
#ifdef SOCK_STREAM
	{ SOCK_STREAM, "SOCK_STREAM" },
#endif	
#ifdef SOCK_DGRAM 
	{ SOCK_DGRAM, "SOCK_DGRAM" },
#endif	
#ifdef SOCK_RAW 
	{ SOCK_RAW, "SOCK_RAW" },
#endif	
#ifdef SOCK_RDM 
	{ SOCK_RDM, "SOCK_RDM" },
#endif	
#ifdef SOCK_SEQPACKET 
	{ SOCK_SEQPACKET, "SOCK_SEQPACKET" },
#endif	
#ifdef SOCK_DCCP 
	{ SOCK_DCCP, "SOCK_DCCP" },
#endif
	{ 0, NULL} // NULL terminated
};

static XTable socket_domain[] = {
#ifdef AF_INET
	{ AF_INET, "AF_INET" },
#endif
#ifdef AF_INET6
	{ AF_INET6, "AF_INET6" },
#endif
#ifdef AF_LOCAL
	{ AF_LOCAL, "AF_LOCAL" },
#endif
#ifdef AF_PACKET
	{ AF_PACKET, "AF_PACKET" },
#endif
#ifdef AF_IPX
	{ AF_IPX, "AF_IPX" },
#endif
#ifdef AF_NETLINK
	{ AF_NETLINK, "AF_NETLINK" },
#endif
#ifdef AF_X25
	{ AF_X25, "AF_X25" },
#endif
#ifdef AF_AX25
	{ AF_AX25, "AF_AX25" },
#endif
#ifdef AF_ATMPVC
	{ AF_ATMPVC, "AF_ATMPVC" },
#endif
#ifdef AF_APPLETALK
	{ AF_APPLETALK, "AF_APPLETALK" },
#endif
	{ 0, NULL} // NULL terminated
};

static XTable socket_protocol[] = {
#ifdef IPPROTO_IP
	{ IPPROTO_IP, "IPPROTO_IP" },
#endif
#ifdef IPPROTO_ICMP
	{ IPPROTO_ICMP, "IPPROTO_ICMP" },
#endif
#ifdef IPPROTO_IGMP
	{ IPPROTO_IGMP, "IPPROTO_IGMP" },
#endif
#ifdef IPPROTO_IPIP
	{ IPPROTO_IPIP, "IPPROTO_IPIP" },
#endif
#ifdef IPPROTO_TCP
	{ IPPROTO_TCP, "IPPROTO_TCP" },
#endif
#ifdef IPPROTO_EGP
	{ IPPROTO_EGP, "IPPROTO_EGP" },
#endif
#ifdef IPPROTO_PUP
	{ IPPROTO_PUP, "IPPROTO_PUP" },
#endif
#ifdef IPPROTO_UDP
	{ IPPROTO_UDP, "IPPROTO_UDP" },
#endif
#ifdef IPPROTO_IDP
	{ IPPROTO_IDP, "IPPROTO_IDP" },
#endif
#ifdef IPPROTO_DCCP
	{ IPPROTO_DCCP, "IPPROTO_DCCP" },
#endif
#ifdef IPPROTO_RSVP
	{ IPPROTO_RSVP, "IPPROTO_RSVP" },
#endif
#ifdef IPPROTO_GRE
	{ IPPROTO_GRE, "IPPROTO_GRE" },
#endif
#ifdef IPPROTO_IPV6
	{ IPPROTO_IPV6, "IPPROTO_IPV6" },
#endif
#ifdef IPPROTO_ESP
	{ IPPROTO_ESP, "IPPROTO_ESP" },
#endif
#ifdef IPPROTO_AH
	{ IPPROTO_AH, "IPPROTO_AH" },
#endif
#ifdef IPPROTO_BEETPH	
	{ IPPROTO_BEETPH, "IPPROTO_BEETPH" },
#endif
#ifdef IPPROTO_PIM
	{ IPPROTO_PIM, "IPPROTO_PIM" },
#endif
#ifdef IPPROTO_COMP
	{ IPPROTO_COMP, "IPPROTO_COMP" },
#endif
#ifdef IPPROTO_SCTP
	{ IPPROTO_SCTP, "IPPROTO_SCTP" },
#endif
#ifdef IPPROTO_UDPLITE
	{ IPPROTO_UDPLITE, "IPPROTO_UDPLITE" },
#endif
#ifdef IPPROTO_RAW
	{ IPPROTO_RAW, "IPPROTO_RAW" },
#endif
	{ 0, NULL} // NULL terminated
};

static char *translate(XTable *table, int val) {
	while (table->name != NULL) {
		if (val == table->val)
			return table->name;
		table++;
	}
	
	return NULL;
}

static void print_sockaddr(const char *call, const struct sockaddr *addr) {
	if (addr->sa_family == AF_INET) {
		struct sockaddr_in *a = (struct sockaddr_in *) addr;
		printf("%u:%s:%s %s:%u\n", pid(), name(), call, inet_ntoa(a->sin_addr), ntohs(a->sin_port));
	}
	else if (addr->sa_family == AF_INET6) {
		struct sockaddr_in6 *a = (struct sockaddr_in6 *) addr;
		char str[INET6_ADDRSTRLEN];
		inet_ntop(AF_INET6, &(a->sin6_addr), str, INET6_ADDRSTRLEN);
		printf("%u:%s:%s %s\n", pid(), name(), call, str);
	}
	else if (addr->sa_family == AF_UNIX) {
		struct sockaddr_un *a = (struct sockaddr_un *) addr;
		if (a->sun_path[0])
			printf("%u:%s:%s %s\n", pid(), name(), call, a->sun_path);
		else
			printf("%u:%s:%s &%s\n", pid(), name(), call, a->sun_path + 1);
	}
	else {
		printf("%u:%s:%s family %d\n", pid(), name(), call, addr->sa_family);
	}
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
	print_sockaddr("connect", addr);

	return rv;
}

// socket
typedef int (*orig_socket_t)(int domain, int type, int protocol);
static orig_socket_t orig_socket = NULL;
static char buf[1024];
int socket(int domain, int type, int protocol) {
	if (!orig_socket)
		orig_socket = (orig_socket_t)dlsym(RTLD_NEXT, "socket");
			
	int rv = orig_socket(domain, type, protocol);
	char *ptr = buf;
	ptr += sprintf(ptr, "%u:%s:socket ", pid(), name());
	char *str = translate(socket_domain, domain);
	if (str == NULL)
		ptr += sprintf(ptr, "%d ", domain);
	else
		ptr += sprintf(ptr, "%s ", str);

	int t = type;	// glibc uses higher bits for various other purposes
#ifdef SOCK_CLOEXEC
	t &= ~SOCK_CLOEXEC;
#endif
#ifdef SOCK_NONBLOCK
	t &= ~SOCK_NONBLOCK;
#endif
	str = translate(socket_type, t);
	if (str == NULL)
		ptr += sprintf(ptr, "%d ", type);
	else
		ptr += sprintf(ptr, "%s ", str);

	str = translate(socket_protocol, protocol);
	if (str == NULL)
		ptr += sprintf(ptr, "%d", protocol);
	else
		ptr += sprintf(ptr, "%s", str);

	printf("%s\n", buf);
	return rv;
}

// bind
typedef int (*orig_bind_t)(int sockfd, const struct sockaddr *addr, socklen_t addrlen);
static orig_bind_t orig_bind = NULL;
int bind(int sockfd, const struct sockaddr *addr, socklen_t addrlen) {
	if (!orig_bind)
		orig_bind = (orig_bind_t)dlsym(RTLD_NEXT, "bind");
			
	int rv = orig_bind(sockfd, addr, addrlen);
	print_sockaddr("bind", addr);

	return rv;
}

#if 0 //todo: fix compilation problems
typedef int (*orig_accept_t)(int sockfd, const struct sockaddr *addr, socklen_t addrlen);
static orig_accept_t orig_accept = NULL;
int accept(int sockfd, struct sockaddr *addr, socklen_t addrlen) {
	if (!orig_accept)
		orig_accept = (orig_accept_t)dlsym(RTLD_NEXT, "accept");
			
	int rv = orig_accept(sockfd, addr,  addrlen);
	print_sockaddr("accept", addr);

	return rv;
}
#endif
