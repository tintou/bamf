/*
 * bamf-tab.c
 * This file is part of BAMF
 *
 * Copyright (C) 2012 Canonical LTD
 * Authors: Robert Carr <racarr@canonical.com>
 *
 * BAMF is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * BAMF is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with BAMF; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin St, Fifth Floor, 
 * Boston, MA  02110-1301  USA
 */

#include "bamf-tab.h"
#include "bamf-marshal.h"
#include "bamf-view-private.h"

#include <dbus/dbus.h>
#include <dbus/dbus-glib.h>
#include <dbus/dbus-glib-lowlevel.h>


#define BAMF_TAB_GET_PRIVATE(object)(G_TYPE_INSTANCE_GET_PRIVATE((object), BAMF_TYPE_TAB, BamfTabPrivate))

enum
{
  PROP_0,
  PROP_LOCATION,
  PROP_DESKTOP_ID,
  PROP_XID,
  PROP_IS_FOREGROUND_TAB
};
    

enum
{
  FILL,
  LAST_SIGNAL,
};

//static guint tab_signals[LAST_SIGNAL] = { 0 };

struct _BamfTabPrivate
{
  DBusGConnection *connection;
  DBusGProxy *tab_proxy;
  DBusGProxy *properties_proxy;
  
  gchar *location;
  gchar *desktop_name;
  guint64 xid;
  gboolean is_foreground;
};

G_DEFINE_TYPE (BamfTab, bamf_tab, BAMF_TYPE_VIEW)

static void
bamf_tab_got_properties (DBusGProxy *proxy,
			 DBusGProxyCall *call_id,
			 void *user_data)
{
  BamfTab *self;
  GHashTable *properties;
  GError *error;
  GHashTableIter iter;
  gpointer key, value;

  
  self = BAMF_TAB (user_data);
  
  error = NULL;
  
  dbus_g_proxy_end_call (proxy, call_id, &error,
			 dbus_g_type_get_map ("GHashTable", G_TYPE_STRING, G_TYPE_VALUE), &properties,
			 G_TYPE_INVALID);

  if (error != NULL)
    {
      g_critical ("Failed to fetch BamfTab properties: %s", error->message);
      g_error_free (error);
      
      return;
    }
  
  if (properties == NULL)
    {
      return;
    }
  
  g_hash_table_iter_init (&iter, properties);
  
  while (g_hash_table_iter_next (&iter, &key, &value))
    {
      g_object_set_property (G_OBJECT (self), (const gchar *)key, (GValue *)value);
    }

}


static void
bamf_tab_fetch_properties (BamfTab *self)
{
  dbus_g_proxy_begin_call (self->priv->properties_proxy,
			   "GetAll", 
			   bamf_tab_got_properties,
			   g_object_ref (G_OBJECT (self)),
			   (GDestroyNotify)g_object_unref,
			   G_TYPE_STRING, "org.ayatana.bamf.tab",
			   G_TYPE_INVALID);
  
}

static void
bamf_tab_on_properties_changed (DBusGProxy *proxy,
				const gchar *interface_name,
				GHashTable *changed_properties,
				const gchar **invalidated_properties,
				gpointer user_data)
{
  BamfTab *self;
  GHashTableIter iter;
  gpointer key, value;
  guint i, len;

  if (g_strcmp0 (interface_name, "org.ayatana.bamf.tab") != 0)
    {
      return;
    }
  
  self = (BamfTab *)user_data;
  
  g_hash_table_iter_init (&iter, changed_properties);
  
  while (g_hash_table_iter_next (&iter, &key, &value))
    {
      g_object_set_property (G_OBJECT (self), (const gchar *)key, (GValue *)value);
    }
  
  if (invalidated_properties == NULL)
    {
      return;
    }
  
  len = g_strv_length ((gchar **)invalidated_properties);
  for (i = 0; i < len; i++)
    {
      g_critical("Invalidated prop: %s\n", invalidated_properties[i]);
    }
}

