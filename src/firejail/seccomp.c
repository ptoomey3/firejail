/*
 * Naive system call dropper built on seccomp_filter.
 *
 * Copyright (c) 2012 The Chromium OS Authors <chromium-os-dev@chromium.org>
 * Author: Will Drewry <wad@chromium.org>
 *
 * The code may be used by anyone for any purpose,
 * and can serve as a starting point for developing
 * applications using prctl(PR_SET_SECCOMP, 2, ...).
 *
 * When run, returns the specified errno for the specified
 * system call number against the given architecture.
 *
 * Run this one as root as PR_SET_NO_NEW_PRIVS is not called.
 */

#include <errno.h>
#include <linux/filter.h>
#include <unistd.h>
#include <stdio.h>
#include <stddef.h>
#include <stdlib.h>
#include <sys/prctl.h>
#include <sys/syscall.h>

#include <sys/prctl.h>
#ifndef PR_SET_NO_NEW_PRIVS
# define PR_SET_NO_NEW_PRIVS 38
#endif

#if HAVE_SECCOMP_H
#include <linux/seccomp.h>
#else
#define SECCOMP_MODE_FILTER	2
#define SECCOMP_RET_KILL	0x00000000U
#define SECCOMP_RET_TRAP	0x00030000U
#define SECCOMP_RET_ALLOW	0x7fff0000U
#define SECCOMP_RET_ERRNO	0x00050000U
#define SECCOMP_RET_DATA        0x0000ffffU
struct seccomp_data {
    int nr;
    __u32 arch;
    __u64 instruction_pointer;
    __u64 args[6];
};
#endif

#define BLACKLIST(syscall_nr)	\
	BPF_STMT(BPF_LD+BPF_W+BPF_ABS,	\
		 (offsetof(struct seccomp_data, nr))),	\
	BPF_JUMP(BPF_JMP+BPF_JEQ+BPF_K, syscall_nr, 0, 1),	\
	BPF_STMT(BPF_RET+BPF_K, SECCOMP_RET_KILL) //	\
//		 SECCOMP_RET_ERRNO|(SECCOMP_RET_DATA))

int seccomp_filter(void) {
	struct sock_filter filter[] = {
		BLACKLIST(SYS_mount),
		BLACKLIST(SYS_umount2),
		BLACKLIST(SYS_ptrace),
		BPF_STMT(BPF_RET+BPF_K, SECCOMP_RET_ALLOW),
	};
	struct sock_fprog prog = {
		.len = (unsigned short)(sizeof(filter)/sizeof(filter[0])),
		.filter = filter,
	};
	if (prctl(PR_SET_NO_NEW_PRIVS, 1, 0, 0, 0)) {
		perror("prctl(NO_NEW_PRIVS)");
		goto failed;
	}
	if (prctl(PR_SET_SECCOMP, SECCOMP_MODE_FILTER, &prog)) {
		perror("prctl(SECCOMP)");
		goto failed;
	}
	
	return 0;
failed:
	if (errno == EINVAL)
		fprintf(stderr, "SECCOMP_FILTER is not available.\n");
	return 1;
}


#if 0
int main(int argc, char **argv) {

printf("mount %d\n", SYS_mount);
printf("umount2 %d\n", SYS_umount2);

	if (argc != 3 ) {
		fprintf(stderr, "Usage:\n"
			"dropper <syscall_nr> <prog>\n");
		return 1;
	}
	if (install_filter(strtol(argv[1], NULL, 0)))
		return 1;

	char *arg[4];
	arg[0] = "bash";
	arg[1] = "-c";
	arg[2] = argv[2];
	argv[3] = NULL;
	execvp("/bin/bash", arg); 

	return 0;
}
#endif

