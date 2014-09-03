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
#include <sys/syscall.h>
#include <linux/capability.h>
#include <linux/audit.h>
#include "firejail.h"

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

#if defined(__i386__)
# define ARCH_NR	AUDIT_ARCH_I386
#elif defined(__x86_64__)
# define ARCH_NR	AUDIT_ARCH_X86_64
#else
# warning "Platform does not support seccomp filter yet"
# define ARCH_NR	0
#endif


#define VALIDATE_ARCHITECTURE \
     BPF_STMT(BPF_LD+BPF_W+BPF_ABS, (offsetof(struct seccomp_data, arch))), \
     BPF_JUMP(BPF_JMP+BPF_JEQ+BPF_K, ARCH_NR, 1, 0), \
     BPF_STMT(BPF_RET+BPF_K, SECCOMP_RET_KILL)

#define EXAMINE_SYSCALL BPF_STMT(BPF_LD+BPF_W+BPF_ABS,	\
		 (offsetof(struct seccomp_data, nr)))
#define BLACKLIST(syscall_nr)	\
	BPF_JUMP(BPF_JMP+BPF_JEQ+BPF_K, syscall_nr, 0, 1),	\
	BPF_STMT(BPF_RET+BPF_K, SECCOMP_RET_KILL) //	\
//		 SECCOMP_RET_ERRNO|(SECCOMP_RET_DATA))

#define RETURN_ALLOW BPF_STMT(BPF_RET+BPF_K, SECCOMP_RET_ALLOW)

void caps_print(void) {
	cap_user_header_t       hdr;
	cap_user_data_t         data;
	hdr = malloc(sizeof(*hdr));
	data = malloc(sizeof(*data));
	memset(hdr, 0, sizeof(*hdr));
	hdr->version = _LINUX_CAPABILITY_VERSION;

	if (capget(hdr, data) < 0) {
		perror("capget");
		goto doexit;
	}
	
	printf("effective\t%x\n", data->effective);
	printf("permitted\t%x\n", data->permitted);
	printf("inheritable\t%x\n", data->inheritable);

doexit:
	free(hdr);
	free(data);
}


// enabled by default 
int caps_filter(void) {
	// drop capabilities
	if (prctl(PR_CAPBSET_DROP, CAP_SYS_MODULE, 0, 0, 0) && arg_debug)
		fprintf(stderr, "Warning: kernel module loading allowed for root user, your kernel does not have support for CAP_SYS_MODULE");
	else if (arg_debug)
		printf("Kernel modules loading disabled\n");

	if (prctl(PR_CAPBSET_DROP, CAP_SYS_RAWIO, 0, 0, 0) && arg_debug)
		fprintf(stderr, "Warning: ioperm/iopl calls allowed for root user, your kernel does not have support for CAP_SYS_RAWIO");
	else if (arg_debug)
		printf("ioperm/iopl calls disabled\n");

	if (prctl(PR_CAPBSET_DROP, CAP_SYS_BOOT, 0, 0, 0) && arg_debug)
		fprintf(stderr, "Warning: system rebooting capability not removed, your kernel does not have support for CAP_SYS_BOOT");
	else if (arg_debug)
		printf("System rebooting disabled\n");

	return 0;
}



// enabled only for --seccomp option
int seccomp_filter(void) {
	// seccomp
	struct sock_filter filter[] = {
		VALIDATE_ARCHITECTURE,
		EXAMINE_SYSCALL,
		BLACKLIST(SYS_mount),  // mount/unmount filesystems
		BLACKLIST(SYS_umount2),
		
		BLACKLIST(SYS_ptrace), // trace processes

		BLACKLIST(SYS_kexec_load), // loading a different kernel

		BLACKLIST(SYS_open_by_handle_at), // open by handle
		
		BLACKLIST(SYS_init_module), // kernel module handling
#ifdef SYS_finit_module // introduced in 2013
		BLACKLIST(SYS_finit_module),
#endif
		BLACKLIST(SYS_delete_module),
		
		BLACKLIST(SYS_iopl), // io permisions
		BLACKLIST(SYS_ioperm),
		
		BLACKLIST(SYS_swapon), // swap on/off
		BLACKLIST(SYS_swapoff),

		RETURN_ALLOW
	};
	
	struct sock_fprog prog = {
		.len = (unsigned short)(sizeof(filter)/sizeof(filter[0])),
		.filter = filter,
	};

	if (prctl(PR_SET_SECCOMP, SECCOMP_MODE_FILTER, &prog) || prctl(PR_SET_NO_NEW_PRIVS, 1, 0, 0, 0)) {
		if (arg_debug)
			fprintf(stderr, "Warning: seccomp disabled, it requires a Linux kernel version 3.5 or newer.\n");
		return 1;
	}
	else if (arg_debug) {
		printf("seccomp enabled\n");
	}
	
	return 0;
}

