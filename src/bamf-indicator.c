/*
 * Copyright (C) 2010-2011 Canonical Ltd
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 3 as
 * published by the Free Software Foundation.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 *
 * Authored by: Jason Smith <jason.smith@canonical.com>
 *              Marco Trevisan (Treviño) <3v1n0@ubuntu.com>
 *
 */

#include "bamf-indicator.h"

#define BAMF_INDICATOR_GET_PRIVATE(object)(G_TYPE_INSTANCE_GET_PRIVATE((object), \
                                           BAMF_TYPE_INDICATOR, BamfIndicatorPrivate))

enum
{
  PROP_0,
  
  PROP_ADDRESS,
  PROP_PID,
  PROP_PATH,
  PROP_ID,
};


struct _BamfIndicatorPrivate
{
  BamfDBusItemIndicator *dbus_iface;
  char *id;
  char *path;
  char *address;
  guint32 pid;
  GDBusProxy *proxy;
};

G_DEFINE_TYPE (BamfIndicator, bamf_indicator, BAMF_TYPE_VIEW)

const char *
bamf_indicator_get_path (BamfIndicator *self)
{
  g_return_val_if_fail (BAMF_IS_INDICATOR (self), NULL);
  return self->priv->path;
}

const char *
bamf_indicator_get_address (BamfIndicator *self)
{
  g_return_val_if_fail (BAMF_IS_INDICATOR (self), NULL);
  return self->priv->address;
}

const char *
bamf_indicator_get_id (BamfIndicator *self)
{
  g_return_val_if_fail (BAMF_IS_INDICATOR (self), NULL);
  return self->priv->id;
}

guint32
bamf_indicator_get_pid (BamfIndicator *self)
{
  g_return_val_if_fail (BAMF_IS_INDICATOR (self), 0);
  return self->priv->pid;
}

gboolean
bamf_indicator_matches_signature (BamfIndicator *self, gint pid, const char *address, const char *path)
{
  g_return_val_if_fail (BAMF_IS_INDICATOR (self), FALSE);

  return g_strcmp0 (self->priv->address, address) == 0 &&
         g_strcmp0 (self->priv->path, path) == 0 &&
         pid == self->priv->pid;         
}

static const char *
bamf_indicator_get_view_type (BamfView *view)
{
  return "indicator";
}

static char *
bamf_indicator_get_stable_bus_name (BamfView *view)
{
  BamfIndicator *self;

  g_return_val_if_fail (BAMF_IS_INDICATOR (view), NULL);  
  self = BAMF_INDICATOR (view);
  
  return g_strdup_printf ("indicator%i", bamf_indicator_get_pid (self));
}

static void
bamf_indicator_set_property (GObject *object, guint property_id,
                             const GValue *value, GParamSpec *pspec)
{
  BamfIndicator *self;

  self = BAMF_INDICATOR (object);

  switch (property_id)
    {
      case PROP_ADDRESS:
        self->priv->address = g_value_dup_string (value);
        break;
      
      case PROP_PATH:
        self->priv->path = g_value_dup_string (value);
        break;
        
      case PROP_ID:
        self->priv->id = g_value_dup_string (value);
        break;
      
      case PROP_PID:
        self->priv->pid = g_value_get_uint (value);
        break;

      default:
        G_OBJECT_WARN_INVALID_PROPERTY_ID (object, property_id, pspec);
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
      
      case PROP_PATH:
        g_value_set_string (value, self->priv->path);
        break;
      
      case PROP_ID:
        g_value_set_string (value, self->priv->id);
        break;
      
      case PROP_PID:
        g_value_set_uint (value, self->priv->pid);
        break;

      default:
        G_OBJECT_WARN_INVALID_PROPERTY_ID (object, property_id, pspec);
    }
}

static void
bamf_indicator_on_destroy (GObject *object, GParamSpec *pspec, BamfIndicator *self)
{
  g_object_unref (self->priv->proxy);
  self->priv->proxy = NULL;

  bamf_view_close (BAMF_VIEW (self));
}

static gboolean
on_dbus_handle_path (BamfDBusItemIndicator *interface,
                     GDBusMethodInvocation *invocation,
                     BamfIndicator *self)
{
  const char *path = bamf_indicator_get_path (self);
  g_dbus_method_invocation_return_value (invocation,
                                         g_variant_new ("(s)", path ? path : ""));

  return TRUE;
}

static gboolean
on_dbus_handle_address (BamfDBusItemIndicator *interface,
                        GDBusMethodInvocation *invocation,
                        BamfIndicator *self)
{
  const char *address = bamf_indicator_get_address (self);
  g_dbus_method_invocation_return_value (invocation,
                                         g_variant_new ("(s)", address ? address : ""));

  return TRUE;
}

