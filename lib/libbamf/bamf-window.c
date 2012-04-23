/*
 * Copyright 2010 Canonical Ltd.
 *
 * This program is free software: you can redistribute it and/or modify it
 * under the terms of either or both of the following licenses:
 *
 * 1) the GNU Lesser General Public License version 3, as published by the
 * Free Software Foundation; and/or
 * 2) the GNU Lesser General Public License version 2.1, as published by
 * the Free Software Foundation.
 *
 * This program is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranties of
 * MERCHANTABILITY, SATISFACTORY QUALITY or FITNESS FOR A PARTICULAR
 * PURPOSE.  See the applicable version of the GNU Lesser General Public
 * License for more details.
 *
 * You should have received a copy of both the GNU Lesser General Public
 * License version 3 and version 2.1 along with this program.  If not, see
 * <http://www.gnu.org/licenses/>
 *
 * Authored by: Jason Smith <jason.smith@canonical.com>
 *              Neil Jagdish Patel <neil.patel@canonical.com>
 *
 */
/**
 * SECTION:bamf-window
 * @short_description: The base class for all windows
 *
 * #BamfWindow is the base class that all windows need to derive from.
 */

#if HAVE_CONFIG_H
#include <config.h>
#endif

#include "bamf-view-private.h"
#include "bamf-window.h"
#include "bamf-factory.h"
#include "bamf-marshal.h"
#include <dbus/dbus.h>
#include <dbus/dbus-glib.h>
#include <dbus/dbus-glib-lowlevel.h>

G_DEFINE_TYPE (BamfWindow, bamf_window, BAMF_TYPE_VIEW);

#define BAMF_WINDOW_GET_PRIVATE(o) \
  (G_TYPE_INSTANCE_GET_PRIVATE ((o), BAMF_TYPE_WINDOW, BamfWindowPrivate))

struct _BamfWindowPrivate
{
  DBusGConnection *connection;
  DBusGProxy      *proxy;
  guint32          xid;
  guint32          pid;
  time_t           last_active;
  gint             monitor;
  BamfWindowMaximizationType maximized;
};

enum
{
  MONITOR_CHANGED,
  MAXIMIZED_CHANGED,

  LAST_SIGNAL,
};

static guint window_signals[LAST_SIGNAL] = { 0 };

time_t bamf_window_last_active (BamfWindow *self)
{
  g_return_val_if_fail (BAMF_IS_WINDOW (self), (time_t) 0);
  
  return self->priv->last_active;
}

BamfWindow * bamf_window_get_transient (BamfWindow *self)
{
  BamfWindowPrivate *priv;
  BamfView *transient;
  char *path = NULL;
  GError *error = NULL;

  g_return_val_if_fail (BAMF_IS_WINDOW (self), FALSE);
  priv = self->priv;

  if (!bamf_view_remote_ready (BAMF_VIEW (self)))
    return NULL;

  if (!dbus_g_proxy_call (priv->proxy,
                          "Transient",
                          &error,
                          G_TYPE_INVALID,
                          G_TYPE_STRING, &path,
                          G_TYPE_INVALID))
    {
      g_warning ("Failed to fetch path: %s", error->message);
      g_error_free (error);
      return 0;
    }
  
  if (!path)
    return NULL;

  transient = bamf_factory_view_for_path_type (bamf_factory_get_default (), path, "window");
  g_free (path);  

  if (!BAMF_IS_WINDOW (transient))
    return NULL;
    
  return BAMF_WINDOW (transient);
}

BamfWindowType bamf_window_get_window_type (BamfWindow *self)
{
  BamfWindowPrivate *priv;
  guint32 type = 0;
  GError *error = NULL;

  g_return_val_if_fail (BAMF_IS_WINDOW (self), FALSE);
  priv = self->priv;

  if (!bamf_view_remote_ready (BAMF_VIEW (self)))
    return 0;

  if (!dbus_g_proxy_call (priv->proxy,
                          "WindowType",
                          &error,
                          G_TYPE_INVALID,
                          G_TYPE_UINT, &type,
                          G_TYPE_INVALID))
    {
      g_warning ("Failed to fetch type: %s", error->message);
      g_error_free (error);
      return 0;
    }

  return (BamfWindowType) type;
}

guint32 bamf_window_get_pid (BamfWindow *self)
{
  BamfWindowPrivate *priv;
  guint32 pid = 0;
  GError *error = NULL;

  g_return_val_if_fail (BAMF_IS_WINDOW (self), FALSE);
  priv = self->priv;

  if (priv->pid != 0)
    return priv->pid;

  if (!bamf_view_remote_ready (BAMF_VIEW (self)))
    return 0;

  if (!dbus_g_proxy_call (priv->proxy,
                          "GetPid",
                          &error,
                          G_TYPE_INVALID,
                          G_TYPE_UINT, &pid,
                          G_TYPE_INVALID))
    {
      g_warning ("Failed to fetch pid: %s", error->message);
      g_error_free (error);
      return 0;
    }

  return pid;
}

