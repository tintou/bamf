/*
 * Copyright 2009 Canonical Ltd.
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
 * SECTION:bamf-view
 * @short_description: The base class for all views
 *
 * #BamfView is the base class that all views need to derive from.
 */

#if HAVE_CONFIG_H
#include <config.h>
#endif

#include "bamf-view.h"
#include "bamf-factory.h"
#include "bamf-window.h"

#include <dbus/dbus.h>
#include <dbus/dbus-glib.h>
#include <dbus/dbus-glib-lowlevel.h>

G_DEFINE_TYPE (BamfView, bamf_view, G_TYPE_OBJECT);

#define BAMF_VIEW_GET_PRIVATE(o) \
  (G_TYPE_INSTANCE_GET_PRIVATE ((o), BAMF_TYPE_VIEW, BamfViewPrivate))

enum
{
  ACTIVE_CHANGED,
  CLOSED,
  CHILD_ADDED,
  CHILD_REMOVED,
  RUNNING_CHANGED,
  
  LAST_SIGNAL,
};

static guint view_signals[LAST_SIGNAL] = { 0 };

struct _BamfViewPrivate
{
  DBusGConnection *connection;
  DBusGProxy      *proxy;
  gchar           *path;
};

enum
{
  PROP_0,

  PROP_PATH,
};

static void
bamf_view_on_active_changed (DBusGProxy *proxy, gboolean active, BamfView *self)
{
  g_signal_emit (G_OBJECT(self), view_signals[ACTIVE_CHANGED], 0, active);
}

static void
bamf_view_on_closed (DBusGProxy *proxy, BamfView *self)
{
  g_signal_emit (G_OBJECT (self), view_signals[CLOSED], 0);

  g_object_unref (self);
}

static void
bamf_view_on_child_added (DBusGProxy *proxy, char *path, BamfView *self)
{
  BamfView *view;

  view = bamf_factory_view_for_path (bamf_factory_get_default (), path);

  g_signal_emit (G_OBJECT (self), view_signals[CHILD_ADDED], 0, view);
}

static void
bamf_view_on_child_removed (DBusGProxy *proxy, char *path, BamfView *self)
{
  BamfView *view;

  view = bamf_factory_view_for_path (bamf_factory_get_default (), path);

  g_signal_emit (G_OBJECT (self), view_signals[CHILD_ADDED], 0, view);
}

static void
bamf_view_on_running_changed (DBusGProxy *proxy, gboolean running, BamfView *self)
{
  g_signal_emit (G_OBJECT (self), view_signals[ACTIVE_CHANGED], 0, running);
}

static void
bamf_view_set_property (GObject *object, guint property_id, const GValue *value, GParamSpec *pspec)
{
  BamfView *self;

  self = BAMF_VIEW (object);

  switch (property_id)
    {
      case PROP_PATH:
        self->priv->path = g_value_dup_string (value);

        break;
        
      default:
        G_OBJECT_WARN_INVALID_PROPERTY_ID(object, property_id, pspec);
    }
}

static void
bamf_view_get_property (GObject *object, guint property_id, GValue *value, GParamSpec *pspec)
{
  BamfView *self;

  self = BAMF_VIEW (object);

  switch (property_id)
    {
      case PROP_PATH:
        g_value_set_string (value, self->priv->path);

        break;
        
      default:
        G_OBJECT_WARN_INVALID_PROPERTY_ID(object, property_id, pspec);
    }
}

static void
bamf_view_dispose (GObject *object)
{
  BamfView *view;
  BamfViewPrivate *priv;

  view = BAMF_VIEW (object);

  priv = view->priv;

  dbus_g_proxy_disconnect_signal (priv->proxy,
                                  "ActiveChanged",
                                  (GCallback) bamf_view_on_active_changed,
                                  view);

  dbus_g_proxy_disconnect_signal (priv->proxy,
                                  "Closed",
                                  (GCallback) bamf_view_on_closed,
                                  view);

  dbus_g_proxy_disconnect_signal (priv->proxy,
                                  "ChildAdded",
                                  (GCallback) bamf_view_on_child_added,
                                  view);

  dbus_g_proxy_disconnect_signal (priv->proxy,
                                  "ChildRemoved",
                                  (GCallback) bamf_view_on_child_removed,
                                  view);

  dbus_g_proxy_disconnect_signal (priv->proxy,
                                  "RunningChanged",
                                  (GCallback) bamf_view_on_running_changed,
                                   view);
}

