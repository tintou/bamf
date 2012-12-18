noinst_PROGRAMS = \
  test-bamf

bamf_test_extra_built_sources = \
		$(top_builddir)/src/bamf-marshal.c \
		$(top_builddir)/src/bamf-marshal.h \
		$(top_builddir)/src/bamf-gdbus-generated.c \
		$(top_builddir)/src/bamf-gdbus-generated.h \
		$(top_builddir)/src/bamf-gdbus-view-generated.c \
		$(top_builddir)/src/bamf-gdbus-view-generated.h \
		$(top_builddir)/src/bamf-gdbus-browser-generated.c \
		$(top_builddir)/src/bamf-gdbus-browser-generated.h \
		$(NULL)

bamf_test_extra_sources = \
		$(top_srcdir)/src/bamf-daemon.c \
		$(top_srcdir)/src/bamf-legacy-window.c \
		$(top_srcdir)/src/bamf-legacy-window-test.c \
		$(top_srcdir)/src/bamf-legacy-screen.c \
		$(top_srcdir)/src/bamf-view.c \
		$(top_srcdir)/src/bamf-control.c \
		$(top_srcdir)/src/bamf-matcher.c \
		$(top_srcdir)/src/bamf-application.c \
		$(top_srcdir)/src/bamf-window.c \
		$(top_srcdir)/src/bamf-tab.c \
		$(top_srcdir)/src/bamf-xutils.c \
		$(NULL)

bamf_test_extra_headers = \
		$(top_srcdir)/src/main.h \
		$(top_srcdir)/src/bamf.h \
		$(top_srcdir)/src/bamf-daemon.h \
		$(top_srcdir)/src/bamf-legacy-window.h \
		$(top_srcdir)/src/bamf-legacy-window-test.h \
		$(top_srcdir)/src/bamf-legacy-screen.h \
		$(top_srcdir)/src/bamf-view.h \
		$(top_srcdir)/src/bamf-control.h \
		$(top_srcdir)/src/bamf-matcher.h \
		$(top_srcdir)/src/bamf-window.h \
		$(top_srcdir)/src/bamf-tab.h \
		$(top_srcdir)/src/bamf-application.h \
		$(top_srcdir)/src/bamf-xutils.h \
		$(NULL)

bamf_test_webapps_sources = \
		$(top_srcdir)/src/bamf-unity-webapps-tab.c \
		$(top_srcdir)/src/bamf-unity-webapps-observer.c \
		$(top_srcdir)/src/bamf-unity-webapps-application.c \
		$(NULL)

test_bamf_SOURCES = \
  $(bamf_test_extra_sources) \
  $(bamf_test_extra_headers) \
  test-bamf.c \
  test-view.c \
  test-application.c \
  test-window.c \
  test-matcher.c

nodist_test_bamf_SOURCES = \
  $(bamf_test_extra_built_sources)

BUILT_SOURCES = \
  $(bamf_test_extra_built_sources)

CLEANFILES = \
  $(bamf_test_extra_built_sources)

if HAVE_WEBAPPS
test_bamf_SOURCES += \
		$(bamf_test_webapps_sources) \
		$(NULL)
endif

test_bamf_CPPFLAGS = \
  -I$(top_srcdir) \
  -I$(top_srcdir)/src \
  -I$(top_builddir)/src \
  -DWNCK_I_KNOW_THIS_IS_UNSTABLE \
  -DTESTDIR=\""$(abs_top_srcdir)/tests"\" \
  $(GCC_FLAGS) \
  $(GTK_CFLAGS) \
  $(GLIB_CFLAGS) \
  $(WNCK_CFLAGS) \
  $(GTOP_CFLAGS) \
  $(LIBUNITY_WEBAPPS_CFLAGS) \
  $(MAINTAINER_CFLAGS)

test_bamf_LDADD = \
  $(X_LIBS) \
  $(GLIB_LIBS) \
  $(WNCK_LIBS) \
  $(GTOP_LIBS) \
  $(GTK_LIBS) \
  $(LIBUNITY_WEBAPPS_LIBS) 

test_bamf_LDFLAGS = \
		-Xlinker -export-dynamic -Wl,-O1 -Wl,-Bsymbolic-functions \
		$(NULL)

EXTRA_DIST = \
		data \
		$(bamf_test_webapps_sources) \
		$(NULL)

TEST_BINARY = test-bamf
include $(top_srcdir)/tests/Makefile.am.gtests
