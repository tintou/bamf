noinst_PROGRAMS = \
  test-libbamf

test_libbamf_SOURCES = \
  test-libbamf.c \
  test-matcher.c

test_libbamf_CFLAGS = \
  -I$(top_srcdir)/lib \
  -I$(top_srcdir)/lib/libbamf \
  -I$(top_builddir)/lib/libbamf \
  -DTESTDIR=\""$(abs_top_srcdir)/tests/libbamf"\" \
  $(GCC_FLAGS) \
  $(DBUS_CFLAGS) \
  $(GLIB_CFLAGS)

test_libbamf_LDADD = \
  $(top_builddir)/lib/libbamf/libbamf$(LIBBAMF_VER).la \
  $(GLIB_LIBS) \
  $(DBUS_LIBS)

TEST_BINARY = test-libbamf
include $(top_srcdir)/tests/Makefile.am.gtests
