noinst_PROGRAMS = \
  test-libbamf

test_libbamf_SOURCES = \
  test-libbamf.c \
  test-matcher.c

test_libbamf_CPPFLAGS = \
  -I$(top_srcdir) \
  -I$(top_srcdir)/lib \
  -I$(top_srcdir)/lib/libbamf \
  -I$(top_builddir)/lib/libbamf \
  -DTESTDIR=\""$(top_srcdir)/tests/libbamf"\" \
  $(GCC_FLAGS) \
  $(GTK_CFLAGS) \
  $(WNCK_CFLAGS) \
  $(LIBWSBIND_CFLAGS) \
  $(DBUS_CFLAGS) \
  $(GLIB_CFLAGS)

test_libbamf_LDADD = \
  $(top_builddir)/lib/libbamf/libbamf.la \
  $(LIBWSBIND_LIBS) \
  $(WNCK_LIBS) \
  $(GLIB_LIBS) \
  $(GTK_LIBS) \
  $(DBUS_LIBS)

#run make test as part of make check
check-local: test
test:
	@gtester -k -o=test-libbamf-results.xml ./test-libbamf

.PHONY: check-report full-report
check-report:
	@gtester -k -o=test-libbamf-results.xml -k ./test-libbamf \
		&& ( gtester-report test-libbamf-results.xml \
				| sed 's/GTester Unit Test Report</>GTester Unit Test Report (normal)</' \
				> test-libbamf-results.html ) \
		&& gnome-open ./test-libbamf-results.html

full-report:
		@gtester -k -o=test-libbamf-results.xml -k -m=slow ./test-libbamf \
	  && ( gtester-report test-libbamf-results.xml \
	      | sed 's/>GTester Unit Test Report</>GTester Unit Test Report (normal)</' \
	      > test-libbamf-results.html )

clean-generic:
	rm -f test-libbamf-results.xml test-libbamf-results.html
