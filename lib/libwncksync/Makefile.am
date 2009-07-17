lib_LTLIBRARIES = libwncksync.la

libwncksync_la_SOURCES = \
		libwncksync.h \
		libwncksync.a
			
libwncksync_la_LIBADD = \
		$(GLIB_LIBS) \
		$(DBUS_LIBS) \
		$(NULL)
		
libwncksync_la_CFLAGS = \
		-Wall -std=c99 \
		-DDATADIR=\""$(datadir)"\" \
		$(GTK_CFLAGS) \
		$(DBUS_CFLAGS) \
		$(NULL)

libwncksync_la_LDFLAGS = \
		-Xlinker -export-dynamic -Wl,-O1 -Wl,-Bsymbolic-functions \
		$(NULL)
