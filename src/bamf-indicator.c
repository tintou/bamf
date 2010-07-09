/*
 * bamf-indicator.c
 * This file is part of BAMF
 *
 * Copyright (C) 2010 - Jason Smith
 *
 * BAMF is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * BAMF is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANINDICATORILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with BAMF; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin St, Fifth Floor, 
 * Boston, MA  02110-1301  USA
 */

#include "bamf-indicator.h"
#include "bamf-indicator-glue.h"


#define BAMF_INDICATOR_GET_PRIVATE(object)(G_TYPE_INSTANCE_GET_PRIVATE((object), BAMF_TYPE_INDICATOR, BamfIndicatorPrivate))

enum
{
  PROP_0,
  
  PROP_ADDRESS,
  PROP_PID,
  PROP_PROXY,
};


struct _BamfIndicatorPrivate
{
  char *path;
  char *address;
  guint32 pid;
  DBusGProxy *proxy;
};

G_DEFINE_TYPE (BamfIndicator, bamf_indicator, BAMF_TYPE_VIEW)

char *
bamf_indicator_get_path (BamfIndicator *self)
{
  g_return_val_if_fail (BAMF_IS_INDICATOR (self), NULL);
  return g_strdup (self->priv->path);
}

char *
bamf_indicator_get_address (BamfIndicator *self)
{
  g_return_val_if_fail (BAMF_IS_INDICATOR (self), NULL);
  return g_strdup (self->priv->address);
}

guint32
bamf_indicator_get_pid (BamfIndicator *self)
{
  g_return_val_if_fail (BAMF_IS_INDICATOR (self), 0);
  return self->priv->pid;
}

static char *
bamf_indicator_get_view_type (BamfView *view)
{
  return g_strdup ("indicator");
}

static void
bamf_indicator_set_property (GObject *object, guint property_id, const GValue *value, GParamSpec *pspec)
{
  BamfIndicator *self;

  self = BAMF_INDICATOR (object);

  switch (property_id)
    {
      case PROP_ADDRESS:
        self->priv->address = g_value_dup_string (value);
        break;
      
      case PROP_PROXY:
        self->priv->path = g_value_dup_string (value);
        break;
      
      case PROP_PID:
        self->priv->pid = g_value_get_uint (value);
        break;

      default:
        G_OBJECT_WARN_INVALID_PROPERTY_ID(object, property_id, pspec);
    }
}

static void
bamf_indicator_get_property (GObject *object, guint property_id, GValue *value, GParamSpec *pspec)
{
  BamfIndicator *self;

  self = BAMF_INDICATOR (object);

  switch (property_id)
    {
      case PROP_ADDRESS:
        g_value_set_string (value, self->priv->address);
        break;
      
      case PROP_PROXY:
        g_value_set_string (value, self->priv->path);
        break;
      
      case PROP_PID:
        g_value_set_uint (value, self->priv->pid);
        break;

      default:
        G_OBJECT_WARN_INVALID_PROPERTY_ID(object, property_id, pspec);
    }
}

static void
bamf_indicator_on_destroy (DBusGProxy *proxy, BamfIndicator *self)
{
  bamf_view_close (BAMF_VIEW (self));
}

static void
bamf_indicator_constructed (GObject *object)
{
  BamfIndicator *self;
  BamfIndicatorPrivate *priv;
  DBusGProxy *proxy;
  DBusGConnection *bus;
  GError *error = NULL;

  if (G_OBJECT_CLASS (bamf_indicator_parent_class)->constructed)
    G_OBJECT_CLASS (bamf_indicator_parent_class)->constructed (object);
  
  self = BAMF_INDICATOR (object);
  priv = self->priv;
  
  bus = dbus_g_bus_get (DBUS_BUS_SESSION, &error);
  
  if (!bus)
    {
      g_warning ("Could not get session bus\n");
      return;
    }
  
  error = NULL;
  if (!(proxy = dbus_g_proxy_new_for_name_owner (bus, 
                                                 priv->address,
                                                 priv->path,
                                                 "org.ayatana.indicator.application.service",
                                                 &error)))
    {
      g_warning ("Could not setup proxy: %s %s\n", priv->address, priv->path);
      return;
    }
  
  priv->proxy = proxy;
  
  g_signal_connect (G_OBJECT (proxy), "destroy", (GCallback) bamf_indicator_on_destroy, self);
}

static void
bamf_indicator_dispose (GObject *object)
{
  BamfIndicatorPrivate *priv;
  
  priv = BAMF_INDICATOR (object)->priv;

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

  if (priv->address)
    {
      g_free (priv->address);
      priv->address = NULL;
    }

  G_OBJECT_CLASS (bamf_indicator_parent_class)->dispose (object);
}

static void
bamf_indicator_class_init (BamfIndicatorClass *klass)
{
  GParamSpec *pspec;
  GObjectClass *object_class = G_OBJECT_CLASS (klass);
  BamfViewClass *view_class = BAMF_VIEW_CLASS (klass);
	
  object_class->constructed  = bamf_indicator_constructed;
  object_class->get_property = bamf_indicator_get_property;
  object_class->set_property = bamf_indicator_set_property;
  object_class->dispose      = bamf_indicator_dispose;
  view_class->view_type      = bamf_indicator_get_view_type;

  pspec = g_param_spec_string ("address", "address", "address", NULL, G_PARAM_READWRITE | G_PARAM_CONSTRUCT);
  g_object_class_install_property (object_class, PROP_ADDRESS, pspec);
  
  pspec = g_param_spec_string ("path", "path", "path", NULL, G_PARAM_READWRITE | G_PARAM_CONSTRUCT);
  g_object_class_install_property (object_class, PROP_PROXY, pspec);
  
  pspec = g_param_spec_uint ("pid", "pid", "pid", 0, G_MAXUINT32, 0, G_PARAM_READWRITE);
  g_object_class_install_property (object_class, PROP_PID, pspec);

  dbus_g_object_type_install_info (BAMF_TYPE_INDICATOR,
				   &dbus_glib_bamf_indicator_object_info);

  g_type_class_add_private (object_class, sizeof (BamfIndicatorPrivate));
}

static void
bamf_indicator_init (BamfIndicator *self)
{
  self->priv = BAMF_INDICATOR_GET_PRIVATE (self);
}

BamfIndicator *
bamf_indicator_new (const char *path, const char *address, guint32 pid)
{
  BamfIndicator *self;
  self = g_object_new (BAMF_TYPE_INDICATOR, 
                       "address", address,
                       "path", path,
                       "pid", pid,
                       NULL);
                       
  return self;
}
