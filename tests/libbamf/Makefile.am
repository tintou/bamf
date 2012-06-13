if USE_GTK3
VER=3
else
VER=
endif

noinst_PROGRAMS = \
  test-libbamf

test_libbamf_SOURCES = \
  test-libbamf.c \
  test-matcher.c

test_libbamf_CPPFLAGS = \
  -I$(top_srcdir) \
  -I$(top_srcdir)/lib \
  -I$(top_srcdir)/src \
  -I$(top_srcdir)/lib/libbamf \
  -I$(top_builddir)/lib/libbamf \
  -DTESTDIR=\""$(top_srcdir)/tests/libbamf"\" \
  $(GCC_FLAGS) \
  $(GLIB_CFLAGS)

test_libbamf_LDADD = \
  $(top_builddir)/lib/libbamf/libbamf$(VER).la \
  $(GLIB_LIBS)

# Run tests as part of make check
if ENABLE_HEADLESS_TESTS
check-local: test-headless
else
check-local: test
endif

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
	sleep 1; \
	\
	kill `grep DBUS_SESSION_BUS_PID $(LOG_PATH)/sessionbus.sh | grep -oE '[0-9]+'`; \
	pkill Xvfb;
	
endif
# END HEADLESS TESTS
