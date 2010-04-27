INCLUDE = 		\
	-I.		\
	-I$(srcdir)	\
	$(NULL)

bamfdaemondir = $(libexecdir)

bamfdaemon_PROGRAMS = \
		bamfdaemon \
		$(NULL)

bamfdaemon_sources = \
		main.c \
		bamf-view.c \
		bamf-control.c \
		bamf-matcher.c \
		bamf-application.c \
		marshal.c \
		$(NULL)

bamfdaemon_headers = \
		main.h \
		bamf.h \
		bamf-view.h \
		bamf-control.h \
		bamf-matcher.h \
		bamf-application.h \
		bamfdbus-glue.h \
		marshal.h \
		$(NULL)

bamfdaemon_SOURCES = \
		$(bamfdaemon_sources) \
		$(bamfdaemon_headers) \
		$(NULL)

nodist_EXTRA_bamfdaemon_SOURCES = \
	bamfdbus-glue.h

DISTCLEANFILES = \
	bamfdbus-glue.h

bamfdaemon_LDADD = \
		$(X_LIBS) \
		$(GLIB_LIBS) \
		$(GTK_LIBS) \
		$(DBUS_LIBS) \
		$(WNCK_LIBS) \
		$(GTOP_LIBS) \
		$(NULL)

bamfdaemon_CFLAGS = \
		-Wall -std=c99 \
		-DDATADIR=\""$(datadir)"\" \
		$(GLIB_CFLAGS) \
		$(GTK_CFLAGS) \
		$(DBUS_CFLAGS) \
		$(WNCK_CFLAGS) \
		$(GTOP_CFLAGS) \
		$(NULL)

bamfdaemon_LDFLAGS = \
		-Xlinker -export-dynamic -Wl,-O1 -Wl,-Bsymbolic-functions \
		$(NULL)

bamfdbus-glue.h: bamfdbus-glue.xml Makefile
		$(LIBTOOL) --mode=execute $(DBUS_GLIB_BIN)/dbus-binding-tool --prefix=bamf --mode=glib-server --output=$@ $<

BUILT_SOURCES = \
	bamfdbus-glue.h

EXTRA_DIST = \
	bamfdbus-glue.xml
