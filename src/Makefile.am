# Defined as these will be needed by Makefile.am.marshal
CLEANFILES =
DISTCLEANFILES =
BUILT_SOURCES =
EXTRA_DIST =

include $(top_srcdir)/Makefile.am.marshal

bamfdaemondir = $(libexecdir)

bamfdaemon_PROGRAMS = \
		bamfdaemon \
		$(NULL)

bamfdaemon_gdbus_files = \
		bamf-gdbus-generated.c \
		bamf-gdbus-generated.h \
		bamf-gdbus-view-generated.c \
		bamf-gdbus-view-generated.h \
		bamf-gdbus-browser-generated.c \
		bamf-gdbus-browser-generated.h \
		bamf-gdbus-indicator-source-generated.c \
		bamf-gdbus-indicator-source-generated.h \
		$(NULL)

bamfdaemon_xml_gdbus_files = \
		org.ayatana.bamf.xml \
		org.ayatana.bamf.view.xml \
		org.ayatana.bamf.browser.xml \
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
		bamf-indicator-source.c \
		bamf-indicator.c \
		bamf-xutils.c \
		$(NULL)

bamfdaemon_headers = \
		main.h \
		bamf.h \
		bamf-daemon.h \
		bamf-legacy-window.h \
		bamf-legacy-window-test.h \
		bamf-legacy-screen.h \
		bamf-legacy-screen-private.h \
		bamf-view.h \
		bamf-control.h \
		bamf-matcher.h \
		bamf-matcher-private.h \
		bamf-window.h \
		bamf-application.h \
		bamf-tab.h \
		bamf-indicator-source.h \
		bamf-indicator.h \
		bamf-xutils.h \
		$(NULL)

bamfdaemon_webapps_sources = \
		bamf-unity-webapps-tab.c \
		bamf-unity-webapps-observer.c \
		bamf-unity-webapps-application.c \
		$(NULL)

bamfdaemon_webapps_headers = \
		bamf-unity-webapps-tab.h \
		bamf-unity-webapps-observer.h \
		bamf-unity-webapps-application.h \
		$(NULL)

bamfdaemon_SOURCES = \
		$(bamfdaemon_sources) \
		$(bamfdaemon_headers) \
		$(NULL)

nodist_bamfdaemon_SOURCES = \
		$(bamfdaemon_built_sources) \
		$(NULL)

if HAVE_WEBAPPS
bamfdaemon_SOURCES += \
		$(bamfdaemon_webapps_sources) \
		$(bamfdaemon_webapps_headers) \
		$(NULL)
endif

EXTRA_DIST += \
		$(bamfdaemon_webapps_sources) \
		$(bamfdaemon_webapps_headers) \
		$(NULL)

glib_marshal_list = bamf-marshal.list
glib_marshal_prefix = bamf_marshal

# resetting BUILT_SOURCES here is intended
BUILT_SOURCES = \
		$(bamfdaemon_built_sources) \
		$(NULL)

CLEANFILES += \
		$(bamfdaemon_built_sources) \
		$(NULL)

bamfdaemon_LDADD = \
		$(X_LIBS) \
		$(GLIB_LIBS) \
		$(GTK_LIBS) \
		$(WNCK_LIBS) \
		$(GTOP_LIBS) \
		$(LIBUNITY_WEBAPPS_LIBS) \
		$(NULL)

bamfdaemon_CFLAGS = \
		-Wall -std=c99 \
		-DDATADIR=\""$(datadir)"\" \
		-DWNCK_I_KNOW_THIS_IS_UNSTABLE \
		-I$(srcdir) \
		-I$(builddir) \
		$(GLIB_CFLAGS) \
		$(GTK_CFLAGS) \
		$(WNCK_CFLAGS) \
		$(GTOP_CFLAGS) \
		$(LIBUNITY_WEBAPPS_CFLAGS) \
		$(NULL)

bamfdaemon_LDFLAGS = \
		-Xlinker -export-dynamic -Wl,-O1 -Wl,-Bsymbolic-functions \
		$(NULL)

bamf-gdbus-generated.c bamf-gdbus-generated.h: org.ayatana.bamf.xml
		$(AM_V_GEN) $(GDBUS_CODEGEN) \
		--generate-c-code bamf-gdbus-generated \
		--c-namespace BamfDBus \
		--interface-prefix org.ayatana.bamf \
		$<

bamf-gdbus-view-generated.c bamf-gdbus-view-generated.h: org.ayatana.bamf.view.xml
		$(AM_V_GEN) $(GDBUS_CODEGEN) \
		--c-generate-object-manager \
		--generate-c-code bamf-gdbus-view-generated \
		--c-namespace BamfDBusItem \
		--interface-prefix org.ayatana.bamf \
		$<

bamf-gdbus-browser-generated.c bamf-gdbus-browser-generated.h: org.ayatana.bamf.browser.xml
		$(AM_V_GEN) $(GDBUS_CODEGEN) \
		--generate-c-code bamf-gdbus-browser-generated \
		--c-namespace BamfDBusBrowser \
		--interface-prefix org.ayatana.bamf.browser \
		$<

bamf-gdbus-indicator-source-generated.c bamf-gdbus-indicator-source-generated.h: com.canonical.StatusNotifierApprover.xml
		$(AM_V_GEN) $(GDBUS_CODEGEN) \
		--generate-c-code bamf-gdbus-indicator-source-generated \
		--c-namespace StatusNotifierApprover \
		--interface-prefix com.canonical.StatusNotifierApprover \
		$<

EXTRA_DIST += \
		$(bamfdaemon_xml_gdbus_files) \
		$(NULL)

