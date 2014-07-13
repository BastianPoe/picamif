AC_INIT([PiCamIf], 0.0.1, bastian@poettner.de)
AC_CONFIG_MACRO_DIR([m4])
AC_CONFIG_SRCDIR(src/Main.cpp)
AC_CONFIG_HEADER([src/config.h])
LT_INIT
AM_INIT_AUTOMAKE
AM_MAINTAINER_MODE

# Test for new silent rules and enable only if they are available
m4_ifdef([AM_SILENT_RULES], [AM_SILENT_RULES([yes])])

AC_PROG_CC
AC_PROG_CXX

AC_PROG_INSTALL
AC_LANG_CPLUSPLUS

PKG_CHECK_MODULES([GPHOTO], [libgphoto2])
AC_SUBST(GPHOTO_CFLAGS)
AC_SUBST(GPHOTO_LIBS)

AC_CONFIG_FILES([Makefile src/Makefile])

AC_OUTPUT
