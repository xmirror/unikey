#                                               -*- Autoconf -*-
# Process this file with autoconf to produce a configure script.
AC_PREREQ(2.50)
AC_INIT([Unikey X Input Method], [1.0.4], [unikey@gmail.com], [x-unikey])
AC_CONFIG_SRCDIR([src/xim/xim.c])
AM_INIT_AUTOMAKE([dist-bzip2])
AC_CONFIG_HEADER([config.h])
AC_PROG_LIBTOOL

# Checks for programs.
AC_PROG_CXX
AC_PROG_CC
AC_PROG_CPP
AC_PROG_RANLIB
AC_PROG_INSTALL
AC_PROG_LN_S
AC_PROG_MAKE_SET
#AC_PROG_RANLIB

#### customization

AC_MSG_CHECKING([for some Win32 platform])
case "$host" in
  *-*-mingw*|*-*-cygwin*)
    platform_win32=yes
    ;;
  *)
    platform_win32=no
    ;;
esac
AC_MSG_RESULT([$platform_win32])
AM_CONDITIONAL(PLATFORM_WIN32, test "$platform_win32" = "yes")
####

# Checks for header files.
AC_HEADER_STDC
AC_HEADER_STDBOOL
AC_PATH_X
AC_HEADER_SYS_WAIT
AC_CHECK_HEADERS([fcntl.h libintl.h locale.h malloc.h memory.h stddef.h stdlib.h string.h sys/param.h unistd.h time.h])

# Checks for typedefs, structures, and compiler characteristics.
AC_C_CONST
AC_C_INLINE
AC_TYPE_PID_T
AC_TYPE_SIZE_T

#Check for X
AC_PATH_XTRA

# Checks for library functions.
AC_FUNC_FORK
AC_FUNC_MALLOC
AC_FUNC_SETVBUF_REVERSED
AC_TYPE_SIGNAL
AC_CHECK_FUNCS([gethostname memmove memset setlocale strcasecmp strchr strdup strrchr strtol strtoul mkdir strstr])

AC_ARG_WITH(unikey-gtk, 
	AC_HELP_STRING([--with-unikey-gtk],[Include Unikey GTK IM module (Default: excluded)]), 
	[unikey_gtk=$withval], [unikey_gtk=no])
AC_SUBST(unikey_gtk)

AC_ARG_WITH(gtk-sysconfdir, 
	AC_HELP_STRING([--with-gtk-sysconfdir=PATH],
		[GTK sysconfdir [[default=/etc/gtk-2.0]]]), 
	gtk_sysconfdir=$with_gtk_sysconfdir, [gtk_sysconfdir=/etc/gtk-2.0])
AC_SUBST(gtk_sysconfdir)

AC_ARG_WITH(libdir, 
	AC_HELP_STRING([--with-libdir=PATH],
		[System lib dir to install gtk module. [[automatic]]]), 
	sys_libdir=$with_libdir, [sys_libdir=""])
AC_SUBST(sys_libdir)

if test "X$unikey_gtk" = "Xyes" ; then
	PKG_CHECK_MODULES(GTK, gtk+-2.0 >= 2.2.0)

	AC_MSG_CHECKING([gtk version])
		GTK_VERSION=`pkg-config gtk+-2.0 --variable=gtk_binary_version`
		AC_MSG_RESULT([$GTK_VERSION])
		AC_SUBST(GTK_VERSION)

	AC_MSG_CHECKING([gtk prefix])
		GTK_PREFIX=`pkg-config gtk+-2.0 --variable=prefix`
		AC_MSG_RESULT([$GTK_PREFIX])
		AC_SUBST(GTK_PREFIX)

	if test "X$sys_libdir" = "X" ; then
	    AC_MSG_CHECKING([gtk libdir])
	    GTK_LIBDIR=`pkg-config gtk+-2.0 --variable=libdir`
	    AC_MSG_RESULT([$GTK_LIBDIR])
	else
	    GTK_LIBDIR=$sys_libdir
	fi
	AC_SUBST(GTK_LIBDIR)
	    
	AC_MSG_CHECKING(whether make is GNU Make)
	STRIP_BEGIN=
	STRIP_END=
	if $ac_make --version 2>/dev/null | grep '^GNU Make ' >/dev/null ; then
		STRIP_BEGIN='$(strip $(STRIP_DUMMY)'
		STRIP_END=')'
		AC_MSG_RESULT(yes)
	else
		AC_MSG_RESULT(no)
	fi
	STRIP_DUMMY=
	AC_SUBST(STRIP_DUMMY)
	AC_SUBST(STRIP_BEGIN)
	AC_SUBST(STRIP_END)

	# Some variables used in source file
	AM_GLIB_DEFINE_LOCALEDIR(GTK_LOCALEDIR)
	GETTEXT_PACKAGE=gtk20
	AC_SUBST(GETTEXT_PACKAGE)
	AC_DEFINE_UNQUOTED(GETTEXT_PACKAGE,"$GETTEXT_PACKAGE", ["$GETTEXT_PACKAGE"])

fi

AM_CONDITIONAL([COND_UNIKEY_GTK], [test "X$unikey_gtk" = "Xyes"])

AH_VERBATIM([_GNU_SOURCE],
[/* Enable GNU extensions on systems that have them.  */
#ifndef _GNU_SOURCE
# define _GNU_SOURCE
#endif])

AC_CONFIG_FILES([Makefile 
  src/Makefile 
  src/ukengine/Makefile
  src/unikey-gtk/Makefile
  src/IMdkit/Makefile
  src/gui/Makefile
  src/byteio/Makefile
  src/ukinterface/Makefile
  src/vnconv/Makefile
  src/xim/Makefile])
AC_OUTPUT
