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
 * SECTION:bamf-application
 * @short_description: The base class for all applications
 *
 * #BamfApplication is the base class that all applications need to derive from.
 */

#if HAVE_CONFIG_H
#include <config.h>
#endif

#include "bamf-application.h"
#include "bamf-window.h"
#include "bamf-factory.h"

#include <dbus/dbus.h>
#include <dbus/dbus-glib.h>
#include <dbus/dbus-glib-lowlevel.h>
#include <string.h>

G_DEFINE_TYPE (BamfApplication, bamf_application, BAMF_TYPE_VIEW);

#define BAMF_APPLICATION_GET_PRIVATE(o) \
  (G_TYPE_INSTANCE_GET_PRIVATE ((o), BAMF_TYPE_APPLICATION, BamfApplicationPrivate))

enum
{
  WINDOW_ADDED,
  WINDOW_REMOVED,
  
  LAST_SIGNAL,
};

static guint application_signals[LAST_SIGNAL] = { 0 };

struct _BamfApplicationPrivate
{
  DBusGConnection *connection;
  DBusGProxy      *proxy;
  gchar           *application_type;
  gchar           *desktop_file;
};

const gchar *
bamf_application_get_desktop_file (BamfApplication *application)
{
  BamfApplicationPrivate *priv;
  gchar *file;
  GError *error = NULL;

  g_return_val_if_fail (BAMF_IS_APPLICATION (application), FALSE);
  priv = application->priv;
  
  if (priv->desktop_file)
    return priv->desktop_file;

  if (!dbus_g_proxy_call (priv->proxy,
                          "DesktopFile",
                          &error,
                          G_TYPE_INVALID,
                          G_TYPE_STRING, &file,
                          G_TYPE_INVALID))
    {
      g_warning ("Failed to fetch path: %s", error->message);
      g_error_free (error);
      
      return NULL;
    }

  priv->desktop_file = file;
  return file;
}

const gchar *
bamf_application_get_application_type (BamfApplication *application)
{
  BamfApplicationPrivate *priv;
  gchar *type;
  GError *error = NULL;

  g_return_val_if_fail (BAMF_IS_APPLICATION (application), FALSE);
  priv = application->priv;

  if (priv->application_type)
    return priv->application_type;

  if (!dbus_g_proxy_call (priv->proxy,
                          "ApplicationType",
                          &error,
                          G_TYPE_INVALID,
                          G_TYPE_STRING, &type,
                          G_TYPE_INVALID))
    {
      g_warning ("Failed to fetch path: %s", error->message);
      g_error_free (error);
      
      return NULL;
    }
  
  priv->application_type = type;
  return type;
}

GArray *
bamf_application_get_xids (BamfApplication *application)
{
  BamfApplicationPrivate *priv;
  GArray *xids;
  GError *error = NULL;

  g_return_val_if_fail (BAMF_IS_APPLICATION (application), FALSE);
  priv = application->priv;

  if (!dbus_g_proxy_call (priv->proxy,
                          "Xids",
                          &error,
                          G_TYPE_INVALID,
                          DBUS_TYPE_G_UINT_ARRAY, &xids,
                          G_TYPE_INVALID))
    {
      g_warning ("Failed to fetch xids: %s", error->message);
      g_error_free (error);
      
      return NULL;
    }

  return xids;
}

GList *
bamf_application_get_windows (BamfApplication *application)
{
  GList *children, *l;
  GList *windows = NULL;
  BamfView *view;

  g_return_val_if_fail (BAMF_IS_APPLICATION (application), NULL);

  children = bamf_view_get_children (BAMF_VIEW (application));

  for (l = children; l; l = l->next)
    {
      view = l->data;
    
      if (BAMF_IS_WINDOW (view));
        {
          windows = g_list_prepend (windows, view);
        }
    }

  return windows;
}

static void
bamf_application_on_window_added (DBusGProxy *proxy, char *path, BamfApplication *self)
{
  BamfView *view;

  view = bamf_factory_view_for_path (bamf_factory_get_default (), path);

  g_signal_emit (G_OBJECT (self), application_signals[WINDOW_ADDED], 0, view);
}

