lib_LTLIBRARIES = libbamf.la

CLEANFILES =
DISTCLEANFILES =
BUILT_SOURCES =
EXTRA_DIST = 

include $(top_srcdir)/Makefile.am.marshal

libbamf_sources = \
  bamf-application.c \
  bamf-control.c \
  bamf-matcher.c \
  bamf-view.c \
  bamf-window.c \
  bamf-factory.c \
  bamf-tab-source.c \
  bamf-marshal.c \
  $(NULL)

sources_h = \
  bamf-application.h \
  bamf-control.h \
  bamf-matcher.h \
  bamf-view.h \
  bamf-window.h \
  bamf-factory.h \
  bamf-tab-source.h \
  libbamf.h \
  $(NULL)

libbamf_la_SOURCES = \
  $(sources_h) \
  $(libbamf_sources) \
  bamf-tab-source-glue.h \
  bamf-marshal.h \
  $(NULL)

libbamf_la_LIBADD = \
  $(LIBWSBIND_LIBS) \
  $(GLIB_LIBS) \
  $(DBUS_LIBS) \
  $(NULL)

libbamf_la_CPPFLAGS = \
  -Wall -std=c99 \
  -fPIC \
  -DDATADIR=\""$(datadir)"\" \
  -I$(top_srcdir)/lib \
  $(LIBWSBIND_CFLAGS) \
  $(GLIB_CFLAGS) \
  $(DBUS_CFLAGS) \
  $(NULL)

libbamf_la_LDFLAGS = \
  -shared -Xlinker -export-dynamic -Wl,-O1 -Wl,-Bsymbolic-functions \
  $(NULL)

DISTCLEANFILES += \
	bamf-tab-source-glue.h \
	$(NULL)

glib_marshal_list = bamf-marshal.list
glib_marshal_prefix = bamf_marshal

bamf-tab-source-glue.h: bamf-tab-source-glue.xml
	$(DBUS_GLIB_BIN)/dbus-binding-tool --prefix=bamf_tab_source --mode=glib-server --output=$@ $<

BUILT_SOURCES += \
	bamf-tab-source-glue.h \
	$(NULL)

EXTRA_DIST += \
	bamf-tab-source-glue.xml \
	$(NULL)


library_includedir=$(includedir)/libbamf/libbamf
library_include_HEADERS = \
  $(sources_h)

libbamf_pcdir = $(libdir)/pkgconfig
libbamf_pc_DATA = \
  libbamf.pc


#########################
# GObject Introsepction 
#########################

-include $(INTROSPECTION_MAKEFILE)
INTROSPECTION_GIRS = 
INTROSPECTION_SCANNER_ARGS = --add-include-path=$(srcdir)
INTROSPECTION_COMPILER_ARGS = --includedir=$(srcdir)

if HAVE_INTROSPECTION

introspection_sources = $(library_include_HEADERS)

Bamf-0.2.gir: libbamf.la
Bamf_0_2_gir_INCLUDES = \
	GObject-2.0 \
	Wnck-1.0
Bamf_0_2_gir_CFLAGS = --c-include='libbamf/libbamf.h' $(LIBBAMF_CFLAGS) -I ../ $(WNCK_CFLAGS) $(WNCK_LIBS) 
Bamf_0_2_gir_LIBS = libbamf.la 
Bamf_0_2_gir_FILES = $(addprefix $(srcdir)/, $(introspection_sources))

INTROSPECTION_GIRS += Bamf-0.2.gir

girdir = $(datadir)/gir-1.0
dist_gir_DATA = $(INTROSPECTION_GIRS)

typelibdir = $(libdir)/girepository-1.0
typelib_DATA = $(INTROSPECTION_GIRS:.gir=.typelib)

CLEANFILES += $(dist_gir_DATA) $(typelib_DATA)

endif

#########################
# VAPI Files
#########################

if HAVE_INTROSPECTION

vapidir = $(datadir)/vala/vapi
vapi_DATA = Bamf-0.2.vapi

Bamf-0.2.vapi: Bamf-0.2.gir
	$(VALA_API_GEN) --pkg libwnck-1.0 --library=Bamf-0.2 $<

CLEANFILES += $(vapi_DATA)

endif

