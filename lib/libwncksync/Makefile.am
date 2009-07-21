PKG_CONFIG_FILES = libwncksync.pc

lib_LTLIBRARIES = libwncksync.la

libwncksync_la_SOURCES = \
		libwncksync.h \
		libwncksync.c
		
libwncksync_ladir = $(libdir)/pkgconfig
libwncksync_la_DATA = \
		libwncksync.pc
			
libwncksync_la_LIBADD = \
		$(GLIB_LIBS) \
		$(DBUS_LIBS) \
		$(WNCK_LIBS) \
		$(NULL)
		
libwncksync_la_CFLAGS = \
		-Wall -std=c99 \
		-fPIC \
		-DDATADIR=\""$(datadir)"\" \
		$(GTK_CFLAGS) \
		$(DBUS_CFLAGS) \
		$(WNCK_CFLAGS) \
		$(NULL)

libwncksync_la_LDFLAGS = \
		-shared -Xlinker -export-dynamic -Wl,-O1 -Wl,-Bsymbolic-functions \
		$(NULL)
		
library_includedir=$(includedir)/libwncksync/libwncksync
library_include_HEADERS = libwncksync.h
