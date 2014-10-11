#include <sys/time.h>
#include <sys/resource.h>
#include "firejail.h"

void set_rlimits(void) {
	// resource limits
	struct rlimit rl;
	if (arg_rlimit_nofile) {
		rl.rlim_cur = (rlim_t) cfg.rlimit_nofile;
		rl.rlim_max = (rlim_t) cfg.rlimit_nofile;
		if (setrlimit(RLIMIT_NOFILE, &rl) == -1)
			errExit("setrlimit");
		if (arg_debug)
			printf("Config rlimit: number of open file descriptors %u\n", cfg.rlimit_nofile);
	}

	if (arg_rlimit_nproc) {
		rl.rlim_cur = (rlim_t) cfg.rlimit_nproc;
		rl.rlim_max = (rlim_t) cfg.rlimit_nproc;
		if (setrlimit(RLIMIT_NPROC, &rl) == -1)
			errExit("setrlimit");
		if (arg_debug)
			printf("Config rlimit: number of processes %u\n", cfg.rlimit_nproc);
	}
	
	if (arg_rlimit_fsize) {
		rl.rlim_cur = (rlim_t) cfg.rlimit_fsize;
		rl.rlim_max = (rlim_t) cfg.rlimit_fsize;
		if (setrlimit(RLIMIT_FSIZE, &rl) == -1)
			errExit("setrlimit");
		if (arg_debug)
			printf("Config rlimit: maximum file size %u\n", cfg.rlimit_fsize);
	}
	
	if (arg_rlimit_sigpending) {
		rl.rlim_cur = (rlim_t) cfg.rlimit_sigpending;
		rl.rlim_max = (rlim_t) cfg.rlimit_sigpending;
		if (setrlimit(RLIMIT_SIGPENDING, &rl) == -1)
			errExit("setrlimit");
		if (arg_debug)
			printf("Config rlimit: maximum number of signals pending %u\n", cfg.rlimit_sigpending);
	}
}
