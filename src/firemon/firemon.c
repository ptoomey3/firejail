#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <errno.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <time.h>
#include <dirent.h>
#include <linux/connector.h>
#include <linux/netlink.h>
#include <linux/cn_proc.h>

#define BUFLEN 4096
#define MAX_PIDS 32769

unsigned char pids[MAX_PIDS];

static char *proc_cmdline(const pid_t pid) {
	// open /proc/pid/cmdline file
	char *fname;
	int fd;
	if (asprintf(&fname, "/proc/%d/cmdline", pid) == -1)
		return NULL;
	if ((fd = open(fname, O_RDONLY)) < 0) {
		free(fname);
		return NULL;
	}
	free(fname);

	// read file
	char buffer[BUFLEN];
	ssize_t len;
	if ((len = read(fd, buffer, sizeof(buffer) - 1)) <= 0) {
		close(fd);
		return NULL;
	}
	buffer[len] = '\0';
	close(fd);

	// clean data
	int i;
	for (i = 0; i < len; i++)
		if (buffer[i] == '\0')
			buffer[i] = ' ';

	// return a malloc copy of the command line
	char *rv = strdup(buffer);
	return rv;
}

static void read_pids(pid_t mypid) {
	pids[mypid] = 1;

	DIR *dir;
	if (!(dir = opendir("/proc"))) {
		fprintf(stderr, "Error: cannot open /proc directory\n");
		exit(1);
	}
	
	struct dirent *entry;
	char *end;
	while ((entry = readdir(dir))) {
		pid_t pid = strtol(entry->d_name, &end, 10);
		if (end == entry->d_name || *end)
			continue;
		if (pid == mypid)
			continue;

		// open stat file and find the parent; 
		// if the parent is flagged in pids array, flag this process also
		char *file;
		if (asprintf(&file, "/proc/%u/status", pid) == -1) {
			fprintf(stderr, "Error: cannot allocate memory\n");
			exit(1);
		}
		FILE *fp = fopen(file, "r");
		if (!fp) {
			free(file);
			continue;
		}

		// look for firejail executable name
		char buf[BUFLEN + 1];
		while (fgets(buf, BUFLEN, fp)) {
			if (strncmp(buf, "PPid:", 5) == 0) {
				char *ptr = buf + 5;
				while (*ptr != '\0' && (*ptr == ' ' || *ptr == '\t')) {
					ptr++;
				}
				if (*ptr == '\0') {
					fprintf(stderr, "Error: cannot read /proc file\n");
					exit(1);
				}
				unsigned parent = atoi(ptr);
				parent %= MAX_PIDS;
				if (pids[parent])
					pids[pid] = 1;
				break;
			}
		}
		fclose(fp);
		free(file);
	}
	closedir(dir);
}

static void print_pids(void) {
	int i;
	printf("Monitoring pids: ");
	for (i = 0; i < MAX_PIDS; i++) {
		if (pids[i]) {
			char *cmd = proc_cmdline(i);
			if (cmd) {
				printf("%d (%s), ", i, cmd);
				free(cmd);
			}
			else
				printf("%d, ", i);
		}
	}
	printf("\n");
}

static int netlink_setup(void)
{
	// open socket
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
					if (pids[pid]) {
						child = proc_ev->event_data.fork.child_tgid;
						child %= MAX_PIDS;
						pids[child] = 1;
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
			if (pids[pid] == 0)
				continue;
				
			lineptr += strlen(lineptr);
			sprintf(lineptr, " %u", pid);
			lineptr += strlen(lineptr);
			
			char *cmd = proc_cmdline(pid);
			if (cmd == NULL)
				sprintf(lineptr, "\n");
			else {
				sprintf(lineptr, "\t%s\n", cmd);
				free(cmd);
			}
			lineptr += strlen(lineptr);
			printf("%s", line);			
			fflush(0);
			
			// unflag pid for exit events
			if (remove_pid)
				pids[pid] = 0;

			// print forked child
			if (child) {
				cmd = proc_cmdline(child);
				if (cmd) {
					printf("\tchild %u\t%s\n", child, cmd);
					free(cmd);
				}
				else
					printf("\tchild %u\n", child);
			}
		}
	}
	return 0;
}



int main(int argc, char **argv) {
	unsigned pid;
	if (argc != 2) {
		fprintf(stderr, "Error: pid argument missing\n");
		return 1;
	}
	
	if (sscanf(argv[1], "%u", &pid) != 1) {
		fprintf(stderr, "Error: invalid pid number\n");
		return 1;
	}
	if (pid > MAX_PIDS) {
		fprintf(stderr, "Error: invalid pid number\n");
		return 1;
	}
	memset(pids, 0, sizeof(pids));
	read_pids(pid);

	int sock = netlink_setup();
	if (sock < 0) {
		fprintf(stderr, "Error: cannot open netlink socket\n");
		return 1;
	}

	print_pids();
	monitor(sock,pid); // it will never return from here

	return 0;
}
