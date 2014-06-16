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

void usage(void) {
	printf("firemon - version %s\n", VERSION);
	printf("Usage: firemon [OPTIONS] [PID]\n\n");
	printf("Monitor processes started in a Firejail sandbox. Without any PID specified,\n");
	printf("all processes started by Firejail are monitored. Descendants of these processes\n");
	printf("are also being monitored.\n\n");
	printf("Options:\n");
	printf("\t--help, -? - this help screen\n");
	printf("\t--cpu - monitor CPU utilization for all sandboxes\n");
	printf("\t--list - monitor all sandboxed processes\n");
	printf("\t--mem - monitor the memory consumed by each sandbox\n");
	printf("\t--uptime - monitor uptime for all sandboxes\n");
	printf("\t--version - print program version and exit\n\n");
	printf("License GPL version 2 or later\n");
	printf("Homepage: http://firejail.sourceforge.net\n");
	printf("\n");
}
