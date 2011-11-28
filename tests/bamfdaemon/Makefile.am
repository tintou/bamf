noinst_PROGRAMS = \
  test-bamf

bamf_test_extra_sources = \
		$(top_srcdir)/src/bamf-legacy-window.c \
		$(top_srcdir)/src/bamf-legacy-window-test.c \
		$(top_srcdir)/src/bamf-legacy-screen.c \
		$(top_srcdir)/src/bamf-view.c \
		$(top_srcdir)/src/bamf-control.c \
		$(top_srcdir)/src/bamf-matcher.c \
		$(top_srcdir)/src/bamf-application.c \
		$(top_srcdir)/src/bamf-window.c \
		$(top_srcdir)/src/bamf-tab.c \
		$(top_srcdir)/src/bamf-tab-source.c \		
		$(top_srcdir)/src/bamf-indicator.c \
		$(top_srcdir)/src/bamf-indicator-source.c \
		$(top_builddir)/src/bamf-marshal.c \
		$(NULL)

bamf_test_extra_headers = \
		$(top_srcdir)/src/main.h \
		$(top_srcdir)/src/bamf.h \
		$(top_srcdir)/src/bamf-legacy-window.h \
		$(top_srcdir)/src/bamf-legacy-window-test.h \
	  $(top_srcdir)/src/bamf-legacy-screen.h \
		$(top_srcdir)/src/bamf-view.h \
		$(top_srcdir)/src/bamf-control.h \
		$(top_srcdir)/src/bamf-matcher.h \
		$(top_srcdir)/src/bamf-window.h \
		$(top_srcdir)/src/bamf-tab.h \
		$(top_srcdir)/src/bamf-tab-source.h \
		$(top_srcdir)/src/bamf-application.h \
		$(top_srcdir)/src/bamf-indicator-source.h \
		$(top_srcdir)/src/bamf-indicator.h \
		$(top_builddir)/src/bamf-application-glue.h \
		$(top_builddir)/src/bamf-control-glue.h \
		$(top_builddir)/src/bamf-matcher-glue.h \
		$(top_builddir)/src/bamf-window-glue.h \
		$(top_builddir)/src/bamf-view-glue.h \
		$(top_builddir)/src/bamf-marshal.h \
		$(top_builddir)/src/bamf-indicator-glue.h \
		$(top_builddir)/src/bamf-indicator-source-glue.h \
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
  -DTESTDIR=\""$(top_srcdir)/tests/unit"\" \
  $(GCC_FLAGS) \
  $(GTK_CFLAGS) \
  $(GLIB_CFLAGS) \
  $(DBUS_CFLAGS) \
  $(WNCK_CFLAGS) \
  $(GTOP_CFLAGS) \
  $(MAINTAINER_CFLAGS)

test_bamf_LDADD = \
  $(X_LIBS) \
  $(GLIB_LIBS) \
  $(DBUS_LIBS) \
  $(WNCK_LIBS) \
  $(GTOP_LIBS) \
  $(GTK_LIBS)

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
	@gtester -k -o=test-bamf-results.xml ./test-bamf

.PHONY: check-report full-report
check-report:
	@gtester -k -o=test-bamf-results.xml -k ./test-bamf \
		&& ( gtester-report test-bamf-results.xml \
				| sed 's/GTester Unit Test Report</>GTester Unit Test Report (normal)</' \
				> test-bamf-results.html ) \
		&& gnome-open ./test-bamf-results.html

full-report:
		@gtester -k -o=test-bamf-results.xml -k -m=slow ./test-bamf \
	  && ( gtester-report test-bamf-results.xml \
	      | sed 's/>GTester Unit Test Report</>GTester Unit Test Report (normal)</' \
	      > test-bamf-results.html )

clean-generic:
	rm -f test-bamf-results.xml test-bamf-results.html

# START HEADLESS TESTS
if ENABLE_HEADLESS_TESTS
DISPLAY = :27
LOG_PATH = headless-logs
test-headless:
	set -e; \
	export DISPLAY=$(DISPLAY); \
	rm -rf $(LOG_PATH); \
	mkdir $(LOG_PATH); \
	$(XVFB) $(DISPLAY) -screen 0 1024x768x8 > $(LOG_PATH)/Xvfb.out 2>&1 & \
	sleep 1; \
	\
	$(DBUS_LAUNCH) > $(LOG_PATH)/sessionbus.sh; \
	source $(LOG_PATH)/sessionbus.sh; \
	sleep 1; \
	\
	make test; \
	TEST_RESULTS=$?; \
	sleep 1; \
	\
	kill `grep DBUS_SESSION_BUS_PID $(LOG_PATH)/sessionbus.sh | grep -oE '[0-9]+'`; \
	pkill Xvfb;
	
endif
# END HEADLESS TESTS
