#ifndef FIREJAIL_LKM_H
#define FIREJAIL_LKM_H
#include <linux/version.h>
#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/init.h>
#include <linux/skbuff.h>
#include <linux/netfilter.h>
#include <linux/netfilter_ipv4.h>
#include <linux/ip.h>
#include <linux/udp.h>
#include <linux/proc_fs.h>
#include <linux/tracepoint.h>
#include <asm/syscall.h>
#include <linux/un.h>

// one second timer
extern struct timer_list rate_timer;

// rules
typedef struct nsrule_t {
	struct nsrule_t *next;	// linked list
	
	struct nsproxy *nsproxy;		// namespace proxy pointer
	pid_t sandbox_pid;			// pid of the controll process for the sandbox
	struct timespec real_start_time;	// time when the sandbox was registered
	unsigned active : 1;	// rule active flag; inactive rules are reused or deallocated

	// unix sockets connections that will kill the process
#define MAX_UNIX_PATH 8
	char *unix_path[MAX_UNIX_PATH];
	int unix_path_len[MAX_UNIX_PATH];
} NsRule;
extern NsRule head;
extern NsRule tmp_head;
extern spinlock_t head_lock;
#define CLEANUP_CNT 60 	// clean the list every 60 seconds

// trace
#define TRACE_MAX 60		// max value for trace_cnt, decremented every second
extern unsigned short trace_udp_port;
extern int trace_cnt;

static inline NsRule *find_rule(struct nsproxy *nsproxy) {
	NsRule *ptr;

	// look for an exiting active namespace entry in the list
	ptr = head.next;
	while (ptr) {
		if (ptr->active && ptr->nsproxy == nsproxy)
			break;
		ptr = ptr->next;
	}
	return ptr;
}	

// timeout.c
void firejail_timeout(unsigned long dummy);

// split.c
#define CMD_MAX_SIZE 2048
#define SARG_MAX 30
extern int sargc;
extern char *sargv[SARG_MAX];
extern void split_command(char *cmd);

// syscall.c
void syscall_probe_open(struct pt_regs *regs, long id);
void syscall_probe_socket(struct pt_regs *regs, long id);
void syscall_probe_connect(struct pt_regs *regs, long id, NsRule *rule);

// uptime.c
extern const struct file_operations uptime_fops;

// utils.c
struct task_struct *get_sandbox(void);
NsRule *find_or_create_rule(void);

#endif