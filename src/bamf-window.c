/*
 * Copyright (C) 2010 Canonical Ltd
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
 *
 */

#include "bamf-application.h"
#include "bamf-window.h"
#include "bamf-legacy-screen.h"

#include <glib.h>
#include <string.h>

#ifdef EXPORT_ACTIONS_MENU
#include <glib/gi18n.h>
#include <libdbusmenu-glib/dbusmenu-glib.h>
#include <libdbusmenu-gtk/parser.h>
#endif

#define _GTK_APPLICATION_ID "_GTK_APPLICATION_ID"
#define SNAP_SECURITY_LABEL_PREFIX "snap."

#define BAMF_WINDOW_GET_PRIVATE(obj) (G_TYPE_INSTANCE_GET_PRIVATE(obj, \
BAMF_TYPE_WINDOW, BamfWindowPrivate))

static void bamf_window_dbus_iface_init (BamfDBusItemWindowIface *iface);
G_DEFINE_TYPE_WITH_CODE (BamfWindow, bamf_window, BAMF_TYPE_VIEW,
                         G_IMPLEMENT_INTERFACE (BAMF_DBUS_ITEM_TYPE_WINDOW,
                                                bamf_window_dbus_iface_init));

static GList *bamf_windows = NULL;

enum
{
  PROP_0,

  PROP_WINDOW,
};

struct _BamfWindowPrivate
{
  BamfDBusItemWindow *dbus_iface;
  BamfLegacyWindow *legacy_window;
  BamfWindowMaximizationType maximized;
  gint monitor;

#ifdef EXPORT_ACTIONS_MENU
  DbusmenuServer *dbusmenu_server;
  GtkWidget *action_menu;
  gboolean was_active;
#endif
};

BamfLegacyWindow *
bamf_window_get_window (BamfWindow *self)
{
  g_return_val_if_fail (BAMF_IS_WINDOW (self), NULL);

  if (BAMF_WINDOW_GET_CLASS (self)->get_window)
    return BAMF_WINDOW_GET_CLASS (self)->get_window (self);

  return self->priv->legacy_window;
}

BamfWindow *
bamf_window_get_transient (BamfWindow *self)
{
  BamfLegacyWindow *legacy, *transient;
  BamfWindow *other;
  GList *l;

  g_return_val_if_fail (BAMF_IS_WINDOW (self), NULL);

  legacy = bamf_window_get_window (self);
  transient = bamf_legacy_window_get_transient (legacy);

  if (transient)
    {
      for (l = bamf_windows; l; l = l->next)
        {
          other = l->data;

          if (!BAMF_IS_WINDOW (other))
            continue;

          if (transient == bamf_window_get_window (other))
            return other;
        }
    }
  return NULL;
}

const char *
bamf_window_get_transient_path (BamfWindow *self)
{
  BamfWindow *transient;

  g_return_val_if_fail (BAMF_IS_WINDOW (self), NULL);

  transient = bamf_window_get_transient (self);

  if (transient == NULL)
    return "";

  return bamf_view_get_path (BAMF_VIEW (transient));
}

guint32
bamf_window_get_window_type (BamfWindow *window)
{
  g_return_val_if_fail (BAMF_IS_WINDOW (window), 0);

  return (guint32) bamf_legacy_window_get_window_type (window->priv->legacy_window);
}

guint32
bamf_window_get_pid (BamfWindow *window)
{
  g_return_val_if_fail (BAMF_IS_WINDOW (window), 0);

  return bamf_legacy_window_get_pid (window->priv->legacy_window);
}

guint32
bamf_window_get_xid (BamfWindow *window)
{
  g_return_val_if_fail (BAMF_IS_WINDOW (window), 0);


  if (BAMF_WINDOW_GET_CLASS (window)->get_xid)
    return BAMF_WINDOW_GET_CLASS (window)->get_xid (window);

  return (guint32) bamf_legacy_window_get_xid (window->priv->legacy_window);
}

static void
handle_window_closed (BamfLegacyWindow * window, gpointer data)
{
  BamfWindow *self;
  self = (BamfWindow *) data;

  g_return_if_fail (BAMF_IS_WINDOW (self));
  g_return_if_fail (BAMF_IS_LEGACY_WINDOW (window));

  if (window == self->priv->legacy_window)
    {
      bamf_view_close (BAMF_VIEW (self));
    }
}

static void
handle_name_changed (BamfLegacyWindow *window, BamfWindow *self)
{
  g_return_if_fail (BAMF_IS_LEGACY_WINDOW (window));

  bamf_view_set_name (BAMF_VIEW (self), bamf_legacy_window_get_name (window));
}

