noinst_PROGRAMS = \
  test-bamf

XVFB_RUN=$(srcdir)/run-xvfb.sh

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
		$(top_srcdir)/src/bamf-unity-webapps-tab.c \
		$(top_srcdir)/src/bamf-unity-webapps-observer.c \
		$(top_srcdir)/src/bamf-unity-webapps-application.c \
		$(top_srcdir)/src/bamf-indicator.c \
		$(top_srcdir)/src/bamf-indicator-source.c \
		$(top_srcdir)/src/bamf-xutils.c \
		$(top_builddir)/src/bamf-marshal.c \
		$(top_builddir)/src/bamf-gdbus-generated.c \
		$(top_builddir)/src/bamf-gdbus-view-generated.c \
		$(top_builddir)/src/bamf-gdbus-browser-generated.c \
		$(top_builddir)/src/bamf-gdbus-indicator-source-generated.c \
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
		$(top_srcdir)/src/bamf-indicator-source.h \
		$(top_srcdir)/src/bamf-indicator.h \
		$(top_srcdir)/src/bamf-xutils.h \
		$(top_builddir)/src/bamf-marshal.h \
		$(top_builddir)/src/bamf-gdbus-generated.h \
		$(top_builddir)/src/bamf-gdbus-view-generated.h \
		$(top_builddir)/src/bamf-gdbus-browser-generated.h \
		$(top_builddir)/src/bamf-gdbus-indicator-source-generated.h \
		$(NULL)

test_bamf_SOURCES = \
  $(bamf_test_extra_sources) \
  $(bamf_test_extra_headers) \
  test-bamf.c \
  test-view.c \
  test-application.c \
  test-window.c \
  test-matcher.c

test_bamf_CPPFLAGS = \
  -I$(top_srcdir) \
  -I$(top_srcdir)/src \
  -I$(top_builddir)/src \
  -DWNCK_I_KNOW_THIS_IS_UNSTABLE \
  -DTESTDIR=\""$(top_srcdir)/tests"\" \
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

# Run tests as part of make check
if ENABLE_HEADLESS_TESTS
check-local: test-headless
else
check-local: test
endif

test:
	@gtester --verbose -k -o=test-bamf-results.xml ./test-bamf

.PHONY: check-report full-report
check-report:
	@gtester --verbose -k -o=test-bamf-results.xml -k ./test-bamf \
		&& ( gtester-report test-bamf-results.xml \
				| sed 's/GTester Unit Test Report</>GTester Unit Test Report (normal)</' \
				> test-bamf-results.html ) \
		&& gnome-open ./test-bamf-results.html

full-report:
		@gtester --verbose-k -o=test-bamf-results.xml -k -m=slow ./test-bamf \
	  && ( gtester-report test-bamf-results.xml \
	      | sed 's/>GTester Unit Test Report</>GTester Unit Test Report (normal)</' \
	      > test-bamf-results.html )

clean-generic:
	rm -f test-bamf-results.xml test-bamf-results.html

# START HEADLESS TESTS
if ENABLE_HEADLESS_TESTS
LOG_PATH = headless-logs
test-headless:
	set -e; \
	rm -rf $(LOG_PATH); \
	mkdir $(LOG_PATH); \
	export LOG_PATH=$(LOG_PATH); \
	export XVFB_PATH=$(XVFB); \
	export DISPLAY=""; \
	source $(XVFB_RUN); \
	\
	$(DBUS_LAUNCH) > $(LOG_PATH)/sessionbus.sh; \
	source $(LOG_PATH)/sessionbus.sh; \
	sleep 3; \
	\
	make test;
endif
# END HEADLESS TESTS
