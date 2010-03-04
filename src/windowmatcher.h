//  
//  Copyright (C) 2009 Canonical Ltd.
// 
//  This program is free software: you can redistribute it and/or modify
//  it under the terms of the GNU General Public License as published by
//  the Free Software Foundation, either version 3 of the License, or
//  (at your option) any later version.
// 
//  This program is distributed in the hope that it will be useful,
//  but WITHOUT ANY WARRANTY; without even the implied warranty of
//  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//  GNU General Public License for more details.
// 
//  You should have received a copy of the GNU General Public License
//  along with this program.  If not, see <http://www.gnu.org/licenses/>.
// 

#ifndef __WINDOWMATCHER_H__
#define __WINDOWMATCHER_H__

#define WNCK_I_KNOW_THIS_IS_UNSTABLE

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <gdk/gdk.h>
#include <glib.h>
#include <glib-object.h>
#include <gio/gio.h>
#include <gio/gdesktopappinfo.h>
#include <libwnck/libwnck.h>
#include <libgtop-2.0/glibtop.h>
#include <glibtop/procargs.h>
#include <glibtop/procuid.h>
#include <X11/Xatom.h>
#include <X11/Xlib.h>

#define WNCKSYNC_MATCHER_TYPE			(wncksync_matcher_get_type ())
#define WNCKSYNC_MATCHER(obj)			(G_TYPE_CHECK_INSTANCE_CAST ((obj), WNCKSYNC_MATCHER_TYPE, WncksyncMatcher))
#define WNCKSYNC_IS_MATCHER(obj)			(G_TYPE_CHECK_INSTANCE_TYPE ((obj), WNCKSYNC_MATCHER_TYPE))
#define WNCKSYNC_MATCHER_CLASS(klass)		(G_TYPE_CHECK_CLASS_CAST ((klass), WNCKSYNC_MATCHER_TYPE, WncksyncMatcherClass))
#define WNCKSYNC_IS_MATCHER_CLASS(klass)		(G_TYPE_CHECK_CLASA_TYPE ((klass), WNCKSYNC_MATCHER_TYPE))
#define WNCKSYNC_MATCHER_GET_CLASS(obj)		(G_TYPE_INSTANCE_GET_CLASS ((obj), WNCKSYNC_MATCHER_TYPE, WncksyncMatcherClass))

#define _NET_WM_DESKTOP_FILE			"_NET_WM_DESKTOP_FILE"
#define _CHROME_APP_URL                         "_CHROME_APP_URL"

typedef struct _WncksyncMatcher WncksyncMatcher;
typedef struct _WncksyncMatcherClass WncksyncMatcherClass;
typedef struct _WncksyncMatcherPrivate WncksyncMatcherPrivate;

struct _WncksyncMatcher
{
  GObject parent;

  /*< private > */
  WncksyncMatcherPrivate *priv;
};

struct _WncksyncMatcherClass
{
  GObjectClass parent;
};

GType wncksync_matcher_get_type (void);

WncksyncMatcher *wncksync_matcher_new (void);

gboolean wncksync_matcher_window_is_match_ready (WncksyncMatcher * self,
					       WnckWindow * window);

void wncksync_matcher_register_desktop_file_for_pid (WncksyncMatcher * self,
						   GString * desktopFile,
						   gint pid);

GString *wncksync_matcher_desktop_file_for_window (WncksyncMatcher * self,
						 WnckWindow * window);

GArray *wncksync_matcher_window_list_for_desktop_file (WncksyncMatcher * self,
						     GString * desktopFile);


#endif
