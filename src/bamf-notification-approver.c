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


#include "bamf-notification-approver.h"
#include "bamf-notification-approver-glue.h"

#define BAMF_NOTIFICATION_APPROVER_GET_PRIVATE(object)(G_TYPE_INSTANCE_GET_PRIVATE((object), BAMF_TYPE_NOTIFICATION_APPROVER, BamfNotificationApproverPrivate))
#define BAMF_NOTIFICATION_APPROVER_PATH "/org/ayatana/bamf/NotificationApprover"

enum
{
  INDICATOR_OPENED,
  INDICATOR_CLOSED,

  LAST_SIGNAL,
};

static guint notification_approver_signals[LAST_SIGNAL] = { 0 };

struct _BamfNotificationApproverPrivate
{
  GList *indicators;
};

G_DEFINE_TYPE (BamfNotificationApprover, bamf_notification_approver, G_TYPE_OBJECT)

GList *
bamf_notification_approver_get_indicators (BamfNotificationApprover *self)
{
  g_return_val_if_fail (BAMF_IS_NOTIFICATION_APPROVER (self), NULL);
  
  return self->priv->indicators;
}

gboolean 
bamf_notification_approver_approve_item (BamfNotificationApprover *self,
                                         const char *id,
                                         const char *category,
                                         const guint32 pid,
                                         const char *address,
                                         DBusGProxy *proxy,
                                         gboolean *approve,
                                         GError **error)
{
  BamfIndicator *indicator;
  
  *approve = TRUE;
  g_return_val_if_fail (BAMF_IS_NOTIFICATION_APPROVER (self), TRUE);

  indicator = bamf_indicator_new (id, proxy, address, pid);
  self->priv->indicators = g_list_prepend (self->priv->indicators, indicator);
  
  g_signal_emit (self, notification_approver_signals[INDICATOR_OPENED], 0, indicator);

  if (g_list_length (bamf_view_get_parents (BAMF_VIEW (indicator))) > 0)
    *approve = FALSE;

  return TRUE;
}

static void
bamf_notification_approver_constructed (GObject *object)
{
  DBusGConnection *bus;
  GError *error = NULL;

  if (G_OBJECT_CLASS (bamf_notification_approver_parent_class)->constructed)
    G_OBJECT_CLASS (bamf_notification_approver_parent_class)->constructed (object);
  
  bus = dbus_g_bus_get (DBUS_BUS_SESSION, &error);
  g_return_if_fail (bus);

  dbus_g_connection_register_g_object (bus, BAMF_NOTIFICATION_APPROVER_PATH, object);
}

static void
bamf_notification_approver_dispose (GObject *object)
{
  G_OBJECT_CLASS (bamf_notification_approver_parent_class)->dispose (object);
}

static void
bamf_notification_approver_class_init (BamfNotificationApproverClass *klass)
{
  GObjectClass *object_class = G_OBJECT_CLASS (klass);
	
  object_class->dispose = bamf_notification_approver_dispose;
  object_class->constructed = bamf_notification_approver_constructed;

  notification_approver_signals [INDICATOR_OPENED] =
  	g_signal_new ("indicator-opened",
  	              G_OBJECT_CLASS_TYPE (klass),
  	              G_SIGNAL_RUN_FIRST,
  	              G_STRUCT_OFFSET (BamfNotificationApproverClass, indicator_opened),
  	              NULL, NULL,
  	              g_cclosure_marshal_VOID__OBJECT,
  	              G_TYPE_NONE, 1,
  	              BAMF_TYPE_INDICATOR);

  notification_approver_signals [INDICATOR_CLOSED] =
  	g_signal_new ("indicator-closed",
  	              G_OBJECT_CLASS_TYPE (klass),
  	              G_SIGNAL_RUN_FIRST,
  	              G_STRUCT_OFFSET (BamfNotificationApproverClass, indicator_closed),
  	              NULL, NULL,
  	              g_cclosure_marshal_VOID__OBJECT,
  	              G_TYPE_NONE, 1,
  	              BAMF_TYPE_INDICATOR);

  dbus_g_object_type_install_info (BAMF_TYPE_NOTIFICATION_APPROVER,
				   &dbus_glib_bamf_notification_approver_object_info);

  g_type_class_add_private (object_class, sizeof (BamfNotificationApproverPrivate));
}

static void
bamf_notification_approver_init (BamfNotificationApprover *self)
{
  self->priv = BAMF_NOTIFICATION_APPROVER_GET_PRIVATE (self);
}

static BamfNotificationApprover *approver = NULL;

BamfNotificationApprover *
bamf_notification_approver_get_default ()
{
  if (!approver)
    return approver = g_object_new (BAMF_TYPE_NOTIFICATION_APPROVER, NULL);
  return BAMF_NOTIFICATION_APPROVER (g_object_ref (approver));
}
