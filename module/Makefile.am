libdir = $(giomodulesdir)
lib_LTLIBRARIES = libgiobamf.la


libgiobamf_la_SOURCES =					\
	gapplaunchhandlerdbus.c gapplaunchhandlerdbus.h	\
	dbus-module.c					\
	$(NULL)

libgiobamf_la_CFLAGS =				\
	-I$(top_srcdir)/common                  \
	$(GLIB_CFLAGS)                          \
	$(GCONF_CFLAGS)                         \
	$(DBUS_CFLAGS)				\
	-DG_DISABLE_DEPRECATED			\
	$(NULL)

module_flags = -export_dynamic -avoid-version -module -no-undefined -export-symbols-regex '^g_io_module_(load|unload)'

libgiobamf_la_LDFLAGS = 	\
	$(module_flags)         \
	$(NULL)

libgiobamf_la_LIBADD  =		\
	$(GLIB_LIBS)            \
	$(GCONF_LIBS)           \
	$(DBUS_LIBS)		\
	$(NULL)

