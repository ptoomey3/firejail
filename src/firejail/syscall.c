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

#ifdef HAVE_SECCOMP
#include <sys/syscall.h>
#include <string.h>
#include "firejail.h"

typedef struct {
	char *name;
	int nr;
} SyscallEntry;

static SyscallEntry syslist[] = {
//
// code generated using tools/extract-syscall
//
#ifndef _SYSCALL_H
#endif
#if !defined __x86_64__
#ifdef SYS__llseek
	{"_llseek", __NR__llseek},
#endif
#ifdef SYS__newselect
	{"_newselect", __NR__newselect},
#endif
#ifdef SYS__sysctl
	{"_sysctl", __NR__sysctl},
#endif
#ifdef SYS_access
	{"access", __NR_access},
#endif
#ifdef SYS_acct
	{"acct", __NR_acct},
#endif
#ifdef SYS_add_key
	{"add_key", __NR_add_key},
#endif
#ifdef SYS_adjtimex
	{"adjtimex", __NR_adjtimex},
#endif
#ifdef SYS_afs_syscall
	{"afs_syscall", __NR_afs_syscall},
#endif
#ifdef SYS_alarm
	{"alarm", __NR_alarm},
#endif
#ifdef SYS_bdflush
	{"bdflush", __NR_bdflush},
#endif
#ifdef SYS_break
	{"break", __NR_break},
#endif
#ifdef SYS_brk
	{"brk", __NR_brk},
#endif
#ifdef SYS_capget
	{"capget", __NR_capget},
#endif
#ifdef SYS_capset
	{"capset", __NR_capset},
#endif
#ifdef SYS_chdir
	{"chdir", __NR_chdir},
#endif
#ifdef SYS_chmod
	{"chmod", __NR_chmod},
#endif
#ifdef SYS_chown
	{"chown", __NR_chown},
#endif
#ifdef SYS_chown32
	{"chown32", __NR_chown32},
#endif
#ifdef SYS_chroot
	{"chroot", __NR_chroot},
#endif
#ifdef SYS_clock_adjtime
	{"clock_adjtime", __NR_clock_adjtime},
#endif
#ifdef SYS_clock_getres
	{"clock_getres", __NR_clock_getres},
#endif
#ifdef SYS_clock_gettime
	{"clock_gettime", __NR_clock_gettime},
#endif
#ifdef SYS_clock_nanosleep
	{"clock_nanosleep", __NR_clock_nanosleep},
#endif
#ifdef SYS_clock_settime
	{"clock_settime", __NR_clock_settime},
#endif
#ifdef SYS_clone
	{"clone", __NR_clone},
#endif
#ifdef SYS_close
	{"close", __NR_close},
#endif
#ifdef SYS_creat
	{"creat", __NR_creat},
#endif
#ifdef SYS_create_module
	{"create_module", __NR_create_module},
#endif
#ifdef SYS_delete_module
	{"delete_module", __NR_delete_module},
#endif
#ifdef SYS_dup
	{"dup", __NR_dup},
#endif
#ifdef SYS_dup2
	{"dup2", __NR_dup2},
#endif
#ifdef SYS_dup3
	{"dup3", __NR_dup3},
#endif
#ifdef SYS_epoll_create
	{"epoll_create", __NR_epoll_create},
#endif
#ifdef SYS_epoll_create1
	{"epoll_create1", __NR_epoll_create1},
#endif
#ifdef SYS_epoll_ctl
	{"epoll_ctl", __NR_epoll_ctl},
#endif
#ifdef SYS_epoll_pwait
	{"epoll_pwait", __NR_epoll_pwait},
#endif
#ifdef SYS_epoll_wait
	{"epoll_wait", __NR_epoll_wait},
#endif
#ifdef SYS_eventfd
	{"eventfd", __NR_eventfd},
#endif
#ifdef SYS_eventfd2
	{"eventfd2", __NR_eventfd2},
#endif
#ifdef SYS_execve
	{"execve", __NR_execve},
#endif
#ifdef SYS_exit
	{"exit", __NR_exit},
#endif
#ifdef SYS_exit_group
	{"exit_group", __NR_exit_group},
#endif
#ifdef SYS_faccessat
	{"faccessat", __NR_faccessat},
#endif
#ifdef SYS_fadvise64
	{"fadvise64", __NR_fadvise64},
#endif
#ifdef SYS_fadvise64_64
	{"fadvise64_64", __NR_fadvise64_64},
#endif
#ifdef SYS_fallocate
	{"fallocate", __NR_fallocate},
#endif
#ifdef SYS_fanotify_init
	{"fanotify_init", __NR_fanotify_init},
#endif
#ifdef SYS_fanotify_mark
	{"fanotify_mark", __NR_fanotify_mark},
#endif
#ifdef SYS_fchdir
	{"fchdir", __NR_fchdir},
#endif
#ifdef SYS_fchmod
	{"fchmod", __NR_fchmod},
#endif
#ifdef SYS_fchmodat
	{"fchmodat", __NR_fchmodat},
#endif
#ifdef SYS_fchown
	{"fchown", __NR_fchown},
#endif
#ifdef SYS_fchown32
	{"fchown32", __NR_fchown32},
#endif
#ifdef SYS_fchownat
	{"fchownat", __NR_fchownat},
#endif
#ifdef SYS_fcntl
	{"fcntl", __NR_fcntl},
#endif
#ifdef SYS_fcntl64
	{"fcntl64", __NR_fcntl64},
#endif
#ifdef SYS_fdatasync
	{"fdatasync", __NR_fdatasync},
#endif
#ifdef SYS_fgetxattr
	{"fgetxattr", __NR_fgetxattr},
#endif
#ifdef SYS_finit_module
	{"finit_module", __NR_finit_module},
#endif
#ifdef SYS_flistxattr
	{"flistxattr", __NR_flistxattr},
#endif
#ifdef SYS_flock
	{"flock", __NR_flock},
#endif
#ifdef SYS_fork
	{"fork", __NR_fork},
#endif
#ifdef SYS_fremovexattr
	{"fremovexattr", __NR_fremovexattr},
#endif
#ifdef SYS_fsetxattr
	{"fsetxattr", __NR_fsetxattr},
#endif
#ifdef SYS_fstat
	{"fstat", __NR_fstat},
#endif
#ifdef SYS_fstat64
	{"fstat64", __NR_fstat64},
#endif
#ifdef SYS_fstatat64
	{"fstatat64", __NR_fstatat64},
#endif
#ifdef SYS_fstatfs
	{"fstatfs", __NR_fstatfs},
#endif
#ifdef SYS_fstatfs64
	{"fstatfs64", __NR_fstatfs64},
#endif
#ifdef SYS_fsync
	{"fsync", __NR_fsync},
#endif
#ifdef SYS_ftime
	{"ftime", __NR_ftime},
#endif
#ifdef SYS_ftruncate
	{"ftruncate", __NR_ftruncate},
#endif
#ifdef SYS_ftruncate64
	{"ftruncate64", __NR_ftruncate64},
#endif
#ifdef SYS_futex
	{"futex", __NR_futex},
#endif
#ifdef SYS_futimesat
	{"futimesat", __NR_futimesat},
#endif
#ifdef SYS_get_kernel_syms
	{"get_kernel_syms", __NR_get_kernel_syms},
#endif
#ifdef SYS_get_mempolicy
	{"get_mempolicy", __NR_get_mempolicy},
#endif
#ifdef SYS_get_robust_list
	{"get_robust_list", __NR_get_robust_list},
#endif
#ifdef SYS_get_thread_area
	{"get_thread_area", __NR_get_thread_area},
#endif
#ifdef SYS_getcpu
	{"getcpu", __NR_getcpu},
#endif
#ifdef SYS_getcwd
	{"getcwd", __NR_getcwd},
#endif
#ifdef SYS_getdents
	{"getdents", __NR_getdents},
#endif
#ifdef SYS_getdents64
	{"getdents64", __NR_getdents64},
#endif
#ifdef SYS_getegid
	{"getegid", __NR_getegid},
#endif
#ifdef SYS_getegid32
	{"getegid32", __NR_getegid32},
#endif
#ifdef SYS_geteuid
	{"geteuid", __NR_geteuid},
#endif
#ifdef SYS_geteuid32
	{"geteuid32", __NR_geteuid32},
#endif
#ifdef SYS_getgid
	{"getgid", __NR_getgid},
#endif
#ifdef SYS_getgid32
	{"getgid32", __NR_getgid32},
#endif
#ifdef SYS_getgroups
	{"getgroups", __NR_getgroups},
#endif
#ifdef SYS_getgroups32
	{"getgroups32", __NR_getgroups32},
#endif
#ifdef SYS_getitimer
	{"getitimer", __NR_getitimer},
#endif
#ifdef SYS_getpgid
	{"getpgid", __NR_getpgid},
#endif
#ifdef SYS_getpgrp
	{"getpgrp", __NR_getpgrp},
#endif
#ifdef SYS_getpid
	{"getpid", __NR_getpid},
#endif
#ifdef SYS_getpmsg
	{"getpmsg", __NR_getpmsg},
#endif
#ifdef SYS_getppid
	{"getppid", __NR_getppid},
#endif
#ifdef SYS_getpriority
	{"getpriority", __NR_getpriority},
#endif
#ifdef SYS_getresgid
	{"getresgid", __NR_getresgid},
#endif
#ifdef SYS_getresgid32
	{"getresgid32", __NR_getresgid32},
#endif
#ifdef SYS_getresuid
	{"getresuid", __NR_getresuid},
#endif
#ifdef SYS_getresuid32
	{"getresuid32", __NR_getresuid32},
#endif
#ifdef SYS_getrlimit
	{"getrlimit", __NR_getrlimit},
#endif
#ifdef SYS_getrusage
	{"getrusage", __NR_getrusage},
#endif
#ifdef SYS_getsid
	{"getsid", __NR_getsid},
#endif
#ifdef SYS_gettid
	{"gettid", __NR_gettid},
#endif
#ifdef SYS_gettimeofday
	{"gettimeofday", __NR_gettimeofday},
#endif
#ifdef SYS_getuid
	{"getuid", __NR_getuid},
#endif
#ifdef SYS_getuid32
	{"getuid32", __NR_getuid32},
#endif
#ifdef SYS_getxattr
	{"getxattr", __NR_getxattr},
#endif
#ifdef SYS_gtty
	{"gtty", __NR_gtty},
#endif
#ifdef SYS_idle
	{"idle", __NR_idle},
#endif
#ifdef SYS_init_module
	{"init_module", __NR_init_module},
#endif
#ifdef SYS_inotify_add_watch
	{"inotify_add_watch", __NR_inotify_add_watch},
#endif
#ifdef SYS_inotify_init
	{"inotify_init", __NR_inotify_init},
#endif
#ifdef SYS_inotify_init1
	{"inotify_init1", __NR_inotify_init1},
#endif
#ifdef SYS_inotify_rm_watch
	{"inotify_rm_watch", __NR_inotify_rm_watch},
#endif
#ifdef SYS_io_cancel
	{"io_cancel", __NR_io_cancel},
#endif
#ifdef SYS_io_destroy
	{"io_destroy", __NR_io_destroy},
#endif
#ifdef SYS_io_getevents
	{"io_getevents", __NR_io_getevents},
#endif
#ifdef SYS_io_setup
	{"io_setup", __NR_io_setup},
#endif
#ifdef SYS_io_submit
	{"io_submit", __NR_io_submit},
#endif
#ifdef SYS_ioctl
	{"ioctl", __NR_ioctl},
#endif
#ifdef SYS_ioperm
	{"ioperm", __NR_ioperm},
#endif
#ifdef SYS_iopl
	{"iopl", __NR_iopl},
#endif
#ifdef SYS_ioprio_get
	{"ioprio_get", __NR_ioprio_get},
#endif
#ifdef SYS_ioprio_set
	{"ioprio_set", __NR_ioprio_set},
#endif
#ifdef SYS_ipc
	{"ipc", __NR_ipc},
#endif
#ifdef SYS_kcmp
	{"kcmp", __NR_kcmp},
#endif
#ifdef SYS_kexec_load
	{"kexec_load", __NR_kexec_load},
#endif
#ifdef SYS_keyctl
	{"keyctl", __NR_keyctl},
#endif
#ifdef SYS_kill
	{"kill", __NR_kill},
#endif
#ifdef SYS_lchown
	{"lchown", __NR_lchown},
#endif
#ifdef SYS_lchown32
	{"lchown32", __NR_lchown32},
#endif
#ifdef SYS_lgetxattr
	{"lgetxattr", __NR_lgetxattr},
#endif
#ifdef SYS_link
	{"link", __NR_link},
#endif
#ifdef SYS_linkat
	{"linkat", __NR_linkat},
#endif
#ifdef SYS_listxattr
	{"listxattr", __NR_listxattr},
#endif
#ifdef SYS_llistxattr
	{"llistxattr", __NR_llistxattr},
#endif
#ifdef SYS_lock
	{"lock", __NR_lock},
#endif
#ifdef SYS_lookup_dcookie
	{"lookup_dcookie", __NR_lookup_dcookie},
#endif
#ifdef SYS_lremovexattr
	{"lremovexattr", __NR_lremovexattr},
#endif
#ifdef SYS_lseek
	{"lseek", __NR_lseek},
#endif
#ifdef SYS_lsetxattr
	{"lsetxattr", __NR_lsetxattr},
#endif
#ifdef SYS_lstat
	{"lstat", __NR_lstat},
#endif
#ifdef SYS_lstat64
	{"lstat64", __NR_lstat64},
#endif
#ifdef SYS_madvise
	{"madvise", __NR_madvise},
#endif
#ifdef SYS_mbind
	{"mbind", __NR_mbind},
#endif
#ifdef SYS_migrate_pages
	{"migrate_pages", __NR_migrate_pages},
#endif
#ifdef SYS_mincore
	{"mincore", __NR_mincore},
#endif
#ifdef SYS_mkdir
	{"mkdir", __NR_mkdir},
#endif
#ifdef SYS_mkdirat
	{"mkdirat", __NR_mkdirat},
#endif
#ifdef SYS_mknod
	{"mknod", __NR_mknod},
#endif
#ifdef SYS_mknodat
	{"mknodat", __NR_mknodat},
#endif
#ifdef SYS_mlock
	{"mlock", __NR_mlock},
#endif
#ifdef SYS_mlockall
	{"mlockall", __NR_mlockall},
#endif
#ifdef SYS_mmap
	{"mmap", __NR_mmap},
#endif
#ifdef SYS_mmap2
	{"mmap2", __NR_mmap2},
#endif
#ifdef SYS_modify_ldt
	{"modify_ldt", __NR_modify_ldt},
#endif
#ifdef SYS_mount
	{"mount", __NR_mount},
#endif
#ifdef SYS_move_pages
	{"move_pages", __NR_move_pages},
#endif
#ifdef SYS_mprotect
	{"mprotect", __NR_mprotect},
#endif
#ifdef SYS_mpx
	{"mpx", __NR_mpx},
#endif
#ifdef SYS_mq_getsetattr
	{"mq_getsetattr", __NR_mq_getsetattr},
#endif
#ifdef SYS_mq_notify
	{"mq_notify", __NR_mq_notify},
#endif
#ifdef SYS_mq_open
	{"mq_open", __NR_mq_open},
#endif
#ifdef SYS_mq_timedreceive
	{"mq_timedreceive", __NR_mq_timedreceive},
#endif
#ifdef SYS_mq_timedsend
	{"mq_timedsend", __NR_mq_timedsend},
#endif
#ifdef SYS_mq_unlink
	{"mq_unlink", __NR_mq_unlink},
#endif
#ifdef SYS_mremap
	{"mremap", __NR_mremap},
#endif
#ifdef SYS_msync
	{"msync", __NR_msync},
#endif
#ifdef SYS_munlock
	{"munlock", __NR_munlock},
#endif
#ifdef SYS_munlockall
	{"munlockall", __NR_munlockall},
#endif
#ifdef SYS_munmap
	{"munmap", __NR_munmap},
#endif
#ifdef SYS_name_to_handle_at
	{"name_to_handle_at", __NR_name_to_handle_at},
#endif
#ifdef SYS_nanosleep
	{"nanosleep", __NR_nanosleep},
#endif
#ifdef SYS_nfsservctl
	{"nfsservctl", __NR_nfsservctl},
#endif
#ifdef SYS_nice
	{"nice", __NR_nice},
#endif
#ifdef SYS_oldfstat
	{"oldfstat", __NR_oldfstat},
#endif
#ifdef SYS_oldlstat
	{"oldlstat", __NR_oldlstat},
#endif
#ifdef SYS_oldolduname
	{"oldolduname", __NR_oldolduname},
#endif
#ifdef SYS_oldstat
	{"oldstat", __NR_oldstat},
#endif
#ifdef SYS_olduname
	{"olduname", __NR_olduname},
#endif
#ifdef SYS_open
	{"open", __NR_open},
#endif
#ifdef SYS_open_by_handle_at
	{"open_by_handle_at", __NR_open_by_handle_at},
#endif
#ifdef SYS_openat
	{"openat", __NR_openat},
#endif
#ifdef SYS_pause
	{"pause", __NR_pause},
#endif
#ifdef SYS_perf_event_open
	{"perf_event_open", __NR_perf_event_open},
#endif
#ifdef SYS_personality
	{"personality", __NR_personality},
#endif
#ifdef SYS_pipe
	{"pipe", __NR_pipe},
#endif
#ifdef SYS_pipe2
	{"pipe2", __NR_pipe2},
#endif
#ifdef SYS_pivot_root
	{"pivot_root", __NR_pivot_root},
#endif
#ifdef SYS_poll
	{"poll", __NR_poll},
#endif
#ifdef SYS_ppoll
	{"ppoll", __NR_ppoll},
#endif
#ifdef SYS_prctl
	{"prctl", __NR_prctl},
#endif
#ifdef SYS_pread64
	{"pread64", __NR_pread64},
#endif
#ifdef SYS_preadv
	{"preadv", __NR_preadv},
#endif
#ifdef SYS_prlimit64
	{"prlimit64", __NR_prlimit64},
#endif
#ifdef SYS_process_vm_readv
	{"process_vm_readv", __NR_process_vm_readv},
#endif
#ifdef SYS_process_vm_writev
	{"process_vm_writev", __NR_process_vm_writev},
#endif
#ifdef SYS_prof
	{"prof", __NR_prof},
#endif
#ifdef SYS_profil
	{"profil", __NR_profil},
#endif
#ifdef SYS_pselect6
	{"pselect6", __NR_pselect6},
#endif
#ifdef SYS_ptrace
	{"ptrace", __NR_ptrace},
#endif
#ifdef SYS_putpmsg
	{"putpmsg", __NR_putpmsg},
#endif
#ifdef SYS_pwrite64
	{"pwrite64", __NR_pwrite64},
#endif
#ifdef SYS_pwritev
	{"pwritev", __NR_pwritev},
#endif
#ifdef SYS_query_module
	{"query_module", __NR_query_module},
#endif
#ifdef SYS_quotactl
	{"quotactl", __NR_quotactl},
#endif
#ifdef SYS_read
	{"read", __NR_read},
#endif
#ifdef SYS_readahead
	{"readahead", __NR_readahead},
#endif
#ifdef SYS_readdir
	{"readdir", __NR_readdir},
#endif
#ifdef SYS_readlink
	{"readlink", __NR_readlink},
#endif
#ifdef SYS_readlinkat
	{"readlinkat", __NR_readlinkat},
#endif
#ifdef SYS_readv
	{"readv", __NR_readv},
#endif
#ifdef SYS_reboot
	{"reboot", __NR_reboot},
#endif
#ifdef SYS_recvmmsg
	{"recvmmsg", __NR_recvmmsg},
#endif
#ifdef SYS_remap_file_pages
	{"remap_file_pages", __NR_remap_file_pages},
#endif
#ifdef SYS_removexattr
	{"removexattr", __NR_removexattr},
#endif
#ifdef SYS_rename
	{"rename", __NR_rename},
#endif
#ifdef SYS_renameat
	{"renameat", __NR_renameat},
#endif
#ifdef SYS_request_key
	{"request_key", __NR_request_key},
#endif
#ifdef SYS_restart_syscall
	{"restart_syscall", __NR_restart_syscall},
#endif
#ifdef SYS_rmdir
	{"rmdir", __NR_rmdir},
#endif
#ifdef SYS_rt_sigaction
	{"rt_sigaction", __NR_rt_sigaction},
#endif
#ifdef SYS_rt_sigpending
	{"rt_sigpending", __NR_rt_sigpending},
#endif
#ifdef SYS_rt_sigprocmask
	{"rt_sigprocmask", __NR_rt_sigprocmask},
#endif
#ifdef SYS_rt_sigqueueinfo
	{"rt_sigqueueinfo", __NR_rt_sigqueueinfo},
#endif
#ifdef SYS_rt_sigreturn
	{"rt_sigreturn", __NR_rt_sigreturn},
#endif
#ifdef SYS_rt_sigsuspend
	{"rt_sigsuspend", __NR_rt_sigsuspend},
#endif
#ifdef SYS_rt_sigtimedwait
	{"rt_sigtimedwait", __NR_rt_sigtimedwait},
#endif
#ifdef SYS_rt_tgsigqueueinfo
	{"rt_tgsigqueueinfo", __NR_rt_tgsigqueueinfo},
#endif
#ifdef SYS_sched_get_priority_max
	{"sched_get_priority_max", __NR_sched_get_priority_max},
#endif
#ifdef SYS_sched_get_priority_min
	{"sched_get_priority_min", __NR_sched_get_priority_min},
#endif
#ifdef SYS_sched_getaffinity
	{"sched_getaffinity", __NR_sched_getaffinity},
#endif
#ifdef SYS_sched_getparam
	{"sched_getparam", __NR_sched_getparam},
#endif
#ifdef SYS_sched_getscheduler
	{"sched_getscheduler", __NR_sched_getscheduler},
#endif
#ifdef SYS_sched_rr_get_interval
	{"sched_rr_get_interval", __NR_sched_rr_get_interval},
#endif
#ifdef SYS_sched_setaffinity
	{"sched_setaffinity", __NR_sched_setaffinity},
#endif
#ifdef SYS_sched_setparam
	{"sched_setparam", __NR_sched_setparam},
#endif
#ifdef SYS_sched_setscheduler
	{"sched_setscheduler", __NR_sched_setscheduler},
#endif
#ifdef SYS_sched_yield
	{"sched_yield", __NR_sched_yield},
#endif
#ifdef SYS_select
	{"select", __NR_select},
#endif
#ifdef SYS_sendfile
	{"sendfile", __NR_sendfile},
#endif
#ifdef SYS_sendfile64
	{"sendfile64", __NR_sendfile64},
#endif
#ifdef SYS_sendmmsg
	{"sendmmsg", __NR_sendmmsg},
#endif
#ifdef SYS_set_mempolicy
	{"set_mempolicy", __NR_set_mempolicy},
#endif
#ifdef SYS_set_robust_list
	{"set_robust_list", __NR_set_robust_list},
#endif
#ifdef SYS_set_thread_area
	{"set_thread_area", __NR_set_thread_area},
#endif
#ifdef SYS_set_tid_address
	{"set_tid_address", __NR_set_tid_address},
#endif
#ifdef SYS_setdomainname
	{"setdomainname", __NR_setdomainname},
#endif
#ifdef SYS_setfsgid
	{"setfsgid", __NR_setfsgid},
#endif
#ifdef SYS_setfsgid32
	{"setfsgid32", __NR_setfsgid32},
#endif
#ifdef SYS_setfsuid
	{"setfsuid", __NR_setfsuid},
#endif
#ifdef SYS_setfsuid32
	{"setfsuid32", __NR_setfsuid32},
#endif
#ifdef SYS_setgid
	{"setgid", __NR_setgid},
#endif
#ifdef SYS_setgid32
	{"setgid32", __NR_setgid32},
#endif
#ifdef SYS_setgroups
	{"setgroups", __NR_setgroups},
#endif
#ifdef SYS_setgroups32
	{"setgroups32", __NR_setgroups32},
#endif
#ifdef SYS_sethostname
	{"sethostname", __NR_sethostname},
#endif
#ifdef SYS_setitimer
	{"setitimer", __NR_setitimer},
#endif
#ifdef SYS_setns
	{"setns", __NR_setns},
#endif
#ifdef SYS_setpgid
	{"setpgid", __NR_setpgid},
#endif
#ifdef SYS_setpriority
	{"setpriority", __NR_setpriority},
#endif
#ifdef SYS_setregid
	{"setregid", __NR_setregid},
#endif
#ifdef SYS_setregid32
	{"setregid32", __NR_setregid32},
#endif
#ifdef SYS_setresgid
	{"setresgid", __NR_setresgid},
#endif
#ifdef SYS_setresgid32
	{"setresgid32", __NR_setresgid32},
#endif
#ifdef SYS_setresuid
	{"setresuid", __NR_setresuid},
#endif
#ifdef SYS_setresuid32
	{"setresuid32", __NR_setresuid32},
#endif
#ifdef SYS_setreuid
	{"setreuid", __NR_setreuid},
#endif
#ifdef SYS_setreuid32
	{"setreuid32", __NR_setreuid32},
#endif
#ifdef SYS_setrlimit
	{"setrlimit", __NR_setrlimit},
#endif
#ifdef SYS_setsid
	{"setsid", __NR_setsid},
#endif
#ifdef SYS_settimeofday
	{"settimeofday", __NR_settimeofday},
#endif
#ifdef SYS_setuid
	{"setuid", __NR_setuid},
#endif
#ifdef SYS_setuid32
	{"setuid32", __NR_setuid32},
#endif
#ifdef SYS_setxattr
	{"setxattr", __NR_setxattr},
#endif
#ifdef SYS_sgetmask
	{"sgetmask", __NR_sgetmask},
#endif
#ifdef SYS_sigaction
	{"sigaction", __NR_sigaction},
#endif
#ifdef SYS_sigaltstack
	{"sigaltstack", __NR_sigaltstack},
#endif
#ifdef SYS_signal
	{"signal", __NR_signal},
#endif
#ifdef SYS_signalfd
	{"signalfd", __NR_signalfd},
#endif
#ifdef SYS_signalfd4
	{"signalfd4", __NR_signalfd4},
#endif
#ifdef SYS_sigpending
	{"sigpending", __NR_sigpending},
#endif
#ifdef SYS_sigprocmask
	{"sigprocmask", __NR_sigprocmask},
#endif
#ifdef SYS_sigreturn
	{"sigreturn", __NR_sigreturn},
#endif
#ifdef SYS_sigsuspend
	{"sigsuspend", __NR_sigsuspend},
#endif
#ifdef SYS_socketcall
	{"socketcall", __NR_socketcall},
#endif
#ifdef SYS_splice
	{"splice", __NR_splice},
#endif
#ifdef SYS_ssetmask
	{"ssetmask", __NR_ssetmask},
#endif
#ifdef SYS_stat
	{"stat", __NR_stat},
#endif
#ifdef SYS_stat64
	{"stat64", __NR_stat64},
#endif
#ifdef SYS_statfs
	{"statfs", __NR_statfs},
#endif
#ifdef SYS_statfs64
	{"statfs64", __NR_statfs64},
#endif
#ifdef SYS_stime
	{"stime", __NR_stime},
#endif
#ifdef SYS_stty
	{"stty", __NR_stty},
#endif
#ifdef SYS_swapoff
	{"swapoff", __NR_swapoff},
#endif
#ifdef SYS_swapon
	{"swapon", __NR_swapon},
#endif
#ifdef SYS_symlink
	{"symlink", __NR_symlink},
#endif
#ifdef SYS_symlinkat
	{"symlinkat", __NR_symlinkat},
#endif
#ifdef SYS_sync
	{"sync", __NR_sync},
#endif
#ifdef SYS_sync_file_range
	{"sync_file_range", __NR_sync_file_range},
#endif
#ifdef SYS_syncfs
	{"syncfs", __NR_syncfs},
#endif
#ifdef SYS_sysfs
	{"sysfs", __NR_sysfs},
#endif
#ifdef SYS_sysinfo
	{"sysinfo", __NR_sysinfo},
#endif
#ifdef SYS_syslog
	{"syslog", __NR_syslog},
#endif
#ifdef SYS_tee
	{"tee", __NR_tee},
#endif
#ifdef SYS_tgkill
	{"tgkill", __NR_tgkill},
#endif
#ifdef SYS_time
	{"time", __NR_time},
#endif
#ifdef SYS_timer_create
	{"timer_create", __NR_timer_create},
#endif
#ifdef SYS_timer_delete
	{"timer_delete", __NR_timer_delete},
#endif
#ifdef SYS_timer_getoverrun
	{"timer_getoverrun", __NR_timer_getoverrun},
#endif
#ifdef SYS_timer_gettime
	{"timer_gettime", __NR_timer_gettime},
#endif
#ifdef SYS_timer_settime
	{"timer_settime", __NR_timer_settime},
#endif
#ifdef SYS_timerfd_create
	{"timerfd_create", __NR_timerfd_create},
#endif
#ifdef SYS_timerfd_gettime
	{"timerfd_gettime", __NR_timerfd_gettime},
#endif
#ifdef SYS_timerfd_settime
	{"timerfd_settime", __NR_timerfd_settime},
#endif
#ifdef SYS_times
	{"times", __NR_times},
#endif
#ifdef SYS_tkill
	{"tkill", __NR_tkill},
#endif
#ifdef SYS_truncate
	{"truncate", __NR_truncate},
#endif
#ifdef SYS_truncate64
	{"truncate64", __NR_truncate64},
#endif
#ifdef SYS_ugetrlimit
	{"ugetrlimit", __NR_ugetrlimit},
#endif
#ifdef SYS_ulimit
	{"ulimit", __NR_ulimit},
#endif
#ifdef SYS_umask
	{"umask", __NR_umask},
#endif
#ifdef SYS_umount
	{"umount", __NR_umount},
#endif
#ifdef SYS_umount2
	{"umount2", __NR_umount2},
#endif
#ifdef SYS_uname
	{"uname", __NR_uname},
#endif
#ifdef SYS_unlink
	{"unlink", __NR_unlink},
#endif
#ifdef SYS_unlinkat
	{"unlinkat", __NR_unlinkat},
#endif
#ifdef SYS_unshare
	{"unshare", __NR_unshare},
#endif
#ifdef SYS_uselib
	{"uselib", __NR_uselib},
#endif
#ifdef SYS_ustat
	{"ustat", __NR_ustat},
#endif
#ifdef SYS_utime
	{"utime", __NR_utime},
#endif
#ifdef SYS_utimensat
	{"utimensat", __NR_utimensat},
#endif
#ifdef SYS_utimes
	{"utimes", __NR_utimes},
#endif
#ifdef SYS_vfork
	{"vfork", __NR_vfork},
#endif
#ifdef SYS_vhangup
	{"vhangup", __NR_vhangup},
#endif
#ifdef SYS_vm86
	{"vm86", __NR_vm86},
#endif
#ifdef SYS_vm86old
	{"vm86old", __NR_vm86old},
#endif
#ifdef SYS_vmsplice
	{"vmsplice", __NR_vmsplice},
#endif
#ifdef SYS_vserver
	{"vserver", __NR_vserver},
#endif
#ifdef SYS_wait4
	{"wait4", __NR_wait4},
#endif
#ifdef SYS_waitid
	{"waitid", __NR_waitid},
#endif
#ifdef SYS_waitpid
	{"waitpid", __NR_waitpid},
#endif
#ifdef SYS_write
	{"write", __NR_write},
#endif
#ifdef SYS_writev
	{"writev", __NR_writev},
#endif
#endif
#if defined __x86_64__ && defined __LP64__
#ifdef SYS__sysctl
	{"_sysctl", __NR__sysctl},
#endif
#ifdef SYS_accept
	{"accept", __NR_accept},
#endif
#ifdef SYS_accept4
	{"accept4", __NR_accept4},
#endif
#ifdef SYS_access
	{"access", __NR_access},
#endif
#ifdef SYS_acct
	{"acct", __NR_acct},
#endif
#ifdef SYS_add_key
	{"add_key", __NR_add_key},
#endif
#ifdef SYS_adjtimex
	{"adjtimex", __NR_adjtimex},
#endif
#ifdef SYS_afs_syscall
	{"afs_syscall", __NR_afs_syscall},
#endif
#ifdef SYS_alarm
	{"alarm", __NR_alarm},
#endif
#ifdef SYS_arch_prctl
	{"arch_prctl", __NR_arch_prctl},
#endif
#ifdef SYS_bind
	{"bind", __NR_bind},
#endif
#ifdef SYS_brk
	{"brk", __NR_brk},
#endif
#ifdef SYS_capget
	{"capget", __NR_capget},
#endif
#ifdef SYS_capset
	{"capset", __NR_capset},
#endif
#ifdef SYS_chdir
	{"chdir", __NR_chdir},
#endif
#ifdef SYS_chmod
	{"chmod", __NR_chmod},
#endif
#ifdef SYS_chown
	{"chown", __NR_chown},
#endif
#ifdef SYS_chroot
	{"chroot", __NR_chroot},
#endif
#ifdef SYS_clock_adjtime
	{"clock_adjtime", __NR_clock_adjtime},
#endif
#ifdef SYS_clock_getres
	{"clock_getres", __NR_clock_getres},
#endif
#ifdef SYS_clock_gettime
	{"clock_gettime", __NR_clock_gettime},
#endif
#ifdef SYS_clock_nanosleep
	{"clock_nanosleep", __NR_clock_nanosleep},
#endif
#ifdef SYS_clock_settime
	{"clock_settime", __NR_clock_settime},
#endif
#ifdef SYS_clone
	{"clone", __NR_clone},
#endif
#ifdef SYS_close
	{"close", __NR_close},
#endif
#ifdef SYS_connect
	{"connect", __NR_connect},
#endif
#ifdef SYS_creat
	{"creat", __NR_creat},
#endif
#ifdef SYS_create_module
	{"create_module", __NR_create_module},
#endif
#ifdef SYS_delete_module
	{"delete_module", __NR_delete_module},
#endif
#ifdef SYS_dup
	{"dup", __NR_dup},
#endif
#ifdef SYS_dup2
	{"dup2", __NR_dup2},
#endif
#ifdef SYS_dup3
	{"dup3", __NR_dup3},
#endif
#ifdef SYS_epoll_create
	{"epoll_create", __NR_epoll_create},
#endif
#ifdef SYS_epoll_create1
	{"epoll_create1", __NR_epoll_create1},
#endif
#ifdef SYS_epoll_ctl
	{"epoll_ctl", __NR_epoll_ctl},
#endif
#ifdef SYS_epoll_ctl_old
	{"epoll_ctl_old", __NR_epoll_ctl_old},
#endif
#ifdef SYS_epoll_pwait
	{"epoll_pwait", __NR_epoll_pwait},
#endif
#ifdef SYS_epoll_wait
	{"epoll_wait", __NR_epoll_wait},
#endif
#ifdef SYS_epoll_wait_old
	{"epoll_wait_old", __NR_epoll_wait_old},
#endif
#ifdef SYS_eventfd
	{"eventfd", __NR_eventfd},
#endif
#ifdef SYS_eventfd2
	{"eventfd2", __NR_eventfd2},
#endif
#ifdef SYS_execve
	{"execve", __NR_execve},
#endif
#ifdef SYS_exit
	{"exit", __NR_exit},
#endif
#ifdef SYS_exit_group
	{"exit_group", __NR_exit_group},
#endif
#ifdef SYS_faccessat
	{"faccessat", __NR_faccessat},
#endif
#ifdef SYS_fadvise64
	{"fadvise64", __NR_fadvise64},
#endif
#ifdef SYS_fallocate
	{"fallocate", __NR_fallocate},
#endif
#ifdef SYS_fanotify_init
	{"fanotify_init", __NR_fanotify_init},
#endif
#ifdef SYS_fanotify_mark
	{"fanotify_mark", __NR_fanotify_mark},
#endif
#ifdef SYS_fchdir
	{"fchdir", __NR_fchdir},
#endif
#ifdef SYS_fchmod
	{"fchmod", __NR_fchmod},
#endif
#ifdef SYS_fchmodat
	{"fchmodat", __NR_fchmodat},
#endif
#ifdef SYS_fchown
	{"fchown", __NR_fchown},
#endif
#ifdef SYS_fchownat
	{"fchownat", __NR_fchownat},
#endif
#ifdef SYS_fcntl
	{"fcntl", __NR_fcntl},
#endif
#ifdef SYS_fdatasync
	{"fdatasync", __NR_fdatasync},
#endif
#ifdef SYS_fgetxattr
	{"fgetxattr", __NR_fgetxattr},
#endif
#ifdef SYS_finit_module
	{"finit_module", __NR_finit_module},
#endif
#ifdef SYS_flistxattr
	{"flistxattr", __NR_flistxattr},
#endif
#ifdef SYS_flock
	{"flock", __NR_flock},
#endif
#ifdef SYS_fork
	{"fork", __NR_fork},
#endif
#ifdef SYS_fremovexattr
	{"fremovexattr", __NR_fremovexattr},
#endif
#ifdef SYS_fsetxattr
	{"fsetxattr", __NR_fsetxattr},
#endif
#ifdef SYS_fstat
	{"fstat", __NR_fstat},
#endif
#ifdef SYS_fstatfs
	{"fstatfs", __NR_fstatfs},
#endif
#ifdef SYS_fsync
	{"fsync", __NR_fsync},
#endif
#ifdef SYS_ftruncate
	{"ftruncate", __NR_ftruncate},
#endif
#ifdef SYS_futex
	{"futex", __NR_futex},
#endif
#ifdef SYS_futimesat
	{"futimesat", __NR_futimesat},
#endif
#ifdef SYS_get_kernel_syms
	{"get_kernel_syms", __NR_get_kernel_syms},
#endif
#ifdef SYS_get_mempolicy
	{"get_mempolicy", __NR_get_mempolicy},
#endif
#ifdef SYS_get_robust_list
	{"get_robust_list", __NR_get_robust_list},
#endif
#ifdef SYS_get_thread_area
	{"get_thread_area", __NR_get_thread_area},
#endif
#ifdef SYS_getcpu
	{"getcpu", __NR_getcpu},
#endif
#ifdef SYS_getcwd
	{"getcwd", __NR_getcwd},
#endif
#ifdef SYS_getdents
	{"getdents", __NR_getdents},
#endif
#ifdef SYS_getdents64
	{"getdents64", __NR_getdents64},
#endif
#ifdef SYS_getegid
	{"getegid", __NR_getegid},
#endif
#ifdef SYS_geteuid
	{"geteuid", __NR_geteuid},
#endif
#ifdef SYS_getgid
	{"getgid", __NR_getgid},
#endif
#ifdef SYS_getgroups
	{"getgroups", __NR_getgroups},
#endif
#ifdef SYS_getitimer
	{"getitimer", __NR_getitimer},
#endif
#ifdef SYS_getpeername
	{"getpeername", __NR_getpeername},
#endif
#ifdef SYS_getpgid
	{"getpgid", __NR_getpgid},
#endif
#ifdef SYS_getpgrp
	{"getpgrp", __NR_getpgrp},
#endif
#ifdef SYS_getpid
	{"getpid", __NR_getpid},
#endif
#ifdef SYS_getpmsg
	{"getpmsg", __NR_getpmsg},
#endif
#ifdef SYS_getppid
	{"getppid", __NR_getppid},
#endif
#ifdef SYS_getpriority
	{"getpriority", __NR_getpriority},
#endif
#ifdef SYS_getresgid
	{"getresgid", __NR_getresgid},
#endif
#ifdef SYS_getresuid
	{"getresuid", __NR_getresuid},
#endif
#ifdef SYS_getrlimit
	{"getrlimit", __NR_getrlimit},
#endif
#ifdef SYS_getrusage
	{"getrusage", __NR_getrusage},
#endif
#ifdef SYS_getsid
	{"getsid", __NR_getsid},
#endif
#ifdef SYS_getsockname
	{"getsockname", __NR_getsockname},
#endif
#ifdef SYS_getsockopt
	{"getsockopt", __NR_getsockopt},
#endif
#ifdef SYS_gettid
	{"gettid", __NR_gettid},
#endif
#ifdef SYS_gettimeofday
	{"gettimeofday", __NR_gettimeofday},
#endif
#ifdef SYS_getuid
	{"getuid", __NR_getuid},
#endif
#ifdef SYS_getxattr
	{"getxattr", __NR_getxattr},
#endif
#ifdef SYS_init_module
	{"init_module", __NR_init_module},
#endif
#ifdef SYS_inotify_add_watch
	{"inotify_add_watch", __NR_inotify_add_watch},
#endif
#ifdef SYS_inotify_init
	{"inotify_init", __NR_inotify_init},
#endif
#ifdef SYS_inotify_init1
	{"inotify_init1", __NR_inotify_init1},
#endif
#ifdef SYS_inotify_rm_watch
	{"inotify_rm_watch", __NR_inotify_rm_watch},
#endif
#ifdef SYS_io_cancel
	{"io_cancel", __NR_io_cancel},
#endif
#ifdef SYS_io_destroy
	{"io_destroy", __NR_io_destroy},
#endif
#ifdef SYS_io_getevents
	{"io_getevents", __NR_io_getevents},
#endif
#ifdef SYS_io_setup
	{"io_setup", __NR_io_setup},
#endif
#ifdef SYS_io_submit
	{"io_submit", __NR_io_submit},
#endif
#ifdef SYS_ioctl
	{"ioctl", __NR_ioctl},
#endif
#ifdef SYS_ioperm
	{"ioperm", __NR_ioperm},
#endif
#ifdef SYS_iopl
	{"iopl", __NR_iopl},
#endif
#ifdef SYS_ioprio_get
	{"ioprio_get", __NR_ioprio_get},
#endif
#ifdef SYS_ioprio_set
	{"ioprio_set", __NR_ioprio_set},
#endif
#ifdef SYS_kcmp
	{"kcmp", __NR_kcmp},
#endif
#ifdef SYS_kexec_load
	{"kexec_load", __NR_kexec_load},
#endif
#ifdef SYS_keyctl
	{"keyctl", __NR_keyctl},
#endif
#ifdef SYS_kill
	{"kill", __NR_kill},
#endif
#ifdef SYS_lchown
	{"lchown", __NR_lchown},
#endif
#ifdef SYS_lgetxattr
	{"lgetxattr", __NR_lgetxattr},
#endif
#ifdef SYS_link
	{"link", __NR_link},
#endif
#ifdef SYS_linkat
	{"linkat", __NR_linkat},
#endif
#ifdef SYS_listen
	{"listen", __NR_listen},
#endif
#ifdef SYS_listxattr
	{"listxattr", __NR_listxattr},
#endif
#ifdef SYS_llistxattr
	{"llistxattr", __NR_llistxattr},
#endif
#ifdef SYS_lookup_dcookie
	{"lookup_dcookie", __NR_lookup_dcookie},
#endif
#ifdef SYS_lremovexattr
	{"lremovexattr", __NR_lremovexattr},
#endif
#ifdef SYS_lseek
	{"lseek", __NR_lseek},
#endif
#ifdef SYS_lsetxattr
	{"lsetxattr", __NR_lsetxattr},
#endif
#ifdef SYS_lstat
	{"lstat", __NR_lstat},
#endif
#ifdef SYS_madvise
	{"madvise", __NR_madvise},
#endif
#ifdef SYS_mbind
	{"mbind", __NR_mbind},
#endif
#ifdef SYS_migrate_pages
	{"migrate_pages", __NR_migrate_pages},
#endif
#ifdef SYS_mincore
	{"mincore", __NR_mincore},
#endif
#ifdef SYS_mkdir
	{"mkdir", __NR_mkdir},
#endif
#ifdef SYS_mkdirat
	{"mkdirat", __NR_mkdirat},
#endif
#ifdef SYS_mknod
	{"mknod", __NR_mknod},
#endif
#ifdef SYS_mknodat
	{"mknodat", __NR_mknodat},
#endif
#ifdef SYS_mlock
	{"mlock", __NR_mlock},
#endif
#ifdef SYS_mlockall
	{"mlockall", __NR_mlockall},
#endif
#ifdef SYS_mmap
	{"mmap", __NR_mmap},
#endif
#ifdef SYS_modify_ldt
	{"modify_ldt", __NR_modify_ldt},
#endif
#ifdef SYS_mount
	{"mount", __NR_mount},
#endif
#ifdef SYS_move_pages
	{"move_pages", __NR_move_pages},
#endif
#ifdef SYS_mprotect
	{"mprotect", __NR_mprotect},
#endif
#ifdef SYS_mq_getsetattr
	{"mq_getsetattr", __NR_mq_getsetattr},
#endif
#ifdef SYS_mq_notify
	{"mq_notify", __NR_mq_notify},
#endif
#ifdef SYS_mq_open
	{"mq_open", __NR_mq_open},
#endif
#ifdef SYS_mq_timedreceive
	{"mq_timedreceive", __NR_mq_timedreceive},
#endif
#ifdef SYS_mq_timedsend
	{"mq_timedsend", __NR_mq_timedsend},
#endif
#ifdef SYS_mq_unlink
	{"mq_unlink", __NR_mq_unlink},
#endif
#ifdef SYS_mremap
	{"mremap", __NR_mremap},
#endif
#ifdef SYS_msgctl
	{"msgctl", __NR_msgctl},
#endif
#ifdef SYS_msgget
	{"msgget", __NR_msgget},
#endif
#ifdef SYS_msgrcv
	{"msgrcv", __NR_msgrcv},
#endif
#ifdef SYS_msgsnd
	{"msgsnd", __NR_msgsnd},
#endif
#ifdef SYS_msync
	{"msync", __NR_msync},
#endif
#ifdef SYS_munlock
	{"munlock", __NR_munlock},
#endif
#ifdef SYS_munlockall
	{"munlockall", __NR_munlockall},
#endif
#ifdef SYS_munmap
	{"munmap", __NR_munmap},
#endif
#ifdef SYS_name_to_handle_at
	{"name_to_handle_at", __NR_name_to_handle_at},
#endif
#ifdef SYS_nanosleep
	{"nanosleep", __NR_nanosleep},
#endif
#ifdef SYS_newfstatat
	{"newfstatat", __NR_newfstatat},
#endif
#ifdef SYS_nfsservctl
	{"nfsservctl", __NR_nfsservctl},
#endif
#ifdef SYS_open
	{"open", __NR_open},
#endif
#ifdef SYS_open_by_handle_at
	{"open_by_handle_at", __NR_open_by_handle_at},
#endif
#ifdef SYS_openat
	{"openat", __NR_openat},
#endif
#ifdef SYS_pause
	{"pause", __NR_pause},
#endif
#ifdef SYS_perf_event_open
	{"perf_event_open", __NR_perf_event_open},
#endif
#ifdef SYS_personality
	{"personality", __NR_personality},
#endif
#ifdef SYS_pipe
	{"pipe", __NR_pipe},
#endif
#ifdef SYS_pipe2
	{"pipe2", __NR_pipe2},
#endif
#ifdef SYS_pivot_root
	{"pivot_root", __NR_pivot_root},
#endif
#ifdef SYS_poll
	{"poll", __NR_poll},
#endif
#ifdef SYS_ppoll
	{"ppoll", __NR_ppoll},
#endif
#ifdef SYS_prctl
	{"prctl", __NR_prctl},
#endif
#ifdef SYS_pread64
	{"pread64", __NR_pread64},
#endif
#ifdef SYS_preadv
	{"preadv", __NR_preadv},
#endif
#ifdef SYS_prlimit64
	{"prlimit64", __NR_prlimit64},
#endif
#ifdef SYS_process_vm_readv
	{"process_vm_readv", __NR_process_vm_readv},
#endif
#ifdef SYS_process_vm_writev
	{"process_vm_writev", __NR_process_vm_writev},
#endif
#ifdef SYS_pselect6
	{"pselect6", __NR_pselect6},
#endif
#ifdef SYS_ptrace
	{"ptrace", __NR_ptrace},
#endif
#ifdef SYS_putpmsg
	{"putpmsg", __NR_putpmsg},
#endif
#ifdef SYS_pwrite64
	{"pwrite64", __NR_pwrite64},
#endif
#ifdef SYS_pwritev
	{"pwritev", __NR_pwritev},
#endif
#ifdef SYS_query_module
	{"query_module", __NR_query_module},
#endif
#ifdef SYS_quotactl
	{"quotactl", __NR_quotactl},
#endif
#ifdef SYS_read
	{"read", __NR_read},
#endif
#ifdef SYS_readahead
	{"readahead", __NR_readahead},
#endif
#ifdef SYS_readlink
	{"readlink", __NR_readlink},
#endif
#ifdef SYS_readlinkat
	{"readlinkat", __NR_readlinkat},
#endif
#ifdef SYS_readv
	{"readv", __NR_readv},
#endif
#ifdef SYS_reboot
	{"reboot", __NR_reboot},
#endif
#ifdef SYS_recvfrom
	{"recvfrom", __NR_recvfrom},
#endif
#ifdef SYS_recvmmsg
	{"recvmmsg", __NR_recvmmsg},
#endif
#ifdef SYS_recvmsg
	{"recvmsg", __NR_recvmsg},
#endif
#ifdef SYS_remap_file_pages
	{"remap_file_pages", __NR_remap_file_pages},
#endif
#ifdef SYS_removexattr
	{"removexattr", __NR_removexattr},
#endif
#ifdef SYS_rename
	{"rename", __NR_rename},
#endif
#ifdef SYS_renameat
	{"renameat", __NR_renameat},
#endif
#ifdef SYS_request_key
	{"request_key", __NR_request_key},
#endif
#ifdef SYS_restart_syscall
	{"restart_syscall", __NR_restart_syscall},
#endif
#ifdef SYS_rmdir
	{"rmdir", __NR_rmdir},
#endif
#ifdef SYS_rt_sigaction
	{"rt_sigaction", __NR_rt_sigaction},
#endif
#ifdef SYS_rt_sigpending
	{"rt_sigpending", __NR_rt_sigpending},
#endif
#ifdef SYS_rt_sigprocmask
	{"rt_sigprocmask", __NR_rt_sigprocmask},
#endif
#ifdef SYS_rt_sigqueueinfo
	{"rt_sigqueueinfo", __NR_rt_sigqueueinfo},
#endif
#ifdef SYS_rt_sigreturn
	{"rt_sigreturn", __NR_rt_sigreturn},
#endif
#ifdef SYS_rt_sigsuspend
	{"rt_sigsuspend", __NR_rt_sigsuspend},
#endif
#ifdef SYS_rt_sigtimedwait
	{"rt_sigtimedwait", __NR_rt_sigtimedwait},
#endif
#ifdef SYS_rt_tgsigqueueinfo
	{"rt_tgsigqueueinfo", __NR_rt_tgsigqueueinfo},
#endif
#ifdef SYS_sched_get_priority_max
	{"sched_get_priority_max", __NR_sched_get_priority_max},
#endif
#ifdef SYS_sched_get_priority_min
	{"sched_get_priority_min", __NR_sched_get_priority_min},
#endif
#ifdef SYS_sched_getaffinity
	{"sched_getaffinity", __NR_sched_getaffinity},
#endif
#ifdef SYS_sched_getparam
	{"sched_getparam", __NR_sched_getparam},
#endif
#ifdef SYS_sched_getscheduler
	{"sched_getscheduler", __NR_sched_getscheduler},
#endif
#ifdef SYS_sched_rr_get_interval
	{"sched_rr_get_interval", __NR_sched_rr_get_interval},
#endif
#ifdef SYS_sched_setaffinity
	{"sched_setaffinity", __NR_sched_setaffinity},
#endif
#ifdef SYS_sched_setparam
	{"sched_setparam", __NR_sched_setparam},
#endif
#ifdef SYS_sched_setscheduler
	{"sched_setscheduler", __NR_sched_setscheduler},
#endif
#ifdef SYS_sched_yield
	{"sched_yield", __NR_sched_yield},
#endif
#ifdef SYS_security
	{"security", __NR_security},
#endif
#ifdef SYS_select
	{"select", __NR_select},
#endif
#ifdef SYS_semctl
	{"semctl", __NR_semctl},
#endif
#ifdef SYS_semget
	{"semget", __NR_semget},
#endif
#ifdef SYS_semop
	{"semop", __NR_semop},
#endif
#ifdef SYS_semtimedop
	{"semtimedop", __NR_semtimedop},
#endif
#ifdef SYS_sendfile
	{"sendfile", __NR_sendfile},
#endif
#ifdef SYS_sendmmsg
	{"sendmmsg", __NR_sendmmsg},
#endif
#ifdef SYS_sendmsg
	{"sendmsg", __NR_sendmsg},
#endif
#ifdef SYS_sendto
	{"sendto", __NR_sendto},
#endif
#ifdef SYS_set_mempolicy
	{"set_mempolicy", __NR_set_mempolicy},
#endif
#ifdef SYS_set_robust_list
	{"set_robust_list", __NR_set_robust_list},
#endif
#ifdef SYS_set_thread_area
	{"set_thread_area", __NR_set_thread_area},
#endif
#ifdef SYS_set_tid_address
	{"set_tid_address", __NR_set_tid_address},
#endif
#ifdef SYS_setdomainname
	{"setdomainname", __NR_setdomainname},
#endif
#ifdef SYS_setfsgid
	{"setfsgid", __NR_setfsgid},
#endif
#ifdef SYS_setfsuid
	{"setfsuid", __NR_setfsuid},
#endif
#ifdef SYS_setgid
	{"setgid", __NR_setgid},
#endif
#ifdef SYS_setgroups
	{"setgroups", __NR_setgroups},
#endif
#ifdef SYS_sethostname
	{"sethostname", __NR_sethostname},
#endif
#ifdef SYS_setitimer
	{"setitimer", __NR_setitimer},
#endif
#ifdef SYS_setns
	{"setns", __NR_setns},
#endif
#ifdef SYS_setpgid
	{"setpgid", __NR_setpgid},
#endif
#ifdef SYS_setpriority
	{"setpriority", __NR_setpriority},
#endif
#ifdef SYS_setregid
	{"setregid", __NR_setregid},
#endif
#ifdef SYS_setresgid
	{"setresgid", __NR_setresgid},
#endif
#ifdef SYS_setresuid
	{"setresuid", __NR_setresuid},
#endif
#ifdef SYS_setreuid
	{"setreuid", __NR_setreuid},
#endif
#ifdef SYS_setrlimit
	{"setrlimit", __NR_setrlimit},
#endif
#ifdef SYS_setsid
	{"setsid", __NR_setsid},
#endif
#ifdef SYS_setsockopt
	{"setsockopt", __NR_setsockopt},
#endif
#ifdef SYS_settimeofday
	{"settimeofday", __NR_settimeofday},
#endif
#ifdef SYS_setuid
	{"setuid", __NR_setuid},
#endif
#ifdef SYS_setxattr
	{"setxattr", __NR_setxattr},
#endif
#ifdef SYS_shmat
	{"shmat", __NR_shmat},
#endif
#ifdef SYS_shmctl
	{"shmctl", __NR_shmctl},
#endif
#ifdef SYS_shmdt
	{"shmdt", __NR_shmdt},
#endif
#ifdef SYS_shmget
	{"shmget", __NR_shmget},
#endif
#ifdef SYS_shutdown
	{"shutdown", __NR_shutdown},
#endif
#ifdef SYS_sigaltstack
	{"sigaltstack", __NR_sigaltstack},
#endif
#ifdef SYS_signalfd
	{"signalfd", __NR_signalfd},
#endif
#ifdef SYS_signalfd4
	{"signalfd4", __NR_signalfd4},
#endif
#ifdef SYS_socket
	{"socket", __NR_socket},
#endif
#ifdef SYS_socketpair
	{"socketpair", __NR_socketpair},
#endif
#ifdef SYS_splice
	{"splice", __NR_splice},
#endif
#ifdef SYS_stat
	{"stat", __NR_stat},
#endif
#ifdef SYS_statfs
	{"statfs", __NR_statfs},
#endif
#ifdef SYS_swapoff
	{"swapoff", __NR_swapoff},
#endif
#ifdef SYS_swapon
	{"swapon", __NR_swapon},
#endif
#ifdef SYS_symlink
	{"symlink", __NR_symlink},
#endif
#ifdef SYS_symlinkat
	{"symlinkat", __NR_symlinkat},
#endif
#ifdef SYS_sync
	{"sync", __NR_sync},
#endif
#ifdef SYS_sync_file_range
	{"sync_file_range", __NR_sync_file_range},
#endif
#ifdef SYS_syncfs
	{"syncfs", __NR_syncfs},
#endif
#ifdef SYS_sysfs
	{"sysfs", __NR_sysfs},
#endif
#ifdef SYS_sysinfo
	{"sysinfo", __NR_sysinfo},
#endif
#ifdef SYS_syslog
	{"syslog", __NR_syslog},
#endif
#ifdef SYS_tee
	{"tee", __NR_tee},
#endif
#ifdef SYS_tgkill
	{"tgkill", __NR_tgkill},
#endif
#ifdef SYS_time
	{"time", __NR_time},
#endif
#ifdef SYS_timer_create
	{"timer_create", __NR_timer_create},
#endif
#ifdef SYS_timer_delete
	{"timer_delete", __NR_timer_delete},
#endif
#ifdef SYS_timer_getoverrun
	{"timer_getoverrun", __NR_timer_getoverrun},
#endif
#ifdef SYS_timer_gettime
	{"timer_gettime", __NR_timer_gettime},
#endif
#ifdef SYS_timer_settime
	{"timer_settime", __NR_timer_settime},
#endif
#ifdef SYS_timerfd_create
	{"timerfd_create", __NR_timerfd_create},
#endif
#ifdef SYS_timerfd_gettime
	{"timerfd_gettime", __NR_timerfd_gettime},
#endif
#ifdef SYS_timerfd_settime
	{"timerfd_settime", __NR_timerfd_settime},
#endif
#ifdef SYS_times
	{"times", __NR_times},
#endif
#ifdef SYS_tkill
	{"tkill", __NR_tkill},
#endif
#ifdef SYS_truncate
	{"truncate", __NR_truncate},
#endif
#ifdef SYS_tuxcall
	{"tuxcall", __NR_tuxcall},
#endif
#ifdef SYS_umask
	{"umask", __NR_umask},
#endif
#ifdef SYS_umount2
	{"umount2", __NR_umount2},
#endif
#ifdef SYS_uname
	{"uname", __NR_uname},
#endif
#ifdef SYS_unlink
	{"unlink", __NR_unlink},
#endif
#ifdef SYS_unlinkat
	{"unlinkat", __NR_unlinkat},
#endif
#ifdef SYS_unshare
	{"unshare", __NR_unshare},
#endif
#ifdef SYS_uselib
	{"uselib", __NR_uselib},
#endif
#ifdef SYS_ustat
	{"ustat", __NR_ustat},
#endif
#ifdef SYS_utime
	{"utime", __NR_utime},
#endif
#ifdef SYS_utimensat
	{"utimensat", __NR_utimensat},
#endif
#ifdef SYS_utimes
	{"utimes", __NR_utimes},
#endif
#ifdef SYS_vfork
	{"vfork", __NR_vfork},
#endif
#ifdef SYS_vhangup
	{"vhangup", __NR_vhangup},
#endif
#ifdef SYS_vmsplice
	{"vmsplice", __NR_vmsplice},
#endif
#ifdef SYS_vserver
	{"vserver", __NR_vserver},
#endif
#ifdef SYS_wait4
	{"wait4", __NR_wait4},
#endif
#ifdef SYS_waitid
	{"waitid", __NR_waitid},
#endif
#ifdef SYS_write
	{"write", __NR_write},
#endif
#ifdef SYS_writev
	{"writev", __NR_writev},
#endif
#endif
#if defined __x86_64__ && defined __ILP32__
#ifdef SYS_accept
	{"accept", __NR_accept},
#endif
#ifdef SYS_accept4
	{"accept4", __NR_accept4},
#endif
#ifdef SYS_access
	{"access", __NR_access},
#endif
#ifdef SYS_acct
	{"acct", __NR_acct},
#endif
#ifdef SYS_add_key
	{"add_key", __NR_add_key},
#endif
#ifdef SYS_adjtimex
	{"adjtimex", __NR_adjtimex},
#endif
#ifdef SYS_afs_syscall
	{"afs_syscall", __NR_afs_syscall},
#endif
#ifdef SYS_alarm
	{"alarm", __NR_alarm},
#endif
#ifdef SYS_arch_prctl
	{"arch_prctl", __NR_arch_prctl},
#endif
#ifdef SYS_bind
	{"bind", __NR_bind},
#endif
#ifdef SYS_brk
	{"brk", __NR_brk},
#endif
#ifdef SYS_capget
	{"capget", __NR_capget},
#endif
#ifdef SYS_capset
	{"capset", __NR_capset},
#endif
#ifdef SYS_chdir
	{"chdir", __NR_chdir},
#endif
#ifdef SYS_chmod
	{"chmod", __NR_chmod},
#endif
#ifdef SYS_chown
	{"chown", __NR_chown},
#endif
#ifdef SYS_chroot
	{"chroot", __NR_chroot},
#endif
#ifdef SYS_clock_adjtime
	{"clock_adjtime", __NR_clock_adjtime},
#endif
#ifdef SYS_clock_getres
	{"clock_getres", __NR_clock_getres},
#endif
#ifdef SYS_clock_gettime
	{"clock_gettime", __NR_clock_gettime},
#endif
#ifdef SYS_clock_nanosleep
	{"clock_nanosleep", __NR_clock_nanosleep},
#endif
#ifdef SYS_clock_settime
	{"clock_settime", __NR_clock_settime},
#endif
#ifdef SYS_clone
	{"clone", __NR_clone},
#endif
#ifdef SYS_close
	{"close", __NR_close},
#endif
#ifdef SYS_connect
	{"connect", __NR_connect},
#endif
#ifdef SYS_creat
	{"creat", __NR_creat},
#endif
#ifdef SYS_delete_module
	{"delete_module", __NR_delete_module},
#endif
#ifdef SYS_dup
	{"dup", __NR_dup},
#endif
#ifdef SYS_dup2
	{"dup2", __NR_dup2},
#endif
#ifdef SYS_dup3
	{"dup3", __NR_dup3},
#endif
#ifdef SYS_epoll_create
	{"epoll_create", __NR_epoll_create},
#endif
#ifdef SYS_epoll_create1
	{"epoll_create1", __NR_epoll_create1},
#endif
#ifdef SYS_epoll_ctl
	{"epoll_ctl", __NR_epoll_ctl},
#endif
#ifdef SYS_epoll_pwait
	{"epoll_pwait", __NR_epoll_pwait},
#endif
#ifdef SYS_epoll_wait
	{"epoll_wait", __NR_epoll_wait},
#endif
#ifdef SYS_eventfd
	{"eventfd", __NR_eventfd},
#endif
#ifdef SYS_eventfd2
	{"eventfd2", __NR_eventfd2},
#endif
#ifdef SYS_execve
	{"execve", __NR_execve},
#endif
#ifdef SYS_exit
	{"exit", __NR_exit},
#endif
#ifdef SYS_exit_group
	{"exit_group", __NR_exit_group},
#endif
#ifdef SYS_faccessat
	{"faccessat", __NR_faccessat},
#endif
#ifdef SYS_fadvise64
	{"fadvise64", __NR_fadvise64},
#endif
#ifdef SYS_fallocate
	{"fallocate", __NR_fallocate},
#endif
#ifdef SYS_fanotify_init
	{"fanotify_init", __NR_fanotify_init},
#endif
#ifdef SYS_fanotify_mark
	{"fanotify_mark", __NR_fanotify_mark},
#endif
#ifdef SYS_fchdir
	{"fchdir", __NR_fchdir},
#endif
#ifdef SYS_fchmod
	{"fchmod", __NR_fchmod},
#endif
#ifdef SYS_fchmodat
	{"fchmodat", __NR_fchmodat},
#endif
#ifdef SYS_fchown
	{"fchown", __NR_fchown},
#endif
#ifdef SYS_fchownat
	{"fchownat", __NR_fchownat},
#endif
#ifdef SYS_fcntl
	{"fcntl", __NR_fcntl},
#endif
#ifdef SYS_fdatasync
	{"fdatasync", __NR_fdatasync},
#endif
#ifdef SYS_fgetxattr
	{"fgetxattr", __NR_fgetxattr},
#endif
#ifdef SYS_finit_module
	{"finit_module", __NR_finit_module},
#endif
#ifdef SYS_flistxattr
	{"flistxattr", __NR_flistxattr},
#endif
#ifdef SYS_flock
	{"flock", __NR_flock},
#endif
#ifdef SYS_fork
	{"fork", __NR_fork},
#endif
#ifdef SYS_fremovexattr
	{"fremovexattr", __NR_fremovexattr},
#endif
#ifdef SYS_fsetxattr
	{"fsetxattr", __NR_fsetxattr},
#endif
#ifdef SYS_fstat
	{"fstat", __NR_fstat},
#endif
#ifdef SYS_fstatfs
	{"fstatfs", __NR_fstatfs},
#endif
#ifdef SYS_fsync
	{"fsync", __NR_fsync},
#endif
#ifdef SYS_ftruncate
	{"ftruncate", __NR_ftruncate},
#endif
#ifdef SYS_futex
	{"futex", __NR_futex},
#endif
#ifdef SYS_futimesat
	{"futimesat", __NR_futimesat},
#endif
#ifdef SYS_get_mempolicy
	{"get_mempolicy", __NR_get_mempolicy},
#endif
#ifdef SYS_get_robust_list
	{"get_robust_list", __NR_get_robust_list},
#endif
#ifdef SYS_getcpu
	{"getcpu", __NR_getcpu},
#endif
#ifdef SYS_getcwd
	{"getcwd", __NR_getcwd},
#endif
#ifdef SYS_getdents
	{"getdents", __NR_getdents},
#endif
#ifdef SYS_getdents64
	{"getdents64", __NR_getdents64},
#endif
#ifdef SYS_getegid
	{"getegid", __NR_getegid},
#endif
#ifdef SYS_geteuid
	{"geteuid", __NR_geteuid},
#endif
#ifdef SYS_getgid
	{"getgid", __NR_getgid},
#endif
#ifdef SYS_getgroups
	{"getgroups", __NR_getgroups},
#endif
#ifdef SYS_getitimer
	{"getitimer", __NR_getitimer},
#endif
#ifdef SYS_getpeername
	{"getpeername", __NR_getpeername},
#endif
#ifdef SYS_getpgid
	{"getpgid", __NR_getpgid},
#endif
#ifdef SYS_getpgrp
	{"getpgrp", __NR_getpgrp},
#endif
#ifdef SYS_getpid
	{"getpid", __NR_getpid},
#endif
#ifdef SYS_getpmsg
	{"getpmsg", __NR_getpmsg},
#endif
#ifdef SYS_getppid
	{"getppid", __NR_getppid},
#endif
#ifdef SYS_getpriority
	{"getpriority", __NR_getpriority},
#endif
#ifdef SYS_getresgid
	{"getresgid", __NR_getresgid},
#endif
#ifdef SYS_getresuid
	{"getresuid", __NR_getresuid},
#endif
#ifdef SYS_getrlimit
	{"getrlimit", __NR_getrlimit},
#endif
#ifdef SYS_getrusage
	{"getrusage", __NR_getrusage},
#endif
#ifdef SYS_getsid
	{"getsid", __NR_getsid},
#endif
#ifdef SYS_getsockname
	{"getsockname", __NR_getsockname},
#endif
#ifdef SYS_getsockopt
	{"getsockopt", __NR_getsockopt},
#endif
#ifdef SYS_gettid
	{"gettid", __NR_gettid},
#endif
#ifdef SYS_gettimeofday
	{"gettimeofday", __NR_gettimeofday},
#endif
#ifdef SYS_getuid
	{"getuid", __NR_getuid},
#endif
#ifdef SYS_getxattr
	{"getxattr", __NR_getxattr},
#endif
#ifdef SYS_init_module
	{"init_module", __NR_init_module},
#endif
#ifdef SYS_inotify_add_watch
	{"inotify_add_watch", __NR_inotify_add_watch},
#endif
#ifdef SYS_inotify_init
	{"inotify_init", __NR_inotify_init},
#endif
#ifdef SYS_inotify_init1
	{"inotify_init1", __NR_inotify_init1},
#endif
#ifdef SYS_inotify_rm_watch
	{"inotify_rm_watch", __NR_inotify_rm_watch},
#endif
#ifdef SYS_io_cancel
	{"io_cancel", __NR_io_cancel},
#endif
#ifdef SYS_io_destroy
	{"io_destroy", __NR_io_destroy},
#endif
#ifdef SYS_io_getevents
	{"io_getevents", __NR_io_getevents},
#endif
#ifdef SYS_io_setup
	{"io_setup", __NR_io_setup},
#endif
#ifdef SYS_io_submit
	{"io_submit", __NR_io_submit},
#endif
#ifdef SYS_ioctl
	{"ioctl", __NR_ioctl},
#endif
#ifdef SYS_ioperm
	{"ioperm", __NR_ioperm},
#endif
#ifdef SYS_iopl
	{"iopl", __NR_iopl},
#endif
#ifdef SYS_ioprio_get
	{"ioprio_get", __NR_ioprio_get},
#endif
#ifdef SYS_ioprio_set
	{"ioprio_set", __NR_ioprio_set},
#endif
#ifdef SYS_kcmp
	{"kcmp", __NR_kcmp},
#endif
#ifdef SYS_kexec_load
	{"kexec_load", __NR_kexec_load},
#endif
#ifdef SYS_keyctl
	{"keyctl", __NR_keyctl},
#endif
#ifdef SYS_kill
	{"kill", __NR_kill},
#endif
#ifdef SYS_lchown
	{"lchown", __NR_lchown},
#endif
#ifdef SYS_lgetxattr
	{"lgetxattr", __NR_lgetxattr},
#endif
#ifdef SYS_link
	{"link", __NR_link},
#endif
#ifdef SYS_linkat
	{"linkat", __NR_linkat},
#endif
#ifdef SYS_listen
	{"listen", __NR_listen},
#endif
#ifdef SYS_listxattr
	{"listxattr", __NR_listxattr},
#endif
#ifdef SYS_llistxattr
	{"llistxattr", __NR_llistxattr},
#endif
#ifdef SYS_lookup_dcookie
	{"lookup_dcookie", __NR_lookup_dcookie},
#endif
#ifdef SYS_lremovexattr
	{"lremovexattr", __NR_lremovexattr},
#endif
#ifdef SYS_lseek
	{"lseek", __NR_lseek},
#endif
#ifdef SYS_lsetxattr
	{"lsetxattr", __NR_lsetxattr},
#endif
#ifdef SYS_lstat
	{"lstat", __NR_lstat},
#endif
#ifdef SYS_madvise
	{"madvise", __NR_madvise},
#endif
#ifdef SYS_mbind
	{"mbind", __NR_mbind},
#endif
#ifdef SYS_migrate_pages
	{"migrate_pages", __NR_migrate_pages},
#endif
#ifdef SYS_mincore
	{"mincore", __NR_mincore},
#endif
#ifdef SYS_mkdir
	{"mkdir", __NR_mkdir},
#endif
#ifdef SYS_mkdirat
	{"mkdirat", __NR_mkdirat},
#endif
#ifdef SYS_mknod
	{"mknod", __NR_mknod},
#endif
#ifdef SYS_mknodat
	{"mknodat", __NR_mknodat},
#endif
#ifdef SYS_mlock
	{"mlock", __NR_mlock},
#endif
#ifdef SYS_mlockall
	{"mlockall", __NR_mlockall},
#endif
#ifdef SYS_mmap
	{"mmap", __NR_mmap},
#endif
#ifdef SYS_modify_ldt
	{"modify_ldt", __NR_modify_ldt},
#endif
#ifdef SYS_mount
	{"mount", __NR_mount},
#endif
#ifdef SYS_move_pages
	{"move_pages", __NR_move_pages},
#endif
#ifdef SYS_mprotect
	{"mprotect", __NR_mprotect},
#endif
#ifdef SYS_mq_getsetattr
	{"mq_getsetattr", __NR_mq_getsetattr},
#endif
#ifdef SYS_mq_notify
	{"mq_notify", __NR_mq_notify},
#endif
#ifdef SYS_mq_open
	{"mq_open", __NR_mq_open},
#endif
#ifdef SYS_mq_timedreceive
	{"mq_timedreceive", __NR_mq_timedreceive},
#endif
#ifdef SYS_mq_timedsend
	{"mq_timedsend", __NR_mq_timedsend},
#endif
#ifdef SYS_mq_unlink
	{"mq_unlink", __NR_mq_unlink},
#endif
#ifdef SYS_mremap
	{"mremap", __NR_mremap},
#endif
#ifdef SYS_msgctl
	{"msgctl", __NR_msgctl},
#endif
#ifdef SYS_msgget
	{"msgget", __NR_msgget},
#endif
#ifdef SYS_msgrcv
	{"msgrcv", __NR_msgrcv},
#endif
#ifdef SYS_msgsnd
	{"msgsnd", __NR_msgsnd},
#endif
#ifdef SYS_msync
	{"msync", __NR_msync},
#endif
#ifdef SYS_munlock
	{"munlock", __NR_munlock},
#endif
#ifdef SYS_munlockall
	{"munlockall", __NR_munlockall},
#endif
#ifdef SYS_munmap
	{"munmap", __NR_munmap},
#endif
#ifdef SYS_name_to_handle_at
	{"name_to_handle_at", __NR_name_to_handle_at},
#endif
#ifdef SYS_nanosleep
	{"nanosleep", __NR_nanosleep},
#endif
#ifdef SYS_newfstatat
	{"newfstatat", __NR_newfstatat},
#endif
#ifdef SYS_open
	{"open", __NR_open},
#endif
#ifdef SYS_open_by_handle_at
	{"open_by_handle_at", __NR_open_by_handle_at},
#endif
#ifdef SYS_openat
	{"openat", __NR_openat},
#endif
#ifdef SYS_pause
	{"pause", __NR_pause},
#endif
#ifdef SYS_perf_event_open
	{"perf_event_open", __NR_perf_event_open},
#endif
#ifdef SYS_personality
	{"personality", __NR_personality},
#endif
#ifdef SYS_pipe
	{"pipe", __NR_pipe},
#endif
#ifdef SYS_pipe2
	{"pipe2", __NR_pipe2},
#endif
#ifdef SYS_pivot_root
	{"pivot_root", __NR_pivot_root},
#endif
#ifdef SYS_poll
	{"poll", __NR_poll},
#endif
#ifdef SYS_ppoll
	{"ppoll", __NR_ppoll},
#endif
#ifdef SYS_prctl
	{"prctl", __NR_prctl},
#endif
#ifdef SYS_pread64
	{"pread64", __NR_pread64},
#endif
#ifdef SYS_preadv
	{"preadv", __NR_preadv},
#endif
#ifdef SYS_prlimit64
	{"prlimit64", __NR_prlimit64},
#endif
#ifdef SYS_process_vm_readv
	{"process_vm_readv", __NR_process_vm_readv},
#endif
#ifdef SYS_process_vm_writev
	{"process_vm_writev", __NR_process_vm_writev},
#endif
#ifdef SYS_pselect6
	{"pselect6", __NR_pselect6},
#endif
#ifdef SYS_ptrace
	{"ptrace", __NR_ptrace},
#endif
#ifdef SYS_putpmsg
	{"putpmsg", __NR_putpmsg},
#endif
#ifdef SYS_pwrite64
	{"pwrite64", __NR_pwrite64},
#endif
#ifdef SYS_pwritev
	{"pwritev", __NR_pwritev},
#endif
#ifdef SYS_quotactl
	{"quotactl", __NR_quotactl},
#endif
#ifdef SYS_read
	{"read", __NR_read},
#endif
#ifdef SYS_readahead
	{"readahead", __NR_readahead},
#endif
#ifdef SYS_readlink
	{"readlink", __NR_readlink},
#endif
#ifdef SYS_readlinkat
	{"readlinkat", __NR_readlinkat},
#endif
#ifdef SYS_readv
	{"readv", __NR_readv},
#endif
#ifdef SYS_reboot
	{"reboot", __NR_reboot},
#endif
#ifdef SYS_recvfrom
	{"recvfrom", __NR_recvfrom},
#endif
#ifdef SYS_recvmmsg
	{"recvmmsg", __NR_recvmmsg},
#endif
#ifdef SYS_recvmsg
	{"recvmsg", __NR_recvmsg},
#endif
#ifdef SYS_remap_file_pages
	{"remap_file_pages", __NR_remap_file_pages},
#endif
#ifdef SYS_removexattr
	{"removexattr", __NR_removexattr},
#endif
#ifdef SYS_rename
	{"rename", __NR_rename},
#endif
#ifdef SYS_renameat
	{"renameat", __NR_renameat},
#endif
#ifdef SYS_request_key
	{"request_key", __NR_request_key},
#endif
#ifdef SYS_restart_syscall
	{"restart_syscall", __NR_restart_syscall},
#endif
#ifdef SYS_rmdir
	{"rmdir", __NR_rmdir},
#endif
#ifdef SYS_rt_sigaction
	{"rt_sigaction", __NR_rt_sigaction},
#endif
#ifdef SYS_rt_sigpending
	{"rt_sigpending", __NR_rt_sigpending},
#endif
#ifdef SYS_rt_sigprocmask
	{"rt_sigprocmask", __NR_rt_sigprocmask},
#endif
#ifdef SYS_rt_sigqueueinfo
	{"rt_sigqueueinfo", __NR_rt_sigqueueinfo},
#endif
#ifdef SYS_rt_sigreturn
	{"rt_sigreturn", __NR_rt_sigreturn},
#endif
#ifdef SYS_rt_sigsuspend
	{"rt_sigsuspend", __NR_rt_sigsuspend},
#endif
#ifdef SYS_rt_sigtimedwait
	{"rt_sigtimedwait", __NR_rt_sigtimedwait},
#endif
#ifdef SYS_rt_tgsigqueueinfo
	{"rt_tgsigqueueinfo", __NR_rt_tgsigqueueinfo},
#endif
#ifdef SYS_sched_get_priority_max
	{"sched_get_priority_max", __NR_sched_get_priority_max},
#endif
#ifdef SYS_sched_get_priority_min
	{"sched_get_priority_min", __NR_sched_get_priority_min},
#endif
#ifdef SYS_sched_getaffinity
	{"sched_getaffinity", __NR_sched_getaffinity},
#endif
#ifdef SYS_sched_getparam
	{"sched_getparam", __NR_sched_getparam},
#endif
#ifdef SYS_sched_getscheduler
	{"sched_getscheduler", __NR_sched_getscheduler},
#endif
#ifdef SYS_sched_rr_get_interval
	{"sched_rr_get_interval", __NR_sched_rr_get_interval},
#endif
#ifdef SYS_sched_setaffinity
	{"sched_setaffinity", __NR_sched_setaffinity},
#endif
#ifdef SYS_sched_setparam
	{"sched_setparam", __NR_sched_setparam},
#endif
#ifdef SYS_sched_setscheduler
	{"sched_setscheduler", __NR_sched_setscheduler},
#endif
#ifdef SYS_sched_yield
	{"sched_yield", __NR_sched_yield},
#endif
#ifdef SYS_security
	{"security", __NR_security},
#endif
#ifdef SYS_select
	{"select", __NR_select},
#endif
#ifdef SYS_semctl
	{"semctl", __NR_semctl},
#endif
#ifdef SYS_semget
	{"semget", __NR_semget},
#endif
#ifdef SYS_semop
	{"semop", __NR_semop},
#endif
#ifdef SYS_semtimedop
	{"semtimedop", __NR_semtimedop},
#endif
#ifdef SYS_sendfile
	{"sendfile", __NR_sendfile},
#endif
#ifdef SYS_sendmmsg
	{"sendmmsg", __NR_sendmmsg},
#endif
#ifdef SYS_sendmsg
	{"sendmsg", __NR_sendmsg},
#endif
#ifdef SYS_sendto
	{"sendto", __NR_sendto},
#endif
#ifdef SYS_set_mempolicy
	{"set_mempolicy", __NR_set_mempolicy},
#endif
#ifdef SYS_set_robust_list
	{"set_robust_list", __NR_set_robust_list},
#endif
#ifdef SYS_set_tid_address
	{"set_tid_address", __NR_set_tid_address},
#endif
#ifdef SYS_setdomainname
	{"setdomainname", __NR_setdomainname},
#endif
#ifdef SYS_setfsgid
	{"setfsgid", __NR_setfsgid},
#endif
#ifdef SYS_setfsuid
	{"setfsuid", __NR_setfsuid},
#endif
#ifdef SYS_setgid
	{"setgid", __NR_setgid},
#endif
#ifdef SYS_setgroups
	{"setgroups", __NR_setgroups},
#endif
#ifdef SYS_sethostname
	{"sethostname", __NR_sethostname},
#endif
#ifdef SYS_setitimer
	{"setitimer", __NR_setitimer},
#endif
#ifdef SYS_setns
	{"setns", __NR_setns},
#endif
#ifdef SYS_setpgid
	{"setpgid", __NR_setpgid},
#endif
#ifdef SYS_setpriority
	{"setpriority", __NR_setpriority},
#endif
#ifdef SYS_setregid
	{"setregid", __NR_setregid},
#endif
#ifdef SYS_setresgid
	{"setresgid", __NR_setresgid},
#endif
#ifdef SYS_setresuid
	{"setresuid", __NR_setresuid},
#endif
#ifdef SYS_setreuid
	{"setreuid", __NR_setreuid},
#endif
#ifdef SYS_setrlimit
	{"setrlimit", __NR_setrlimit},
#endif
#ifdef SYS_setsid
	{"setsid", __NR_setsid},
#endif
#ifdef SYS_setsockopt
	{"setsockopt", __NR_setsockopt},
#endif
#ifdef SYS_settimeofday
	{"settimeofday", __NR_settimeofday},
#endif
#ifdef SYS_setuid
	{"setuid", __NR_setuid},
#endif
#ifdef SYS_setxattr
	{"setxattr", __NR_setxattr},
#endif
#ifdef SYS_shmat
	{"shmat", __NR_shmat},
#endif
#ifdef SYS_shmctl
	{"shmctl", __NR_shmctl},
#endif
#ifdef SYS_shmdt
	{"shmdt", __NR_shmdt},
#endif
#ifdef SYS_shmget
	{"shmget", __NR_shmget},
#endif
#ifdef SYS_shutdown
	{"shutdown", __NR_shutdown},
#endif
#ifdef SYS_sigaltstack
	{"sigaltstack", __NR_sigaltstack},
#endif
#ifdef SYS_signalfd
	{"signalfd", __NR_signalfd},
#endif
#ifdef SYS_signalfd4
	{"signalfd4", __NR_signalfd4},
#endif
#ifdef SYS_socket
	{"socket", __NR_socket},
#endif
#ifdef SYS_socketpair
	{"socketpair", __NR_socketpair},
#endif
#ifdef SYS_splice
	{"splice", __NR_splice},
#endif
#ifdef SYS_stat
	{"stat", __NR_stat},
#endif
#ifdef SYS_statfs
	{"statfs", __NR_statfs},
#endif
#ifdef SYS_swapoff
	{"swapoff", __NR_swapoff},
#endif
#ifdef SYS_swapon
	{"swapon", __NR_swapon},
#endif
#ifdef SYS_symlink
	{"symlink", __NR_symlink},
#endif
#ifdef SYS_symlinkat
	{"symlinkat", __NR_symlinkat},
#endif
#ifdef SYS_sync
	{"sync", __NR_sync},
#endif
#ifdef SYS_sync_file_range
	{"sync_file_range", __NR_sync_file_range},
#endif
#ifdef SYS_syncfs
	{"syncfs", __NR_syncfs},
#endif
#ifdef SYS_sysfs
	{"sysfs", __NR_sysfs},
#endif
#ifdef SYS_sysinfo
	{"sysinfo", __NR_sysinfo},
#endif
#ifdef SYS_syslog
	{"syslog", __NR_syslog},
#endif
#ifdef SYS_tee
	{"tee", __NR_tee},
#endif
#ifdef SYS_tgkill
	{"tgkill", __NR_tgkill},
#endif
#ifdef SYS_time
	{"time", __NR_time},
#endif
#ifdef SYS_timer_create
	{"timer_create", __NR_timer_create},
#endif
#ifdef SYS_timer_delete
	{"timer_delete", __NR_timer_delete},
#endif
#ifdef SYS_timer_getoverrun
	{"timer_getoverrun", __NR_timer_getoverrun},
#endif
#ifdef SYS_timer_gettime
	{"timer_gettime", __NR_timer_gettime},
#endif
#ifdef SYS_timer_settime
	{"timer_settime", __NR_timer_settime},
#endif
#ifdef SYS_timerfd_create
	{"timerfd_create", __NR_timerfd_create},
#endif
#ifdef SYS_timerfd_gettime
	{"timerfd_gettime", __NR_timerfd_gettime},
#endif
#ifdef SYS_timerfd_settime
	{"timerfd_settime", __NR_timerfd_settime},
#endif
#ifdef SYS_times
	{"times", __NR_times},
#endif
#ifdef SYS_tkill
	{"tkill", __NR_tkill},
#endif
#ifdef SYS_truncate
	{"truncate", __NR_truncate},
#endif
#ifdef SYS_tuxcall
	{"tuxcall", __NR_tuxcall},
#endif
#ifdef SYS_umask
	{"umask", __NR_umask},
#endif
#ifdef SYS_umount2
	{"umount2", __NR_umount2},
#endif
#ifdef SYS_uname
	{"uname", __NR_uname},
#endif
#ifdef SYS_unlink
	{"unlink", __NR_unlink},
#endif
#ifdef SYS_unlinkat
	{"unlinkat", __NR_unlinkat},
#endif
#ifdef SYS_unshare
	{"unshare", __NR_unshare},
#endif
#ifdef SYS_ustat
	{"ustat", __NR_ustat},
#endif
#ifdef SYS_utime
	{"utime", __NR_utime},
#endif
#ifdef SYS_utimensat
	{"utimensat", __NR_utimensat},
#endif
#ifdef SYS_utimes
	{"utimes", __NR_utimes},
#endif
#ifdef SYS_vfork
	{"vfork", __NR_vfork},
#endif
#ifdef SYS_vhangup
	{"vhangup", __NR_vhangup},
#endif
#ifdef SYS_vmsplice
	{"vmsplice", __NR_vmsplice},
#endif
#ifdef SYS_wait4
	{"wait4", __NR_wait4},
#endif
#ifdef SYS_waitid
	{"waitid", __NR_waitid},
#endif
#ifdef SYS_write
	{"write", __NR_write},
#endif
#ifdef SYS_writev
	{"writev", __NR_writev},
#endif
#endif
//
// end of generated code
//
}; // end of syslist

