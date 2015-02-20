/*
 * Copyright (C) 2014, 2015 netblue30 (netblue30@yahoo.com)
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

#include "firejail.h"
#include <errno.h>
#include <linux/filter.h>
#include <stddef.h>
#include <linux/capability.h>
#include <linux/audit.h>
#include <sys/prctl.h>

// todo: figure out where this definitins should be; on Debian 7 capget and <sys/capability.h> are not found
extern int capget(cap_user_header_t hdrp, cap_user_data_t datap);
extern int capset(cap_user_header_t hdrp, const cap_user_data_t datap);


// enabled by default
int caps_default_filter(void) {
	// drop capabilities
	if (prctl(PR_CAPBSET_DROP, CAP_SYS_MODULE, 0, 0, 0) && arg_debug)
		fprintf(stderr, "Warning: cannot drop CAP_SYS_MODULE");
	else if (arg_debug)
		printf("Drop CAP_SYS_MODULE\n");

	if (prctl(PR_CAPBSET_DROP, CAP_SYS_RAWIO, 0, 0, 0) && arg_debug)
		fprintf(stderr, "Warning: cannot drop CAP_SYS_RAWIO");
	else if (arg_debug)
		printf("Drop CAP_SYS_RAWIO\n");

	if (prctl(PR_CAPBSET_DROP, CAP_SYS_BOOT, 0, 0, 0) && arg_debug)
		fprintf(stderr, "Warning: cannot drop CAP_SYS_BOOT");
	else if (arg_debug)
		printf("Drop CAP_SYS_BOOT\n");

	if (prctl(PR_CAPBSET_DROP, CAP_SYS_NICE, 0, 0, 0) && arg_debug)
		fprintf(stderr, "Warning: cannot drop CAP_SYS_NICE");
	else if (arg_debug)
		printf("Drop CAP_SYS_NICE\n");

	if (prctl(PR_CAPBSET_DROP, CAP_SYS_TTY_CONFIG, 0, 0, 0) && arg_debug)
		fprintf(stderr, "Warning: cannot drop CAP_SYS_TTY_CONFIG");
	else if (arg_debug)
		printf("Drop CAP_SYS_TTY_CONFIG\n");

	if (prctl(PR_CAPBSET_DROP, CAP_SYSLOG, 0, 0, 0) && arg_debug)
		fprintf(stderr, "Warning: cannot drop CAP_SYSLOG");
	else if (arg_debug)
		printf("Drop CAP_SYSLOG\n");

	if (prctl(PR_CAPBSET_DROP, CAP_MKNOD, 0, 0, 0) && arg_debug)
		fprintf(stderr, "Warning: cannot drop CAP_MKNOD");
	else if (arg_debug)
		printf("Drop CAP_SYSLOG\n");

	if (prctl(PR_CAPBSET_DROP, CAP_SYS_ADMIN, 0, 0, 0) && arg_debug)
		fprintf(stderr, "Warning: cannot drop CAP_SYS_ADMIN");
	else if (arg_debug)
		printf("Drop CAP_SYS_ADMIN\n");

	return 0;
}

void caps_drop_all(void) {
	if (arg_debug)
		printf("Droping all capabilities\n");

	unsigned long cap;
	for (cap=0; cap <= 63; cap++) {
		int code = prctl(PR_CAPBSET_DROP, cap, 0, 0, 0);
		if (code == -1 && errno != EINVAL)
			errExit("PR_CAPBSET_DROP");
	}
}


void caps_set(uint64_t caps) {
	if (arg_debug)
		printf("Set caps filter %llx\n", (unsigned long long) caps);

	unsigned long i;
	uint64_t mask = 1LLU;
	for (i = 0; i < 64; i++, mask <<= 1) {
		if ((mask & caps) == 0) {
			int code = prctl(PR_CAPBSET_DROP, i, 0, 0, 0);
			if (code == -1 && errno != EINVAL)
				errExit("PR_CAPBSET_DROP");
		}
	}
}
