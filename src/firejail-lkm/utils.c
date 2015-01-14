#include "firejail.h"


struct task_struct *get_sandbox(void) {
	struct task_struct *rv;
	rv = current->real_parent;
	while (rv && rv->nsproxy == current->nsproxy)
		rv = rv->real_parent;
	return rv;
}

// return NULL if error (such as path limit reached)
NsRule *find_or_create_rule(void) {
	struct task_struct *real_parent;

	// look for an exiting active namespace entry in the list
	NsRule *ptr = find_rule(current->nsproxy);
	if (ptr)
		return ptr;
	
	// find the sandbox task
	real_parent = get_sandbox();

	// reuse an inactive entry
	ptr = head.next;
	while (ptr) {
		if (ptr->active == 0)
			break;
		ptr = ptr->next;
	}
	if (ptr) { // found an inactive entry
		int i;
		
		// clean unix path
		for (i = 0; i < MAX_UNIX_PATH; i++) {
			ptr->unix_path_len[i] = 0;
			if (ptr->unix_path[i]) {
				char *tmp = ptr->unix_path[i];
				ptr->unix_path[i] = NULL;
				kfree(tmp);
			}
		}
		
		// set the rule
		ptr->nsproxy = current->nsproxy;
		ptr->sandbox_pid = real_parent->pid;
		ptr->active = 1;
		printk(KERN_INFO "firejail[%u]: setup sandbox.\n", ptr->sandbox_pid);
		return ptr;
	}
	
	// allocate a new rule
	ptr = kmalloc(sizeof(NsRule), GFP_KERNEL);
	memset(ptr, 0, sizeof(NsRule));
	ptr->nsproxy = current->nsproxy;
	ptr->sandbox_pid = real_parent->pid;

	// insert the new rule in the rule list
	ptr->next = head.next;
	head.next = ptr;
	ptr->active = 1;
	printk(KERN_INFO "firejail[%u]: setup sandbox.\n", ptr->sandbox_pid);
	return ptr;
}