static void
bamf_window_ensure_flags (BamfWindow *self)
{
  g_return_if_fail (BAMF_IS_WINDOW (self));

#ifdef EXPORT_ACTIONS_MENU
  self->priv->was_active = bamf_view_is_active (BAMF_VIEW (self));
#endif

  bamf_view_set_active       (BAMF_VIEW (self), bamf_legacy_window_is_active (self->priv->legacy_window));
  bamf_view_set_urgent       (BAMF_VIEW (self), bamf_legacy_window_needs_attention (self->priv->legacy_window));
  bamf_view_set_user_visible (BAMF_VIEW (self), !bamf_legacy_window_is_skip_tasklist (self->priv->legacy_window));

  BamfWindowMaximizationType maximized = bamf_window_maximized (self);

  if (self->priv->maximized != maximized)
  {
    BamfWindowMaximizationType old_state = self->priv->maximized;
    self->priv->maximized = maximized;
    g_signal_emit_by_name (self, "maximized-changed", old_state, maximized);
  }

#ifdef EXPORT_ACTIONS_MENU
  if (self->priv->dbusmenu_server && !self->priv->was_active &&
      !bamf_view_is_active (BAMF_VIEW (self)))
    {
      g_clear_object (&self->priv->dbusmenu_server);
      g_clear_object (&self->priv->action_menu);
    }
#endif
}

static void
bamf_window_ensure_monitor (BamfWindow *self)
{
  g_return_if_fail (BAMF_IS_WINDOW (self));

  gint monitor = bamf_window_get_monitor (self);

  if (self->priv->monitor != monitor)
  {
    gint old_monitor = self->priv->monitor;
    self->priv->monitor = monitor;
    g_signal_emit_by_name (self, "monitor-changed", old_monitor, monitor);
  }
}

static void
handle_state_changed (BamfLegacyWindow *window, BamfWindow *self)
{
  bamf_window_ensure_flags (self);
}

static void
handle_geometry_changed (BamfLegacyWindow *window, BamfWindow *self)
{
  bamf_window_ensure_monitor (self);
}

static const char *
bamf_window_get_view_type (BamfView *view)
{
  return "window";
}

char *
bamf_window_get_string_hint (BamfWindow *self, const char* prop)
{
  g_return_val_if_fail (BAMF_IS_WINDOW (self), NULL);
  return bamf_legacy_window_get_hint (self->priv->legacy_window, prop);
}

static char *
get_snap_desktop_id (guint pid)
{
  char *security_label_filename = NULL;
  char *security_label_contents = NULL;
  gsize i, security_label_contents_size = 0;
  char *contents_start;
  char *contents_end;
  char *sandboxed_app_id;

  g_return_val_if_fail (pid != 0, NULL);

  security_label_filename = g_strdup_printf ("/proc/%u/attr/current", pid);

  if (!g_file_get_contents (security_label_filename,
                            &security_label_contents,
                            &security_label_contents_size,
                            NULL))
    {
      g_free (security_label_filename);
      return NULL;
    }

  if (!g_str_has_prefix (security_label_contents, SNAP_SECURITY_LABEL_PREFIX))
    {
      g_free (security_label_filename);
      g_free (security_label_contents);
      return NULL;
    }

  /* We need to translate the security profile into the desktop-id.
   * The profile is in the form of 'snap.name-space.binary-name (current)'
   * while the desktop id will be name-space_binary-name.
   */
  security_label_contents_size -= sizeof (SNAP_SECURITY_LABEL_PREFIX) - 1;
  contents_start = security_label_contents + sizeof (SNAP_SECURITY_LABEL_PREFIX) - 1;
  contents_end = strchr (contents_start, ' ');

  if (contents_end)
    security_label_contents_size = contents_end - contents_start;

  for (i = 0; i < security_label_contents_size; ++i)
    {
      if (contents_start[i] == '.')
        contents_start[i] = '_';
    }

  sandboxed_app_id = g_malloc0 (security_label_contents_size + 1);
  memcpy (sandboxed_app_id, contents_start, security_label_contents_size);

  g_free (security_label_filename);
  g_free (security_label_contents);

  return sandboxed_app_id;
}

static char *
get_flatpak_desktop_id (guint pid)
{
  GKeyFile *key_file = NULL;
  char *info_filename = NULL;
  char *app_id = NULL;

  g_return_val_if_fail (pid != 0, NULL);

  key_file = g_key_file_new ();
  info_filename = g_strdup_printf ("/proc/%u/root/.flatpak-info", pid);

  if (!g_key_file_load_from_file (key_file, info_filename, G_KEY_FILE_NONE, NULL))
    {
      g_free (info_filename);
      return NULL;
    }

  app_id = g_key_file_get_string (key_file, "Application", "name", NULL);

  g_key_file_free (key_file);
  g_free (info_filename);

  return app_id;
}

