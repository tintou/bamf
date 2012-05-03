/*
 * Copyright 2010-2011 Canonical Ltd.
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
 *              Marco Trevisan (Treviño) <3v1n0@ubuntu.com>
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
// Move to a general place!
#include <libbamf-private/bamf-private.h>

G_DEFINE_TYPE (BamfWindow, bamf_window, BAMF_TYPE_VIEW);

#define BAMF_WINDOW_GET_PRIVATE(o) \
  (G_TYPE_INSTANCE_GET_PRIVATE ((o), BAMF_TYPE_WINDOW, BamfWindowPrivate))

struct _BamfWindowPrivate
{
  BamfDBusItemWindow        *proxy;
  guint32                    xid;
  guint32                    pid;
  time_t                     last_active;
  gint                       monitor;
  BamfWindowMaximizationType maximized;
};

enum
{
  MONITOR_CHANGED,
  MAXIMIZED_CHANGED,

  LAST_SIGNAL,
};

static guint window_signals[LAST_SIGNAL] = { 0 };

time_t
bamf_window_last_active (BamfWindow *self)
{
  g_return_val_if_fail (BAMF_IS_WINDOW (self), 0);
  
  return self->priv->last_active;
}

BamfWindow *
bamf_window_get_transient (BamfWindow *self)
{
  BamfWindowPrivate *priv;
  BamfView *transient;
  char *path = NULL;
  GError *error = NULL;

  g_return_val_if_fail (BAMF_IS_WINDOW (self), FALSE);
  priv = self->priv;

  if (!bamf_view_remote_ready (BAMF_VIEW (self)))
    return NULL;

  if (!bamf_dbus_item_window_call_transient_sync (priv->proxy, &path, NULL, &error))
    {
      g_warning ("Failed to fetch path: %s", error ? error->message : "");
      g_error_free (error);
      return NULL;
    }
  
  if (!path)
    return NULL;

  if (path[0] == '\0')
    {
      g_free (path);
      return NULL;
    }

  BamfFactory *factory = bamf_factory_get_default ();
  transient = bamf_factory_view_for_path_type (factory, path, BAMF_FACTORY_WINDOW);
  g_free (path);  

  if (!BAMF_IS_WINDOW (transient))
    return NULL;
    
  return BAMF_WINDOW (transient);
}

BamfWindowType
bamf_window_get_window_type (BamfWindow *self)
{
  BamfWindowPrivate *priv;
  guint32 type = 0;
  GError *error = NULL;

  g_return_val_if_fail (BAMF_IS_WINDOW (self), FALSE);
  priv = self->priv;

  if (!bamf_view_remote_ready (BAMF_VIEW (self)))
    return 0;

  if (!bamf_dbus_item_window_call_window_type_sync (priv->proxy, &type, NULL, &error))
    {
      g_warning ("Failed to fetch type: %s", error ? error->message : "");
      g_error_free (error);
      return BAMF_WINDOW_UNKNOWN;
    }

  return type;
}

guint32
bamf_window_get_pid (BamfWindow *self)
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

  if (!bamf_dbus_item_window_call_get_pid_sync (priv->proxy, &pid, NULL, &error))
    {
      g_warning ("Failed to fetch pid: %s", error ? error->message : "");
      g_error_free (error);
      return 0;
    }

  return pid;
}

guint32
bamf_window_get_xid (BamfWindow *self)
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

  if (!bamf_dbus_item_window_call_get_xid_sync (priv->proxy, &xid, NULL, &error))
    {
      g_warning ("Failed to fetch xid: %s", error ? error->message : "");
      g_error_free (error);
      return 0;
    }

  return xid;
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

  if (!bamf_dbus_item_window_call_xprop_sync (priv->proxy, xprop, &result, NULL, &error))
    {
      g_warning ("Failed to fetch property: %s", error ? error->message : "");
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

  if (!bamf_dbus_item_window_call_monitor_sync (priv->proxy, &monitor, NULL, &error))
    {
      g_warning ("Failed to fetch monitor: %s", error ? error->message : "");
      g_error_free (error);

      monitor = -1;
    }

  return monitor;
}

BamfWindowMaximizationType
bamf_window_maximized (BamfWindow *self)
{
  BamfWindowPrivate *priv;
  gint maximized = -1;
  GError *error = NULL;

  g_return_val_if_fail (BAMF_IS_WINDOW (self), -1);
  priv = self->priv;

  if (priv->maximized != -1 || !bamf_view_remote_ready (BAMF_VIEW (self)))
    {
      return priv->maximized;
    }

  if (!bamf_dbus_item_window_call_maximized_sync (priv->proxy, &maximized, NULL, &error))
    {
      g_warning ("Failed to fetch maximized state: %s", error->message);
      g_error_free (error);

      return -1;
    }

  return maximized;
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
bamf_window_on_monitor_changed (BamfDBusItemWindow *proxy, gint old, gint new, BamfWindow *self)
{
  self->priv->monitor = new;
  g_signal_emit (G_OBJECT (self), window_signals[MONITOR_CHANGED], 0, old, new);
}

static void
bamf_window_on_maximized_changed (BamfDBusItemWindow *proxy, gint old, gint new, BamfWindow *self)
{
  self->priv->maximized = new;
  g_signal_emit (G_OBJECT (self), window_signals[MAXIMIZED_CHANGED], 0, old, new);
}

static void
bamf_window_set_path (BamfView *view, const char *path)
{
  BamfWindow *self;
  BamfWindowPrivate *priv;
  GError *error = NULL;
  
  self = BAMF_WINDOW (view);
  priv = self->priv;

  priv->proxy = bamf_dbus_item_window_proxy_new_for_bus_sync (G_BUS_TYPE_SESSION,
                                                              G_DBUS_PROXY_FLAGS_NONE,
                                                              "org.ayatana.bamf",
                                                              path, NULL, &error);
  if (priv->proxy == NULL)
    {
      g_error ("Unable to get org.ayatana.bamf.window window: %s", error ? error->message : "");
      g_error_free (error);
      return;
    }

  priv->xid = bamf_window_get_xid (self);
  priv->monitor = bamf_window_get_monitor (self);
  priv->maximized = bamf_window_maximized (self);

  g_signal_connect (priv->proxy, "monitor-changed",
                    G_CALLBACK (bamf_window_on_monitor_changed), self);

  g_signal_connect (priv->proxy, "maximized-changed",
                    G_CALLBACK (bamf_window_on_maximized_changed), self);
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
      g_signal_handlers_disconnect_by_func (priv->proxy, bamf_window_on_maximized_changed, self);
      g_signal_handlers_disconnect_by_func (priv->proxy, bamf_window_on_monitor_changed, self);

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
  
  obj_class->dispose = bamf_window_dispose;
  view_class->active_changed = bamf_window_active_changed;
  view_class->set_path = bamf_window_set_path;
  
  window_signals[MONITOR_CHANGED] = 
    g_signal_new (BAMF_WINDOW_SIGNAL_MONITOR_CHANGED,
                  G_OBJECT_CLASS_TYPE (klass),
                  G_SIGNAL_RUN_FIRST,
                  G_STRUCT_OFFSET (BamfWindowClass, monitor_changed),
                  NULL, NULL,
                  bamf_marshal_VOID__INT_INT,
                  G_TYPE_NONE, 2,
                  G_TYPE_INT, G_TYPE_INT);

  window_signals[MAXIMIZED_CHANGED] = 
    g_signal_new (BAMF_WINDOW_SIGNAL_MAXIMIZED_CHANGED,
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

  priv = self->priv = BAMF_WINDOW_GET_PRIVATE (self);
  priv->xid = 0;
  priv->pid = 0;
  priv->monitor = -2;
  priv->maximized = -1;
}

BamfWindow *
bamf_window_new (const char * path)
{
  BamfWindow *self;
  self = g_object_new (BAMF_TYPE_WINDOW, NULL);
  
  bamf_view_set_path (BAMF_VIEW (self), path);

  return self;
}
