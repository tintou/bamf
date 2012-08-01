ACLOCAL_AMFLAGS = -I m4

SUBDIRS = \
  lib/libbamf \
  src \
  tests/functional \
  tests/bamfdaemon \
  tests/libbamf \
  data \
  doc

EXTRA_DIST = COPYING.LGPL autogen.sh build.rules.mk
DISTCHECK_CONFIGURE_FLAGS = --enable-introspection=no --enable-gtk-doc=yes

