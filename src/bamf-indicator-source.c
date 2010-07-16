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

static void
on_indicator_closed (BamfView *view, BamfIndicatorSource *self)
{
  BamfIndicator *indicator;
 
  g_return_if_fail (BAMF_IS_INDICATOR (view));
  g_return_if_fail (BAMF_IS_INDICATOR_SOURCE (self));
  
  indicator = BAMF_INDICATOR (view);
  
  self->priv->indicators = g_list_remove (self->priv->indicators, indicator);
  g_signal_emit (self, indicator_source_signals[INDICATOR_CLOSED], 0, indicator);
  
  g_object_unref (G_OBJECT (indicator));
}

gboolean 
bamf_indicator_source_approve_item (BamfIndicatorSource *self,
                                    const char *id,
                                    const char *category,
                                    guint32 pid,
                                    const char *address,
                                    const char *path,
                                    gboolean *approve,
                                    GError **remote_error)
{
  DBusGConnection *bus;
  DBusGProxy *dbus_proxy;
  GList *l;
  GError *error = NULL;
  BamfIndicator *indicator = NULL;
  *approve = TRUE;
  guint remote_pid;
  
  g_return_val_if_fail (category, TRUE);
  g_return_val_if_fail (path, TRUE);
  g_return_val_if_fail (address, TRUE);
  g_return_val_if_fail (BAMF_IS_INDICATOR_SOURCE (self), TRUE);
  
  if (g_strcmp0 (category, "ApplicationStatus") != 0)
    return TRUE;
    
  if (pid == 0)
    {
      g_warning ("Remote service passed indicator with pid of 0, manually fetching pid\n");
      
      bus = dbus_g_bus_get (DBUS_BUS_SESSION, &error);
      g_return_val_if_fail (bus, TRUE);
 
      dbus_proxy = dbus_g_proxy_new_for_name (bus,
                                              DBUS_SERVICE_DBUS,
                                              DBUS_PATH_DBUS,
                                              DBUS_INTERFACE_DBUS);
      g_return_val_if_fail (dbus_proxy, TRUE);
   
      error = NULL;
      if (!org_freedesktop_DBus_get_connection_unix_process_id (dbus_proxy, address, &remote_pid, &error))
        {
          g_warning ("Could not get indicator process id: %s\n", error->message);
          g_error_free (error);
        }
     
      g_object_unref (G_OBJECT (dbus_proxy));
      pid = (guint32) remote_pid;
    }
  
  for (l = self->priv->indicators; l; l = l->next)
    {
      if (bamf_indicator_matches_signature (BAMF_INDICATOR (l->data), pid, address, path))
        {
          g_debug ("NotifierWatcher service passed already known indicator, ignoring\n");
          indicator = BAMF_INDICATOR (l->data);
          break;
        }
    }
  
  if (!indicator)
    {
      indicator = bamf_indicator_new (id, path, address,  pid);
      self->priv->indicators = g_list_prepend (self->priv->indicators, indicator);
  
      g_signal_connect (G_OBJECT (indicator), "closed", (GCallback) on_indicator_closed, self);
      g_signal_emit (self, indicator_source_signals[INDICATOR_OPENED], 0, indicator);
    }
  
  if (indicator && g_list_length (bamf_view_get_parents (BAMF_VIEW (indicator))) > 0)
    *approve = FALSE;
  
  return TRUE;
}

static void on_proxy_destroy (DBusGProxy *proxy, BamfIndicatorSource *self);

static gboolean
bamf_indicator_source_register_notification_approver (BamfIndicatorSource *self)
{
  DBusGConnection *bus;
  GError *error = NULL;
  static int retry_count = 0;

  bus = dbus_g_bus_get (DBUS_BUS_SESSION, &error);
  g_return_val_if_fail (bus, FALSE);

  self->priv->proxy = dbus_g_proxy_new_for_name_owner (bus,
                                                       "org.ayatana.indicator.application",
                                                       "/StatusNotifierWatcher",
                                                       "org.kde.StatusNotifierWatcher",
                                                       &error);
  
  if (self->priv->proxy)
    {
      if (dbus_g_proxy_call (self->priv->proxy,
                             "RegisterNotificationApprover",
                             &error,
                             DBUS_TYPE_G_OBJECT_PATH, BAMF_INDICATOR_SOURCE_PATH,
                             G_TYPE_STRV, NULL,
                             G_TYPE_INVALID,
                             G_TYPE_INVALID))
        {
          g_debug ("Remote notification approver registered\n");
          g_signal_connect (G_OBJECT (self->priv->proxy), "destroy", (GCallback) on_proxy_destroy, self);
          retry_count = 0;
          return FALSE;
        }
      else
        {
          g_warning ("Failed to register as approver: %s\n", error->message);
          g_error_free (error);
        }
    }
  else
    {
      g_warning ("Failed to get notification approver proxy: %s\n", error->message);
      g_error_free (error);
    }
  retry_count++;
      
  if (retry_count > 10)
    {
      g_warning ("Retry count expired\n");
      return FALSE;
    }
  
  g_debug ("Retrying registration in 2 seconds\n");
  return TRUE;
}

static void
on_proxy_destroy (DBusGProxy *proxy, BamfIndicatorSource *self)
{
  g_return_if_fail (BAMF_IS_INDICATOR_SOURCE (self));
 
  g_debug ("Remote approver service died, re-establishing connection\n");
  
  if (bamf_indicator_source_register_notification_approver (self))
    g_timeout_add (2 * 1000, (GSourceFunc) bamf_indicator_source_register_notification_approver, self);
}

static void
bamf_indicator_source_constructed (GObject *object)
{
  BamfIndicatorSource *self;
  DBusGConnection *bus;
  GError *error = NULL;

  bus = dbus_g_bus_get (DBUS_BUS_SESSION, &error);
  g_return_if_fail (bus);

  self = BAMF_INDICATOR_SOURCE (object);

  if (G_OBJECT_CLASS (bamf_indicator_source_parent_class)->constructed)
    G_OBJECT_CLASS (bamf_indicator_source_parent_class)->constructed (object);

  
  dbus_g_connection_register_g_object (bus, BAMF_INDICATOR_SOURCE_PATH, object);
  
  if (bamf_indicator_source_register_notification_approver (self))
    g_timeout_add (10 * 1000, (GSourceFunc) bamf_indicator_source_register_notification_approver, self);
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