const char *syscall_find_nr(int nr) {
	int i;
	int elems = sizeof(syslist) / sizeof(syslist[0]);
	for (i = 0; i < elems; i++) {
		if (nr == syslist[i].nr)
			return syslist[i].name;
	}
	
	return "unknown";
}

// return -1 if error, or syscall number
static int syscall_find_name(const char *name) {
	int i;
	int elems = sizeof(syslist) / sizeof(syslist[0]);
	for (i = 0; i < elems; i++) {
		if (strcmp(name, syslist[i].name) == 0)
			return syslist[i].nr;
	}
	
	return -1;
}

// return 1 if error, 0 if OK
int syscall_check_list(const char *slist, void (*callback)(int)) {
	// work on a copy of the string
	char *str = strdup(slist);
	if (!str)
		errExit("strdup");

	char *ptr = str;
	char *start = str;
	while (*ptr != '\0') {
		if (islower(*ptr) || isdigit(*ptr) || *ptr == '_')
			;
		else if (*ptr == ',') {
			*ptr = '\0';
			int nr = syscall_find_name(start);
			if (nr == -1) {
				fprintf(stderr, "Error: syscall %s not found\n", start);
				return -1;
			}
			else if (callback != NULL)
				callback(nr);
				
			start = ptr + 1;
		}
		ptr++;
	}
	if (*start != '\0') {
		int nr = syscall_find_name(start);
		if (nr == -1) {
			fprintf(stderr, "Error: syscall %s not found\n", start);
			return -1;
		}
		else if (callback != NULL)
			callback(nr);
	}
	
	return 0;
}
#endif // HAVE_SECCOMP