static void
bamf_indicator_constructed (GObject *object)
{
  BamfIndicator *self;
  BamfIndicatorPrivate *priv;
  GDBusProxy *proxy;
  GError *error = NULL;

  if (G_OBJECT_CLASS (bamf_indicator_parent_class)->constructed)
    G_OBJECT_CLASS (bamf_indicator_parent_class)->constructed (object);

  self = BAMF_INDICATOR (object);
  priv = self->priv;

  proxy = g_dbus_proxy_new_for_bus_sync (G_BUS_TYPE_SESSION,
                                         G_DBUS_PROXY_FLAGS_DO_NOT_LOAD_PROPERTIES|
                                         G_DBUS_PROXY_FLAGS_DO_NOT_CONNECT_SIGNALS|
                                         G_DBUS_PROXY_FLAGS_DO_NOT_AUTO_START,
                                         NULL,
                                         priv->address,
                                         priv->path,
                                         "org.ayatana.indicator.application.service",
                                         NULL,
                                         &error);

  if (error)
    {
      g_debug ("Could not setup proxy: %s %s: %s", priv->address, priv->path,
                                                   error->message);
      g_error_free (error);
    }
  else
    {
      gchar *owner = g_dbus_proxy_get_name_owner (proxy);

      if (owner)
        {
          g_free (owner);

          g_signal_connect (G_OBJECT (proxy), "notify::g-name-owner",
                            G_CALLBACK (bamf_indicator_on_destroy), self);

          if (priv->proxy)
            g_object_unref (priv->proxy);

          priv->proxy = proxy;
        }
      else
        {
           g_debug ("Failed to get notification approver proxy: no owner available");
           g_object_unref (proxy);
        }
    }
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
    
  if (priv->id)
    {
      g_free (priv->id);
      priv->id = NULL;
    }  

  G_OBJECT_CLASS (bamf_indicator_parent_class)->dispose (object);
}

static void
bamf_indicator_finalize (GObject *object)
{
  BamfIndicatorPrivate *priv;
  
  priv = BAMF_INDICATOR (object)->priv;

  g_object_unref (priv->dbus_iface);

  G_OBJECT_CLASS (bamf_indicator_parent_class)->finalize (object);
}

static void
bamf_indicator_class_init (BamfIndicatorClass *klass)
{
  GParamSpec *pspec;
  GObjectClass *object_class = G_OBJECT_CLASS (klass);
  BamfViewClass *view_class = BAMF_VIEW_CLASS (klass);

  object_class->constructed   = bamf_indicator_constructed;
  object_class->get_property  = bamf_indicator_get_property;
  object_class->set_property  = bamf_indicator_set_property;
  object_class->dispose       = bamf_indicator_dispose;
  object_class->finalize      = bamf_indicator_finalize;

  view_class->view_type       = bamf_indicator_get_view_type;
  view_class->stable_bus_name = bamf_indicator_get_stable_bus_name;

  pspec = g_param_spec_string ("address", "address", "address", NULL, G_PARAM_READWRITE | G_PARAM_CONSTRUCT);
  g_object_class_install_property (object_class, PROP_ADDRESS, pspec);
  
  pspec = g_param_spec_string ("path", "path", "path", NULL, G_PARAM_READWRITE | G_PARAM_CONSTRUCT);
  g_object_class_install_property (object_class, PROP_PATH, pspec);
  
  pspec = g_param_spec_string ("id", "id", "id", NULL, G_PARAM_READWRITE);
  g_object_class_install_property (object_class, PROP_ID, pspec);
  
  pspec = g_param_spec_uint ("pid", "pid", "pid", 0, G_MAXUINT32, 0, G_PARAM_READWRITE);
  g_object_class_install_property (object_class, PROP_PID, pspec);

  g_type_class_add_private (object_class, sizeof (BamfIndicatorPrivate));
}

static void
bamf_indicator_init (BamfIndicator *self)
{
  self->priv = BAMF_INDICATOR_GET_PRIVATE (self);

  self->priv->dbus_iface = bamf_dbus_item_indicator_skeleton_new ();

  /* Registering signal callbacks to reply to dbus method calls */
  g_signal_connect (self->priv->dbus_iface, "handle-path",
                    G_CALLBACK (on_dbus_handle_path), self);

  g_signal_connect (self->priv->dbus_iface, "handle-address",
                    G_CALLBACK (on_dbus_handle_address), self);

  /* Setting the interface for the dbus object */
  bamf_dbus_item_object_skeleton_set_indicator (BAMF_DBUS_ITEM_OBJECT_SKELETON (self),
                                                self->priv->dbus_iface);
}

BamfIndicator *
bamf_indicator_new (const char *id, const char *path, const char *address, guint32 pid)
{
  BamfIndicator *self;
  self = g_object_new (BAMF_TYPE_INDICATOR, 
                       "address", address,
                       "path", path,
                       "pid", pid,
                       "id", id,
                       NULL);
                       
  return self;
}
