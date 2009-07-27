/* project created on 7/7/2009 at 2:02 PM */
#ifndef __LIBWNCKSYNC_H__
#define __LIBWNCKSYNC_H__

#define WNCK_I_KNOW_THIS_IS_UNSTABLE

#include <glib.h>
#include <libwnck/libwnck.h>

/* Add function prototypes here */

GArray * wncksync_windows_for_desktop_file (gchar *desktop_file);

gchar * wncksync_desktop_item_for_window (WnckWindow *window);

void wncksync_register_desktop_file_for_pid (gchar *desktop_file, gint pid);

void wncksync_init ();

void wncksync_quit ();

#endif
