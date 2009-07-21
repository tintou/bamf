lib_LTLIBRARIES = libgiodbus.la

libgiodbus_la_SOURCES =				\
	gapplaunchhandlerdbus.c gapplaunchhandlerdbus.h		\
	dbus-module.c					\
	$(NULL)

libgiodbus_la_CFLAGS =		\
	-I$(top_srcdir)/common                  \
	$(GLIB_CFLAGS)                          \
	$(GCONF_CFLAGS)                         \
	-DGIO_MODULE_DIR=\"$(GIO_MODULE_DIR)\"	\
	-DG_DISABLE_DEPRECATED			\
	$(NULL)

module_flags = -export_dynamic -avoid-version -module -no-undefined -export-symbols-regex '^g_io_module_(load|unload)'

libgiodbus_la_LDFLAGS = 	\
	$(module_flags)         \
	$(NULL)

libgiodbus_la_LIBADD  =	     \
	$(GLIB_LIBS)                 \
	$(GCONF_LIBS)                \
	$(NULL)

