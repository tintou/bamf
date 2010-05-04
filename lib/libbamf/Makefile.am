lib_LTLIBRARIES = libbamf.la

sources_h = \
  bamf-application.h \
  bamf-control.h \
  bamf-matcher.h \
  bamf-view.h \
  bamf-window.h \
  bamf-factory.h \
  marshal.h \
  libbamf.h

libbamf_la_SOURCES = \
  bamf-application.c \
  bamf-control.c \
  bamf-matcher.c \
  bamf-view.c \
  bamf-window.c \
  bamf-factory.c \
  marshal.c \
  $(sources_h)

libbamf_la_LIBADD = \
  $(LIBWSBIND_LIBS) \
  $(GLIB_LIBS) \
  $(DBUS_LIBS) \
  $(NULL)

libbamf_la_CPPFLAGS = \
  -Wall -std=c99 \
  -fPIC \
  -DDATADIR=\""$(datadir)"\" \
  -I$(top_srcdir)/lib \
  $(LIBWSBIND_CFLAGS) \
  $(GLIB_CFLAGS) \
  $(DBUS_CFLAGS) \
  $(WNCK_CFLAGS) \
  $(NULL)

libbamf_la_LDFLAGS = \
  -shared -Xlinker -export-dynamic -Wl,-O1 -Wl,-Bsymbolic-functions \
  $(NULL)

library_includedir=$(includedir)/libbamf/libbamf
library_include_HEADERS = \
  $(sources_h)

libbamf_pcdir = $(libdir)/pkgconfig
libbamf_pc_DATA = \
  libbamf.pc
