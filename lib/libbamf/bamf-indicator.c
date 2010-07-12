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
  gchar *address;
};

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
  
  if (!dbus_g_proxy_call (priv->proxy,
                          "Adress",
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
bamf_indicator_constructed (GObject *object)
{
  BamfIndicator *self;
  BamfIndicatorPrivate *priv;
  gchar *path;
  
  if (G_OBJECT_CLASS (bamf_indicator_parent_class)->constructed)
    G_OBJECT_CLASS (bamf_indicator_parent_class)->constructed (object);
  
  g_object_get (object, "path", &path, NULL);
  
  self = BAMF_INDICATOR (object);
  priv = self->priv;

  priv->proxy = dbus_g_proxy_new_for_name (priv->connection,
                                           "org.ayatana.bamf",
                                           path,
                                           "org.ayatana.bamf.indicator");
  if (priv->proxy == NULL)
    {
      g_error ("Unable to get org.ayatana.bamf.indicator indicator");
    }

  g_free (path);
  
}

static void
bamf_indicator_class_init (BamfIndicatorClass *klass)
{
  GObjectClass  *obj_class  = G_OBJECT_CLASS (klass);

  g_type_class_add_private (obj_class, sizeof (BamfIndicatorPrivate));
  
  obj_class->constructed = bamf_indicator_constructed;
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
  self = g_object_new (BAMF_TYPE_INDICATOR, "path", path, NULL);

  return self;
}
