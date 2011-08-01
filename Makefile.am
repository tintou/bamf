ACLOCAL_AMFLAGS = -I m4

SUBDIRS = \
  lib/libbamf \
  src \
  tests/functional \
  tests/bamfdaemon \
  tests/libbamf \
  data \
  doc

EXTRA_DIST = COPYING.LGPL autogen.sh build.rules.mk m4/i18n.m4 m4/programs.m4 m4/util.m4
DISTCHECK_CONFIGURE_FLAGS = --enable-introspection=no --enable-gtk-doc

