# Simple component buildsystem
include $(top_srcdir)/build.rules.mk

ASSEMBLY = WindowSwitcher
TARGET = exe

FILES = \
	CairoContext_Extensions.cs \
	GtkWidget_Extensions.cs \
	SwitcherArea.cs \
	WindowSwitcher.cs \
	XKeybinder.cs

REFERENCES =  \
	System \
	System.Core \
	Mono.Cairo \
	$(GTK_SHARP_20_LIBS) \
	$(GNOME_DESKTOP_SHARP_20_LIBS) \
	$(LIBRSVG_SHARP_LIBS) \
	$(WNCK_SHARP_10_LIBS)

PROJECT_REFERENCES = \
	LibWnckSync
	
bin_SCRIPTS = window-switcher
