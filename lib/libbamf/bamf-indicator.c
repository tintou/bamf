/*
 * Copyright 2010 Canonical Ltd.
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
 * SECTION:bamf-indicator
 * @short_description: The base class for all indicators
 *
 * #BamfIndicator is the base class that all indicators need to derive from.
 */

#if HAVE_CONFIG_H
#include <config.h>
#endif

#include "bamf-view-private.h"
#include "bamf-indicator.h"
#include "bamf-factory.h"
#include <dbus/dbus.h>
#include <dbus/dbus-glib.h>
#include <dbus/dbus-glib-lowlevel.h>

G_DEFINE_TYPE (BamfIndicator, bamf_indicator, BAMF_TYPE_VIEW);

#define BAMF_INDICATOR_GET_PRIVATE(o) \
  (G_TYPE_INSTANCE_GET_PRIVATE ((o), BAMF_TYPE_INDICATOR, BamfIndicatorPrivate))

struct _BamfIndicatorPrivate
{
  DBusGConnection *connection;
  DBusGProxy      *proxy;
  gchar *path;
  gchar *dbus_menu;
  gchar *address;
};

const gchar * 
bamf_indicator_get_dbus_menu_path (BamfIndicator *self)
{
  BamfIndicatorPrivate *priv;
  gchar *path = NULL;
  DBusGProxy *proxy;
  GValue value = {0};
  GError *error = NULL;

  g_return_val_if_fail (BAMF_IS_INDICATOR (self), NULL);
  priv = self->priv;
  
  if (priv->dbus_menu)
    return priv->dbus_menu;
  
  if (!bamf_view_remote_ready (BAMF_VIEW (self)))
    return NULL;
  
  proxy = dbus_g_proxy_new_for_name (priv->connection,
                                     bamf_indicator_get_remote_address (self),
                                     bamf_indicator_get_remote_path (self),
                                     "org.freedesktop.DBus.Properties");
  
  if (!dbus_g_proxy_call (proxy,
                          "Get",
                          &error,
                          G_TYPE_STRING, "org.kde.StatusNotifierItem",
                          G_TYPE_STRING, "Menu",
                          G_TYPE_INVALID,
                          G_TYPE_VALUE, &value,
                          G_TYPE_INVALID))
    {
      g_warning ("Failed to fetch remote path: %s", error->message);
      g_error_free (error);
      
      return NULL;
    }
  
  path = g_value_get_boxed (&value);
  priv->dbus_menu = path;

  g_object_unref (proxy);
  return priv->dbus_menu;
}

const gchar * 
bamf_indicator_get_remote_path (BamfIndicator *self)
{
  BamfIndicatorPrivate *priv;
  char *path = NULL;
  GError *error = NULL;

  g_return_val_if_fail (BAMF_IS_INDICATOR (self), NULL);
  priv = self->priv;
  
  if (priv->path)
    return priv->path;
  
  if (!bamf_view_remote_ready (BAMF_VIEW (self)))
    return NULL;
  
  if (!dbus_g_proxy_call (priv->proxy,
                          "Path",
                          &error,
                          G_TYPE_INVALID,
                          G_TYPE_STRING, &path,
                          G_TYPE_INVALID))
    {
      g_warning ("Failed to fetch remote path: %s", error->message);
      g_error_free (error);
      
      return NULL;
    }

  priv->path = path;

  return path;
}

const gchar * 
bamf_indicator_get_remote_address (BamfIndicator *self)
{
  BamfIndicatorPrivate *priv;
  char *address = NULL;
  GError *error = NULL;

  g_return_val_if_fail (BAMF_IS_INDICATOR (self), NULL);
  priv = self->priv;
  
  if (priv->address)
    return priv->address;
  
  if (!bamf_view_remote_ready (BAMF_VIEW (self)))
    return NULL;
  
  if (!dbus_g_proxy_call (priv->proxy,
                          "Address",
                          &error,
                          G_TYPE_INVALID,
                          G_TYPE_STRING, &address,
                          G_TYPE_INVALID))
    {
      g_warning ("Failed to fetch remote path: %s", error->message);
      g_error_free (error);
      
      return NULL;
    }

  priv->address = address;

  return address;
}

static void
bamf_indicator_dispose (GObject *object)
{
  BamfIndicator *self;
  BamfIndicatorPrivate *priv;
  
  self = BAMF_INDICATOR (object);
  priv = self->priv;

  if (priv->proxy)
    {
      g_object_unref (priv->proxy);
      priv->proxy = NULL;
    }
  
  if (priv->path)
    {
      g_free (priv->path);
      priv->path = NULL;
    }
  
  if (priv->dbus_menu)
    {
      g_free (priv->dbus_menu);
      priv->dbus_menu = NULL;
    }
  
  if (priv->address)
    {
      g_free (priv->address);
      priv->address = NULL;
    }
  
  if (G_OBJECT_CLASS (bamf_indicator_parent_class)->dispose)
    G_OBJECT_CLASS (bamf_indicator_parent_class)->dispose (object);
}

static void
bamf_indicator_set_path (BamfView *view, const char *path)
{
  BamfIndicator *self;
  BamfIndicatorPrivate *priv;
  
  self = BAMF_INDICATOR (view);
  priv = self->priv;

  if (priv->proxy)
    {
      g_object_unref (priv->proxy);
    }

  priv->proxy = dbus_g_proxy_new_for_name (priv->connection,
                                           "org.ayatana.bamf",
                                           path,
                                           "org.ayatana.bamf.indicator");
  if (priv->proxy == NULL)
    {
      g_error ("Unable to get org.ayatana.bamf.indicator indicator");
    }
}

static void
bamf_indicator_class_init (BamfIndicatorClass *klass)
{
  GObjectClass  *obj_class  = G_OBJECT_CLASS (klass);
  BamfViewClass *view_class = BAMF_VIEW_CLASS (klass);

  g_type_class_add_private (obj_class, sizeof (BamfIndicatorPrivate));
  
  obj_class->dispose     = bamf_indicator_dispose;
  view_class->set_path   = bamf_indicator_set_path;
}


static void
bamf_indicator_init (BamfIndicator *self)
{
  BamfIndicatorPrivate *priv;
  GError           *error = NULL;

  priv = self->priv = BAMF_INDICATOR_GET_PRIVATE (self);

  priv->connection = dbus_g_bus_get (DBUS_BUS_SESSION, &error);
  if (priv->connection == NULL)
    {
      g_critical ("Failed to open connection to bus: %s",
               error != NULL ? error->message : "Unknown");
      if (error)
        g_error_free (error);
      return;
    }
}

BamfIndicator *
bamf_indicator_new (const char * path)
{
  BamfIndicator *self;
  self = g_object_new (BAMF_TYPE_INDICATOR, NULL);

  bamf_view_set_path (BAMF_VIEW (self), path);

  return self;
}
