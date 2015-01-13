#include "firejail.h"


static int uptime_show(struct seq_file *m, void *v) {
	NsRule *ptr = find_rule(current->nsproxy);
	if (ptr) {
		unsigned long long delta;
		struct timespec uptime;
		get_monotonic_boottime(&uptime);
		
		delta =  (unsigned long long) uptime.tv_sec - (unsigned long long) ptr->real_start_time.tv_sec;
		seq_printf(m, "%llu 0\n", delta);
	}
	else
		seq_printf(m, "0 0\n");
	return 0;
}

static int uptime_open(struct inode *inode, struct file *file) {
	return single_open(file, uptime_show, NULL);
}


const struct file_operations uptime_fops = {
	.owner     = THIS_MODULE,
	.open      = uptime_open,
	.read      = seq_read,
	.llseek    = seq_lseek,
	.release   = single_release,
};
