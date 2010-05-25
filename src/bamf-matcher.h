/*
 * Copyright (C) 2010 Canonical Ltd
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 3 as
 * published by the Free Software Foundation.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 *
 * Authored by: Jason Smith <jason.smith@canonical.com>
 *
 */


#ifndef __BAMFMATCHER_H__
#define __BAMFMATCHER_H__

#include "bamf.h"
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <dbus/dbus.h>
#include <dbus/dbus-glib.h>
#include <dbus/dbus-glib-lowlevel.h>
#include <gdk/gdk.h>
#include <glib.h>
#include <glib-object.h>
#include <gio/gio.h>
#include <gio/gdesktopappinfo.h>
#include <libgtop-2.0/glibtop.h>
#include <glibtop/procargs.h>
#include <glibtop/procuid.h>
#include <X11/Xatom.h>
#include <X11/Xlib.h>

#define BAMF_TYPE_MATCHER			(bamf_matcher_get_type ())
#define BAMF_MATCHER(obj)			(G_TYPE_CHECK_INSTANCE_CAST ((obj), BAMF_TYPE_MATCHER, BamfMatcher))
#define BAMF_IS_MATCHER(obj)			(G_TYPE_CHECK_INSTANCE_TYPE ((obj), BAMF_TYPE_MATCHER))
#define BAMF_MATCHER_CLASS(klass)		(G_TYPE_CHECK_CLASS_CAST ((klass), BAMF_TYPE_MATCHER, BamfMatcherClass))
#define BAMF_IS_MATCHER_CLASS(klass)		(G_TYPE_CHECK_CLASS_TYPE ((klass), BAMF_TYPE_MATCHER))
#define BAMF_MATCHER_GET_CLASS(obj)		(G_TYPE_INSTANCE_GET_CLASS ((obj), BAMF_TYPE_MATCHER, BamfMatcherClass))

#define _NET_WM_DESKTOP_FILE			"_NET_WM_DESKTOP_FILE"
#define _CHROME_APP_URL                         "_CHROME_APP_URL"

typedef struct _BamfMatcher BamfMatcher;
typedef struct _BamfMatcherClass BamfMatcherClass;
typedef struct _BamfMatcherPrivate BamfMatcherPrivate;

struct _BamfMatcherClass
{
  GObjectClass parent;
};

struct _BamfMatcher
{
  GObject parent;

  /* private */
  BamfMatcherPrivate *priv;
};

GType         bamf_matcher_get_type                      (void) G_GNUC_CONST;

void          bamf_matcher_load_desktop_file             (BamfMatcher * self,
                                                          char * desktop_file);

void          bamf_matcher_register_desktop_file_for_pid (BamfMatcher * self,
                                                          char * desktopFile,
                                                          gint pid);

char        * bamf_matcher_get_active_application        (BamfMatcher *matcher);

char        * bamf_matcher_get_active_window             (BamfMatcher *matcher);

char        * bamf_matcher_application_for_xid           (BamfMatcher *matcher,
                                                          guint32 xid);

gboolean      bamf_matcher_application_is_running        (BamfMatcher *matcher,
                                                          char *application);

char       ** bamf_matcher_application_dbus_paths        (BamfMatcher *matcher);

char        * bamf_matcher_dbus_path_for_application     (BamfMatcher *matcher,
                                                          char *application);

char       ** bamf_matcher_running_application_paths     (BamfMatcher *matcher);

char       ** bamf_matcher_tab_dbus_paths                (BamfMatcher *matcher);

GArray      * bamf_matcher_xids_for_application          (BamfMatcher *matcher,
                                                          char *application);

BamfMatcher * bamf_matcher_get_default                   (void);

#endif
