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
  
  PROP_ID,
  PROP_CATEGORY,
  PROP_PATH,
  PROP_ADDRESS,
  PROP_PID,
};


struct _BamfIndicatorPrivate
{
  char *id;
  char *path;
  char *address;
  char *category;
  guint32 pid;
};

G_DEFINE_TYPE (BamfIndicator, bamf_indicator, BAMF_TYPE_VIEW)

char *
bamf_indicator_get_id (BamfIndicator *self)
{
  g_return_val_if_fail (BAMF_IS_INDICATOR (self), NULL);
  return self->priv->id;
}

char *
bamf_indicator_get_path (BamfIndicator *self)
{
  g_return_val_if_fail (BAMF_IS_INDICATOR (self), NULL);
  return self->priv->path;
}

char *
bamf_indicator_get_address (BamfIndicator *self)
{
  g_return_val_if_fail (BAMF_IS_INDICATOR (self), NULL);
  return self->priv->address;
}

char *
bamf_indicator_get_category (BamfIndicator *self)
{
  g_return_val_if_fail (BAMF_IS_INDICATOR (self), NULL);
  return self->priv->category;
}

guint32
bamf_indicator_get_pid (BamfIndicator *self)
{
  g_return_val_if_fail (BAMF_IS_INDICATOR (self), 0);
  return self->priv->pid;
}

static void
bamf_indicator_set_property (GObject *object, guint property_id, const GValue *value, GParamSpec *pspec)
{
  BamfIndicator *self;

  self = BAMF_INDICATOR (object);

  switch (property_id)
    {
      case PROP_ID:
        self->priv->id = g_value_dup_string (value);
        break;
      
      case PROP_ADDRESS:
        self->priv->address = g_value_dup_string (value);
        break;
      
      case PROP_PATH:
        self->priv->path = g_value_dup_string (value);
        break;
      
      case PROP_CATEGORY:
        self->priv->category = g_value_dup_string (value);
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
      case PROP_ID:
        g_value_set_string (value, self->priv->id);
        break;
      
      case PROP_ADDRESS:
        g_value_set_string (value, self->priv->address);
        break;
      
      case PROP_PATH:
        g_value_set_string (value, self->priv->path);
        break;
      
      case PROP_CATEGORY:
        g_value_set_string (value, self->priv->category);
        break;
 
      case PROP_PID:
        g_value_set_uint (value, self->priv->pid);
        break;

      default:
        G_OBJECT_WARN_INVALID_PROPERTY_ID(object, property_id, pspec);
    }
}

static void
bamf_indicator_constructed (GObject *object)
{
  if (G_OBJECT_CLASS (bamf_indicator_parent_class)->constructed)
    G_OBJECT_CLASS (bamf_indicator_parent_class)->constructed (object);
}

static void
bamf_indicator_dispose (GObject *object)
{
  G_OBJECT_CLASS (bamf_indicator_parent_class)->dispose (object);
}

static void
bamf_indicator_class_init (BamfIndicatorClass *klass)
{
  GParamSpec *pspec;
  GObjectClass *object_class = G_OBJECT_CLASS (klass);
	
  object_class->constructed  = bamf_indicator_constructed;
  object_class->get_property = bamf_indicator_get_property;
  object_class->set_property = bamf_indicator_set_property;
  object_class->dispose      = bamf_indicator_dispose;

  pspec = g_param_spec_string ("id", "id", "id", NULL, G_PARAM_READABLE | G_PARAM_CONSTRUCT);
  g_object_class_install_property (object_class, PROP_ID, pspec);
  
  pspec = g_param_spec_string ("path", "path", "path", NULL, G_PARAM_READABLE | G_PARAM_CONSTRUCT);
  g_object_class_install_property (object_class, PROP_PATH, pspec);
  
  pspec = g_param_spec_string ("address", "address", "address", NULL, G_PARAM_READABLE | G_PARAM_CONSTRUCT);
  g_object_class_install_property (object_class, PROP_ADDRESS, pspec);
  
  pspec = g_param_spec_string ("category", "category", "category", NULL, G_PARAM_READABLE | G_PARAM_CONSTRUCT);
  g_object_class_install_property (object_class, PROP_CATEGORY, pspec);

  pspec = g_param_spec_uint ("pid", "pid", "pid", 0, G_MAXUINT, 0, G_PARAM_READABLE | G_PARAM_CONSTRUCT);
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
bamf_indicator_new (char *id, char* path, char *address, char *category, guint32 pid)
{
  return g_object_new (BAMF_TYPE_INDICATOR, 
                       "id", id,
                       "path", path,
                       "address", address,
                       "category", category,
                       "pid", pid,
                       NULL);
}
