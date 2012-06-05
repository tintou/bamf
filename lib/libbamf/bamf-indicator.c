/*
 * Copyright 2010-2012 Canonical Ltd.
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
 *              Marco Trevisan (Treviño) <3v1n0@ubuntu.com>
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

#include <libbamf-private/bamf-private.h>
#include "bamf-view-private.h"
#include "bamf-indicator.h"
#include "bamf-factory.h"

G_DEFINE_TYPE (BamfIndicator, bamf_indicator, BAMF_TYPE_VIEW);

#define BAMF_INDICATOR_GET_PRIVATE(o) \
  (G_TYPE_INSTANCE_GET_PRIVATE ((o), BAMF_TYPE_INDICATOR, BamfIndicatorPrivate))

struct _BamfIndicatorPrivate
{
  BamfDBusItemIndicator *proxy;
  gchar                 *path;
  gchar                 *dbus_menu;
  gchar                 *address;
};

const gchar * 
bamf_indicator_get_dbus_menu_path (BamfIndicator *self)
{
  BamfIndicatorPrivate *priv;
  GDBusProxy *gproxy;
  GVariant *value;
  GError *error = NULL;

  g_return_val_if_fail (BAMF_IS_INDICATOR (self), NULL);
  priv = self->priv;

  if (priv->dbus_menu)
    return priv->dbus_menu;

  if (!_bamf_view_remote_ready (BAMF_VIEW (self)))
    return NULL;

  gproxy = g_dbus_proxy_new_for_bus_sync (G_BUS_TYPE_SESSION,
                                          G_DBUS_PROXY_FLAGS_DO_NOT_LOAD_PROPERTIES|
                                          G_DBUS_PROXY_FLAGS_DO_NOT_CONNECT_SIGNALS|
                                          G_DBUS_PROXY_FLAGS_DO_NOT_AUTO_START,
                                          NULL,
                                          "org.freedesktop.DBus.Properties",
                                          bamf_indicator_get_remote_path (self),
                                          bamf_indicator_get_remote_address (self),
                                          NULL, &error);

  if (!gproxy || error)
    {
      g_warning ("Failed to connect to proxy: %s", error ? error->message : "");
      g_error_free (error);

      return NULL;
    }

  value = g_dbus_proxy_call_sync (gproxy, "Get",
                                  g_variant_new ("(ss)", "org.kde.StatusNotifierItem", "Menu"),
                                  G_DBUS_CALL_FLAGS_NONE,
                                  -1,
                                  NULL,
                                  &error);

  if (error)
    {
      g_warning ("Failed to fetch remote path: %s", error->message);
      g_error_free (error);

      return NULL;
    }

  priv->dbus_menu = g_variant_dup_string (value, NULL);

  g_variant_unref (value);
  g_object_unref (gproxy);
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
  
  if (!_bamf_view_remote_ready (BAMF_VIEW (self)))
    return NULL;
  
  if (!bamf_dbus_item_indicator_call_path_sync (priv->proxy, &path, NULL, &error))
    {
      g_warning ("Failed to fetch remote path: %s", error ? error->message : "");
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
  
  if (!_bamf_view_remote_ready (BAMF_VIEW (self)))
    return NULL;
  
  if (!bamf_dbus_item_indicator_call_address_sync (priv->proxy, &address, NULL, &error))
    {
      g_warning ("Failed to fetch remote path: %s", error ? error->message : "");
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
  GError *error = NULL;

  self = BAMF_INDICATOR (view);
  priv = self->priv;

  if (priv->proxy)
    {
      g_object_unref (priv->proxy);
    }

  priv->proxy = bamf_dbus_item_indicator_proxy_new_for_bus_sync (G_BUS_TYPE_SESSION,
                                                                 G_DBUS_PROXY_FLAGS_NONE,
                                                                 BAMF_DBUS_SERVICE_NAME,
                                                                 path, NULL, &error);
  if (priv->proxy == NULL)
    {
      g_error ("Unable to get "BAMF_DBUS_SERVICE_NAME" indicator: %s", error ? error->message : "");
      g_error_free (error);
    }
}

static void
bamf_indicator_class_init (BamfIndicatorClass *klass)
{
  GObjectClass  *obj_class  = G_OBJECT_CLASS (klass);
  BamfViewClass *view_class = BAMF_VIEW_CLASS (klass);

  g_type_class_add_private (obj_class, sizeof (BamfIndicatorPrivate));

  obj_class->dispose = bamf_indicator_dispose;
  view_class->set_path = bamf_indicator_set_path;
}

static void
bamf_indicator_init (BamfIndicator *self)
{
  self->priv = BAMF_INDICATOR_GET_PRIVATE (self);
}

BamfIndicator *
bamf_indicator_new (const char * path)
{
  BamfIndicator *self;
  self = g_object_new (BAMF_TYPE_INDICATOR, NULL);

  _bamf_view_set_path (BAMF_VIEW (self), path);

  return self;
}
