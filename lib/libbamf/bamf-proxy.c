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
/**
 * SECTION:bamf-proxy
 * @short_description: The base class for all proxys
 *
 * #BamfProxy is the base class that all proxys need to derive from.
 */

#if HAVE_CONFIG_H
#include <config.h>
#endif

#include "bamf-proxy.h"

#include <dbus/dbus.h>
#include <dbus/dbus-glib.h>
#include <dbus/dbus-glib-lowlevel.h>

G_DEFINE_TYPE (BamfProxy, bamf_proxy, G_TYPE_OBJECT);

#define BAMF_PROXY_GET_PRIVATE(o) \
  (G_TYPE_INSTANCE_GET_PRIVATE ((o), BAMF_TYPE_PROXY, BamfProxyPrivate))

struct _BamfProxyPrivate
{
  DBusGConnection *connection;
  DBusGProxy      *proxy;
};

struct _ArrayCallbackData
{
  BamfProxy         *proxy;
  BamfArrayCallback  callback;
  gpointer           user_data;
};
typedef struct _ArrayCallbackData ArrayCallbackData;

struct _FileCallbackData
{
  BamfProxy        *proxy;
  BamfFileCallback  callback;
  gpointer          user_data;
};
typedef struct _FileCallbackData FileCallbackData;

/* Globals */
static BamfProxy * default_proxy = NULL;

/* Forwards */

/*
 * GObject stuff
 */

static void
bamf_proxy_class_init (BamfProxyClass *klass)
{
  GObjectClass *obj_class = G_OBJECT_CLASS (klass);

  g_type_class_add_private (obj_class, sizeof (BamfProxyPrivate));
}


static void
bamf_proxy_init (BamfProxy *self)
{
  BamfProxyPrivate *priv;
  GError           *error = NULL;

  priv = self->priv = BAMF_PROXY_GET_PRIVATE (self);

  priv->connection = dbus_g_bus_get (DBUS_BUS_SESSION, &error);
  if (priv->connection == NULL)
    {
      g_error ("Failed to open connection to bus: %s",
               error != NULL ? error->message : "Unknown");
      if (error)
        g_error_free (error);
      return;
    }

  priv->proxy = dbus_g_proxy_new_for_name (priv->connection,
                                           "org.ayatana.bamf.matcher",
                                           "/org/ayatana/bamf/matcher",
                                           "org.ayatana.bamf.matcher");
  if (priv->proxy == NULL)
    {
      g_error ("Unable to get org.bamf.Matcher proxy");
    }
}

/*
 * Private Methods
 */
static void
bamf_proxy_get_xids_async_callback (DBusGProxy     *proxy,
                                    DBusGProxyCall *call,
                                    gpointer        user_data)
{
  GArray            *array = NULL;
  GError            *error = NULL;
  ArrayCallbackData *cb_data;

  cb_data = (ArrayCallbackData *)user_data;

  if (!dbus_g_proxy_end_call (proxy,
                              call,
                              &error,
                              DBUS_TYPE_G_UINT_ARRAY, &array,
                              G_TYPE_INVALID))
    {
      g_error ("Failed to complete async callback: %s", error->message);
      g_error_free (error);

      return;
    }

  (cb_data->callback) (cb_data->proxy, array, cb_data->user_data);
}

static void
bamf_proxy_get_desktop_file_async_callback (DBusGProxy     *proxy,
                                            DBusGProxyCall *call,
                                            gpointer        user_data)
{
  gchar            *desktop_file = NULL;
  GError           *error = NULL;
  FileCallbackData *cb_data;

  cb_data = (FileCallbackData *)user_data;

  if (!dbus_g_proxy_end_call (proxy,
                              call,
                              &error,
                              G_TYPE_STRING, &desktop_file,
                              G_TYPE_INVALID))
    {
      g_error ("Failed to complete async callback: %s", error->message);
      g_error_free (error);
      return;
    }

  /* transfer-full the desktop_file pointer */
  (cb_data->callback) (cb_data->proxy, desktop_file, cb_data->user_data);
}
/*
 * Public Methods
 */
