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

#define BAMF_MATCHER_TYPE			(bamf_matcher_get_type ())
#define BAMF_MATCHER(obj)			(G_TYPE_CHECK_INSTANCE_CAST ((obj), BAMF_MATCHER_TYPE, BamfMatcher))
#define BAMF_IS_MATCHER(obj)			(G_TYPE_CHECK_INSTANCE_TYPE ((obj), BAMF_MATCHER_TYPE))
#define BAMF_MATCHER_CLASS(klass)		(G_TYPE_CHECK_CLASS_CAST ((klass), BAMF_MATCHER_TYPE, BamfMatcherClass))
#define BAMF_IS_MATCHER_CLASS(klass)		(G_TYPE_CHECK_CLASA_TYPE ((klass), BAMF_MATCHER_TYPE))
#define BAMF_MATCHER_GET_CLASS(obj)		(G_TYPE_INSTANCE_GET_CLASS ((obj), BAMF_MATCHER_TYPE, BamfMatcherClass))

#define _NET_WM_DESKTOP_FILE			"_NET_WM_DESKTOP_FILE"
#define _CHROME_APP_URL                         "_CHROME_APP_URL"

typedef struct _BamfMatcher BamfMatcher;
typedef struct _BamfMatcherClass BamfMatcherClass;
typedef struct _BamfMatcherPrivate BamfMatcherPrivate;

struct _BamfMatcher
{
  GObject parent;

  /*< private > */
  BamfMatcherPrivate *priv;
};

struct _BamfMatcherClass
{
  GObjectClass parent;
};

GType bamf_matcher_get_type (void);

BamfMatcher *bamf_matcher_new (void);

gboolean bamf_matcher_window_is_match_ready (BamfMatcher * self,
					       WnckWindow * window);

void bamf_matcher_register_desktop_file_for_pid (BamfMatcher * self,
						   GString * desktopFile,
						   gint pid);

GString *bamf_matcher_desktop_file_for_window (BamfMatcher * self,
						 WnckWindow * window);

GArray *bamf_matcher_window_list_for_desktop_file (BamfMatcher * self,
						     GString * desktopFile);


#endif
