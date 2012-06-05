if USE_GTK3
VER=3
lib_LTLIBRARIES = libbamf3.la
else
VER=
lib_LTLIBRARIES = libbamf.la
endif

CLEANFILES =
DISTCLEANFILES =
BUILT_SOURCES =
EXTRA_DIST = 

INCLUDE = \
  -I$(top_builddir)/lib \
  $(NULL)


include $(top_srcdir)/Makefile.am.marshal

libbamf_sources = \
  bamf-application.c \
  bamf-control.c \
  bamf-matcher.c \
  bamf-view.c \
  bamf-window.c \
  bamf-factory.c \
  bamf-tab-source.c \
  bamf-tab.c \
  bamf-indicator.c \
  bamf-marshal.c \
  $(NULL)

sources_h = \
  bamf-application.h \
  bamf-indicator.h \
  bamf-control.h \
  bamf-matcher.h \
  bamf-view.h \
  bamf-window.h \
  bamf-tab-source.h \
  bamf-tab.h \
  libbamf.h \
  $(NULL)

libbamf_la_SOURCES = \
  bamf-factory.h \
  bamf-application-private.h \
  bamf-view-private.h \
  $(sources_h) \
  $(libbamf_sources) \
  bamf-tab-source-glue.h \
  bamf-marshal.h \
  $(NULL)

libbamf_la_LIBADD = \
  $(top_builddir)/lib/libbamf-private/libbamf-private.la \
  $(LIBWSBIND_LIBS) \
  $(GLIB_LIBS) \
  $(DBUS_LIBS) \
  $(NULL)

libbamf_la_CPPFLAGS = \
  -Wall -std=c99 \
  -fPIC \
  -DDATADIR=\""$(datadir)"\" \
  -I$(top_srcdir)/lib \
  -I$(top_srcdir)/lib/libbamf \
  -I$(top_builddir)/lib/libbamf \
  $(LIBWSBIND_CFLAGS) \
  $(GLIB_CFLAGS) \
  $(DBUS_CFLAGS) \
  $(NULL)

libbamf_la_LDFLAGS = \
  -shared \
  -export-symbols-regex "^bamf_" \
  $(NULL)

libbamf3_la_SOURCES = $(libbamf_la_SOURCES)
libbamf3_la_LIBADD = $(libbamf_la_LIBADD)
libbamf3_la_CPPFLAGS = $(libbamf_la_CPPFLAGS)
libbamf3_la_LDFLAGS = $(libbamf_la_LDFLAGS)

DISTCLEANFILES += \
	bamf-tab-source-glue.h \
	$(NULL)

glib_marshal_list = bamf-marshal.list
glib_marshal_prefix = _bamf_marshal

bamf-tab-source-glue.h: bamf-tab-source-glue.xml
	$(DBUS_GLIB_BIN)/dbus-binding-tool --prefix=bamf_tab_source --mode=glib-server --output=$@ $<

BUILT_SOURCES += \
	bamf-tab-source-glue.h \
	$(NULL)

EXTRA_DIST += \
	bamf-tab-source-glue.xml \
	$(NULL)


library_includedir=$(includedir)/libbamf$(VER)/libbamf
library_include_HEADERS = \
  $(sources_h)

libbamf_pcdir = $(libdir)/pkgconfig
libbamf_pc_DATA = \
  libbamf$(VER).pc


#########################
# GObject Introsepction 
#########################

#-include $(INTROSPECTION_MAKEFILE)
#INTROSPECTION_GIRS = 
#INTROSPECTION_SCANNER_ARGS = --add-include-path=$(srcdir)
#INTROSPECTION_COMPILER_ARGS = --includedir=$(srcdir)

#if HAVE_INTROSPECTION

#introspection_sources = $(library_include_HEADERS)

#Bamf-0.2.gir: libbamf.la
#Bamf_0_2_gir_INCLUDES = \
#	GObject-2.0
#Bamf_0_2_gir_CFLAGS = --c-include='libbamf/libbamf.h' $(LIBBAMF_CFLAGS) -I$(top_srcdir)/lib -I$(top_builddir)/lib
#Bamf_0_2_gir_LIBS = libbamf.la 
#Bamf_0_2_gir_FILES = $(addprefix $(srcdir)/, $(introspection_sources))
#
#INTROSPECTION_GIRS += Bamf-0.2.gir
#
#girdir = $(datadir)/gir-1.0
#dist_gir_DATA = $(INTROSPECTION_GIRS)

#typelibdir = $(libdir)/girepository-1.0
#typelib_DATA = $(INTROSPECTION_GIRS:.gir=.typelib)

#CLEANFILES += $(dist_gir_DATA) $(typelib_DATA)

#endif

#########################
# VAPI Files
#########################

#if HAVE_INTROSPECTION

#vapidir = $(datadir)/vala/vapi
#vapi_DATA = Bamf-0.2.vapi

#Bamf-0.2.vapi: Bamf-0.2.gir
#	$(VALA_API_GEN) --library=Bamf-0.2 $<

#CLEANFILES += $(vapi_DATA)

#endif

