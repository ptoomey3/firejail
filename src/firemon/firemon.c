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
#define _GNU_SOURCE
#include <stdio.h>
#include <sys/types.h>
#include <unistd.h>
#include <termios.h>
#include <sys/ioctl.h>
#include <sys/prctl.h>
#include <signal.h>
#include <grp.h>
#include "firemon.h"
static int arg_top = 0;

static struct termios tlocal;	// startup terminal setting
static struct termios twait;		// no wait on key press
static int terminal_set = 0;

static void my_handler(int s){
	if (terminal_set)
		tcsetattr(0, TCSANOW, &tlocal);
	exit(0); 
}

// drop privileges
void firemon_drop_privs(void) {
	// drop privileges
	if (setgroups(0, NULL) < 0)
		errExit("setgroups");
	if (setgid(getgid()) < 0)
		errExit("setgid/getgid");
	if (setuid(getuid()) < 0)
		errExit("setuid/getuid");
}

// sleep and wait for a key to be pressed
void firemon_sleep(int st) {
	if (terminal_set == 0) {
		tcgetattr(0, &twait);          // get current terminal attirbutes; 0 is the file descriptor for stdin
		memcpy(&tlocal, &twait, sizeof(tlocal));
		twait.c_lflag &= ~ICANON;      // disable canonical mode
		twait.c_lflag &= ~ECHO;	// no echo
		twait.c_cc[VMIN] = 1;          // wait until at least one keystroke available
		twait.c_cc[VTIME] = 0;         // no timeout
		terminal_set = 1;
	}
	tcsetattr(0, TCSANOW, &twait);


	fd_set fds;
	FD_ZERO(&fds);
	FD_SET(0,&fds);
	int maxfd = 1;

	struct timeval ts;
	ts.tv_sec = st;
	ts.tv_usec = 0;

	int ready = select(maxfd, &fds, (fd_set *) 0, (fd_set *) 0, &ts);
	if( FD_ISSET(0, &fds)) {
		getchar();
		tcsetattr(0, TCSANOW, &tlocal);
		printf("\n");
		exit(0);
	}
	tcsetattr(0, TCSANOW, &tlocal);
}


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
		
		// options without a pid argument
		else if (strcmp(argv[i], "--top") == 0) {
			arg_top = 1;
			break;
		}
		
		// PID argument
		else {
			// this should be a pid number
			sscanf(argv[i], "%u", &pid);
			break;
		}
	}

	// handle CTRL-C
	signal (SIGINT, my_handler);
	signal (SIGTERM, my_handler);

	if (arg_top)
		top(); // never to return
	else
		procevent((pid_t) pid); // never to return
		
	return 0;
}
