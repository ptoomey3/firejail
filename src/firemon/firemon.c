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

static void usage(void) {
	printf("firemon - version %s\n", VERSION);
	printf("Firemon is a monitoring program for processes started in a Firejail sandbox.\n\n");
	printf("Usage: firemon [OPTIONS]\n");
	printf("All processes started by firejail are monitored. Descendants of these processes\n");
	printf("are also being monitored\n\n");
	printf("Options:\n");
	printf("\t--help, -? - this help screen\n");
	printf("\t--version - print program version and exit\n\n");
	printf("Copyright @ 2014 netblue30@yahoo.com\n");
	printf("License GPL version 2 or later\n");
	printf("Homepage: http://firejail.sourceforge.net\n");
	printf("\n");
}

int main(int argc, char **argv) {
	unsigned pid = 0;
	if (argc != 1 && argc != 2) {
		fprintf(stderr, "Error: pid argument missing\n");
		usage();
		return 1;
	}
	
	int i;
	for (i = 1; i < argc; i++) {
		if (strcmp(argv[i], "--help") == 0 ||
		    strcmp(argv[i], "-?") == 0) {
			usage();
			return 0;
		}
		if (strcmp(argv[i], "--version") == 0) {
			printf("firemon version %s\n\n", VERSION);
			return 0;
		}
	}

	pid_read(); // pass a pid of 0 if the program was run without arguments

	int sock = procevent_netlink_setup();
	if (sock < 0) {
		fprintf(stderr, "Error: cannot open netlink socket\n");
		return 1;
	}

	procevent_print_pids();
	procevent_monitor(sock,pid); // it will never return from here

	return 0;
}
