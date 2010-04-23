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

#define WNCK_I_KNOW_THIS_IS_UNSTABLE

#include "config.h"

#include <gtk/gtk.h>
#include <gdk/gdk.h>
#include <dbus/dbus.h>
#include <dbus/dbus-glib.h>
#include <dbus/dbus-glib-lowlevel.h>

#include "libbamf.h"

G_DEFINE_TYPE (BamfProxy, bamf_proxy, G_TYPE_OBJECT);
#define BAMF_PROXY_GET_PRIVATE(obj) (G_TYPE_INSTANCE_GET_PRIVATE(obj, \
BAMF_TYPE_PROXY, BamfProxyPrivate))

struct _BamfProxyPrivate
{
  DBusGConnection *connection;
  DBusGProxy      *proxy;
};

typedef struct _FileCallbackData FileCallbackData;
struct _FileCallbackData
{
  BamfProxy        *proxy;
  BamfFileCallback  callback;
  gpointer              user_data;
};

typedef struct _ArrayCallbackData ArrayCallbackData;
struct _ArrayCallbackData
{
  BamfProxy         *proxy;
  BamfArrayCallback  callback;
  gpointer               user_data;
};

void
bamf_proxy_register_match (BamfProxy *proxy, gchar * desktop_file, gint pid)
{
  g_return_if_fail (BAMF_IS_PROXY (proxy));
  g_return_if_fail (desktop_file);
  
  dbus_g_proxy_call_no_reply (proxy->priv->proxy,
			      "RegisterDesktopFileForPid",
			      G_TYPE_STRING, desktop_file,
			      G_TYPE_INT, pid, G_TYPE_INVALID);
}

GArray *
bamf_proxy_get_xids (BamfProxy *proxy, gchar * desktop_file)
{
  GArray *arr = NULL;
  GError *error = NULL;

  g_return_val_if_fail (BAMF_IS_PROXY (proxy), arr);

  if (!dbus_g_proxy_call (proxy->priv->proxy,
			  "XidsForDesktopFile",
			  &error,
			  G_TYPE_STRING, desktop_file,
			  G_TYPE_INVALID,
			  DBUS_TYPE_G_UINT_ARRAY, &arr, G_TYPE_INVALID))
    {
      g_printerr ("Failed to fetch xid array: %s\n", error->message);
      g_error_free (error);
      //Error Handling
      arr = g_array_new (FALSE, TRUE, sizeof (guint32));
    }

  return arr;
}

static void
xids_for_file_async_callback (DBusGProxy     *proxy, 
                              DBusGProxyCall *call, 
                              gpointer       *user_data)
{
  GArray *arr = NULL;
  GError *error = NULL;
  ArrayCallbackData *callback_data;
  
  callback_data = (ArrayCallbackData *) user_data;
  
  if (!dbus_g_proxy_end_call (proxy,
                              call,
                              &error,
                              DBUS_TYPE_G_UINT_ARRAY, &arr,
                              G_TYPE_INVALID))
    {
      g_printerr ("Failed to complete async callback: %s\n",
                  error->message);
      g_error_free (error);
      return;
    }
  
  (callback_data->callback) (callback_data->proxy, arr, callback_data->user_data);
}

void
bamf_proxy_get_xids_async (BamfProxy *proxy,
                               gchar *desktop_file,
                               BamfArrayCallback callback,
                               gpointer user_data)
{
  DBusGProxyCall *call;
  BamfProxyPrivate *priv;
  ArrayCallbackData *data;
  
  g_return_if_fail (BAMF_IS_PROXY (proxy));

  priv = proxy->priv;

  data = (ArrayCallbackData *) g_malloc0 (sizeof (ArrayCallbackData));
  
  data->callback = callback;
  data->proxy = proxy;
  data->user_data = user_data;

  call = dbus_g_proxy_begin_call (priv->proxy,
                                  "XidsForDesktopFile",
                                  (DBusGProxyCallNotify) xids_for_file_async_callback,
                                  data,
                                  g_free,
                                  G_TYPE_STRING, desktop_file,
                                  G_TYPE_INVALID);
}

