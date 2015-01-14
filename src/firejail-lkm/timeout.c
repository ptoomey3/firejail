#include "firejail.h"

static struct task_struct *find_pid(pid_t pid) {
	struct task_struct *p;
	for_each_process(p) {
		if (p->pid == pid)
			return p;
	}
	
	return NULL;
}

//*****************************************************
// 1 second timer
//*****************************************************
void firejail_timeout(unsigned long dummy) {
	NsRule *ptr = head.next;
	
	// decrement trace
	if (--trace_cnt < 0)
		trace_cnt = 0;

	spin_lock(&head_lock);
	// walk the rules list and disable rules if no process left in the namespace
	while (ptr) {
		if (ptr->active && find_pid(ptr->sandbox_pid) == NULL) {
			printk(KERN_INFO "firejail[%u]: release sandbox.\n", ptr->sandbox_pid);
			ptr->active = 0;
		}
			
		ptr = ptr->next;
	}
	spin_unlock(&head_lock);
	
	// restart timer
	mod_timer(&rate_timer, jiffies + HZ);
}
