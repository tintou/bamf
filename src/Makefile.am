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
		bamf-window.c \
		marshal.c \
		$(NULL)

bamfdaemon_headers = \
		main.h \
		bamf.h \
		bamf-view.h \
		bamf-control.h \
		bamf-matcher.h \
		bamf-window.h \
		bamf-application.h \
		bamf-application-glue.h \
		bamf-control-glue.h \
		bamf-matcher-glue.h \
		bamf-window-glue.h \
		bamf-view-glue.h \
		marshal.h \
		$(NULL)

bamfdaemon_SOURCES = \
		$(bamfdaemon_sources) \
		$(bamfdaemon_headers) \
		$(NULL)

nodist_EXTRA_bamfdaemon_SOURCES = \
	bamf-application-glue.h \
	bamf-control-glue.h \
	bamf-matcher-glue.h \
        bamf-window-glue.h \
        bamf-view-glue.h \
	$(NULL)

DISTCLEANFILES = \
	bamf-application-glue.h \
	bamf-control-glue.h \
	bamf-matcher-glue.h \
	bamf-window-glue.h \
	bamf-view-glue.h \
	$(NULL)

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

bamf-application-glue.h: bamf-application-glue.xml Makefile
		$(LIBTOOL) --mode=execute $(DBUS_GLIB_BIN)/dbus-binding-tool --prefix=bamf_application --mode=glib-server --output=$@ $<

bamf-control-glue.h: bamf-control-glue.xml Makefile
		$(LIBTOOL) --mode=execute $(DBUS_GLIB_BIN)/dbus-binding-tool --prefix=bamf_control --mode=glib-server --output=$@ $<

bamf-matcher-glue.h: bamf-matcher-glue.xml Makefile
		$(LIBTOOL) --mode=execute $(DBUS_GLIB_BIN)/dbus-binding-tool --prefix=bamf_matcher --mode=glib-server --output=$@ $<

bamf-window-glue.h: bamf-window-glue.xml Makefile
		$(LIBTOOL) --mode=execute $(DBUS_GLIB_BIN)/dbus-binding-tool --prefix=bamf_window --mode=glib-server --output=$@ $<

bamf-view-glue.h: bamf-view-glue.xml Makefile
		$(LIBTOOL) --mode=execute $(DBUS_GLIB_BIN)/dbus-binding-tool --prefix=bamf_view --mode=glib-server --output=$@ $<

BUILT_SOURCES = \
	bamf-application-glue.h \
	bamf-control-glue.h \
	bamf-matcher-glue.h \
	bamf-window-glue.h \
	bamf-view-glue.h \
	$(NULL)

EXTRA_DIST = \
	bamf-application-glue.xml \
	bamf-control-glue.xml \
	bamf-matcher-glue.xml \
	bamf-window-glue.xml \
	bamf-view-glue.xml \
	$(NULL)
