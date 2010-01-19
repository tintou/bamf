NULL = 

#noinst_PROGRAMS    = \
#		alt-tabber \
#		desktop-file-launcher

#alt_tabber_SOURCES = alt-tabber.c
#desktop_file_launcher_SOURCES = desktop-file-launcher.c

#alt_tabber_CFLAGS  = -I. -O0 -ggdb -Wall -Werror `pkg-config --cflags glib-2.0 gtk+-2.0 gdk-2.0 libwnck-1.0 libwncksync`
#desktop_file_launcher_CFLAGS  = -I. -O0 -ggdb -Wall -Werror `pkg-config --cflags gio-unix-2.0 glib-2.0 gtk+-2.0 gdk-2.0`

#alt_tabber_LDFLAGS = `pkg-config --libs glib-2.0 gtk+-2.0 gdk-2.0 libwnck-1.0 libwncksync`
#desktop_file_launcher_LDFLAGS = `pkg-config --libs gio-unix-2.0 glib-2.0 gtk+-2.0 gdk-2.0`

###

noinst_PROGRAMS = \
  alt-tabber \
  desktop-file-launcher

# alt-tabber
alt_tabber_SOURCES = alt-tabber.c
alt_tabber_CPPFLAGS = \
  -I$(top_srcdir) \
  -I$(top_srcdir)/lib \
  -I$(top_builddir)/lib \
	-DDATADIR=\""$(datadir)"\" \
  $(LIBWSBIND_CFLAGS) \
  $(GLIB_CFLAGS) \
  $(DBUS_CFLAGS) \
  $(WNCK_CFLAGS) \
  $(NULL)

alt_tabber_LDADD = \
  $(LIBWSBIND_LIBS) \
  $(GLIB_LIBS) \
  $(DBUS_LIBS) \
  $(WNCK_LIBS) \
  $(top_builddir)/lib/libwncksync/libwncksync.la

# desktop-file-launcher
desktop_file_launcher_SOURCES = desktop-file-launcher.c
desktop_file_launcher_CPPFLAGS = $(GLIB_CFLAGS) $(LIBWSBIND_CFLAGS)
desktop_file_launcher_LDFLAGS = $(GLIB_LIBS) $(LIBWSBIND_LIBS)
