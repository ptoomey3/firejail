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
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <errno.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <pwd.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <time.h>
#include <dirent.h>
#include <linux/connector.h>
#include <linux/netlink.h>
#include <linux/cn_proc.h>
#include "../include/pid.h"

#define BUFLEN 4096

static int netlink_setup(void)
{
	// open socket for process event connector
	int sock;
	if ((sock = socket(PF_NETLINK, SOCK_DGRAM, NETLINK_CONNECTOR)) < 0) {
		fprintf(stderr, "Error: cannot open netlink socket\n");
		exit(1);
	}

	// bind socket
	struct sockaddr_nl addr;
	memset(&addr, 0, sizeof(addr));
	addr.nl_pid = getpid();
	addr.nl_family = AF_NETLINK;
	addr.nl_groups = CN_IDX_PROC;
	if (bind(sock, (struct sockaddr *)&addr, sizeof(addr)) < 0) {
		fprintf(stderr, "Error: cannot bind to netlink socket\n");
		exit(1);
	}

	// send monitoring message
	struct nlmsghdr nlmsghdr;
	memset(&nlmsghdr, 0, sizeof(nlmsghdr));
	nlmsghdr.nlmsg_len = NLMSG_LENGTH(sizeof(struct cn_msg) + sizeof(enum proc_cn_mcast_op));
	nlmsghdr.nlmsg_pid = getpid();
	nlmsghdr.nlmsg_type = NLMSG_DONE;

	struct cn_msg cn_msg;
	memset(&cn_msg, 0, sizeof(cn_msg));
	cn_msg.id.idx = CN_IDX_PROC;
	cn_msg.id.val = CN_VAL_PROC;
	cn_msg.len = sizeof(enum proc_cn_mcast_op);

	struct iovec iov[3];
	iov[0].iov_base = &nlmsghdr;
	iov[0].iov_len = sizeof(nlmsghdr);
	iov[1].iov_base = &cn_msg;
	iov[1].iov_len = sizeof(cn_msg);

	enum proc_cn_mcast_op op = PROC_CN_MCAST_LISTEN;
	iov[2].iov_base = &op;
	iov[2].iov_len = sizeof(op);

	if (writev(sock, iov, 3) == -1) {
		fprintf(stderr, "Error: cannot write to netlink socket\n");
		exit(1);
	}
	
	return sock;
}

