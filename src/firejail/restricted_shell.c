#include "firejail.h"

void restricted_shell(const char *parent, const char *user) {
	assert(parent);
	assert(user);


	char *arg[4];
	arg[0] = "firejail";
	arg[1] = "--net=none";
	arg[2] = NULL;
	execvp("firejail", arg); 
}

