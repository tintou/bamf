noinst_PROGRAMS    = \
		alt-tabber \
		desktop-file-launcher

alt_tabber_SOURCES = alt-tabber.c
desktop_file_launcher_SOURCES = desktop-file-launcher.c

alt_tabber_CFLAGS  = -I. -O0 -ggdb -Wall -Werror `pkg-config --cflags glib-2.0 gtk+-2.0 gdk-2.0 libwnck-1.0 libwncksync`
desktop_file_launcher_CFLAGS  = -I. -O0 -ggdb -Wall -Werror `pkg-config --cflags gio-unix-2.0 glib-2.0 gtk+-2.0 gdk-2.0`

alt_tabber_LDFLAGS = `pkg-config --libs glib-2.0 gtk+-2.0 gdk-2.0 libwnck-1.0 libwncksync`
desktop_file_launcher_LDFLAGS = `pkg-config --libs gio-unix-2.0 glib-2.0 gtk+-2.0 gdk-2.0`
