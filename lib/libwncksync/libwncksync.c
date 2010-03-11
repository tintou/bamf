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

#include "libwncksync.h"

G_DEFINE_TYPE (WncksyncProxy, wncksync_proxy, G_TYPE_OBJECT);
#define WNCKSYNC_PROXY_GET_PRIVATE(obj) (G_TYPE_INSTANCE_GET_PRIVATE(obj, \
WNCKSYNC_TYPE_PROXY, WncksyncProxyPrivate))

struct _WncksyncProxyPrivate
{
  DBusGConnection *connection;
  DBusGProxy      *proxy;
};

void
wncksync_proxy_register_match (WncksyncProxy *proxy, gchar * desktop_file, gint pid)
{
  g_return_if_fail (WNCKSYNC_IS_PROXY (proxy));
  g_return_if_fail (desktop_file);
  
  dbus_g_proxy_call_no_reply (proxy->priv->proxy,
			      "RegisterDesktopFileForPid",
			      G_TYPE_STRING, desktop_file,
			      G_TYPE_INT, pid, G_TYPE_INVALID);
}

GArray *
wncksync_proxy_get_xids (WncksyncProxy *proxy, gchar * desktop_file)
{
  GArray *arr = NULL;
  GError *error = NULL;

  g_return_val_if_fail (WNCKSYNC_IS_PROXY (proxy), arr);

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

gchar *
wncksync_proxy_get_desktop_file (WncksyncProxy *proxy, guint32 xid)
{
  gchar *desktop_file;
  GError *error = NULL;

  g_return_val_if_fail (WNCKSYNC_IS_PROXY (proxy), "");
  
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
wncksync_proxy_class_init (WncksyncProxyClass * klass)
{
  g_type_class_add_private (klass, sizeof (WncksyncProxyPrivate));
}

static void
wncksync_proxy_init (WncksyncProxy * self)
{
  WncksyncProxyPrivate *priv;

  self->priv = priv = WNCKSYNC_PROXY_GET_PRIVATE (self);
}

WncksyncProxy *
wncksync_proxy_get_default (void)
{
  static WncksyncProxy *self;
  WncksyncProxyPrivate *priv;
  GError *error = NULL;
  
  if (WNCKSYNC_IS_PROXY (self))
  	return self;
  
  self = (WncksyncProxy *) g_object_new (WNCKSYNC_TYPE_PROXY, NULL);
  priv = self->priv;

  priv->connection = dbus_g_bus_get (DBUS_BUS_SESSION, &error);

  if (priv->connection == NULL)
    {
      g_printerr ("Failed to open bus: %s\n", error->message);
      g_error_free (error);
    }

  priv->proxy = dbus_g_proxy_new_for_name (priv->connection,
					   "org.wncksync.Matcher",
					   "/org/wncksync/Matcher",
					   "org.wncksync.Matcher");

  if (!priv->proxy)
    {
      g_printerr ("Failed to get name owner.\n");
    }

  return self;
}
