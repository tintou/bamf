/*
 * bamf-notification-approver.c
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
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with BAMF; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin St, Fifth Floor, 
 * Boston, MA  02110-1301  USA
 */


#include "bamf-marshal.h"
#include "bamf-indicator-source.h"
#include "bamf-indicator-source-glue.h"
#include <dbus/dbus-glib-bindings.h>

#define BAMF_INDICATOR_SOURCE_GET_PRIVATE(object)(G_TYPE_INSTANCE_GET_PRIVATE((object), BAMF_TYPE_INDICATOR_SOURCE, BamfIndicatorSourcePrivate))
#define BAMF_INDICATOR_SOURCE_PATH "/org/ayatana/bamf/IndicatorSource"

#define DBUS_G_STRUCT_IND (dbus_g_type_get_struct("GValueArray", G_TYPE_STRING, G_TYPE_INT, G_TYPE_STRING, DBUS_TYPE_G_PROXY, G_TYPE_STRING, G_TYPE_INVALID))
#define DBUS_G_COLLECT_IND (dbus_g_type_get_collection ("GPtrArray", DBUS_G_STRUCT_IND))

enum
{
  INDICATOR_OPENED,
  INDICATOR_CLOSED,

  LAST_SIGNAL,
};

static guint indicator_source_signals[LAST_SIGNAL] = { 0 };

struct _BamfIndicatorSourcePrivate
{
  GList *indicators;
  DBusGProxy *proxy;
};

G_DEFINE_TYPE (BamfIndicatorSource, bamf_indicator_source, G_TYPE_OBJECT)

GList *
bamf_indicator_source_get_indicators (BamfIndicatorSource *self)
{
  g_return_val_if_fail (BAMF_IS_INDICATOR_SOURCE (self), NULL);
  
  return self->priv->indicators;
}

gboolean 
bamf_indicator_source_approve_item (BamfIndicatorSource *self,
                                         const char *id,
                                         const char *category,
                                         const guint32 pid,
                                         const char *address,
                                         DBusGProxy *proxy,
                                         gboolean *approve,
                                         GError **error)
{
  *approve = TRUE;
  return TRUE;
}

static void
on_application_added (DBusGProxy *proxy, // may be null
                      char *icon,        // may be null
                      int order,         // may be whatever
                      char *address,     // may not be null
                      char *path,        // may not be null
                      char *icon_path,   // may be null
                      BamfIndicatorSource *self)
{
  BamfIndicator *indicator;
  DBusGConnection *bus;
  DBusGProxy *dbus_proxy;
  guint pid;
  GError *error = NULL;

  g_return_if_fail (BAMF_IS_INDICATOR_SOURCE (self));

  bus = dbus_g_bus_get (DBUS_BUS_SESSION, &error);
  g_return_if_fail (bus);

  dbus_proxy = dbus_g_proxy_new_for_name (bus,
                                          DBUS_SERVICE_DBUS,
                                          DBUS_PATH_DBUS,
                                          DBUS_INTERFACE_DBUS);
  g_return_if_fail (dbus_proxy);
  
  if (!org_freedesktop_DBus_get_connection_unix_process_id (dbus_proxy, address, &pid, &error))
    {
      g_warning ("Could not get indicator process id: %s\n", error->message);
      g_error_free (error);
      goto out;
    }
  
  indicator = bamf_indicator_new (path, address, (guint32) pid);
  self->priv->indicators = g_list_prepend (self->priv->indicators, indicator);
  
  g_signal_emit (self, indicator_source_signals[INDICATOR_OPENED], 0, indicator);
  
out:
  g_object_unref (dbus_proxy);
}

static void
bamf_indicator_source_load_value (gpointer *data, gpointer *user_data)
{
  DBusGProxy *proxy;
  char *address;
  char *path;
  GValue *value;
  GValueArray *arr;
  BamfIndicatorSource *self;
  
  self = BAMF_INDICATOR_SOURCE (user_data);
  arr = (GValueArray *) data;
  
  value = g_value_array_get_nth (arr, 2);
  address = g_value_dup_string (value);
  
  value = g_value_array_get_nth (arr, 3);
  proxy = DBUS_G_PROXY (g_value_get_object (value));
  path = g_strdup (dbus_g_proxy_get_path (proxy));
  
  on_application_added (NULL, NULL, 0, address, path, NULL, self);
  
  g_free (address);
  g_free (path);
  g_value_array_free (arr);
}

