lib_LTLIBRARIES = libbamf.la

CLEANFILES =

sources_h = \
  bamf-application.h \
  bamf-control.h \
  bamf-matcher.h \
  bamf-view.h \
  bamf-window.h \
  bamf-factory.h \
  marshal.h \
  libbamf.h

libbamf_la_SOURCES = \
  bamf-application.c \
  bamf-control.c \
  bamf-matcher.c \
  bamf-view.c \
  bamf-window.c \
  bamf-factory.c \
  marshal.c \
  $(sources_h)

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
  $(WNCK_CFLAGS) \
  $(NULL)

libbamf_la_LDFLAGS = \
  -shared -Xlinker -export-dynamic -Wl,-O1 -Wl,-Bsymbolic-functions \
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

LibBamf-0.2.gir: libbamf.la
LibBamf_0_2_gir_INCLUDES = \
	GObject-2.0 \
	Wnck-1.0
LibBamf_0_2_gir_CFLAGS = $(LIBBAMF_CFLAGS) -I ../ $(WNCK_CFLAGS) $(WNCK_LIBS)
LibBamf_0_2_gir_LIBS = libbamf.la 
LibBamf_0_2_gir_FILES = $(addprefix $(srcdir)/, $(introspection_sources))

INTROSPECTION_GIRS += LibBamf-0.2.gir

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
vapi_DATA = LibBamf-0.2.vapi

LibBamf-0.2.vapi: LibBamf-0.2.gir
	$(VALA_API_GEN) --pkg libwnck-1.0 --library=LibBamf-0.2 $<

CLEANFILES += $(vapi_DATA)

endif

