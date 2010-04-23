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
#ifndef __LIBBAMF_H__
#define __LIBBAMF_H__

#include <glib.h>

G_BEGIN_DECLS

#define BAMF_TYPE_PROXY (bamf_proxy_get_type ())

#define BAMF_PROXY(obj) (G_TYPE_CHECK_INSTANCE_CAST ((obj), \
        BAMF_TYPE_PROXY, BamfProxy))

#define BAMF_PROXY_CLASS(klass) (G_TYPE_CHECK_CLASS_CAST ((klass), \
        BAMF_TYPE_PROXY, BamfProxyClass))

#define BAMF_IS_PROXY(obj) (G_TYPE_CHECK_INSTANCE_TYPE ((obj), \
        BAMF_TYPE_PROXY))

#define BAMF_IS_PROXY_CLASS(klass) (G_TYPE_CHECK_CLASS_TYPE ((klass), \
        BAMF_TYPE_PROXY))

#define BAMF_PROXY_GET_CLASS(obj) (G_TYPE_INSTANCE_GET_CLASS ((obj), \
        BAMF_TYPE_PROXY, BamfProxyClass))

typedef struct _BamfProxy BamfProxy;
typedef struct _BamfProxyClass BamfProxyClass;
typedef struct _BamfProxyPrivate BamfProxyPrivate;

struct _BamfProxyClass
{
  GObjectClass parent_class;
};


struct _BamfProxy
{
  GObject parent_instance;

  /* Private */
  BamfProxyPrivate *priv;
};

typedef void     (*BamfFileCallback)     (BamfProxy *proxy,
                                              gchar *file,
                                              gpointer data);

typedef void     (*BamfArrayCallback)     (BamfProxy *proxy,
                                               GArray *array,
                                               gpointer data);

BamfProxy * bamf_proxy_get_default (void);

GArray * bamf_proxy_get_xids         (BamfProxy *proxy,
                                          gchar * desktop_file);

void bamf_proxy_get_xids_async (BamfProxy *proxy,
                                    gchar *desktop_file,
                                    BamfArrayCallback callback,
                                    gpointer user_data);

gchar *  bamf_proxy_get_desktop_file (BamfProxy *proxy,
                                          guint32 xid);
                                          
void bamf_proxy_get_desktop_file_async (BamfProxy       *proxy,
                                            guint32              xid,
                                            BamfFileCallback callback,
                                            gpointer             user_data);

void     bamf_proxy_register_match   (BamfProxy *proxy,
                                          gchar * desktop_file, 
                                          gint    pid);
G_END_DECLS
#endif