char *
bamf_window_get_application_id (BamfWindow *self)
{
  guint pid;
  char *app_id;

  g_return_val_if_fail (BAMF_IS_WINDOW (self), NULL);

  pid = bamf_window_get_pid (self);

  if (pid > 0)
    {
      app_id = get_snap_desktop_id (pid);

      if (app_id)
        return app_id;

      app_id = get_flatpak_desktop_id (pid);

      if (app_id)
        return app_id;
    }

  app_id = bamf_window_get_string_hint (self, _GTK_APPLICATION_ID);

  return app_id;
}

BamfWindowMaximizationType
bamf_window_maximized (BamfWindow *self)
{
  g_return_val_if_fail (BAMF_IS_WINDOW (self), BAMF_WINDOW_FLOATING);
  return bamf_legacy_window_maximized (self->priv->legacy_window);
}

gint
bamf_window_get_monitor (BamfWindow *self)
{
  gint x, y, width, height;
  g_return_val_if_fail (BAMF_IS_WINDOW (self), -1);

  GdkScreen *gdk_screen =  gdk_screen_get_default ();
  bamf_legacy_window_get_geometry (self->priv->legacy_window, &x, &y, &width, &height);

  return gdk_screen_get_monitor_at_point (gdk_screen, x + width/2, y + height/2);
}

static char *
bamf_window_get_stable_bus_name (BamfView *view)
{
  BamfWindow *self;

  g_return_val_if_fail (BAMF_IS_WINDOW (view), NULL);
  self = BAMF_WINDOW (view);

  return g_strdup_printf ("window/%u", bamf_legacy_window_get_xid (self->priv->legacy_window));
}

gint
bamf_window_get_stack_position (BamfWindow *self)
{
  g_return_val_if_fail (BAMF_IS_WINDOW (self), -1);

  return bamf_legacy_window_get_stacking_position (self->priv->legacy_window);
}

static void
active_window_changed (BamfLegacyScreen *screen, BamfWindow *window)
{
  bamf_window_ensure_flags (window);
}

#ifdef EXPORT_ACTIONS_MENU
static gboolean
bamf_window_active_changed (BamfView *view, gboolean active)
{
  BamfWindow *self = BAMF_WINDOW (view);
  BamfWindowPrivate *priv = self->priv;
  GtkWidget *window_menu;
  BamfWindowType win_type;
  const char *view_path;

  if (!active && !priv->was_active)
    {
      g_clear_object (&priv->dbusmenu_server);
      g_clear_object (&priv->action_menu);
      return FALSE;
    }

  if (priv->dbusmenu_server)
    return FALSE;

  win_type = bamf_window_get_window_type (self);

  if (win_type == BAMF_WINDOW_DOCK ||
      win_type == BAMF_WINDOW_TOOLBAR ||
      win_type == BAMF_WINDOW_MENU ||
      win_type == BAMF_WINDOW_SPLASHSCREEN)
  {
    return FALSE;
  }

  window_menu = bamf_legacy_window_get_action_menu (priv->legacy_window);

  if (!GTK_IS_WIDGET (window_menu))
    return FALSE;

  view_path = bamf_view_get_path (view);
  priv->dbusmenu_server = dbusmenu_server_new (view_path);

  priv->action_menu = gtk_menu_new ();
  g_object_ref_sink (priv->action_menu);

  GtkWidget *menuitem = gtk_menu_item_new_with_label (_("Window actions"));
  gtk_widget_show (menuitem);

  gtk_menu_item_set_submenu (GTK_MENU_ITEM (menuitem), window_menu);
  gtk_menu_shell_append (GTK_MENU_SHELL (priv->action_menu), menuitem);

  dbusmenu_server_set_root (priv->dbusmenu_server,
                            dbusmenu_gtk_parse_menu_structure (priv->action_menu));

  return FALSE;
}
#endif

static gboolean
on_dbus_handle_get_pid (BamfDBusItemWindow *interface,
                        GDBusMethodInvocation *invocation,
                        BamfWindow *self)
{
  gint pid = bamf_window_get_pid (self);
  g_dbus_method_invocation_return_value (invocation, g_variant_new ("(u)", pid));

  return TRUE;
}

static gboolean
on_dbus_handle_get_xid (BamfDBusItemWindow *interface,
                        GDBusMethodInvocation *invocation,
                        BamfWindow *self)
{
  guint32 xid = bamf_window_get_xid (self);
  g_dbus_method_invocation_return_value (invocation, g_variant_new ("(u)", xid));

  return TRUE;
}

