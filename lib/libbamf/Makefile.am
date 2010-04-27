lib_LTLIBRARIES = libbamf.la

sources_h = \
  bamf-proxy.h \
  libbamf.h

libbamf_la_SOURCES = \
  bamf-proxy.c \
  $(sources_h)

libbamf_la_LIBADD = \
  $(LIBWSBIND_LIBS) \
  $(GLIB_LIBS) \
  $(DBUS_LIBS) \
  $(NULL)

libbamf_la_CFLAGS = \
  -Wall -std=c99 \
  -fPIC \
  -DDATADIR=\""$(datadir)"\" \
  $(LIBWSBIND_CFLAGS) \
  $(GLIB_CFLAGS) \
  $(DBUS_CFLAGS) \
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