static void
bamf_application_on_window_removed (DBusGProxy *proxy, char *path, BamfApplication *self)
{
  BamfView *view;

  view = bamf_factory_view_for_path (bamf_factory_get_default (), path);

  g_signal_emit (G_OBJECT (self), application_signals[WINDOW_REMOVED], 0, view);
}

static void
bamf_application_dispose (GObject *object)
{
  BamfApplication *self;
  BamfApplicationPrivate *priv;
  
  self = BAMF_APPLICATION (object);
  priv = self->priv;
  
  if (priv->application_type)
    {
      g_free (priv->application_type);
      priv->application_type = NULL;
    }
  
  if (priv->desktop_file)
    {
      g_free (priv->desktop_file);
      priv->desktop_file = NULL;
    }  
  
  if (priv->proxy)
    {
      g_object_unref (priv->proxy);
      priv->proxy = NULL;
    }

  if (G_OBJECT_CLASS (bamf_application_parent_class)->dispose)
    G_OBJECT_CLASS (bamf_application_parent_class)->dispose (object);
}

static void
bamf_application_constructed (GObject *object)
{
  BamfApplication *self;
  BamfApplicationPrivate *priv;
  char *path;

  if (G_OBJECT_CLASS (bamf_application_parent_class)->constructed)
    G_OBJECT_CLASS (bamf_application_parent_class)->constructed (object);
  
  self = BAMF_APPLICATION (object);
  priv = self->priv;
  
  g_object_get (object, "path", &path, NULL);
  
  priv->proxy = dbus_g_proxy_new_for_name (priv->connection,
                                           "org.ayatana.bamf",
                                           path,
                                           "org.ayatana.bamf.application");
  if (priv->proxy == NULL)
    {
      g_critical ("Unable to get org.ayatana.bamf.application application");
    }

  dbus_g_proxy_add_signal (priv->proxy,
                           "WindowAdded",
                           G_TYPE_STRING, 
                           G_TYPE_INVALID);

  dbus_g_proxy_add_signal (priv->proxy,
                           "WindowRemoved",
                           G_TYPE_STRING, 
                           G_TYPE_INVALID);

  dbus_g_proxy_connect_signal (priv->proxy,
                               "WindowAdded",
                               (GCallback) bamf_application_on_window_added,
                               self,
                               NULL);

  dbus_g_proxy_connect_signal (priv->proxy,
                               "WindowRemoved",
                               (GCallback) bamf_application_on_window_removed,
                               self,
                               NULL);
}

static void
bamf_application_class_init (BamfApplicationClass *klass)
{
  GObjectClass *obj_class = G_OBJECT_CLASS (klass);
  
  obj_class->constructed = bamf_application_constructed;
  obj_class->dispose     = bamf_application_dispose;

  g_type_class_add_private (obj_class, sizeof (BamfApplicationPrivate));

  application_signals [WINDOW_ADDED] = 
  	g_signal_new ("window-added",
  	              G_OBJECT_CLASS_TYPE (klass),
  	              0,
  	              0, NULL, NULL,
  	              g_cclosure_marshal_VOID__OBJECT,
  	              G_TYPE_NONE, 1, 
  	              BAMF_TYPE_VIEW);

  application_signals [WINDOW_REMOVED] = 
  	g_signal_new ("window-removed",
  	              G_OBJECT_CLASS_TYPE (klass),
  	              0,
  	              0, NULL, NULL,
  	              g_cclosure_marshal_VOID__OBJECT,
  	              G_TYPE_NONE, 1, 
  	              BAMF_TYPE_VIEW);
}


static void
bamf_application_init (BamfApplication *self)
{
  BamfApplicationPrivate *priv;
  GError           *error = NULL;

  priv = self->priv = BAMF_APPLICATION_GET_PRIVATE (self);

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

BamfApplication *
bamf_application_new (const char * path)
{
  BamfApplication *self;
  self = g_object_new (BAMF_TYPE_APPLICATION, "path", path, NULL);

  return self;
}
