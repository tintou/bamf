CLEANFILES =
DISTCLEANFILES =
BUILT_SOURCES =
EXTRA_DIST =

include $(top_srcdir)/Makefile.am.marshal
-include $(INTROSPECTION_MAKEFILE)

libbamf_built_sources = \
  bamf-marshal.c \
  bamf-marshal.h \
  bamf-tab-source-glue.h \
  $(NULL)

libbamf_sources = \
  bamf-application.c \
  bamf-control.c \
  bamf-matcher.c \
  bamf-view.c \
  bamf-window.c \
  bamf-factory.c \
  bamf-tab-source.c \
  bamf-tab.c \
  $(NULL)

libbamf_headers = \
  bamf-application.h \
  bamf-control.h \
  bamf-matcher.h \
  bamf-view.h \
  bamf-window.h \
  bamf-tab-source.h \
  bamf-tab.h \
  libbamf.h \
  $(NULL)

lib_LTLIBRARIES = libbamf3.la

libbamf3_la_SOURCES = \
  bamf-factory.h \
  bamf-application-private.h \
  bamf-view-private.h \
  $(libbamf_headers) \
  $(libbamf_sources) \
  $(NULL)

nodist_libbamf3_la_SOURCES = \
  $(libbamf_built_sources)
  $(NULL)

libbamf3_la_LIBADD = \
  $(GLIB_LIBS) \
  $(DBUS_LIBS) \
  $(NULL)

libbamf3_la_CPPFLAGS = \
  -Wall -std=c99 \
  -fPIC \
  -DDATADIR=\""$(datadir)"\" \
  -I$(top_srcdir)/lib \
  -I$(top_builddir)/lib \
  -I$(top_srcdir)/lib/libbamf \
  -I$(top_builddir)/lib/libbamf \
  $(GLIB_CFLAGS) \
  $(DBUS_CFLAGS) \
  $(COVERAGE_CFLAGS) \
  $(NULL)

libbamf3_la_LDFLAGS = \
  -shared \
  -export-symbols-regex "^bamf_" \
  -version-info 1:0:0 \
  $(COVERAGE_LDFLAGS) \
  $(NULL)

glib_marshal_list = bamf-marshal.list
glib_marshal_prefix = _bamf_marshal

# resetting BUILT_SOURCES here is intended
BUILT_SOURCES = \
	$(libbamf_built_sources) \
	$(NULL)

CLEANFILES += \
	$(libbamf_built_sources) \
	$(NULL)

bamf-tab-source-glue.h: bamf-tab-source-glue.xml
	$(AM_V_GEN)$(DBUS_GLIB_BIN)/dbus-binding-tool --prefix=bamf_tab_source --mode=glib-server --output=$@ $<

EXTRA_DIST += bamf-tab-source-glue.xml

libbamf_includedir=$(includedir)/libbamf$(LIBBAMF_VER)/libbamf
libbamf_include_HEADERS = \
	$(libbamf_headers)

libbamf_pcdir = $(libdir)/pkgconfig
libbamf_pc_DATA = \
	libbamf$(LIBBAMF_VER).pc


#########################
# GObject Introsepction
#########################

if HAVE_INTROSPECTION

INTROSPECTION_GIRS = Bamf-3.gir
INTROSPECTION_SCANNER_ARGS = --add-include-path=$(srcdir)
INTROSPECTION_COMPILER_ARGS = --includedir=$(srcdir)

introspection_sources = \
	$(libbamf_headers) \
	$(libbamf_sources) \
	$(NULL)

Bamf-3.gir: libbamf$(LIBBAMF_VER).la
Bamf_3_gir_INCLUDES = GObject-2.0 GLib-2.0
Bamf_3_gir_SCANNERFLAGS = --warn-all --c-include='libbamf/libbamf.h'
Bamf_3_gir_CFLAGS = \
	$(DBUS_CFLAGS) \
	-I$(top_srcdir)/lib \
	-I$(top_builddir)/lib \
	$(NULL)
Bamf_3_gir_LIBS = libbamf$(LIBBAMF_VER).la
Bamf_3_gir_FILES = $(introspection_sources)
Bamf_3_gir_EXPORT_PACKAGES = libbamf$(LIBBAMF_VER)

girdir = $(INTROSPECTION_GIRDIR)
gir_DATA = $(INTROSPECTION_GIRS)

typelibdir = $(INTROSPECTION_TYPELIBDIR)
typelib_DATA = $(INTROSPECTION_GIRS:.gir=.typelib)

CLEANFILES += $(gir_DATA) $(typelib_DATA)

endif

#########################
# VAPI Files
#########################

if HAVE_INTROSPECTION

vapidir = $(datadir)/vala/vapi
vapi_DATA = libbamf$(LIBBAMF_VER).vapi

$(vapi_DATA): $(INTROSPECTION_GIRS)
	$(AM_V_GEN)$(VALA_API_GEN) --library=$(@:.vapi=) $<

CLEANFILES += $(vapi_DATA)

endif