static void
bamf_tab_set_path (BamfView *view, const gchar *path)
{
  BamfTab *self;
  
  self = BAMF_TAB (view);
  
  self->priv->tab_proxy = dbus_g_proxy_new_for_name (self->priv->connection,
						 "org.ayatana.bamf",
						 path,
						 "org.ayatana.bamf.tab");
  
  if (self->priv->tab_proxy == NULL)
    {
      g_warning ("Unable to get org.ayatana.bamf.tab proxy: %s", path);
      return;
    }
  
  self->priv->properties_proxy = dbus_g_proxy_new_for_name (self->priv->connection,
							    "org.ayatana.bamf",
							    path,
							    "org.freedesktop.DBus.Properties");

  if (self->priv->properties_proxy == NULL)
    {
      g_warning ("Unable to get org.freedesktop.DBus.properties proxy on tab object: %s", path);
      return;
    }
  
  bamf_tab_fetch_properties (self);
  
  dbus_g_object_register_marshaller ((GClosureMarshal) bamf_marshal_VOID__STRING_BOXED_POINTER,
				     G_TYPE_NONE,
				     G_TYPE_STRING,
				     G_TYPE_BOXED,
				     G_TYPE_STRV,
				     G_TYPE_INVALID);
  
  dbus_g_proxy_add_signal (self->priv->properties_proxy,
			   "PropertiesChanged",
			   G_TYPE_STRING,
			   dbus_g_type_get_map ("GHashTable", G_TYPE_STRING, G_TYPE_VALUE),
			   G_TYPE_STRV,
			   G_TYPE_INVALID);
  dbus_g_proxy_connect_signal (self->priv->properties_proxy,
			       "PropertiesChanged",
			       (GCallback) bamf_tab_on_properties_changed,
			       self,
			       NULL);
 }

static void
bamf_tab_set_property (GObject *object, guint property_id, const GValue *value, GParamSpec *pspec)
{
  BamfTab *self;
  
  self = BAMF_TAB (object);
  
  switch (property_id)
    {
    case PROP_LOCATION:
      self->priv->location = g_value_dup_string (value);
      break;
    case PROP_DESKTOP_ID:
      g_return_if_fail (self->priv->desktop_name == NULL);
      self->priv->desktop_name = g_value_dup_string (value);
      break;
    case PROP_XID:
      self->priv->xid = g_value_get_uint64 (value);
      break;
    case PROP_IS_FOREGROUND_TAB:
      self->priv->is_foreground = g_value_get_boolean (value);
      break;
    default:
      G_OBJECT_WARN_INVALID_PROPERTY_ID (object, property_id, pspec);
    }
}

static void
bamf_tab_get_property (GObject *object, guint property_id, GValue *value, GParamSpec *pspec)
{
  BamfTab *self;
  
  self = BAMF_TAB (object);
  
  switch (property_id)
    {
    case PROP_LOCATION:
      g_value_set_string (value, self->priv->location);
      break;
    case PROP_DESKTOP_ID:
      g_value_set_string (value, self->priv->desktop_name);
      break;
    case PROP_XID:
      g_value_set_uint64 (value, self->priv->xid);
      break;
    case PROP_IS_FOREGROUND_TAB:
      g_value_set_boolean (value, self->priv->is_foreground);
      break;
    default:
      G_OBJECT_WARN_INVALID_PROPERTY_ID (object, property_id, pspec);
    }
}

static void
bamf_tab_dispose (GObject *object)
{
  BamfTab *self;
  
  self = BAMF_TAB (object);
  
  if (self->priv->location != NULL)
    {
      g_free (self->priv->location);
    }
  if (self->priv->desktop_name != NULL)
    {
      g_free (self->priv->desktop_name);
    }
  if (self->priv->tab_proxy != NULL)
    {
      g_object_unref (G_OBJECT (self->priv->tab_proxy));
    }
  if (self->priv->properties_proxy != NULL)
    {
      dbus_g_proxy_disconnect_signal (self->priv->properties_proxy,
				      "PropertiesChanged",
				      (GCallback) bamf_tab_on_properties_changed,
				      self);
      g_object_unref (G_OBJECT (self->priv->properties_proxy));
    }
  if (G_OBJECT_CLASS (bamf_tab_parent_class)->dispose)
    G_OBJECT_CLASS (bamf_tab_parent_class)->dispose (object);
}


static void
bamf_tab_class_init (BamfTabClass *klass)
{
  GParamSpec *pspec;
  GObjectClass *obj_class = G_OBJECT_CLASS (klass);
  BamfViewClass *view_class = BAMF_VIEW_CLASS (klass);
  
  obj_class->dispose = bamf_tab_dispose;
  obj_class->get_property = bamf_tab_get_property;
  obj_class->set_property = bamf_tab_set_property;
  
  view_class->set_path = bamf_tab_set_path;

  pspec = g_param_spec_string("location", "Location", "The Current location of the remote Tab",
			      NULL, G_PARAM_READWRITE);
  g_object_class_install_property (obj_class, PROP_LOCATION, pspec);
  
  pspec = g_param_spec_string("desktop-id", "Desktop Name", "The Desktop ID assosciated with the application hosted in the remote Tab",
			      NULL, G_PARAM_READWRITE);
  g_object_class_install_property (obj_class, PROP_DESKTOP_ID, pspec);
  
  pspec = g_param_spec_uint64("xid", "xid", "XID for the toplevel window containing the remote Tab",
			      0, G_MAXUINT64, 0, G_PARAM_READWRITE);
  g_object_class_install_property (obj_class, PROP_XID, pspec);
  
  pspec = g_param_spec_boolean("is-foreground-tab", "Foreground tab", "Whether the tab is the foreground tab in it's toplevel container",
			       FALSE, G_PARAM_READWRITE);
  g_object_class_install_property (obj_class, PROP_IS_FOREGROUND_TAB, pspec);
  
  g_type_class_add_private (obj_class, sizeof(BamfTabPrivate));

}