static int monitor(const int sock, pid_t mypid) {
	ssize_t len;
	struct nlmsghdr *nlmsghdr;

	while (1) {
		char __attribute__ ((aligned(NLMSG_ALIGNTO)))buf[4096];

		if ((len = recv(sock, buf, sizeof(buf), 0)) == 0) {
			return 0;
		}
		if (len == -1) {
			if (errno == EINTR) {
				return 0;
			} else {
				fprintf(stderr,"recv: %s\n", strerror(errno));
				return -1;
			}
		}

		for (nlmsghdr = (struct nlmsghdr *)buf;
			NLMSG_OK (nlmsghdr, len);
			nlmsghdr = NLMSG_NEXT (nlmsghdr, len)) {

			struct cn_msg *cn_msg;
			struct proc_event *proc_ev;
			struct tm tm;
			struct timeval tv;
			time_t now;

			if ((nlmsghdr->nlmsg_type == NLMSG_ERROR) ||
			    (nlmsghdr->nlmsg_type == NLMSG_NOOP))
				continue;

			cn_msg = NLMSG_DATA(nlmsghdr);
			if ((cn_msg->id.idx != CN_IDX_PROC) ||
			    (cn_msg->id.val != CN_VAL_PROC))
				continue;

			(void)time(&now);
			(void)localtime_r(&now, &tm);
			char line[BUFLEN];
			char *lineptr = line;
			sprintf(lineptr, "%2.2d:%2.2d:%2.2d", tm.tm_hour, tm.tm_min, tm.tm_sec);
			lineptr += strlen(lineptr);

			proc_ev = (struct proc_event *)cn_msg->data;
			pid_t pid = 0;
			pid_t child = 0;
			int remove_pid = 0;
			switch (proc_ev->what) {
				case PROC_EVENT_FORK:
					if (proc_ev->event_data.fork.child_pid !=
					    proc_ev->event_data.fork.child_tgid)
					    	continue; // this is a thread, not a process
					pid = proc_ev->event_data.fork.parent_tgid;
					if (pids[pid].level > 0) {
						child = proc_ev->event_data.fork.child_tgid;
						child %= MAX_PIDS;
						pids[child].level = pids[pid].level + 1;
						pids[child].uid = pid_get_uid(child);
					}
					sprintf(lineptr, " fork");
					break;
				case PROC_EVENT_EXEC:
					pid = proc_ev->event_data.exec.process_tgid;
					sprintf(lineptr, " exec");
					break;
					
				case PROC_EVENT_EXIT:
					if (proc_ev->event_data.exit.process_pid !=
					    proc_ev->event_data.exit.process_tgid)
						continue; // this is a thread, not a process

					pid = proc_ev->event_data.exit.process_tgid;
					remove_pid = 1;
					sprintf(lineptr, " exit");
					break;
					
				case PROC_EVENT_UID:
					pid = proc_ev->event_data.id.process_tgid;
					sprintf(lineptr, " uid ");
					break;

				case PROC_EVENT_GID:
					pid = proc_ev->event_data.id.process_tgid;
					sprintf(lineptr, " gid ");
					break;

				case PROC_EVENT_SID:
					pid = proc_ev->event_data.sid.process_tgid;
					sprintf(lineptr, " sid ");
					break;

				default:
					sprintf(lineptr, "\n");
					continue;
			}

			int add_new = 0;
			if (pids[pid].level < 0)	// not a firejail process
				continue;
			else if (pids[pid].level == 0) { // new porcess, do we have track it?
				if (pid_is_firejail(pid)) {
					pids[pid].level = 1;
					add_new = 1;
				}
				else {
					pids[pid].level = -1;
					continue;
				}
			}
				
			lineptr += strlen(lineptr);
			sprintf(lineptr, " %u", pid);
			lineptr += strlen(lineptr);
			
			char *user = pids[pid].user;
			if (!user)
				user = pid_get_user_name(pids[pid].uid);
			if (user) {
				pids[pid].user = user;
				sprintf(lineptr, " (%s)", user);
				lineptr += strlen(lineptr);
			}
			

			char *cmd = pids[pid].cmd;
			if (add_new) {
				sprintf(lineptr, " NEW SANDBOX\n");
				lineptr += strlen(lineptr);
			}
			else if (proc_ev->what == PROC_EVENT_EXIT && pids[pid].level == 1) {
				sprintf(lineptr, " EXIT SANDBOX\n");
				lineptr += strlen(lineptr);
			}
			else {
				if (!cmd)
					cmd = pid_proc_cmdline(pid);
				if (cmd == NULL)
					sprintf(lineptr, "\n");
				else {
					sprintf(lineptr, " %s\n", cmd);
					free(cmd);
				}
				lineptr += strlen(lineptr);
			}
			
			// print the event
			printf("%s", line);			
			fflush(0);
			
			// unflag pid for exit events
			if (remove_pid) {
				if (pids[pid].user)
					free(pids[pid].user);
				if (pids[pid].cmd)
					free(pids[pid].cmd);
				memset(&pids[pid], 0, sizeof(Process));
			}

			// print forked child
			if (child) {
				cmd = pid_proc_cmdline(child);
				if (cmd) {
					printf("\tchild %u %s\n", child, cmd);
					free(cmd);
				}
				else
					printf("\tchild %u\n", child);
			}
			
			// on uid events the uid is changing
			if (proc_ev->what == PROC_EVENT_UID) {
				if (pids[pid].user)
					free(pids[pid].user);
				pids[pid].user = 0;
				pids[pid].uid = pid_get_uid(pid); 
			}
		}
	}
	return 0;
}

static void print_pids(void) {

	// print files
	int i;
	for (i = 0; i < MAX_PIDS; i++) {
		if (pids[i].level == 1)
			pid_print_tree(i, 0, 1);
	}
	printf("\n");
}


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

	int sock = netlink_setup();
	if (sock < 0) {
		fprintf(stderr, "Error: cannot open netlink socket\n");
		return 1;
	}

	print_pids();
	monitor(sock,pid); // it will never return from here

	return 0;
}
