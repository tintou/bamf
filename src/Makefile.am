INCLUDE = 		\
	-I.		\
	-I$(srcdir)	\
	$(NULL)

wncksyncdaemondir = $(libexecdir)

wncksyncdaemon_PROGRAMS = \
		wncksyncdaemon \
		$(NULL)

wncksyncdaemon_sources = \
		main.c \
		wncksyncdbus.c \
		windowmatcher.c \
		$(NULL)

wncksyncdaemon_headers = \
		main.h \
		windowmatcher.h \
		wncksyncdbus.h \
		$(NULL)

wncksyncdaemon_SOURCES = \
		$(wncksyncdaemon_sources) \
		$(wncksyncdaemon_headers) \
		$(NULL)

nodist_EXTRA_wncksyncdaemon_SOURCES = \
	wncksyncdbus-glue.h

DISTCLEANFILES = \
	wncksyncdbus-glue.h

wncksyncdaemon_LDADD = \
		$(X_LIBS) \
		$(GLIB_LIBS) \
		$(GTK_LIBS) \
		$(DBUS_LIBS) \
		$(WNCK_LIBS) \
		$(GTOP_LIBS) \
		$(NULL)

wncksyncdaemon_CFLAGS = \
		-Wall -std=c99 \
		-DDATADIR=\""$(datadir)"\" \
		$(GLIB_CFLAGS) \
		$(GTK_CFLAGS) \
		$(DBUS_CFLAGS) \
		$(WNCK_CFLAGS) \
		$(GTOP_CFLAGS) \
		$(NULL)

wncksyncdaemon_LDFLAGS = \
		-Xlinker -export-dynamic -Wl,-O1 -Wl,-Bsymbolic-functions \
		$(NULL)

wncksyncdbus-glue.h: wncksyncdbus-glue.xml Makefile
		$(LIBTOOL) --mode=execute $(DBUS_GLIB_BIN)/dbus-binding-tool --prefix=wncksync_dbus --mode=glib-server --output=$@ $<

BUILT_SOURCES = \
	wncksyncdbus-glue.h

EXTRA_DIST = \
	wncksyncdbus-glue.xml