gchar *
bamf_proxy_get_desktop_file (BamfProxy *proxy, guint32 xid)
{
  gchar *desktop_file;
  GError *error = NULL;

  g_return_val_if_fail (BAMF_IS_PROXY (proxy), "");
  
  if (!dbus_g_proxy_call (proxy->priv->proxy,
			  "DesktopFileForXid",
			  &error,
			  G_TYPE_UINT, xid,
			  G_TYPE_INVALID,
			  G_TYPE_STRING, &desktop_file, G_TYPE_INVALID))
    {
      g_printerr ("Failed to fetch desktop file: %s\n",
		  error->message);
      g_error_free (error);

      return "";
    }

  gchar *result = g_strdup (desktop_file);
  g_free (desktop_file);

  return result;
}

static void
file_for_xid_async_callback (DBusGProxy     *proxy, 
                             DBusGProxyCall *call, 
                             gpointer       *user_data)
{
  gchar *desktop_file = NULL;
  GError *error = NULL;
  FileCallbackData *callback_data;
  
  callback_data = (FileCallbackData *) user_data;
  
  if (!dbus_g_proxy_end_call (proxy,
                              call,
                              &error,
                              G_TYPE_STRING, &desktop_file,
                              G_TYPE_INVALID))
    {
      g_printerr ("Failed to complete async callback: %s\n",
                  error->message);
      g_error_free (error);
      return;
    }
  
  (callback_data->callback) (callback_data->proxy, g_strdup (desktop_file), callback_data->user_data);

  if (desktop_file)
    g_free (desktop_file);
}

void
bamf_proxy_get_desktop_file_async (BamfProxy       *proxy,
                                       guint32              xid,
                                       BamfFileCallback callback,
                                       gpointer             user_data)
{
  DBusGProxyCall *call;
  BamfProxyPrivate *priv;
  FileCallbackData *data;
  
  g_return_if_fail (BAMF_IS_PROXY (proxy));

  priv = proxy->priv;

  data = (FileCallbackData *) g_malloc0 (sizeof (FileCallbackData));
  
  data->callback = callback;
  data->proxy = proxy;
  data->user_data = user_data;

  call = dbus_g_proxy_begin_call (priv->proxy,
                                  "DesktopFileForXid",
                                  (DBusGProxyCallNotify) file_for_xid_async_callback,
                                  data,
                                  g_free,
                                  G_TYPE_UINT, xid,
                                  G_TYPE_INVALID);
}

static void
bamf_proxy_class_init (BamfProxyClass * klass)
{
  g_type_class_add_private (klass, sizeof (BamfProxyPrivate));
}

static void
bamf_proxy_init (BamfProxy * self)
{
  BamfProxyPrivate *priv;

  self->priv = priv = BAMF_PROXY_GET_PRIVATE (self);
}

BamfProxy *
bamf_proxy_get_default (void)
{
  static BamfProxy *self;
  BamfProxyPrivate *priv;
  GError *error = NULL;
  
  if (BAMF_IS_PROXY (self))
  	return self;
  
  self = (BamfProxy *) g_object_new (BAMF_TYPE_PROXY, NULL);
  priv = self->priv;
  
  priv->connection = dbus_g_bus_get (DBUS_BUS_SESSION, &error);

  if (priv->connection == NULL)
    {
      g_printerr ("Failed to open bus: %s\n", error->message);
      g_error_free (error);
    }

  priv->proxy = dbus_g_proxy_new_for_name (priv->connection,
					   "org.bamf.Matcher",
					   "/org/bamf/Matcher",
					   "org.bamf.Matcher");

  if (!priv->proxy)
    {
      g_printerr ("Failed to get name owner.\n");
    }

  return self;
}
