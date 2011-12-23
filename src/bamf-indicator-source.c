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

#include "bamf-indicator-source.h"

#define BAMF_INDICATOR_SOURCE_GET_PRIVATE(object)(G_TYPE_INSTANCE_GET_PRIVATE((object), \
                        BAMF_TYPE_INDICATOR_SOURCE, BamfIndicatorSourcePrivate))

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
  GDBusProxy *proxy;
  int behavior;
};

G_DEFINE_TYPE (BamfIndicatorSource, bamf_indicator_source, STATUS_NOTIFIER_APPROVER_TYPE__SKELETON)

const GList *
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

void
rejudge_approval (BamfIndicator *indicator, BamfIndicatorSource *self)
{
  g_return_if_fail (BAMF_IS_INDICATOR (indicator));
  g_return_if_fail (BAMF_IS_INDICATOR_SOURCE (self));

  const char *address = bamf_indicator_get_address (indicator);
  const char *path = bamf_indicator_get_path (indicator);
  const GList *parents = bamf_view_get_parents (BAMF_VIEW (indicator));
  gboolean approve = TRUE;

  switch (self->priv->behavior)
    {
      case BAMF_INDICATOR_SOURCE_APPROVE_NONE:
        approve = FALSE;
        break;
      case BAMF_INDICATOR_SOURCE_APPROVE_MATCHED:
        approve = (parents == NULL);
        break;
      case BAMF_INDICATOR_SOURCE_APPROVE_ALL:
        approve = TRUE;
        break;
    }

  g_signal_emit_by_name (self, "revise-judgement", approve, address, path);
}

void 
bamf_indicator_source_set_behavior (BamfIndicatorSource *self,
                                    gint32 behavior)
{
  BamfIndicatorSourcePrivate *priv;

  g_return_if_fail (BAMF_IS_INDICATOR_SOURCE (self));
  
  priv = self->priv;
  
  if (priv->behavior == behavior)
    return;

  priv->behavior = behavior;

  g_list_foreach (priv->indicators, (GFunc) rejudge_approval, self);
}