static void
bamf_tab_init (BamfTab *self)
{
  GError *error = NULL;
  
  self->priv = BAMF_TAB_GET_PRIVATE (self);

  self->priv->tab_proxy = NULL;
  self->priv->properties_proxy = NULL;
  
  self->priv->location = NULL;
  self->priv->desktop_name = NULL;
  self->priv->xid = 0;

  
  self->priv->connection = dbus_g_bus_get (DBUS_BUS_SESSION, &error);
  
  if (error != NULL)
    {
      g_warning ("Failed to open connection to bus: %s", error->message);
      g_error_free (error);
      
      return;
    }
}

BamfTab *
bamf_tab_new (const gchar *path)
{
  BamfTab *self;
  
  self = g_object_new (BAMF_TYPE_TAB, NULL);
  
  bamf_view_set_path (BAMF_VIEW (self), path);
  
  return self;
}

gboolean
bamf_tab_raise (BamfTab *self)
{
  GError *error;

  g_return_val_if_fail (BAMF_IS_TAB (self), FALSE);
  
  if (!bamf_view_remote_ready (BAMF_VIEW (self)))
    return FALSE;
  
  error = NULL;
  
  if (!dbus_g_proxy_call (self->priv->tab_proxy,
			  "Raise",
			  &error,
			  G_TYPE_INVALID))
    {
      g_warning ("Failed to invoke Raise method: %s", error->message);
      g_error_free (error);
      
      return FALSE;
    }
  
  return TRUE;
}

gboolean
bamf_tab_close (BamfTab *self)
{
  GError *error;

  g_return_val_if_fail (BAMF_IS_TAB (self), FALSE);
  
  if (!bamf_view_remote_ready (BAMF_VIEW (self)))
    return FALSE;
  
  error = NULL;
  
  if (!dbus_g_proxy_call (self->priv->tab_proxy,
			  "Close",
			  &error,
			  G_TYPE_INVALID))
    {
      g_warning ("Failed to invoke Close method: %s", error->message);
      g_error_free (error);
      
      return FALSE;
    }
  
  return TRUE;
}

typedef struct _bamf_tab_preview_request_user_data {
  BamfTab *self;
  BamfTabPreviewReadyCallback callback;
  gpointer user_data;
} bamf_tab_preview_request_user_data;

static void
bamf_tab_on_preview_ready (DBusGProxy *proxy,
			   DBusGProxyCall *call_id,
			   gpointer user_data)
{
  BamfTab *self;
  bamf_tab_preview_request_user_data *data;
  gchar *preview_data = NULL;
  GError *error;
  
  data = (bamf_tab_preview_request_user_data *)user_data;
  self = data->self;
  
  error = NULL;
  
  dbus_g_proxy_end_call (proxy, call_id, &error, G_TYPE_STRING,  &preview_data, G_TYPE_INVALID);
  
  if (error != NULL)
    {
      g_warning ("Error requesting BamfTab preview: %s", error->message);
      g_error_free (error);
      
      return;
    }
  
  data->callback (self, preview_data, data->user_data);
  
  g_free (preview_data);  
}

void
bamf_tab_request_preview (BamfTab *self, BamfTabPreviewReadyCallback callback, gpointer user_data)
{
  bamf_tab_preview_request_user_data *data;

  g_return_if_fail (BAMF_IS_TAB (self));
  
  data = g_malloc0 (sizeof (bamf_tab_preview_request_user_data));
  data->self = self;
  data->callback = callback;
  data->user_data = user_data;
  
  dbus_g_proxy_begin_call (self->priv->tab_proxy,
			   "RequestPreview",
			   bamf_tab_on_preview_ready,
			   data,
			   (GDestroyNotify)g_free,
			   G_TYPE_INVALID);  
  
 }
 
 
const gchar *
bamf_tab_get_location (BamfTab *self)
{
  g_return_val_if_fail (BAMF_IS_TAB (self), NULL);
  
  return self->priv->location;
}

const gchar *
bamf_tab_get_desktop_name (BamfTab *self)
{
  g_return_val_if_fail (BAMF_IS_TAB (self), NULL);
  
  return self->priv->desktop_name;
}

guint64
bamf_tab_get_xid (BamfTab *self)
{
  g_return_val_if_fail (BAMF_IS_TAB (self), 0);
  
  return self->priv->xid;
}

gboolean
bamf_tab_get_is_foreground_tab (BamfTab *self)
{
  g_return_val_if_fail (BAMF_IS_TAB (self), 0);
  
  return self->priv->is_foreground;
}
