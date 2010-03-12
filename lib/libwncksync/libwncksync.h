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

G_BEGIN_DECLS

#define WNCKSYNC_TYPE_PROXY (wncksync_proxy_get_type ())

#define WNCKSYNC_PROXY(obj) (G_TYPE_CHECK_INSTANCE_CAST ((obj), \
        WNCKSYNC_TYPE_PROXY, WncksyncProxy))

#define WNCKSYNC_PROXY_CLASS(klass) (G_TYPE_CHECK_CLASS_CAST ((klass), \
        WNCKSYNC_TYPE_PROXY, WncksyncProxyClass))

#define WNCKSYNC_IS_PROXY(obj) (G_TYPE_CHECK_INSTANCE_TYPE ((obj), \
        WNCKSYNC_TYPE_PROXY))

#define WNCKSYNC_IS_PROXY_CLASS(klass) (G_TYPE_CHECK_CLASS_TYPE ((klass), \
        WNCKSYNC_TYPE_PROXY))

#define WNCKSYNC_PROXY_GET_CLASS(obj) (G_TYPE_INSTANCE_GET_CLASS ((obj), \
        WNCKSYNC_TYPE_PROXY, WncksyncProxyClass))

typedef struct _WncksyncProxy WncksyncProxy;
typedef struct _WncksyncProxyClass WncksyncProxyClass;
typedef struct _WncksyncProxyPrivate WncksyncProxyPrivate;

struct _WncksyncProxyClass
{
  GObjectClass parent_class;
};


struct _WncksyncProxy
{
  GObject parent_instance;

  /* Private */
  WncksyncProxyPrivate *priv;
};

typedef void     (*WncksyncFileCallback)     (WncksyncProxy *proxy,
                                              gchar *file,
                                              gpointer data);

typedef void     (*WncksyncArrayCallback)     (WncksyncProxy *proxy,
                                               GArray *array,
                                               gpointer data);

WncksyncProxy * wncksync_proxy_get_default (void);

GArray * wncksync_proxy_get_xids         (WncksyncProxy *proxy,
                                          gchar * desktop_file);

gchar *  wncksync_proxy_get_desktop_file (WncksyncProxy *proxy,
                                          guint32 xid);
                                          
void wncksync_proxy_get_desktop_file_async (WncksyncProxy       *proxy,
                                            guint32              xid,
                                            WncksyncFileCallback callback,
                                            gpointer             user_data);

void     wncksync_proxy_register_match   (WncksyncProxy *proxy,
                                          gchar * desktop_file, 
                                          gint    pid);
G_END_DECLS
#endif
