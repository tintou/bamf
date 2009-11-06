libdir = $(giomodulesdir)
lib_LTLIBRARIES = libgiowncksync.la


libgiowncksync_la_SOURCES =					\
	gapplaunchhandlerdbus.c gapplaunchhandlerdbus.h	\
	dbus-module.c					\
	$(NULL)

libgiowncksync_la_CFLAGS =				\
	-I$(top_srcdir)/common                  \
	$(GLIB_CFLAGS)                          \
	$(GCONF_CFLAGS)                         \
	$(DBUS_CFLAGS)				\
	-DG_DISABLE_DEPRECATED			\
	$(NULL)

module_flags = -export_dynamic -avoid-version -module -no-undefined -export-symbols-regex '^g_io_module_(load|unload)'

libgiowncksync_la_LDFLAGS = 	\
	$(module_flags)         \
	$(NULL)

libgiowncksync_la_LIBADD  =		\
	$(GLIB_LIBS)            \
	$(GCONF_LIBS)           \
	$(DBUS_LIBS)		\
	$(NULL)

