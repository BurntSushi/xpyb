#                                               -*- Autoconf -*-
# Process this file with autoconf to produce a configure script.

AC_PREREQ(2.57)
AC_INIT([xpyb],
        1.2,
        [xcb@lists.freedesktop.org])
AC_CONFIG_SRCDIR([xpyb.pc.in])
AC_CONFIG_MACRO_DIR([m4])
AC_DISABLE_STATIC
AC_PROG_LIBTOOL
AC_PROG_CC

AC_CONFIG_HEADERS([src/config.h])

AM_INIT_AUTOMAKE([foreign dist-bzip2])
AM_PATH_PYTHON([2.5])
AC_MSG_CHECKING([for python include dir])
PYTHON_INCLUDE=`$PYTHON -c "from distutils import sysconfig; \
			    print sysconfig.get_python_inc()"`
if test -z "${PYTHON_INCLUDE}"; then
   AC_MSG_ERROR([Failed to find Python include directory.
		 Check your Python installation.])
fi

AC_MSG_RESULT([$PYTHON_INCLUDE])
AC_SUBST([PYTHON_INCLUDE])

# Checks for pkg-config packages
PKG_CHECK_MODULES(XCBPROTO, xcb-proto >= 1.6)
PKG_CHECK_MODULES(LIBXCB, xcb > 1.5)

# Find the xcb-proto protocol descriptions
AC_MSG_CHECKING([for xcb-proto include dir])
XCBPROTO_XCBINCLUDEDIR=`$PKG_CONFIG --variable=xcbincludedir xcb-proto`
AC_MSG_RESULT($XCBPROTO_XCBINCLUDEDIR)
AC_SUBST(XCBPROTO_XCBINCLUDEDIR)

# Find the xcbgen Python package
AC_MSG_CHECKING([for xcb-proto python dir])
XCBPROTO_XCBPYTHONDIR=`$PKG_CONFIG --variable=pythondir xcb-proto`
AC_MSG_RESULT($XCBPROTO_XCBPYTHONDIR)
AC_SUBST(XCBPROTO_XCBPYTHONDIR)

AC_HEADER_STDC
if  test "x$GCC" = xyes ; then
    CWARNFLAGS="-Wall -Wmissing-declarations"
else
    AC_CHECK_DECL([__SUNPRO_C], [SUNCC="yes"], [SUNCC="no"])
    if test "x$SUNCC" = "xyes"; then
	CWARNFLAGS="-v"
    fi
fi
AC_SUBST(CWARNFLAGS)

XCB_EXTENSION(Composite, "yes")
XCB_EXTENSION(Damage, "yes")
XCB_EXTENSION(DPMS, "yes")
dnl XCB_EXTENSION(DRI2, "yes")
XCB_EXTENSION(GLX, "yes")
XCB_EXTENSION(RandR, "yes")
XCB_EXTENSION(Record, "yes")
XCB_EXTENSION(Render, "yes")
XCB_EXTENSION(Resource, "yes")
XCB_EXTENSION(Screensaver, "yes")
XCB_EXTENSION(Shape, "yes")
XCB_EXTENSION(Shm, "yes")
XCB_EXTENSION(Sync, "yes")
XCB_EXTENSION(Xevie, "yes")
XCB_EXTENSION(XFixes, "yes")
XCB_EXTENSION(XFree86-DRI, "yes")
XCB_EXTENSION(Xinerama, "yes")
XCB_EXTENSION(XInput, "no")
XCB_EXTENSION(Xprint, "yes")
XCB_EXTENSION(SELinux, "no")
XCB_EXTENSION(XTest, "yes")
XCB_EXTENSION(Xv, "yes")
XCB_EXTENSION(XvMC, "yes")

AC_CONFIG_FILES([Makefile src/Makefile doc/Makefile])
AC_CONFIG_FILES([xpyb.pc])

AC_OUTPUT

dnl Configuration output

echo ""
echo "  Package: ${PACKAGE_NAME} ${PACKAGE_VERSION}"
echo ""
echo "  Used CFLAGS:"
echo "    CPPFLAGS............: ${CPPFLAGS}"
echo "    CFLAGS..............: ${CFLAGS}"
echo "    Warning CFLAGS......: ${CWARNFLAGS}"
echo ""
echo "  Installation:"
echo "    Prefix..............: ${prefix}"
echo ""