static void
bamf_view_constructed (GObject *object)
{
  BamfView *view;
  BamfViewPrivate *priv;

  view = BAMF_VIEW (object);

  priv = view->priv;

  if (priv->proxy)
    {
      g_warning ("bamf_view_set_path called multiple times\n");
      return;
    }
  
  priv->proxy = dbus_g_proxy_new_for_name (priv->connection,
                                           "org.ayatana.bamf",
                                           priv->path,
                                           "org.ayatana.bamf.view");
  if (priv->proxy == NULL)
    {
      g_error ("Unable to get org.ayatana.bamf.view view");
    }

  dbus_g_proxy_add_signal (priv->proxy,
                           "ActiveChanged",
                           G_TYPE_BOOLEAN,
                           G_TYPE_INVALID);

  dbus_g_proxy_add_signal (priv->proxy,
                           "Closed",
                           G_TYPE_INVALID);

  dbus_g_proxy_add_signal (priv->proxy,
                           "ChildAdded",
                           G_TYPE_STRING,
                           G_TYPE_INVALID);

  dbus_g_proxy_add_signal (priv->proxy,
                           "ChildRemoved",
                           G_TYPE_STRING,
                           G_TYPE_INVALID);

  dbus_g_proxy_add_signal (priv->proxy,
                           "RunningChanged",
                           G_TYPE_BOOLEAN,
                           G_TYPE_INVALID);

  dbus_g_proxy_connect_signal (priv->proxy,
                               "ActiveChanged",
                               (GCallback) bamf_view_on_active_changed,
                               view,
                               NULL);

  dbus_g_proxy_connect_signal (priv->proxy,
                               "Closed",
                               (GCallback) bamf_view_on_closed,
                               view,
                               NULL);

  dbus_g_proxy_connect_signal (priv->proxy,
                               "ChildAdded",
                               (GCallback) bamf_view_on_child_added,
                               view,
                               NULL);

  dbus_g_proxy_connect_signal (priv->proxy,
                               "ChildRemoved",
                               (GCallback) bamf_view_on_child_removed,
                               view,
                               NULL);

  dbus_g_proxy_connect_signal (priv->proxy,
                               "RunningChanged",
                               (GCallback) bamf_view_on_running_changed,
                               view,
                               NULL);
}

static void
bamf_view_class_init (BamfViewClass *klass)
{
  GParamSpec *pspec;
  GObjectClass *obj_class = G_OBJECT_CLASS (klass);

  obj_class->constructed  = bamf_view_constructed;
  obj_class->dispose      = bamf_view_dispose;
  obj_class->get_property = bamf_view_get_property;
  obj_class->set_property = bamf_view_set_property;

  pspec = g_param_spec_string ("path", "path", "path", "", G_PARAM_READWRITE | G_PARAM_CONSTRUCT);
  g_object_class_install_property (obj_class, PROP_PATH, pspec);

  g_type_class_add_private (obj_class, sizeof (BamfViewPrivate));

  view_signals [ACTIVE_CHANGED] = 
  	g_signal_new ("active-changed",
  	              G_OBJECT_CLASS_TYPE (klass),
  	              0,
  	              0, NULL, NULL,
  	              g_cclosure_marshal_VOID__BOOLEAN,
  	              G_TYPE_NONE, 1, 
  	              G_TYPE_BOOLEAN);

  view_signals [CLOSED] = 
  	g_signal_new ("closed",
  	              G_OBJECT_CLASS_TYPE (klass),
  	              0,
  	              0, NULL, NULL,
  	              g_cclosure_marshal_VOID__VOID,
  	              G_TYPE_NONE, 0);

  view_signals [CHILD_ADDED] = 
  	g_signal_new ("child-added",
  	              G_OBJECT_CLASS_TYPE (klass),
  	              0,
  	              0, NULL, NULL,
  	              g_cclosure_marshal_VOID__POINTER,
  	              G_TYPE_NONE, 1, 
  	              BAMF_TYPE_VIEW);

  view_signals [CHILD_REMOVED] = 
  	g_signal_new ("child-removed",
  	              G_OBJECT_CLASS_TYPE (klass),
  	              0,
  	              0, NULL, NULL,
  	              g_cclosure_marshal_VOID__POINTER,
  	              G_TYPE_NONE, 1, 
  	              BAMF_TYPE_VIEW);

  view_signals [CHILD_ADDED] = 
  	g_signal_new ("running-changed",
  	              G_OBJECT_CLASS_TYPE (klass),
  	              0,
  	              0, NULL, NULL,
  	              g_cclosure_marshal_VOID__BOOLEAN,
  	              G_TYPE_NONE, 1, 
  	              G_TYPE_BOOLEAN);
}


