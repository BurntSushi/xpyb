dnl Detection and configuration of the visibility feature of gcc
dnl Vincent Torri 2006-02-11
dnl
dnl XCB_EXTENSION(name, default)
dnl set the X extension
dnl
AC_DEFUN([XCB_EXTENSION],
[
pushdef([UP], translit([$1], [-a-z], [_A-Z]))dnl
pushdef([DOWN], translit([$1], [A-Z], [a-z]))dnl

AC_ARG_ENABLE(DOWN,
    [AS_HELP_STRING([--enable-[]DOWN], [Build XPyB $1 Extension (default: $2)])],
    [BUILD_[]UP=$enableval],
    [BUILD_[]UP=$2])

AM_CONDITIONAL(BUILD_[]UP, [test "x$BUILD_[]UP" = "xyes"])
])

dnl End of acinclude.m4
