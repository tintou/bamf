/*
 * Copyright (C) 2009 Canonical, Ltd.
 *
 * This library is free software; you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License
 * version 3.0 as published by the Free Software Foundation.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License version 3.0 for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library. If not, see
 * <http://www.gnu.org/licenses/>.
 *
 * Authored by Jason Smith <jason.smith@canonical.com>
 */

/* project created on 7/7/2009 at 2:02 PM */
#ifndef __LIBWNCKSYNC_H__
#define __LIBWNCKSYNC_H__

#include <glib.h>

/* Add function prototypes here */

GArray * wncksync_xids_for_desktop_file (gchar *desktop_file);

gchar * wncksync_desktop_item_for_xid (guint32 xid);

void wncksync_register_desktop_file_for_pid (gchar *desktop_file, gint pid);

void wncksync_init ();

void wncksync_quit ();

#endif