BamfProxy *
bamf_proxy_get_default (void)
{
  if (BAMF_IS_PROXY (default_proxy))
    return g_object_ref (default_proxy);

  return (default_proxy = g_object_new (BAMF_TYPE_PROXY, NULL));
}

GArray *
bamf_proxy_get_xids (BamfProxy        *self,
                     const gchar      *desktop_file)
{
  BamfProxyPrivate *priv;
  GArray *array = NULL;
  GError *error = NULL;

  g_return_val_if_fail (BAMF_IS_PROXY (self), NULL);
  g_return_val_if_fail (desktop_file != NULL, NULL);
  priv = self->priv;

  if (!dbus_g_proxy_call (priv->proxy,
                          "XidsForDesktopFile",
                          &error,
                          G_TYPE_STRING, desktop_file,
                          G_TYPE_INVALID,
                          DBUS_TYPE_G_UINT_ARRAY, &array,
                          G_TYPE_INVALID))
    {
      g_error ("Failed to fetch xid array: %s", error->message);
      g_error_free (error);

      array = g_array_new (FALSE, TRUE, sizeof (guint32));
    }

  return array;
}

void
bamf_proxy_get_xids_async (BamfProxy         *self,
                           gchar             *desktop_file,
                           BamfArrayCallback  callback,
                           gpointer           user_data)
{
  BamfProxyPrivate  *priv;
  DBusGProxyCall    *call;
  ArrayCallbackData *data;

  g_return_if_fail (BAMF_IS_PROXY (self));
  g_return_if_fail (desktop_file != NULL);
  g_return_if_fail (callback != NULL);
  priv = self->priv;

  data = g_new0 (ArrayCallbackData, 1);
  data->callback = callback;
  data->proxy = self;
  data->user_data = user_data;

  call = dbus_g_proxy_begin_call (priv->proxy,
                                  "XidsForDesktopFile",
                                  bamf_proxy_get_xids_async_callback,
                                  data, g_free,
                                  G_TYPE_STRING, desktop_file,
                                  G_TYPE_INVALID);

}

gchar *
bamf_proxy_get_desktop_file (BamfProxy *self,
                             guint32    xid)

{
  BamfProxyPrivate *priv;
  gchar  *desktop_file = NULL;
  GError *error = NULL;

  g_return_val_if_fail (BAMF_IS_PROXY (self), NULL);
  priv = self->priv;

  if (!dbus_g_proxy_call (priv->proxy,
                          "DesktopFileForXid",
                          &error,
                          G_TYPE_UINT, xid,
                          G_TYPE_INVALID,
                          G_TYPE_STRING, &desktop_file,
                          G_TYPE_INVALID))
    {
      g_error ("Failed to fetch desktop file: %s", error->message);
      g_error_free (error);
    }

  return desktop_file;
}

void
bamf_proxy_get_desktop_file_async (BamfProxy         *self,
                                   guint32            xid,
                                   BamfFileCallback   callback,
                                   gpointer           user_data)
{
  BamfProxyPrivate *priv;
  DBusGProxyCall   *call;
  FileCallbackData *data;

  g_return_if_fail (BAMF_IS_PROXY (self));
  g_return_if_fail (callback != NULL);
  priv = self->priv;

  data = g_new0 (FileCallbackData, 1);
  data->callback = callback;
  data->proxy = self;
  data->user_data = user_data;

  call = dbus_g_proxy_begin_call (priv->proxy,
                                  "DesktopFileForXid",
                                  bamf_proxy_get_desktop_file_async_callback,
                                  data, g_free,
                                  G_TYPE_UINT, xid,
                                  G_TYPE_INVALID);
}

void
bamf_proxy_register_match (BamfProxy         *self,
                           gchar             *desktop_file,
                           gint               pid)
{
  BamfProxyPrivate *priv;

  g_return_if_fail (BAMF_IS_PROXY (self));
  g_return_if_fail (desktop_file != NULL);
  priv = self->priv;

  dbus_g_proxy_call_no_reply (priv->proxy,
                              "RegisterDesktopFileForPid",
                              G_TYPE_STRING, desktop_file,
                              G_TYPE_INT, pid,
                              G_TYPE_INVALID);
}