gboolean 
bamf_indicator_source_approve_item (BamfIndicatorSource *self,
                                    const char *id,
                                    const char *category,
                                    guint32 pid,
                                    const char *address,
                                    const char *path)
{
  GList *l;
  GError *error = NULL;
  BamfIndicator *indicator = NULL;

  g_return_val_if_fail (category, TRUE);
  g_return_val_if_fail (path, TRUE);
  g_return_val_if_fail (address, TRUE);
  g_return_val_if_fail (BAMF_IS_INDICATOR_SOURCE (self), TRUE);

  if (g_strcmp0 (category, "ApplicationStatus") != 0 || 
      g_strcmp0 (path, "/com/canonical/NotificationItem/dropbox_client") == 0)
    return TRUE;

  if (pid == 0)
    {
      GDBusProxy *gproxy;
      GVariant *remote_pid;

      g_warning ("Remote service passed indicator with pid of 0, manually fetching pid");

      gproxy = g_dbus_proxy_new_for_bus_sync (G_BUS_TYPE_SESSION,
                                              G_DBUS_PROXY_FLAGS_DO_NOT_LOAD_PROPERTIES|
                                              G_DBUS_PROXY_FLAGS_DO_NOT_CONNECT_SIGNALS,
                                              NULL,
                                              "org.freedesktop.DBus",
                                              "/org/freedesktop/DBus",
                                              "org.freedesktop.DBus",
                                              NULL,
                                              &error);

      if (error)
        {
          g_warning ("Could not get indicator process id: %s", error->message);
          g_error_free (error);
          error = NULL;
        }

      g_return_val_if_fail (G_IS_DBUS_PROXY (gproxy), TRUE);

      remote_pid = g_dbus_proxy_call_sync (gproxy, "GetConnectionUnixProcessID",
                                           g_variant_new ("(s)", address),
                                           G_DBUS_CALL_FLAGS_NONE,
                                           -1,
                                           NULL,
                                           &error);
      if (error)
        {
          g_warning ("Could not get indicator process id: %s", error->message);
          g_error_free (error);
          return TRUE;
        }

      g_return_val_if_fail (remote_pid, TRUE);

      pid = g_variant_get_uint32 (g_variant_get_child_value (remote_pid, 0));

      g_object_unref (gproxy);
      g_variant_unref (remote_pid);

      if (pid == 0)
        return TRUE;
    }

  for (l = self->priv->indicators; l; l = l->next)
    {
      if (bamf_indicator_matches_signature (BAMF_INDICATOR (l->data), pid, address, path))
        {
          g_debug ("NotifierWatcher service passed already known indicator, ignoring");
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

  const GList *parents = bamf_view_get_parents (BAMF_VIEW (indicator));

  switch (self->priv->behavior)
    {
      case BAMF_INDICATOR_SOURCE_APPROVE_NONE:
        return FALSE;

      case BAMF_INDICATOR_SOURCE_APPROVE_MATCHED:
        return !(indicator && parents);

      case BAMF_INDICATOR_SOURCE_APPROVE_ALL:
        return TRUE;
    }

  return TRUE;
}

static void status_notifier_proxy_owner_changed (GObject *object, GParamSpec *pspec, BamfIndicatorSource *self);

static gboolean
bamf_indicator_source_register_notification_approver (BamfIndicatorSource *self)
{
  static int retry_count = 0;
  GDBusProxy *gproxy;
  GError *error = NULL;
  gchar *owner;

  gproxy = g_dbus_proxy_new_for_bus_sync (G_BUS_TYPE_SESSION,
                                          G_DBUS_PROXY_FLAGS_DO_NOT_LOAD_PROPERTIES|
                                          G_DBUS_PROXY_FLAGS_DO_NOT_CONNECT_SIGNALS|
                                          G_DBUS_PROXY_FLAGS_DO_NOT_AUTO_START,
                                          NULL,
                                          "com.canonical.indicator.application",
                                          "/StatusNotifierWatcher",
                                          "org.kde.StatusNotifierWatcher",
                                          NULL,
                                          &error);

  if (error)
    {
       g_debug ("Failed to get notification approver proxy: %s", error->message);
       g_error_free (error);
    }
  else
    {
      owner = g_dbus_proxy_get_name_owner (gproxy);

      if (owner)
        {
          g_free (owner);

          GVariantBuilder tuple, array;
          g_variant_builder_init (&tuple, G_VARIANT_TYPE_TUPLE);
          g_variant_builder_add_value (&tuple, g_variant_new_object_path (BAMF_INDICATOR_SOURCE_PATH));
          g_variant_builder_init (&array, G_VARIANT_TYPE ("as"));
          g_variant_builder_add_value (&tuple, g_variant_builder_end (&array));

          g_dbus_proxy_call_sync (gproxy, "XAyatanaRegisterNotificationApprover",
                                           g_variant_builder_end (&tuple),
                                           G_DBUS_CALL_FLAGS_NONE,
                                           -1,
                                           NULL,
                                           &error);

          if (!error)
            {
              g_debug ("Remote notification approver registered");
              g_signal_connect (G_OBJECT (gproxy), "notify::g-name-owner",
                                (GCallback) status_notifier_proxy_owner_changed,
                                self);

              if (self->priv->proxy)
                g_object_unref (self->priv->proxy);

              self->priv->proxy = gproxy;
              retry_count = 0;
              return FALSE;
            }
          else
            {
              g_debug ("Failed to register as approver: %s", error->message);
              g_error_free (error);
            }
        }
      else
        {
           g_debug ("Failed to get notification approver proxy: no owner available");
        }

      g_object_unref (gproxy);
    }

  retry_count++;

  if (retry_count > 10)
    {
      g_debug ("Retry count expired");
      return FALSE;
    }

  g_debug ("Retrying registration in 2 seconds");
  return TRUE;
}

static void
status_notifier_proxy_owner_changed (GObject *object, GParamSpec *pspec,
                                     BamfIndicatorSource *self)
{
  g_debug ("Remote approver service died, re-establishing connection");
  g_object_unref (self->priv->proxy);
  self->priv->proxy = NULL;

  if (bamf_indicator_source_register_notification_approver (self))
    g_timeout_add_seconds (2, (GSourceFunc) bamf_indicator_source_register_notification_approver, self);
}

static gboolean
on_dbus_handle_approve_item (BamfDBusMatcher *interface,
                             GDBusMethodInvocation *invocation,
                             const gchar *id,
                             const gchar *category,
                             guint pid,
                             const gchar *address,
                             const gchar *path,
                             BamfIndicatorSource *self)
{
  gboolean approved = bamf_indicator_source_approve_item (self, id, category,
                                                          pid, address, path);
  g_dbus_method_invocation_return_value (invocation,
                                         g_variant_new ("(b)", approved));

  return TRUE;
}

static void
bamf_indicator_source_constructed (GObject *object)
{
  BamfIndicatorSource *self;

  self = BAMF_INDICATOR_SOURCE (object);

  if (G_OBJECT_CLASS (bamf_indicator_source_parent_class)->constructed)
    G_OBJECT_CLASS (bamf_indicator_source_parent_class)->constructed (object);

  self->priv->behavior = BAMF_INDICATOR_SOURCE_APPROVE_ALL;
  
  if (bamf_indicator_source_register_notification_approver (self))
    g_timeout_add_seconds (2, (GSourceFunc) bamf_indicator_source_register_notification_approver, self);
}

static void
bamf_indicator_source_dispose (GObject *object)
{
  BamfIndicatorSource *self = BAMF_INDICATOR_SOURCE (object);

  if (self->priv->indicators)
    {
      g_list_free_full (self->priv->indicators, g_object_unref);
      self->priv->indicators = NULL;
    }

  if (self->priv->proxy)
    {
      g_object_unref (self->priv->proxy);
      self->priv->proxy = NULL;
    }

  G_OBJECT_CLASS (bamf_indicator_source_parent_class)->dispose (object);
}

static void
bamf_indicator_source_class_init (BamfIndicatorSourceClass *klass)
{
  GObjectClass *object_class = G_OBJECT_CLASS (klass);
  
  object_class->dispose = bamf_indicator_source_dispose;
  object_class->constructed = bamf_indicator_source_constructed;

  g_type_class_add_private (object_class, sizeof (BamfIndicatorSourcePrivate));

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
}

static void
bamf_indicator_source_init (BamfIndicatorSource *self)
{
  self->priv = BAMF_INDICATOR_SOURCE_GET_PRIVATE (self);

  /* Registering signal callbacks to reply to dbus method calls */
  g_signal_connect (self, "handle-approve-item",
                    G_CALLBACK (on_dbus_handle_approve_item), self);
}

static BamfIndicatorSource *approver = NULL;

BamfIndicatorSource *
bamf_indicator_source_get_default ()
{
  if (!approver)
    approver = g_object_new (BAMF_TYPE_INDICATOR_SOURCE, NULL);

  return approver;
}
