#include "firejail.h"
#include <linux/in.h>
#include <linux/inet.h>
#include <net/sock.h>
//struct timespec ts;
//ktime_get_ts(&ts); /* get high res monotonic timestamp */
static struct socket *clientsocket=NULL;

static void send_udp(char *str) {
	struct msghdr msg;
	struct iovec iov;
	mm_segment_t oldfs;
	struct sockaddr_in to;
	
	if (trace_udp_port == 0)
		return;

	// set socket
	if (clientsocket == NULL) {
		if( sock_create(PF_INET, SOCK_DGRAM, IPPROTO_UDP, &clientsocket) < 0) {
			printk( KERN_ERR "server: Error creating clientsocket.n" );
			return;
		}
	}
	
	// set destination
	memset(&to, 0, sizeof(to));
	to.sin_family = AF_INET;
	to.sin_addr.s_addr = in_aton( "127.0.0.1" );
	to.sin_port = htons(trace_udp_port);
		
	// set message
	memset(&msg,0,sizeof(msg));
	msg.msg_name = &to;
	msg.msg_namelen = sizeof(to);
	iov.iov_base = str;
	iov.iov_len  = strlen(str);
	msg.msg_control = NULL;
	msg.msg_controllen = 0;
	msg.msg_iov    = &iov;
	msg.msg_iovlen = 1;
	// msg.msg_flags    = MSG_NOSIGNAL;
	
	// send the message
	oldfs = get_fs();
	set_fs( KERNEL_DS );
	sock_sendmsg(clientsocket, &msg, strlen(str));
	set_fs( oldfs );
}


// handle open syscall
void syscall_probe_open(struct pt_regs *regs, long id) {
	unsigned long val;
	int MAXBUF = 256;
	char buf[MAXBUF + 1];
	int cnt = 0;
	memset(buf, 0, sizeof(buf));

	syscall_get_arguments(current, regs, 0, 1, &val);

	// read chunks of 4 bytes from user and look for end of string
	while (cnt < (MAXBUF / 4)) {
		int i;
		char tmp[4];
		char *ptr = tmp;
		int doexit = 0;

		if (__copy_from_user_inatomic(tmp, (const char *) val + cnt * 4, 4) < 0) {
			printk(KERN_INFO "firejail %u: ERROR %d\n", current->pid, __LINE__);
			break;
		}
		for (i = 0; i < 4; i++, ptr++) {
			if (ptr != '\0')
				buf[cnt * 4 + i] = tmp[i];
			else {
				buf[cnt * 4 + i] = '\0';
				doexit = 1;
			}
			if (doexit)
				break;
		}
		if (doexit)
			break;
		cnt++;
	}
	if (strncmp(buf, "/tmp", 4) == 0)
		printk(KERN_INFO "firejail %u: open %s\n", current->pid, buf);
}


void syscall_probe_socket(struct pt_regs *regs, long id) {
	unsigned long __user args[2];
	int socketcall_id;
	syscall_get_arguments(current, regs, 0, 2, args);
	socketcall_id = args[0];

	switch (socketcall_id) {
		case SYS_SOCKET:
			printk(KERN_INFO "firejail %u: socket SOCKET\n", current->pid); break;
		case SYS_BIND:
			printk(KERN_INFO "firejail %u: socket BIND\n", current->pid); break;
		case SYS_CONNECT:
			printk(KERN_INFO "firejail %u: socket CONNECT\n", current->pid); break;
		case SYS_LISTEN:
			printk(KERN_INFO "firejail %u: socket LISTEN\n", current->pid); break;
		case SYS_ACCEPT:
			printk(KERN_INFO "firejail %u: socket ACCEPT\n", current->pid); break;
		case SYS_GETSOCKNAME:
			printk(KERN_INFO "firejail %u: socket GETSOCKNAME\n", current->pid); break;
		case SYS_GETPEERNAME:
			printk(KERN_INFO "firejail %u: socket GETPEERNAME\n", current->pid); break;
		case SYS_SOCKETPAIR:
			printk(KERN_INFO "firejail %u: socket SOCKETPAIR\n", current->pid); break;
		case SYS_SEND:
			printk(KERN_INFO "firejail %u: socket SEND\n", current->pid); break;
		case SYS_SENDTO:
			printk(KERN_INFO "firejail %u: socket SENDTO\n", current->pid); break;
		case SYS_RECV:
			printk(KERN_INFO "firejail %u: socket RECV\n", current->pid); break;
		case SYS_RECVFROM:
			printk(KERN_INFO "firejail %u: socket RECVFROM\n", current->pid); break;
		case SYS_SHUTDOWN:
			printk(KERN_INFO "firejail %u: socket SHUTDOWN\n", current->pid); break;
		case SYS_SETSOCKOPT:
			printk(KERN_INFO "firejail %u: socket SETSOCKOPT\n", current->pid); break;
		case SYS_GETSOCKOPT:
			printk(KERN_INFO "firejail %u: socket GETSOCKOPT\n", current->pid); break;
		case SYS_SENDMSG:
			printk(KERN_INFO "firejail %u: socket SENDMSG\n", current->pid); break;
		case SYS_SENDMMSG:
			printk(KERN_INFO "firejail %u: socket SENDMMSG\n", current->pid); break;
		case SYS_RECVMSG:
			printk(KERN_INFO "firejail %u: socket RECVMSG\n", current->pid); break;
		case SYS_RECVMMSG:
			printk(KERN_INFO "firejail %u: socket RECVMMSG\n", current->pid); break;
		case SYS_ACCEPT4:
			printk(KERN_INFO "firejail %u: socket ACCEPT4\n", current->pid); break;
		default:
			printk(KERN_INFO "firejail %u: socket ???\n", current->pid); break;
	}
}


