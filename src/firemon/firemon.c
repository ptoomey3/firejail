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
#include "firemon.h"
static int arg_list = 0;
static int arg_mem = 0;
static int arg_cpu = 0;
static int arg_uptime = 0;

int main(int argc, char **argv) {
	unsigned pid = 0;
	int i;

	for (i = 1; i < argc; i++) {
		// default options
		if (strcmp(argv[i], "--help") == 0 ||
		    strcmp(argv[i], "-?") == 0) {
			usage();
			return 0;
		}
		else if (strcmp(argv[i], "--version") == 0) {
			printf("firemon version %s\n\n", VERSION);
			return 0;
		}
		
		// list options
		else if (strcmp(argv[i], "--list") == 0)
			arg_list = 1;
		else if (strcmp(argv[i], "--mem") == 0)
			arg_mem = 1;
		else if (strcmp(argv[i], "--cpu") == 0)
			arg_cpu = 1;
		else if (strcmp(argv[i], "--uptime") == 0)
			arg_uptime = 1;
		
		// PID argument
		else {
			// this should be a pid number
			sscanf(argv[i], "%u", &pid);
			break;
		}
	}

	if (arg_list)
		list(pid); // never to return
	else if (arg_mem)
		list_mem(pid); // never to return
	else if (arg_cpu)
		list_cpu(pid); // never to return
	else if (arg_uptime)
		list_uptime(pid); // never to return
	else
		procevent((pid_t) pid); // never to return
		
	return 0;
}
