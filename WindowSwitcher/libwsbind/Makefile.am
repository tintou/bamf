INCLUDES = -I$(top_srcdir)

AM_CFLAGS = -DG_LOG_DOMAIN=\"libdo\"	\
	    -DEGG_COMPILATION 		\
	    -DGTK_DISABLE_DEPRECATED 	\
	    -DGDK_DISABLE_DEPRECATED 	\
	    -DG_DISABLE_DEPRECATED	\
	    $(LIBDO_CFLAGS)


wslibdir = $(pkglibdir)
wslib_LTLIBRARIES = libwsbind.la

libwsbind_la_SOURCES =		\
	keybinder.h	\
	keybinder.c	\
	eggaccelerators.h	\
	eggaccelerators.c

libwsbind_la_LDFLAGS = -export-dynamic -module -avoid-version
libwsbind_la_LIBADD = $(LIBWSBIND_LIBS)

maintainer-clean-local:
	rm -f Makefile.in
