#                                               -*- Autoconf -*-
# Process this file with autoconf to produce a configure script.

AC_PREREQ([2.63])
AC_INIT(bamf, 0.4.0, dx-team@canonical.com)
AC_PREREQ(2.62)
AC_SUBST(LIBBAMF_VER, 3)

AC_CONFIG_SRCDIR(src/main.c)
AC_CONFIG_HEADERS(config.h)

AM_INIT_AUTOMAKE([foreign tar-pax])
AM_SILENT_RULES([yes])

AC_CONFIG_MACRO_DIR([m4])

GNOME_COMMON_INIT
GNOME_COMPILE_WARNINGS(maximum)

AM_MAINTAINER_MODE
AM_DISABLE_STATIC
AM_PROG_LIBTOOL
AC_ISC_POSIX
AC_SUBST(ACLOCAL_AMFLAFS, "$ACLOCAL_FLAGS -I m4")

AC_PATH_PROG(PKG_CONFIG, pkg-config, no)
if test "x$PKG_CONFIG" = "xno"; then
        AC_MSG_ERROR([You need to install pkg-config])
fi

# Checks for programs.
AC_PROG_CC
AM_PROG_CC_C_O
AC_PROG_INSTALL
AC_HEADER_STDC

SHAMROCK_EXPAND_LIBDIR
SHAMROCK_EXPAND_BINDIR
SHAMROCK_EXPAND_DATADIR

#
# pkg-config
#
PKG_PROG_PKG_CONFIG

#
# glib
#
PKG_CHECK_MODULES(GLIB, glib-2.0 >= 2.32.0 gio-2.0 >= 2.30.0 gio-unix-2.0)

#
# gdbus-codegen
#
AC_PATH_PROG([GDBUS_CODEGEN], [gdbus-codegen], AC_MSG_ERROR([gdbus-codegen is required but was not found]))

#
# dbus
#
PKG_CHECK_MODULES(DBUS, dbus-glib-1 >= 0.76)
AC_SUBST(DBUS_LIBS)
AC_SUBST(DBUS_CFLAGS)

DBUS_GLIB_BIN="`$PKG_CONFIG --variable=exec_prefix dbus-glib-1`/bin"
AC_SUBST(DBUS_GLIB_BIN)

#
# libwnck
#
PKG_CHECK_MODULES(WNCK, libwnck-3.0)

#
# libgtop
#
PKG_CHECK_MODULES(GTOP, libgtop-2.0)
PKG_CHECK_MODULES(LIBWSBIND, glib-2.0 gdk-3.0 gdk-x11-3.0 gtk+-3.0)
AC_SUBST(LIBWSBIND_CFLAGS)
AC_SUBST(LIBWSBIND_LIBS)

#
# x11
#
PKG_CHECK_MODULES(X, x11)

###########################
# GObject Introspection
###########################

GOBJECT_INTROSPECTION_CHECK([0.10.2])

GIR_TYPELIBDIR=`$PKG_CONFIG --variable=typelibdir gobject-introspection-1.0`
AC_SUBST(GIR_TYPELIBDIR)
GIR_DATADIR=`$PKG_CONFIG --variable=girdir gobject-introspection-1.0`
AC_SUBST(GIR_DATADIR)

###########################
# Vala API Generation
###########################

AC_PATH_PROG([VALA_API_GEN], [vapigen])
AC_PATH_PROG([GLIB_GENMARSHAL], [glib-genmarshal])

###########################
# GTK-DOC
###########################
GTK_DOC_CHECK(1.0)

###########################
# libunity_webapps
###########################
AC_ARG_ENABLE([webapps],
              AS_HELP_STRING([--enable-webapps],
                             [Enable libunity-webapps support]),
              [enable_webapps=$enableval],
              [enable_webapps=yes])
if test "x$enable_webapps" = "xyes"; then
  PKG_CHECK_MODULES(LIBUNITY_WEBAPPS, libunity_webapps-0.2)
  AC_DEFINE_UNQUOTED(HAVE_WEBAPPS, , [Use libunity-webapps])
fi
AM_CONDITIONAL([HAVE_WEBAPPS], [test "x$enable_webapps" = "xyes"])
AC_SUBST(LIBUNITY_WEBAPPS_CFLAGS)
AC_SUBST(LIBUNITY_WEBAPPS_LIBS)

###########################
# Headless tests
###########################
AC_ARG_ENABLE([headless-tests],
              AS_HELP_STRING([--enable-headless-tests=@<:@no/yes@:>@],[enable headless test suite (requires Xvfb) @<:@default=no@:>@]),,
              [enable_headless_tests=no])

AM_CONDITIONAL([ENABLE_HEADLESS_TESTS],[test "x$enable_headless_tests" != "xno"])

if test "x$enable_headless_tests" = "xyes"; then
  AC_PATH_PROG([XVFB],[Xvfb])
  AC_PATH_PROG([DBUS_LAUNCH],[dbus-launch])
fi

###########################
# gcov coverage reporting
###########################
AC_TDD_GCOV
AM_CONDITIONAL([HAVE_GCOV], [test "x$ac_cv_check_gcov" = xyes])
AM_CONDITIONAL([HAVE_LCOV], [test "x$ac_cv_check_lcov" = xyes])
AM_CONDITIONAL([HAVE_GCOVR], [test "x$ac_cv_check_gcovr" = xyes])
AC_SUBST(COVERAGE_CFLAGS)
AC_SUBST(COVERAGE_LDFLAGS)

#
# Gnome/GTK checks
#
AM_PATH_GTK_3_0

dnl CFLAGS
CFLAGS="$CFLAGS -Wall -Werror -lm"

AC_SUBST(BAMFDAEMON_CFLAGS)
AC_SUBST(BAMFDAEMON_LIBS)

origgiomodulesdir=`pkg-config --variable=giomoduledir gio-2.0`
# Make giomodulesdir honour 'prefix', so that distcheck works. 
giomodulesdir=`echo "$origgiomodulesdir" | sed 's|/usr|${prefix}|'`
AC_SUBST(giomodulesdir)

AC_OUTPUT([
Makefile
lib/libbamf/Makefile
lib/libbamf/libbamf.pc
lib/libbamf/libbamf3.pc
src/Makefile
tests/functional/Makefile
tests/bamfdaemon/Makefile
tests/libbamf/Makefile
data/Makefile
doc/Makefile
doc/reference/Makefile
doc/reference/libbamf/Makefile
])

cat <<EOF

${PACKAGE}-${VERSION}

  Build Environment
    Install Prefix:     ${prefix}
    Introspection:      ${enable_introspection}
    Unity Webapps:      ${enable_webapps}
    Headless tests:     ${enable_headless_tests}
    Coverage Reporting: ${use_gcov}

EOF
