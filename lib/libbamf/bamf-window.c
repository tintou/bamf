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

#include "bamf-window.h"
#include "bamf-factory.h"
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
  time_t           last_active;
};

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

  transient = bamf_factory_view_for_path (bamf_factory_get_default (), path);
  g_free (path);  

  if (!BAMF_IS_WINDOW (transient))
    return NULL;
    
  return BAMF_WINDOW (transient);
}

guint32 bamf_window_get_xid (BamfWindow *self)
{
  BamfWindowPrivate *priv;
  guint32 xid = 0;
  GError *error = NULL;

  g_return_val_if_fail (BAMF_IS_WINDOW (self), FALSE);
  priv = self->priv;

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
bamf_window_constructed (GObject *object)
{
  BamfWindow *self;
  BamfWindowPrivate *priv;
  gchar *path;
  
  if (G_OBJECT_CLASS (bamf_window_parent_class)->constructed)
    G_OBJECT_CLASS (bamf_window_parent_class)->constructed (object);
  
  g_object_get (object, "path", &path, NULL);
  
  self = BAMF_WINDOW (object);
  priv = self->priv;

  priv->proxy = dbus_g_proxy_new_for_name (priv->connection,
                                           "org.ayatana.bamf",
                                           path,
                                           "org.ayatana.bamf.window");
  if (priv->proxy == NULL)
    {
      g_error ("Unable to get org.ayatana.bamf.window window");
    }

  priv->xid = bamf_window_get_xid (self);
  g_free (path);
  
}

static void
bamf_window_class_init (BamfWindowClass *klass)
{
  GObjectClass  *obj_class  = G_OBJECT_CLASS (klass);
  BamfViewClass *view_class = BAMF_VIEW_CLASS (klass);

  g_type_class_add_private (obj_class, sizeof (BamfWindowPrivate));
  
  obj_class->constructed = bamf_window_constructed;
  view_class->active_changed = bamf_window_active_changed;
}


static void
bamf_window_init (BamfWindow *self)
{
  BamfWindowPrivate *priv;
  GError           *error = NULL;

  priv = self->priv = BAMF_WINDOW_GET_PRIVATE (self);

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
  self = g_object_new (BAMF_TYPE_WINDOW, "path", path, NULL);

  return self;
}
