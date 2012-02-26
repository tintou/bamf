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


#include "bamf-matcher.h"
#include "bamf-control.h"
#include "bamf-indicator-source.h"
#include "bamf-daemon.h"

G_DEFINE_TYPE (BamfControl, bamf_control, BAMF_DBUS_TYPE_CONTROL_SKELETON);
#define BAMF_CONTROL_GET_PRIVATE(obj) (G_TYPE_INSTANCE_GET_PRIVATE(obj, \
BAMF_TYPE_CONTROL, BamfControlPrivate))

struct _BamfControlPrivate
{
  GList *sources;
};

static void 
bamf_control_on_launched_callback (GDBusConnection *connection,
                                   const gchar *sender_name,
                                   const gchar *object_path,
                                   const gchar *interface_name,
                                   const gchar *signal_name,
                                   GVariant *parameters,
                                   gpointer user_data)
{
  const gchar *desktop_file;
  gint64 pid;

  g_variant_get_child (parameters, 0, "^&ay", &desktop_file);
  g_variant_get_child (parameters, 2, "x", &pid);

  bamf_matcher_register_desktop_file_for_pid (bamf_matcher_get_default (),
                                              desktop_file, pid);
}

static void
bamf_control_constructed (GObject *object)
{
  GDBusConnection *gbus;

  if (G_OBJECT_CLASS (bamf_control_parent_class)->constructed)
    G_OBJECT_CLASS (bamf_control_parent_class)->constructed (object);

  gbus = g_bus_get_sync (G_BUS_TYPE_SESSION, NULL, NULL);
  g_dbus_connection_signal_subscribe  (gbus,
                                       NULL,
                                       "org.gtk.gio.DesktopAppInfo",
                                       "Launched",
                                       "/org/gtk/gio/DesktopAppInfo",
                                       NULL,
                                       0,
                                       bamf_control_on_launched_callback,
                                       BAMF_CONTROL (object),
                                       NULL);
}

static gboolean
on_dbus_handle_quit (BamfDBusControl *interface,
                     GDBusMethodInvocation *invocation,
                     BamfControl *self)
{
  bamf_control_quit (self);
  g_dbus_method_invocation_return_value (invocation, NULL);

  return TRUE;
}

static gboolean
on_dbus_handle_set_approver_behavior (BamfDBusControl *interface,
                                      GDBusMethodInvocation *invocation,
                                      gint behavior,
                                      BamfControl *self)
{
  bamf_control_set_approver_behavior (self, behavior);
  g_dbus_method_invocation_return_value (invocation, NULL);

  return TRUE;
}

static gboolean
on_dbus_handle_om_nom_nom_desktop_file (BamfDBusControl *interface,
                                        GDBusMethodInvocation *invocation,
                                        const gchar *desktop_file,
                                        BamfControl *self)
{
  bamf_control_insert_desktop_file (self, desktop_file);
  g_dbus_method_invocation_return_value (invocation, NULL);

  return TRUE;
}

static gboolean
on_dbus_handle_register_application_for_pid (BamfDBusControl *interface,
                                             GDBusMethodInvocation *invocation,
                                             const gchar *application,
                                             guint pid,
                                             BamfControl *self)
{
  bamf_control_register_application_for_pid (self, application, pid);
  g_dbus_method_invocation_return_value (invocation, NULL);

  return TRUE;
}

static void
bamf_control_init (BamfControl * self)
{
  self->priv = BAMF_CONTROL_GET_PRIVATE (self);
  self->priv->sources = NULL;

  /* Registering signal callbacks to reply to dbus method calls */
  g_signal_connect (self, "handle-quit",
                    G_CALLBACK (on_dbus_handle_quit), self);

  g_signal_connect (self, "handle-set-approver-behavior",
                    G_CALLBACK (on_dbus_handle_set_approver_behavior), self);

  g_signal_connect (self, "handle-om-nom-nom-desktop-file",
                    G_CALLBACK (on_dbus_handle_om_nom_nom_desktop_file), self);

  g_signal_connect (self, "handle-register-application-for-pid",
                    G_CALLBACK (on_dbus_handle_register_application_for_pid), self);
}

static void
bamf_control_finalize (GObject *object)
{
  BamfControl *self = BAMF_CONTROL (object);
  g_list_free_full (self->priv->sources, g_object_unref);
  self->priv->sources = NULL;
}

static void
bamf_control_class_init (BamfControlClass * klass)
{
  GObjectClass *obj_class = G_OBJECT_CLASS (klass);

  obj_class->constructed = bamf_control_constructed;
  obj_class->finalize = bamf_control_finalize;

  g_type_class_add_private (klass, sizeof (BamfControlPrivate));
}

void
bamf_control_set_approver_behavior (BamfControl *control,
                                    gint32 behavior)
{
  BamfIndicatorSource *indicator_source = bamf_indicator_source_get_default ();
  bamf_indicator_source_set_behavior (indicator_source, behavior);
}

void
bamf_control_register_application_for_pid (BamfControl *control,
                                           const char *application,
                                           gint32 pid)
{
  BamfMatcher *matcher = bamf_matcher_get_default ();
  bamf_matcher_register_desktop_file_for_pid (matcher, application, pid);
}

void
bamf_control_insert_desktop_file (BamfControl *control,
                                  const char *path)
{
  BamfMatcher *matcher = bamf_matcher_get_default ();
  bamf_matcher_load_desktop_file (matcher, path);
}

static gboolean
bamf_control_on_quit (BamfControl *control)
{
  BamfDaemon *daemon = bamf_daemon_get_default ();
  bamf_daemon_stop (daemon);
  return FALSE;
}

void
bamf_control_quit (BamfControl *control)
{
  g_idle_add ((GSourceFunc) bamf_control_on_quit, control);
}

BamfControl *
bamf_control_get_default (void)
{
  static BamfControl *control;

  if (!BAMF_IS_CONTROL (control))
    {
      control = (BamfControl *) g_object_new (BAMF_TYPE_CONTROL, NULL);
    }

  return control;
}