guint32 bamf_window_get_xid (BamfWindow *self)
{
  BamfWindowPrivate *priv;
  guint32 xid = 0;
  GError *error = NULL;
  
  g_return_val_if_fail (BAMF_IS_WINDOW (self), FALSE);
  priv = self->priv;
  
  if (priv->xid != 0)
    return priv->xid;

  if (!bamf_view_remote_ready (BAMF_VIEW (self)))
    return 0;

  if (!dbus_g_proxy_call (priv->proxy,
                          "GetXid",
                          &error,
                          G_TYPE_INVALID,
                          G_TYPE_UINT, &xid,
                          G_TYPE_INVALID))
    {
      g_warning ("Failed to fetch xid: %s", error->message);
      g_error_free (error);
      return 0;
    }

  return xid;
}

static void
bamf_window_active_changed (BamfView *view, gboolean active)
{
  BamfWindow *self;

  g_return_if_fail (BAMF_IS_WINDOW (view));

  self = BAMF_WINDOW (view);
  
  if (active)
    self->priv->last_active = time (NULL);
}

static void
bamf_window_on_monitor_changed (DBusGProxy *proxy, gint old, gint new, BamfWindow *self)
{
  self->priv->monitor = new;
  g_signal_emit (G_OBJECT (self), window_signals[MONITOR_CHANGED], 0, old, new);
}

static void
bamf_window_on_maximized_changed (DBusGProxy *proxy, gint old, gint new, BamfWindow *self)
{
  self->priv->maximized = new;
  g_signal_emit (G_OBJECT (self), window_signals[MAXIMIZED_CHANGED], 0, old, new);
}

static void
bamf_window_set_path (BamfView *view, const char *path)
{
  BamfWindow *self;
  BamfWindowPrivate *priv;
  
  self = BAMF_WINDOW (view);
  priv = self->priv;

  priv->proxy = dbus_g_proxy_new_for_name (priv->connection,
                                           "org.ayatana.bamf",
                                           path,
                                           "org.ayatana.bamf.window");
  if (priv->proxy == NULL)
    {
      g_error ("Unable to get org.ayatana.bamf.window window");
      return;
    }

  priv->xid = bamf_window_get_xid (self);
  priv->monitor = bamf_window_get_monitor (self);
  priv->maximized = bamf_window_maximized (self);

  dbus_g_object_register_marshaller ((GClosureMarshal) bamf_marshal_VOID__INT_INT,
                                     G_TYPE_NONE, 
                                     G_TYPE_INT, G_TYPE_INT,
                                     G_TYPE_INVALID);

  dbus_g_proxy_add_signal (priv->proxy,
                           "MonitorChanged",
                           G_TYPE_INT,
                           G_TYPE_INT,
                           G_TYPE_INVALID);

  dbus_g_proxy_connect_signal (priv->proxy,
                               "MonitorChanged",
                               (GCallback) bamf_window_on_monitor_changed,
                               self, NULL);

  dbus_g_proxy_add_signal (priv->proxy,
                           "MaximizedChanged",
                           G_TYPE_INT,
                           G_TYPE_INT,
                           G_TYPE_INVALID);

  dbus_g_proxy_connect_signal (priv->proxy,
                               "MaximizedChanged",
                               (GCallback) bamf_window_on_maximized_changed,
                               self, NULL);
}

gchar *
bamf_window_get_utf8_prop (BamfWindow *self, const char* xprop)
{
  BamfWindowPrivate *priv;
  char *result = NULL;
  GError *error = NULL;

  g_return_val_if_fail (BAMF_IS_WINDOW (self), NULL);
  priv = self->priv;

  if (!bamf_view_remote_ready (BAMF_VIEW (self)))
    return NULL;

  if (!dbus_g_proxy_call (priv->proxy,
                          "Xprop",
                          &error,
                          G_TYPE_STRING, xprop,
                          G_TYPE_INVALID,
                          G_TYPE_STRING, &result,
                          G_TYPE_INVALID))
    {
      g_warning ("Failed to fetch property: %s", error->message);
      g_error_free (error);

      return NULL;
    }

  if (result && result[0] == '\0')
    {
      g_free (result);
      result = NULL;
    }

  return result;
}

