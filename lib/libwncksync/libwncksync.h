/* project created on 7/7/2009 at 2:02 PM */
#ifndef __LIBWNCKSYNC_H__
#define __LIBWNCKSYNC_H__

#include <glib.h>

/* Add function prototypes here */

GArray * wncksync_windows_for_desktop_file (gchar *desktop_file);

gchar * wncksync_desktop_item_for_window_xid (gulong xid);

void wncksync_init ();

void wncksync_quit ();

#endif