static void
bamf_view_init (BamfView *self)
{
  BamfViewPrivate *priv;
  GError *error = NULL;

  priv = self->priv = BAMF_VIEW_GET_PRIVATE (self);

  priv->connection = dbus_g_bus_get (DBUS_BUS_SESSION, &error);
  if (priv->connection == NULL)
    {
      g_error ("Failed to open connection to bus: %s",
               error != NULL ? error->message : "Unknown");
      if (error)
        g_error_free (error);
      return;
    }
}

GList *
bamf_view_get_children (BamfView *view)
{
  char ** children;
  int i, len;
  GList *results = NULL;
  GError *error = NULL;
  BamfViewPrivate *priv;

  g_return_val_if_fail (BAMF_IS_VIEW (view), NULL);

  priv = view->priv;

  if (!dbus_g_proxy_call (priv->proxy,
                          "Children",
                          &error,
                          G_TYPE_INVALID,
                          G_TYPE_STRV, &children,
                          G_TYPE_INVALID))
    {
      g_error ("Unable to fetch children: %s\n", error->message);
    }

  if (!children)
    return NULL;

  len = g_strv_length (children);

  for (i = 0; i < len; i++)
    {
      BamfView *view = bamf_factory_view_for_path (bamf_factory_get_default (), children[i]);
      if (BAMF_IS_WINDOW (view))
      results = g_list_prepend (results, view);
    }

  return results;
}

gboolean
bamf_view_is_active (BamfView *view)
{
  BamfViewPrivate *priv;
  gboolean active = FALSE;
  GError *error = NULL;

  g_return_val_if_fail (BAMF_IS_VIEW (view), FALSE);
  priv = view->priv;

  if (!dbus_g_proxy_call (priv->proxy,
                          "IsActive",
                          &error,
                          G_TYPE_INVALID,
                          G_TYPE_BOOLEAN, &active,
                          G_TYPE_INVALID))
    {
      g_error ("Failed to fetch active: %s", error->message);
      g_error_free (error);
    }

  return active;
}

gboolean
bamf_view_is_running (BamfView *view)
{
  BamfViewPrivate *priv;
  gboolean running = FALSE;
  GError *error = NULL;

  g_return_val_if_fail (BAMF_IS_VIEW (view), FALSE);
  priv = view->priv;

  if (!dbus_g_proxy_call (priv->proxy,
                          "IsRunning",
                          &error,
                          G_TYPE_INVALID,
                          G_TYPE_BOOLEAN, &running,
                          G_TYPE_INVALID))
    {
      g_error ("Failed to fetch running: %s", error->message);
      g_error_free (error);
    }

  return running;
}

gchar *
bamf_view_get_name (BamfView *view)
{
  BamfViewPrivate *priv;
  char *name = NULL;
  GError *error = NULL;

  g_return_val_if_fail (BAMF_IS_VIEW (view), NULL);
  priv = view->priv;

  if (!dbus_g_proxy_call (priv->proxy,
                          "Name",
                          &error,
                          G_TYPE_INVALID,
                          G_TYPE_STRING, &name,
                          G_TYPE_INVALID))
    {
      g_error ("Failed to fetch name: %s", error->message);
      g_error_free (error);
    }

  return name;
}

BamfView *
bamf_view_get_parent (BamfView *view)
{
  BamfViewPrivate *priv;
  char *parent = NULL;
  GError *error = NULL;

  g_return_val_if_fail (BAMF_IS_VIEW (view), NULL);
  priv = view->priv;

  if (!dbus_g_proxy_call (priv->proxy,
                          "Parent",
                          &error,
                          G_TYPE_INVALID,
                          G_TYPE_STRING, &parent,
                          G_TYPE_INVALID))
    {
      g_error ("Failed to fetch parent: %s", error->message);
      g_error_free (error);
    }

  if (!parent)
    return NULL;
  return bamf_factory_view_for_path (bamf_factory_get_default (), parent);
}

gchar *
bamf_view_get_view_type (BamfView *view)
{
  BamfViewPrivate *priv;
  char *type = NULL;
  GError *error = NULL;

  g_return_val_if_fail (BAMF_IS_VIEW (view), NULL);
  priv = view->priv;

  if (!dbus_g_proxy_call (priv->proxy,
                          "ViewType",
                          &error,
                          G_TYPE_INVALID,
                          G_TYPE_STRING, &type,
                          G_TYPE_INVALID))
    {
      g_error ("Failed to fetch view type: %s", error->message);
      g_error_free (error);
    }

  return type;
}
