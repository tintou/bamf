noinst_PROGRAMS = \
  test-bamf

bamf_test_extra_sources = \
                $(top_srcdir)/src/bamf-legacy-window.c \
     		$(top_srcdir)/src/bamf-legacy-screen.c \
		$(top_srcdir)/src/bamf-view.c \
		$(top_srcdir)/src/bamf-control.c \
		$(top_srcdir)/src/bamf-matcher.c \
		$(top_srcdir)/src/bamf-application.c \
		$(top_srcdir)/src/bamf-window.c \
		$(top_srcdir)/src/bamf-marshal.c \
		$(NULL)

bamf_test_extra_headers = \
		$(top_srcdir)/src/main.h \
		$(top_srcdir)/src/bamf.h \
		$(top_srcdir)/src/bamf-legacy-window.h \
	        $(top_srcdir)/src/bamf-legacy-screen.h \
		$(top_srcdir)/src/bamf-view.h \
		$(top_srcdir)/src/bamf-control.h \
		$(top_srcdir)/src/bamf-matcher.h \
		$(top_srcdir)/src/bamf-window.h \
		$(top_srcdir)/src/bamf-application.h \
		$(top_srcdir)/src/bamf-application-glue.h \
		$(top_srcdir)/src/bamf-control-glue.h \
		$(top_srcdir)/src/bamf-matcher-glue.h \
		$(top_srcdir)/src/bamf-window-glue.h \
		$(top_srcdir)/src/bamf-view-glue.h \
		$(top_srcdir)/src/bamf-marshal.h \
		$(NULL)

test_bamf_SOURCES = \
  $(bamf_test_extra_sources) \
  $(bamf_test_extra_headers) \
  test-mock-window.h \
  test-mock-window.c \
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

#run make test as part of make check
check-local: test
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
