#include "firejail.h"

static void free_nsrule(NsRule *ptr, int free_locals) {
	int i;
	
	if (free_locals) { // free local mallocs
		for (i = 0; i < MAX_UNIX_PATH; i++) {
			if (ptr->unix_path[i])
				kfree(ptr->unix_path[i]);
		}
	}
	
	kfree(ptr);
}

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
	NsRule *tmpptr;
	static int cnt = 0;
	int inactive = 0;
	
	// decrement trace
	if (--trace_cnt < 0)
		trace_cnt = 0;

	// walk the rules list and disable rules if no process left in the namespace
	while (ptr) {
		if (ptr->active && find_pid(ptr->sandbox_pid) == NULL) {
			printk(KERN_INFO "firejail[%u]: release sandbox.\n", ptr->sandbox_pid);
			ptr->active = 0;
		}
		if (!ptr->active)
			inactive++;
		ptr = ptr->next;
	}

	// rules cleanup
	if (++cnt >= CLEANUP_CNT) {
		cnt = 0;

		// free the temporary list
		if (tmp_head.next) {
			tmpptr = tmp_head.next;
			while (ptr) {
				NsRule *next = tmpptr->next;
				free_nsrule(tmpptr, 1);
				tmpptr = next;
			}
			tmp_head.next = NULL;
		}
		
		// free the old list in tmp_head
		// create the new list in tmp_head; take only the active rules
		// switch head and tmp_head
		if (inactive) {
			printk(KERN_INFO "firejail: cleanup\n");
			ptr = head.next;
			while (ptr) {
				if (ptr->active) {
					int i;
					
					// duplicate rule
					NsRule *newrule = kmalloc(sizeof(NsRule), GFP_KERNEL);
					memset(newrule, 0, sizeof(NsRule));
					newrule->nsproxy = ptr->nsproxy;
					newrule->active = 1;
					newrule->sandbox_pid = ptr->sandbox_pid;
				
					for (i = 0; i < MAX_UNIX_PATH; i++) {
						newrule->unix_path[i] = ptr->unix_path[i];
						newrule->unix_path_len[i] = ptr->unix_path_len[i];
					}
					
					// insert new rule in tmp_head
					newrule->next = tmp_head.next;
					tmp_head.next = newrule;
				}
				
				ptr = ptr->next;
			}
			
			// switch head
			spin_lock(&head_lock);
			ptr = head.next;
			head.next = tmp_head.next;
			tmp_head.next = ptr;
			spin_unlock(&head_lock);
		}
	}
	
	// restart timer
	mod_timer(&rate_timer, jiffies + HZ);
}
