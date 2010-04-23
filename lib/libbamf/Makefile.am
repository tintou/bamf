lib_LTLIBRARIES = libbamf.la

libbamf_la_SOURCES = \
		libbamf.h \
		libbamf.c
		
libbamf_ladir = $(libdir)/pkgconfig
libbamf_la_DATA = \
		libbamf.pc
			
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
library_include_HEADERS = libbamf.h