void syscall_probe_connect(struct pt_regs *regs, long id, NsRule *rule) {
	unsigned long val;
	int fd;
	struct sockaddr __user *usraddr;
	struct sockaddr_storage addr;
	memset(&addr, 0, sizeof(addr));

	syscall_get_arguments(current, regs, 0, 1, &val);
	fd = (int) val;

	if (fd > 0) {
		unsigned len = 0;
		syscall_get_arguments(current, regs, 1, 1, &val);
		usraddr = (struct sockaddr __user *)val;

		syscall_get_arguments(current, regs, 2, 1, &val);
		len = (unsigned) val;

		if (usraddr && len) {
			sa_family_t family ;

			if (__copy_from_user_inatomic((char *) &addr, (const char *)usraddr, len) < 0) {
				printk(KERN_INFO "firejail %u: ERROR %d\n", current->pid, __LINE__);
			}

			family = ((struct sockaddr *) &addr)->sa_family;
			if (family == AF_INET) {
				char buf[512];
				struct sockaddr_in *saddr4 = (struct sockaddr_in *) &addr;
				char *a = (char *) &saddr4->sin_addr.s_addr;
//				printk(KERN_INFO "firejail[%u]: connect AF_INET %u.%u.%u.%u\n", rule->sandbox_pid,
//					a[0] & 0xff, a[1] & 0xff, a[2] & 0xff, a[3] & 0xff);
				if (trace_cnt) {
					sprintf(buf, "conn %u AF_INET %u.%u.%u.%u\n", current->pid, a[0] & 0xff, a[1] & 0xff, a[2] & 0xff, a[3] & 0xff);
					send_udp(buf);
				}
			}

			else if (family == AF_INET6) {
//				printk(KERN_INFO "firejail[%u]: connect AF_INET6\n", rule->sandbox_pid);
			}

			else if (family == AF_UNIX) {
				int i;
				char buf[512];
				struct sockaddr_un *a = (struct sockaddr_un *) &addr;
				if (trace_cnt) {
					if (a->sun_path[0]) {
//						printk(KERN_INFO "firejail[%u]: connect AF_UNIX #%s#\n", rule->sandbox_pid, a->sun_path);
						sprintf(buf, "conn %u AF_UNIX %s\n", current->pid, a->sun_path);
					}
					else {
//						printk(KERN_INFO "firejail[%u]: connect AF_UNIX %s\n", rule->sandbox_pid, a->sun_path + 1);
						sprintf(buf, "conn %u AF_UNIX %s\n", current->pid, a->sun_path + 1);
					}
					send_udp(buf);
				}

				for (i = 0; i < MAX_UNIX_PATH; i++) {
					if (rule->unix_path_len[i] && rule->unix_path[i]) {
						if ((a->sun_path[0] && strncmp(a->sun_path, rule->unix_path[i], rule->unix_path_len[i]) == 0) ||
						(a->sun_path[0] == '\0' && strncmp(a->sun_path + 1, rule->unix_path[i], rule->unix_path_len[i]) == 0)) {

							printk(KERN_INFO "firejail[%u]: process %u killed, rule \"no connect unix %s\"\n",
								rule->sandbox_pid, current->pid, rule->unix_path[i]);
							if (trace_cnt) {
								sprintf(buf, "process %u killed, sandbox %u rule \"no connect unix %s\"\n",
									current->pid, rule->sandbox_pid, rule->unix_path[i]);
								send_udp(buf);
							}
							do_exit(SIGSYS);
						}
					}
					else
						break;
				}
			} // end AF_UNIX
		}
	}
}
