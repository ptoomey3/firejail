AC_PREREQ([2.68])
AC_INIT(firejail, 0.9.20-rc2, netblue30@yahoo.com, , http://firejail.sourceforge.net)
AC_CONFIG_SRCDIR([src/firejail/main.c])
#AC_CONFIG_HEADERS([config.h])


AC_PROG_CC
AC_PROG_CXX
AC_PROG_INSTALL
AC_PROG_RANLIB

HAVE_SECCOMP=""
AC_ARG_ENABLE([seccomp],
    AS_HELP_STRING([--disable-seccomp], [Disable seccomp]))
AS_IF([test "x$enable_seccomp" != "xno"], [
	HAVE_SECCOMP="-DHAVE_SECCOMP"
	AC_SUBST(HAVE_SECCOMP)
])


# checking pthread library
AC_CHECK_LIB([pthread], [main], [], AC_MSG_ERROR([*** POSIX thread support not installed ***]))
AC_CHECK_HEADER(pthread.h,,AC_MSG_ERROR([*** POSIX thread support not installed ***]))
AC_CHECK_HEADER([linux/seccomp.h], HAVE_SECCOMP_H="-DHAVE_SECCOMP_H", HAVE_SECCOMP_H="")
AC_SUBST(HAVE_SECCOMP_H)

AC_OUTPUT(Makefile src/lib/Makefile src/firejail/Makefile src/firemon/Makefile src/libtrace/Makefile)

echo "Configuration options:"
echo "   seccomp: $HAVE_SECCOMP"
echo "   found <linux/seccomp.h> header: $HAVE_SECCOMP_H"
echo