static gboolean
bamf_indicator_source_load_existing (BamfIndicatorSource *self)
{
  DBusGProxy *proxy;
  GPtrArray *arr;
  GError *error = NULL;
  
  g_return_val_if_fail (BAMF_IS_INDICATOR_SOURCE (self), FALSE);
  
  proxy = self->priv->proxy;
  
  if (!dbus_g_proxy_call (proxy,
                          "GetApplications",
                          &error,
                          G_TYPE_INVALID,
                          DBUS_G_COLLECT_IND, &arr,
                          G_TYPE_INVALID))
    {
      g_warning ("Failed to fetch applications: %s", error->message);
      g_error_free (error);
      return FALSE;
    }
  
  g_ptr_array_foreach (arr, (GFunc) bamf_indicator_source_load_value, self);   
  
  return FALSE;
}

static void
bamf_indicator_source_constructed (GObject *object)
{
  BamfIndicatorSource *self;
  DBusGConnection *bus;
  GError *error = NULL;

  self = BAMF_INDICATOR_SOURCE (object);

  if (G_OBJECT_CLASS (bamf_indicator_source_parent_class)->constructed)
    G_OBJECT_CLASS (bamf_indicator_source_parent_class)->constructed (object);

  bus = dbus_g_bus_get (DBUS_BUS_SESSION, &error);
  g_return_if_fail (bus);

  self->priv->proxy = dbus_g_proxy_new_for_name (bus,
                                                 "org.ayatana.indicator.application",
                                                 "/org/ayatana/indicator/application/service",
                                                 "org.ayatana.indicator.application.service");
  g_return_if_fail (self->priv->proxy);
  
  dbus_g_object_register_marshaller ((GClosureMarshal) bamf_marshal_VOID__STRING_INT_STRING_STRING_STRING,
                                     G_TYPE_NONE, 
                                     G_TYPE_STRING, G_TYPE_INT, G_TYPE_STRING, G_TYPE_STRING, G_TYPE_STRING,
                                     G_TYPE_INVALID);
  
  dbus_g_proxy_add_signal (self->priv->proxy,
                           "ApplicationAdded",
                           G_TYPE_STRING,
                           G_TYPE_INT,
                           G_TYPE_STRING,
                           G_TYPE_STRING,
                           G_TYPE_STRING,
                           G_TYPE_INVALID);
  
  dbus_g_proxy_connect_signal (self->priv->proxy,
                               "ApplicationAdded",
                               (GCallback) on_application_added,
                               self,
                               NULL);
  
  dbus_g_connection_register_g_object (bus, BAMF_INDICATOR_SOURCE_PATH, object);
  
  g_idle_add ((GSourceFunc) bamf_indicator_source_load_existing, self);
}

static void
bamf_indicator_source_dispose (GObject *object)
{
  G_OBJECT_CLASS (bamf_indicator_source_parent_class)->dispose (object);
}

static void
bamf_indicator_source_class_init (BamfIndicatorSourceClass *klass)
{
  GObjectClass *object_class = G_OBJECT_CLASS (klass);
	
  object_class->dispose = bamf_indicator_source_dispose;
  object_class->constructed = bamf_indicator_source_constructed;

  indicator_source_signals [INDICATOR_OPENED] =
  	g_signal_new ("indicator-opened",
  	              G_OBJECT_CLASS_TYPE (klass),
  	              G_SIGNAL_RUN_FIRST,
  	              G_STRUCT_OFFSET (BamfIndicatorSourceClass, indicator_opened),
  	              NULL, NULL,
  	              g_cclosure_marshal_VOID__OBJECT,
  	              G_TYPE_NONE, 1,
  	              BAMF_TYPE_INDICATOR);

  indicator_source_signals [INDICATOR_CLOSED] =
  	g_signal_new ("indicator-closed",
  	              G_OBJECT_CLASS_TYPE (klass),
  	              G_SIGNAL_RUN_FIRST,
  	              G_STRUCT_OFFSET (BamfIndicatorSourceClass, indicator_closed),
  	              NULL, NULL,
  	              g_cclosure_marshal_VOID__OBJECT,
  	              G_TYPE_NONE, 1,
  	              BAMF_TYPE_INDICATOR);

  dbus_g_object_type_install_info (BAMF_TYPE_INDICATOR_SOURCE,
				   &dbus_glib_bamf_indicator_source_object_info);

  g_type_class_add_private (object_class, sizeof (BamfIndicatorSourcePrivate));
}

static void
bamf_indicator_source_init (BamfIndicatorSource *self)
{
  self->priv = BAMF_INDICATOR_SOURCE_GET_PRIVATE (self);
}

static BamfIndicatorSource *approver = NULL;

BamfIndicatorSource *
bamf_indicator_source_get_default ()
{
  if (!approver)
    return approver = g_object_new (BAMF_TYPE_INDICATOR_SOURCE, NULL);
  return BAMF_INDICATOR_SOURCE (g_object_ref (approver));
}