static gboolean
on_dbus_handle_transient (BamfDBusItemWindow *interface,
                          GDBusMethodInvocation *invocation,
                          BamfWindow *self)
{
  const char *transient_path = bamf_window_get_transient_path (self);
  g_dbus_method_invocation_return_value (invocation,
                                         g_variant_new ("(s)", transient_path));

  return TRUE;
}

static gboolean
on_dbus_handle_window_type (BamfDBusItemWindow *interface,
                            GDBusMethodInvocation *invocation,
                            BamfWindow *self)
{
  BamfWindowType window_type = bamf_window_get_window_type (self);
  g_dbus_method_invocation_return_value (invocation,
                                         g_variant_new ("(u)", window_type));

  return TRUE;
}

static gboolean
on_dbus_handle_xprop (BamfDBusItemWindow *interface,
                      GDBusMethodInvocation *invocation,
                      const gchar *prop,
                      BamfWindow *self)
{
  char *hint = bamf_window_get_string_hint (self, prop);
  g_dbus_method_invocation_return_value (invocation,
                                         g_variant_new ("(s)", hint ? hint : ""));

  g_free (hint);

  return TRUE;
}

static gboolean
on_dbus_handle_monitor (BamfDBusItemWindow *interface,
                        GDBusMethodInvocation *invocation,
                        BamfWindow *self)
{
  gint monitor = bamf_window_get_monitor (self);
  g_dbus_method_invocation_return_value (invocation,
                                         g_variant_new ("(i)", monitor));

  return TRUE;
}

static gboolean
on_dbus_handle_maximized (BamfDBusItemWindow *interface,
                          GDBusMethodInvocation *invocation,
                          BamfWindow *self)
{
  BamfWindowMaximizationType maximized = bamf_window_maximized (self);
  g_dbus_method_invocation_return_value (invocation,
                                         g_variant_new ("(i)", maximized));

  return TRUE;
}

static void
bamf_window_set_property (GObject *object, guint property_id, const GValue *value, GParamSpec *pspec)
{
  BamfWindow *self;

  self = BAMF_WINDOW (object);

  switch (property_id)
    {
      case PROP_WINDOW:
        self->priv->legacy_window = BAMF_LEGACY_WINDOW (g_value_get_object (value));
        break;

      default:
        G_OBJECT_WARN_INVALID_PROPERTY_ID(object, property_id, pspec);
    }
}

static void
bamf_window_get_property (GObject *object, guint property_id, GValue *value, GParamSpec *pspec)
{
  BamfWindow *self;

  self = BAMF_WINDOW (object);

  switch (property_id)
    {
      case PROP_WINDOW:
        g_value_set_object (value, self->priv->legacy_window);

        break;

      default:
        G_OBJECT_WARN_INVALID_PROPERTY_ID(object, property_id, pspec);
    }
}

static void
on_maximized_changed (BamfWindow *self, BamfWindowMaximizationType old,
                      BamfWindowMaximizationType new, gpointer _not_used)
{
  g_return_if_fail (BAMF_IS_WINDOW (self));
  g_signal_emit_by_name (self->priv->dbus_iface, "maximized-changed", old, new);
}

static void
on_monitor_changed (BamfWindow *self, gint old, gint new, gpointer _not_used)
{
  g_return_if_fail (BAMF_IS_WINDOW (self));
  g_signal_emit_by_name (self->priv->dbus_iface, "monitor-changed", old, new);
}

static void
bamf_window_constructed (GObject *object)
{
  BamfWindow *self;
  BamfLegacyWindow *window;

  if (G_OBJECT_CLASS (bamf_window_parent_class)->constructed)
    G_OBJECT_CLASS (bamf_window_parent_class)->constructed (object);

  g_object_get (object, "legacy-window", &window, NULL);

  self = BAMF_WINDOW (object);
  bamf_windows = g_list_prepend (bamf_windows, self);

  bamf_view_set_name (BAMF_VIEW (self), bamf_legacy_window_get_name (window));

  g_signal_connect (G_OBJECT (window), "name-changed",
                    (GCallback) handle_name_changed, self);
  g_signal_connect (G_OBJECT (window), "state-changed",
                    (GCallback) handle_state_changed, self);
  g_signal_connect (G_OBJECT (window), "geometry-changed",
                    (GCallback) handle_geometry_changed, self);
  g_signal_connect (G_OBJECT (window), "closed",
                    (GCallback) handle_window_closed, self);

  self->priv->maximized = -1;
  self->priv->monitor = -1;

  bamf_window_ensure_flags (self);
  bamf_window_ensure_monitor (self);
}

