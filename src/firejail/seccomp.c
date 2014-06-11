/*
 * Copyright (C) 2014 netblue30 (netblue30@yahoo.com)
 *
 * This file is part of firejail project
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License along
 * with this program; if not, write to the Free Software Foundation, Inc.,
 * 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA.
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

#define RETURN_ALLOW BPF_STMT(BPF_RET+BPF_K, SECCOMP_RET_ALLOW)

int seccomp_filter(void) {
	struct sock_filter filter[] = {
		BLACKLIST(SYS_mount),
		BLACKLIST(SYS_umount2),
		BLACKLIST(SYS_ptrace),
		RETURN_ALLOW
	};
	
	struct sock_fprog prog = {
		.len = (unsigned short)(sizeof(filter)/sizeof(filter[0])),
		.filter = filter,
	};

	if (prctl(PR_SET_NO_NEW_PRIVS, 1, 0, 0, 0) || prctl(PR_SET_SECCOMP, SECCOMP_MODE_FILTER, &prog)) {
		fprintf(stderr, "Warning: privilege locking was disabled. It requires a Linux kernel version 3.5 or newer.\n");
		return 1;
	}
	
	return 0;
}


