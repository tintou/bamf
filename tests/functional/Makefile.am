VER=3

NULL =

noinst_PROGRAMS = \
  alt-tabber \
  desktop-file-launcher \
  interaction-tester

# alt-tabber
alt_tabber_SOURCES = alt-tabber.c
alt_tabber_CPPFLAGS = \
  -I$(top_srcdir) \
  -I$(top_srcdir)/lib \
  -I$(top_srcdir)/lib/libbamf \
  -I$(top_builddir)/lib/libbamf \
  -DDATADIR=\""$(datadir)"\" \
  -DWNCK_I_KNOW_THIS_IS_UNSTABLE \
  $(LIBWSBIND_CFLAGS) \
  $(GLIB_CFLAGS) \
  $(DBUS_CFLAGS) \
  $(NULL)

alt_tabber_LDADD = \
  $(LIBWSBIND_LIBS) \
  $(GLIB_LIBS) \
  $(DBUS_LIBS) \
  $(top_builddir)/lib/libbamf/libbamf$(VER).la
  
# tab-source-test
tab_source_test_SOURCES = tab-source-test.c
tab_source_test_CPPFLAGS = \
  -I$(top_srcdir) \
  -I$(top_srcdir)/lib \
  -I$(top_srcdir)/lib/libbamf \
  -I$(top_builddir)/lib/libbamf \
  -DDATADIR=\""$(datadir)"\" \
  -DWNCK_I_KNOW_THIS_IS_UNSTABLE \
  $(LIBWSBIND_CFLAGS) \
  $(GLIB_CFLAGS) \
  $(DBUS_CFLAGS) \
  $(NULL)

tab_source_test_LDADD = \
  $(LIBWSBIND_LIBS) \
  $(GLIB_LIBS) \
  $(DBUS_LIBS) \
  $(top_builddir)/lib/libbamf/libbamf$(VER).la

# interaction-tester
interaction_tester_SOURCES = interaction-tester.c
interaction_tester_CPPFLAGS = \
  -I$(top_srcdir) \
  -I$(top_srcdir)/lib \
  -I$(top_srcdir)/lib/libbamf \
  -I$(top_builddir)/lib/libbamf \
  -DDATADIR=\""$(datadir)"\" \
  -DWNCK_I_KNOW_THIS_IS_UNSTABLE \
  $(LIBWSBIND_CFLAGS) \
  $(GLIB_CFLAGS) \
  $(DBUS_CFLAGS) \
  $(NULL)

interaction_tester_LDADD = \
  $(LIBWSBIND_LIBS) \
  $(GLIB_LIBS) \
  $(DBUS_LIBS) \
  $(top_builddir)/lib/libbamf/libbamf$(VER).la


# desktop-file-launcher
desktop_file_launcher_SOURCES = desktop-file-launcher.c
desktop_file_launcher_CPPFLAGS = $(GLIB_CFLAGS) $(LIBWSBIND_CFLAGS)
desktop_file_launcher_LDFLAGS = $(GLIB_LIBS) $(LIBWSBIND_LIBS)
