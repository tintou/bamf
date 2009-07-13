#ifndef __WINDOWMATCHER_H__
#define __WINDOWMATCHER_H__

#define WNCK_I_KNOW_THIS_IS_UNSTABLE

#include <stdio.h>
#include <stdlib.h>
#include <gdk/gdk.h>
#include <glib.h>
#include <gio/gio.h>
#include <gio/gdesktopappinfo.h>
#include <libwnck/libwnck.h>
#include <libgtop-2.0/glibtop.h>
#include <glibtop/procargs.h>

/* Add function prototypes here */

void windowmatcher_initialize ();

void windowmatcher_quit ();

void windowmatcher_register_desktop_file_for_pid (GString *desktopFile, gulong pid);

GString * windowmatcher_desktop_file_for_window (WnckWindow *window);

GArray * windowmatcher_window_list_for_desktop_file (GString *desktopFile);

#endif