CLEANFILES =
DISTCLEANFILES =
BUILT_SOURCES =
EXTRA_DIST = 

include $(top_srcdir)/Makefile.am.marshal

INCLUDE = \
		-I. \
		-I$(srcdir) \
		$(NULL)

bamfdaemondir = $(libexecdir)

bamfdaemon_PROGRAMS = \
		bamfdaemon \
		$(NULL)

bamfdaemon_gdbus_files = \
		bamf-gdbus-indicator-source-generated.c \
		bamf-gdbus-indicator-source-generated.h \
		$(NULL)

bamfdaemon_xml_gdbus_files = \
		com.canonical.StatusNotifierApprover.xml \
		$(NULL)

bamfdaemon_built_sources = \
		bamf-marshal.c \
		bamf-marshal.h \
		$(bamfdaemon_gdbus_files) \
		$(NULL)

bamfdaemon_sources = \
		main.c \
		bamf-daemon.c \
		bamf-legacy-window.c \
		bamf-legacy-window-test.c \
		bamf-legacy-screen.c \
		bamf-view.c \
		bamf-control.c \
		bamf-matcher.c \
		bamf-application.c \
		bamf-window.c \
		bamf-tab.c \
		bamf-tab-source.c \
		bamf-indicator-source.c \
		bamf-indicator.c \
		$(NULL)

bamfdaemon_headers = \
		main.h \
		bamf.h \
		bamf-daemon.h \
		bamf-legacy-window.h \
		bamf-legacy-window-test.h \
		bamf-legacy-screen.h \
		bamf-view.h \
		bamf-control.h \
		bamf-matcher.h \
		bamf-window.h \
		bamf-application.h \
		bamf-tab.h \
		bamf-tab-source.h \
		bamf-indicator-source.h \
		bamf-indicator.h \
		$(NULL)

bamfdaemon_SOURCES = \
		$(bamfdaemon_built_sources) \
		$(bamfdaemon_sources) \
		$(bamfdaemon_headers) \
		$(NULL)

nodist_EXTRA_bamfdaemon_SOURCES = \
	$(bamfdaemon_gdbus_files) \
	$(NULL)

glib_marshal_list = bamf-marshal.list
glib_marshal_prefix = bamf_marshal

bamfdaemon_LDADD = \
		$(top_builddir)/lib/libbamf-private/libbamf-private.la \
		$(X_LIBS) \
		$(GLIB_LIBS) \
		$(GTK_LIBS) \
		$(WNCK_LIBS) \
		$(GTOP_LIBS) \
		$(NULL)

bamfdaemon_CFLAGS = \
		-I$(top_builddir)/lib \
		-Wall -std=c99 \
		-DDATADIR=\""$(datadir)"\" \
		-DWNCK_I_KNOW_THIS_IS_UNSTABLE \
		$(GLIB_CFLAGS) \
		$(GTK_CFLAGS) \
		$(WNCK_CFLAGS) \
		$(GTOP_CFLAGS) \
		$(NULL)

bamfdaemon_LDFLAGS = \
		-Xlinker -export-dynamic -Wl,-O1 -Wl,-Bsymbolic-functions \
		$(NULL)

bamf-gdbus-indicator-source-generated.c bamf-gdbus-indicator-source-generated.h: com.canonical.StatusNotifierApprover.xml
		$(AM_V_GEN) $(GDBUS_CODEGEN) \
		--generate-c-code bamf-gdbus-indicator-source-generated \
		--c-namespace StatusNotifierApprover \
		--interface-prefix com.canonical.StatusNotifierApprover \
		$<

BUILT_SOURCES += \
		$(bamfdaemon_gdbus_files) \
		$(NULL)

EXTRA_DIST += \
		$(bamfdaemon_xml_gdbus_files) \
		$(NULL)

DISTCLEANFILES += \
		$(bamfdaemon_built_sources) \
		$(NULL)

CLEANFILES += \
		$(bamfdaemon_built_sources) \
		$(NULL)