gint
bamf_window_get_monitor (BamfWindow *self)
{
  BamfWindowPrivate *priv;
  gint monitor = -2;
  GError *error = NULL;

  g_return_val_if_fail (BAMF_IS_WINDOW (self), -1);
  priv = self->priv;

  if (priv->monitor != -2 || !bamf_view_remote_ready (BAMF_VIEW (self)))
    {
      return priv->monitor;
    }

  if (!dbus_g_proxy_call (priv->proxy,
                          "Monitor",
                          &error,
                          G_TYPE_INVALID,
                          G_TYPE_INT, &monitor,
                          G_TYPE_INVALID))
    {
      g_warning ("Failed to fetch monitor: %s", error->message);
      g_error_free (error);

      return -1;
    }

  return monitor;
}

BamfWindowMaximizationType
bamf_window_maximized (BamfWindow *self)
{
  BamfWindowPrivate *priv;
  BamfWindowMaximizationType maximized = -1;
  GError *error = NULL;

  g_return_val_if_fail (BAMF_IS_WINDOW (self), -1);
  priv = self->priv;

  if (priv->maximized != -1 || !bamf_view_remote_ready (BAMF_VIEW (self)))
    {
      return priv->maximized;
    }

  if (!dbus_g_proxy_call (priv->proxy,
                          "Maximized",
                          &error,
                          G_TYPE_INVALID,
                          G_TYPE_INT, &maximized,
                          G_TYPE_INVALID))
    {
      g_warning ("Failed to fetch maximized state: %s", error->message);
      g_error_free (error);

      return -1;
    }

  return maximized;
}

static void
bamf_window_dispose (GObject *object)
{
  BamfWindow *self;
  BamfWindowPrivate *priv;
  
  self = BAMF_WINDOW (object);
  priv = self->priv;
  
  if (priv->proxy)
    {
      dbus_g_proxy_disconnect_signal (priv->proxy,
                                      "MaximizedChanged",
                                      (GCallback) bamf_window_on_maximized_changed,
                                      self);

      dbus_g_proxy_disconnect_signal (priv->proxy,
                                      "MonitorChanged",
                                      (GCallback) bamf_window_on_monitor_changed,
                                      self);

      g_object_unref (priv->proxy);
      priv->proxy = NULL;
    }

  if (G_OBJECT_CLASS (bamf_window_parent_class)->dispose)
    G_OBJECT_CLASS (bamf_window_parent_class)->dispose (object);
}

static void
bamf_window_class_init (BamfWindowClass *klass)
{
  GObjectClass  *obj_class  = G_OBJECT_CLASS (klass);
  BamfViewClass *view_class = BAMF_VIEW_CLASS (klass);

  g_type_class_add_private (obj_class, sizeof (BamfWindowPrivate));
  
  obj_class->dispose     = bamf_window_dispose;
  view_class->active_changed = bamf_window_active_changed;
  view_class->set_path   = bamf_window_set_path;
  
  window_signals[MONITOR_CHANGED] = 
    g_signal_new ("monitor-changed",
                  G_OBJECT_CLASS_TYPE (klass),
                  G_SIGNAL_RUN_FIRST,
                  G_STRUCT_OFFSET (BamfWindowClass, monitor_changed),
                  NULL, NULL,
                  bamf_marshal_VOID__INT_INT,
                  G_TYPE_NONE, 2,
                  G_TYPE_INT, G_TYPE_INT);

  window_signals[MAXIMIZED_CHANGED] = 
    g_signal_new ("maximized-changed",
                  G_OBJECT_CLASS_TYPE (klass),
                  G_SIGNAL_RUN_FIRST,
                  G_STRUCT_OFFSET (BamfWindowClass, maximized_changed),
                  NULL, NULL,
                  bamf_marshal_VOID__INT_INT,
                  G_TYPE_NONE, 2,
                  G_TYPE_INT, G_TYPE_INT);
}

static void
bamf_window_init (BamfWindow *self)
{
  BamfWindowPrivate *priv;
  GError           *error = NULL;

  priv = self->priv = BAMF_WINDOW_GET_PRIVATE (self);

  priv->xid = 0;
  priv->pid = 0;
  priv->monitor = -2;
  priv->maximized = -1;

  priv->connection = dbus_g_bus_get (DBUS_BUS_SESSION, &error);
  if (priv->connection == NULL)
    {
      g_critical ("Failed to open connection to bus: %s",
               error != NULL ? error->message : "Unknown");
      if (error)
        g_error_free (error);
      return;
    }
}

BamfWindow *
bamf_window_new (const char * path)
{
  BamfWindow *self;
  self = g_object_new (BAMF_TYPE_WINDOW, NULL);
  
  bamf_view_set_path (BAMF_VIEW (self), path);

  return self;
}
