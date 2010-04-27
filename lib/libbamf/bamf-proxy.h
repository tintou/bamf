/*
 * Copyright 2009 Canonical Ltd.
 *
 * This program is free software: you can redistribute it and/or modify it
 * under the terms of either or both of the following licenses:
 *
 * 1) the GNU Lesser General Public License version 3, as published by the
 * Free Software Foundation; and/or
 * 2) the GNU Lesser General Public License version 2.1, as published by
 * the Free Software Foundation.
 *
 * This program is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranties of
 * MERCHANTABILITY, SATISFACTORY QUALITY or FITNESS FOR A PARTICULAR
 * PURPOSE.  See the applicable version of the GNU Lesser General Public
 * License for more details.
 *
 * You should have received a copy of both the GNU Lesser General Public
 * License version 3 and version 2.1 along with this program.  If not, see
 * <http://www.gnu.org/licenses/>
 *
 * Authored by: Jason Smith <jason.smith@canonical.com>
 *              Neil Jagdish Patel <neil.patel@canonical.com>
 *
 */

#ifndef _BAMF_PROXY_H_
#define _BAMF_PROXY_H_

#include <glib-object.h>

G_BEGIN_DECLS

#define BAMF_TYPE_PROXY (bamf_proxy_get_type ())

#define BAMF_PROXY(obj) (G_TYPE_CHECK_INSTANCE_CAST ((obj),\
        BAMF_TYPE_PROXY, BamfProxy))

#define BAMF_PROXY_CLASS(klass) (G_TYPE_CHECK_CLASS_CAST ((klass),\
        BAMF_TYPE_PROXY, BamfProxyClass))

#define BAMF_IS_PROXY(obj) (G_TYPE_CHECK_INSTANCE_TYPE ((obj),\
        BAMF_TYPE_PROXY))

#define BAMF_IS_PROXY_CLASS(klass) (G_TYPE_CHECK_CLASS_TYPE ((klass),\
        BAMF_TYPE_PROXY))

#define BAMF_PROXY_GET_CLASS(obj) (G_TYPE_INSTANCE_GET_CLASS ((obj),\
        BAMF_TYPE_PROXY, BamfProxyClass))

typedef struct _BamfProxy        BamfProxy;
typedef struct _BamfProxyClass   BamfProxyClass;
typedef struct _BamfProxyPrivate BamfProxyPrivate;

struct _BamfProxy
{
  GObject           parent;

  BamfProxyPrivate *priv;
};

struct _BamfProxyClass
{
  GObjectClass parent_class;

  /*< private >*/
  void (*_view_padding1) (void);
  void (*_view_padding2) (void);
  void (*_view_padding3) (void);
  void (*_view_padding4) (void);
  void (*_view_padding5) (void);
  void (*_view_padding6) (void);
};

typedef void (*BamfFileCallback)  (BamfProxy   *proxy,
                                   gchar       *file,
                                   gpointer     data);

typedef void (*BamfArrayCallback) (BamfProxy *proxy,
                                   GArray    *array,
                                   gpointer   user_data);

GType       bamf_proxy_get_type               (void) G_GNUC_CONST;

BamfProxy * bamf_proxy_get_default            (void);

GArray *    bamf_proxy_get_xids               (BamfProxy        *proxy,
                                               const gchar      *desktop_file);

void        bamf_proxy_get_xids_async         (BamfProxy        *proxy,
                                               gchar            *desktop_file,
                                               BamfArrayCallback callback,
                                               gpointer          user_data);

gchar *     bamf_proxy_get_desktop_file       (BamfProxy        *proxy,
                                               guint32           xid);

void        bamf_proxy_get_desktop_file_async (BamfProxy        *proxy,
                                               guint32           xid,
                                               BamfFileCallback  callback,
                                               gpointer          user_data);

void        bamf_proxy_register_match         (BamfProxy        *proxy,
                                               gchar            *desktop_file,
                                               gint              pid);
G_END_DECLS

#endif /* _BAMF_PROXY_H_ */

