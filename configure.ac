AC_PREREQ([2.68])
AC_INIT(firejail, 0.9.7-testing6, netblue30@yahoo.com, , http://firejail.sourceforge.net)
AC_CONFIG_SRCDIR([src/firejail/main.c])
#AC_CONFIG_HEADERS([config.h])

AC_PROG_CC
AC_PROG_CXX
AC_PROG_INSTALL
AC_PROG_RANLIB

# checking pthread library
AC_CHECK_LIB([pthread], [main], [], AC_MSG_ERROR([*** POSIX thread support not installed ***]))
AC_CHECK_HEADER(pthread.h,,AC_MSG_ERROR([*** POSIX thread support not installed ***]))
AC_CHECK_HEADER([linux/seccomp.h], SECCOMP="-DHAVE_SECCOMP_H", SECCOMP="")
AC_SUBST(SECCOMP)

AC_OUTPUT(Makefile src/lib/Makefile src/firejail/Makefile src/firemon/Makefile)