static void
bamf_window_dispose (GObject *object)
{
  BamfWindow *self;

  self = BAMF_WINDOW (object);
  bamf_windows = g_list_remove (bamf_windows, self);

  g_signal_handlers_disconnect_by_func (bamf_legacy_screen_get_default (),
                                        active_window_changed, self);

  if (self->priv->legacy_window)
    {
      g_signal_handlers_disconnect_by_data (self->priv->legacy_window, self);
      g_object_unref (self->priv->legacy_window);
      self->priv->legacy_window = NULL;
    }

#ifdef EXPORT_ACTIONS_MENU
    g_clear_object (&self->priv->dbusmenu_server);
    g_clear_object (&self->priv->action_menu);
#endif

  G_OBJECT_CLASS (bamf_window_parent_class)->dispose (object);
}

static void
bamf_window_finalize (GObject *object)
{
  BamfWindow *self;
  self = BAMF_WINDOW (object);

  g_object_unref (self->priv->dbus_iface);

  G_OBJECT_CLASS (bamf_window_parent_class)->finalize (object);
}

static void
bamf_window_init (BamfWindow * self)
{
  self->priv = BAMF_WINDOW_GET_PRIVATE (self);

  /* Initializing the dbus interface */
  self->priv->dbus_iface = _bamf_dbus_item_window_skeleton_new ();

  /* We need to connect to the object own signals to redirect them to the dbus
   * interface                                                                */
  g_signal_connect (self, "maximized-changed", G_CALLBACK (on_maximized_changed), NULL);
  g_signal_connect (self, "monitor-changed", G_CALLBACK (on_monitor_changed), NULL);

  /* Registering signal callbacks to reply to dbus method calls */
  g_signal_connect (self->priv->dbus_iface, "handle-get-pid",
                    G_CALLBACK (on_dbus_handle_get_pid), self);

  g_signal_connect (self->priv->dbus_iface, "handle-get-xid",
                    G_CALLBACK (on_dbus_handle_get_xid), self);

  g_signal_connect (self->priv->dbus_iface, "handle-transient",
                    G_CALLBACK (on_dbus_handle_transient), self);

  g_signal_connect (self->priv->dbus_iface, "handle-window-type",
                    G_CALLBACK (on_dbus_handle_window_type), self);

  g_signal_connect (self->priv->dbus_iface, "handle-xprop",
                    G_CALLBACK (on_dbus_handle_xprop), self);

  g_signal_connect (self->priv->dbus_iface, "handle-monitor",
                    G_CALLBACK (on_dbus_handle_monitor), self);

  g_signal_connect (self->priv->dbus_iface, "handle-maximized",
                    G_CALLBACK (on_dbus_handle_maximized), self);

  /* Setting the interface for the dbus object */
  _bamf_dbus_item_object_skeleton_set_window (BAMF_DBUS_ITEM_OBJECT_SKELETON (self),
                                              self->priv->dbus_iface);

  g_signal_connect (G_OBJECT (bamf_legacy_screen_get_default ()), "active-window-changed",
                    (GCallback) active_window_changed, self);
}

static void
bamf_window_dbus_iface_init (BamfDBusItemWindowIface *iface)
{
}

static void
bamf_window_class_init (BamfWindowClass * klass)
{
  GParamSpec *pspec;
  GObjectClass *object_class = G_OBJECT_CLASS (klass);
  BamfViewClass *view_class = BAMF_VIEW_CLASS (klass);

  object_class->dispose       = bamf_window_dispose;
  object_class->finalize      = bamf_window_finalize;
  object_class->get_property  = bamf_window_get_property;
  object_class->set_property  = bamf_window_set_property;
  object_class->constructed   = bamf_window_constructed;
  view_class->view_type       = bamf_window_get_view_type;
  view_class->stable_bus_name = bamf_window_get_stable_bus_name;
#ifdef EXPORT_ACTIONS_MENU
  view_class->active_changed  = bamf_window_active_changed;
#endif

  pspec = g_param_spec_object ("legacy-window", "legacy-window", "legacy-window",
                               BAMF_TYPE_LEGACY_WINDOW,
                               G_PARAM_READWRITE | G_PARAM_CONSTRUCT);
  g_object_class_install_property (object_class, PROP_WINDOW, pspec);

  g_type_class_add_private (klass, sizeof (BamfWindowPrivate));
}

BamfWindow *
bamf_window_new (BamfLegacyWindow *window)
{
  BamfWindow *self;
  self = (BamfWindow *) g_object_new (BAMF_TYPE_WINDOW, "legacy-window", window, NULL);

  return self;
}
