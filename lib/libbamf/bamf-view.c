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
 * SECTION:bamf-view
 * @short_description: The base class for all views
 *
 * #BamfView is the base class that all views need to derive from.
 */

#if HAVE_CONFIG_H
#include <config.h>
#endif

#include "bamf-view.h"
#include "bamf-view-private.h"
#include "bamf-factory.h"
#include "bamf-window.h"
#include "bamf-marshal.h"
#include "bamf-tab.h"

#include <dbus/dbus.h>
#include <dbus/dbus-glib.h>
#include <dbus/dbus-glib-lowlevel.h>

G_DEFINE_TYPE (BamfView, bamf_view, G_TYPE_INITIALLY_UNOWNED);

#define BAMF_VIEW_GET_PRIVATE(o) \
  (G_TYPE_INSTANCE_GET_PRIVATE ((o), BAMF_TYPE_VIEW, BamfViewPrivate))

enum
{
  BAMF_VIEW_RUNNING_FLAG = 1 << 0,
  BAMF_VIEW_URGENT_FLAG  = 1 << 1,
  BAMF_VIEW_VISIBLE_FLAG = 1 << 2,
  BAMF_VIEW_ACTIVE_FLAG  = 1 << 3,
};

enum
{
  ACTIVE_CHANGED,
  CLOSED,
  CHILD_ADDED,
  CHILD_REMOVED,
  RUNNING_CHANGED,
  URGENT_CHANGED,
  VISIBLE_CHANGED,
  NAME_CHANGED,
  CHILD_MOVED,
  LAST_SIGNAL,
};

enum
{
  PROP_0,

  PROP_PATH,
  PROP_RUNNING,
  PROP_ACTIVE,
  PROP_USER_VISIBLE,
  PROP_URGENT,
};

static guint view_signals[LAST_SIGNAL] = { 0 };

struct _BamfViewPrivate
{
  DBusGConnection *connection;
  DBusGProxy      *proxy;
  gchar           *path;
  gchar           *type;
  gchar           *local_icon;
  gchar           *local_name;
  guint            checked_flags;
  guint            set_flags;
  gboolean         is_closed;
  gboolean         sticky;
  GList           *cached_children;
};

static void bamf_view_unset_proxy (BamfView *self);

static void
bamf_view_set_flag (BamfView *view, guint flag, gboolean value)
{
  BamfViewPrivate *priv;
  
  g_return_if_fail (BAMF_IS_VIEW (view));
  
  priv = view->priv;
  
  if (value)
    priv->set_flags |= flag;
  else
    priv->set_flags &= ~flag;
  
  priv->checked_flags |= flag;
}

static gboolean
bamf_view_flag_is_set (BamfView *view, guint flag)
{
  BamfViewPrivate *priv;
  
  g_return_val_if_fail (BAMF_IS_VIEW (view), FALSE);
  
  priv = view->priv;
  
  return priv->checked_flags & flag;
}

static gboolean
bamf_view_get_flag (BamfView *view, guint flag)
{
  BamfViewPrivate *priv;
  
  g_return_val_if_fail (BAMF_IS_VIEW (view), FALSE);
  
  priv = view->priv;
  
  return priv->set_flags & flag;
}

/**
 * bamf_view_get_children:
 * @view: a #BamfView
 *
 * Note: Makes sever dbus calls the first time this is called on a view. Dbus messaging is reduced afterwards.
 *
 * Returns: (element-type Bamf.View) (transfer container): Returns a list of #BamfView which must be
 *           freed after usage. Elements of the list are owned by bamf and should not be unreffed.
 */
GList *
bamf_view_get_children (BamfView *view)
{
  char ** children;
  int i, len;
  GList *results = NULL;
  GError *error = NULL;
  BamfViewPrivate *priv;

  g_return_val_if_fail (BAMF_IS_VIEW (view), NULL);

  if (BAMF_VIEW_GET_CLASS (view)->get_children)
    return BAMF_VIEW_GET_CLASS (view)->get_children (view);

  if (!_bamf_view_remote_ready (view))
    return NULL;

  priv = view->priv;

  if (priv->cached_children)
    return g_list_copy (priv->cached_children);

  if (!dbus_g_proxy_call (priv->proxy,
                          "Children",
                          &error,
                          G_TYPE_INVALID,
                          G_TYPE_STRV, &children,
                          G_TYPE_INVALID))
    {
      g_warning ("Unable to fetch children: %s\n", error->message);
      g_error_free (error);
      return NULL;
    }

  if (!children)
    return NULL;

  len = g_strv_length (children);

  for (i = len-1; i >= 0; i--)
    {
      view = _bamf_factory_view_for_path (_bamf_factory_get_default (), children[i]);
      results = g_list_prepend (results, g_object_ref (view));
    }

  priv->cached_children = results;
  return g_list_copy (priv->cached_children);
}

static gboolean
bamf_view_get_boolean (BamfView *self, const char *method_name, guint flag)
{
  BamfViewPrivate *priv;
  gboolean result = FALSE;
  GError *error = NULL;

  g_return_val_if_fail (BAMF_IS_VIEW (self), FALSE);
  priv = self->priv;

  if (bamf_view_flag_is_set (self, flag))
    return bamf_view_get_flag (self, flag);

  if (!_bamf_view_remote_ready (self))
    return FALSE;

  if (!dbus_g_proxy_call (priv->proxy,
                          method_name,
                          &error,
                          G_TYPE_INVALID,
                          G_TYPE_BOOLEAN, &result,
                          G_TYPE_INVALID))
    {
      g_warning ("Failed to fetch boolean: %s", error->message);
      g_error_free (error);

      return FALSE;
    }

  bamf_view_set_flag (self, flag, result);
  return result;
}

/**
 * bamf_view_is_closed:
 * @view: a #BamfView
 *
 * Determines if the view is closed or not.
 */
gboolean
bamf_view_is_closed (BamfView *view)
{
  g_return_val_if_fail (BAMF_IS_VIEW (view), TRUE);

  return view->priv->is_closed;
}

/**
 * bamf_view_is_active:
 * @view: a #BamfView
 *
 * Determines if the view is currently active and focused by the user. Useful for an active window indicator. 
 */
gboolean
bamf_view_is_active (BamfView *view)
{
  g_return_val_if_fail (BAMF_IS_VIEW (view), FALSE);

  if (BAMF_VIEW_GET_CLASS (view)->is_active)
    return BAMF_VIEW_GET_CLASS (view)->is_active (view);

  return bamf_view_get_boolean (view, "IsActive", BAMF_VIEW_ACTIVE_FLAG);

}

/**
 * bamf_view_is_user_visible:
 * @view: a #BamfView
 *
 * Returns: a boolean useful for determining if a particular view is "user visible". User visible
 * is a concept relating to whether or not a window should be shown in a launcher tasklist.
 *
 * Since: 0.3.4
 */
gboolean
bamf_view_is_user_visible (BamfView *self)
{
  g_return_val_if_fail (BAMF_IS_VIEW (self), FALSE);
  
  return bamf_view_get_boolean (self, "UserVisible", BAMF_VIEW_VISIBLE_FLAG);
}

/**
 * bamf_view_user_visible: (skip)
 * @view: a #BamfView
 *
 * Returns: a boolean useful for determining if a particular view is "user visible". User visible
 * is a concept relating to whether or not a window should be shown in a launcher tasklist.
 *
 * Deprecated: 0.3.4
 */
gboolean
bamf_view_user_visible (BamfView *self)
{
  return bamf_view_is_user_visible (self);
}

/**
 * bamf_view_is_running:
 * @view: a #BamfView
 *
 * Determines if the view is currently running. Useful for a running window indicator. 
 */
gboolean
bamf_view_is_running (BamfView *self)
{
  g_return_val_if_fail (BAMF_IS_VIEW (self), FALSE);
  
  if (BAMF_VIEW_GET_CLASS (self)->is_running)
    return BAMF_VIEW_GET_CLASS (self)->is_running (self);

  return bamf_view_get_boolean (self, "IsRunning", BAMF_VIEW_RUNNING_FLAG);
}

/**
 * bamf_view_is_urgent:
 * @view: a #BamfView
 *
 * Determines if the view is currently requiring attention. Useful for a running window indicator. 
 */
gboolean
bamf_view_is_urgent (BamfView *self)
{
  g_return_val_if_fail (BAMF_IS_VIEW (self), FALSE);
  
  if (BAMF_VIEW_GET_CLASS (self)->is_urgent)
    return BAMF_VIEW_GET_CLASS (self)->is_urgent (self);

  return bamf_view_get_boolean (self, "IsUrgent", BAMF_VIEW_URGENT_FLAG);
}

void
_bamf_view_set_name (BamfView *view, const char *name)
{
  g_return_if_fail (BAMF_IS_VIEW (view));
  
  if (!g_strcmp0 (name, view->priv->local_name))
    return;

  g_free (view->priv->local_name);

  if (name && name[0] == '\0')
    {
      view->priv->local_name = NULL;
    }
  else
    {
      view->priv->local_name = g_strdup (name);
    }
}

void
_bamf_view_set_icon (BamfView *view, const char *icon)
{
  g_return_if_fail (BAMF_IS_VIEW (view));

  g_free (view->priv->local_icon);

  if (icon && icon[0] == '\0')
    {
      view->priv->local_icon = NULL;
    }
  else
    {
      view->priv->local_icon = g_strdup (icon);
    }
}

gboolean 
bamf_view_is_sticky (BamfView *view)
{
  g_return_val_if_fail (BAMF_IS_VIEW (view), FALSE);
  
  return view->priv->sticky;
}

void 
bamf_view_set_sticky (BamfView *view, gboolean value)
{
  g_return_if_fail (BAMF_IS_VIEW (view));
  
  if (value == view->priv->sticky)
    return;

  view->priv->sticky = value;
  
  if (value)
    g_object_ref_sink (view);
  else
    g_object_unref (view);
}

/**
 * bamf_view_get_icon:
 * @view: a #BamfView
 *
 * Gets the icon of a view. This icon is used to visually represent the view. 
 */
gchar *
bamf_view_get_icon (BamfView *self)
{
  BamfViewPrivate *priv;
  char *icon = NULL;
  GError *error = NULL;

  g_return_val_if_fail (BAMF_IS_VIEW (self), NULL);
  priv = self->priv;
  
  if (BAMF_VIEW_GET_CLASS (self)->get_icon)
    return BAMF_VIEW_GET_CLASS (self)->get_icon (self);

  if (!_bamf_view_remote_ready (self))
    return g_strdup (priv->local_icon);

  if (!dbus_g_proxy_call (priv->proxy,
                          "Icon",
                          &error,
                          G_TYPE_INVALID,
                          G_TYPE_STRING, &icon,
                          G_TYPE_INVALID))
    {
      g_warning ("Failed to fetch icon: %s", error->message);
      g_error_free (error);
      
      return NULL;
    }

  if (icon && icon[0] == '\0')
    {
      g_free (icon);
      return NULL;
    }

  return icon;
}

/**
 * bamf_view_get_name:
 * @view: a #BamfView
 *
 * Gets the name of a view. This name is a short name best used to represent the view with text. 
 */
gchar *
bamf_view_get_name (BamfView *self)
{
  BamfViewPrivate *priv;
  char *name = NULL;
  GError *error = NULL;

  g_return_val_if_fail (BAMF_IS_VIEW (self), NULL);
  priv = self->priv;
  
  if (BAMF_VIEW_GET_CLASS (self)->get_name)
    return BAMF_VIEW_GET_CLASS (self)->get_name (self);

  if (!_bamf_view_remote_ready (self))
    return g_strdup (priv->local_name);
    
  if (!dbus_g_proxy_call (priv->proxy,
                          "Name",
                          &error,
                          G_TYPE_INVALID,
                          G_TYPE_STRING, &name,
                          G_TYPE_INVALID))
    {
      g_warning ("Failed to fetch name: %s", error->message);
      g_error_free (error);
      
      return NULL;
    }

  if (name && name[0] == '\0')
    {
      g_free (name);
      return NULL;
    }

  return name;
}

gboolean 
_bamf_view_remote_ready (BamfView *view)
{
  if (BAMF_IS_VIEW (view) && view->priv->proxy)
    return !view->priv->is_closed;

  return FALSE;
}

/**
 * bamf_view_get_view_type:
 * @view: a #BamfView
 *
 * The view type of a window is a short string used to represent all views of the same class. These
 * descriptions should not be used to do casting as they are not considered stable.
 *
 * Virtual: view_type
 */
const gchar *
bamf_view_get_view_type (BamfView *self)
{
  BamfViewPrivate *priv;
  char *type = NULL;
  GError *error = NULL;

  g_return_val_if_fail (BAMF_IS_VIEW (self), NULL);
  priv = self->priv;
  
  if (BAMF_VIEW_GET_CLASS (self)->view_type)
    return BAMF_VIEW_GET_CLASS (self)->view_type (self);
  
  if (priv->type)
    return priv->type;

  if (!dbus_g_proxy_call (priv->proxy,
                          "ViewType",
                          &error,
                          G_TYPE_INVALID,
                          G_TYPE_STRING, &type,
                          G_TYPE_INVALID))
    {
      g_warning ("Failed to fetch view type at %s: %s", dbus_g_proxy_get_path (priv->proxy), error ? error->message : "");
      g_error_free (error);
      return NULL;
    }

  priv->type = type;
  return type;
}

BamfClickBehavior 
bamf_view_get_click_suggestion (BamfView *self)
{
  g_return_val_if_fail (BAMF_IS_VIEW (self), BAMF_CLICK_BEHAVIOR_NONE);

  if (BAMF_VIEW_GET_CLASS (self)->click_behavior)
    return BAMF_VIEW_GET_CLASS (self)->click_behavior (self);
    
  return BAMF_CLICK_BEHAVIOR_NONE;
}

static void
bamf_view_child_xid_changed (GObject *object, GParamSpec *pspec, gpointer user_data)
{
  BamfView *self;
  
  self = (BamfView *)user_data;
  
  g_signal_emit (G_OBJECT (self), view_signals[CHILD_MOVED], 0, BAMF_VIEW (object));
  g_signal_emit (G_OBJECT (self), view_signals[VISIBLE_CHANGED], 0);
}

static void
bamf_view_on_child_added (DBusGProxy *proxy, char *path, BamfView *self)
{
  BamfView *view;
  BamfViewPrivate *priv;

  view = _bamf_factory_view_for_path (_bamf_factory_get_default (), path);
  priv = self->priv;
  
  if (BAMF_IS_TAB (view))
    g_signal_connect (view, "notify::xid", 
                      G_CALLBACK (bamf_view_child_xid_changed), self);

  if (priv->cached_children)
    {
      g_list_free_full (priv->cached_children, g_object_unref);
      priv->cached_children = NULL;
    }

  g_signal_emit (G_OBJECT (self), view_signals[CHILD_ADDED], 0, view);
}

static void
bamf_view_on_child_removed (DBusGProxy *proxy, char *path, BamfView *self)
{
  BamfView *view;
  BamfViewPrivate *priv;
  view = _bamf_factory_view_for_path (_bamf_factory_get_default (), path);
  priv = self->priv;
  
  if (BAMF_IS_TAB (view))
    g_signal_handlers_disconnect_by_func (view, bamf_view_on_child_added, self);

  if (priv->cached_children)
    {
      g_list_free_full (priv->cached_children, g_object_unref);
      priv->cached_children = NULL;
    }

  g_signal_emit (G_OBJECT (self), view_signals[CHILD_REMOVED], 0, view);
}

static void
bamf_view_on_active_changed (DBusGProxy *proxy, gboolean active, BamfView *self)
{
  bamf_view_set_flag (self, BAMF_VIEW_ACTIVE_FLAG, active);

  g_signal_emit (G_OBJECT(self), view_signals[ACTIVE_CHANGED], 0, active);
  g_object_notify (G_OBJECT (self), "active");
}

static void
bamf_view_on_name_changed (DBusGProxy*  proxy,
                           const gchar* old_name,
                           const gchar* new_name,
                           BamfView*    self)
{
  g_free (self->priv->local_name);
  self->priv->local_name = g_strdup (new_name);

  g_signal_emit (self, view_signals[NAME_CHANGED], 0, old_name, new_name);
}

static void
bamf_view_on_running_changed (DBusGProxy *proxy, gboolean running, BamfView *self)
{
  bamf_view_set_flag (self, BAMF_VIEW_RUNNING_FLAG, running);

  g_signal_emit (G_OBJECT (self), view_signals[RUNNING_CHANGED], 0, running);
  g_object_notify (G_OBJECT (self), "running");
}

static void
bamf_view_on_urgent_changed (DBusGProxy *proxy, gboolean urgent, BamfView *self)
{
  bamf_view_set_flag (self, BAMF_VIEW_URGENT_FLAG, urgent);

  g_signal_emit (G_OBJECT (self), view_signals[URGENT_CHANGED], 0, urgent);
  g_object_notify (G_OBJECT (self), "urgent");
}

static void
bamf_view_on_user_visible_changed (DBusGProxy *proxy, gboolean visible, BamfView *self)
{
  bamf_view_set_flag (self, BAMF_VIEW_VISIBLE_FLAG, visible);

  g_signal_emit (G_OBJECT (self), view_signals[VISIBLE_CHANGED], 0, visible);
  g_object_notify (G_OBJECT (self), "user-visible");
}

static void
on_view_proxy_destroyed (GObject *proxy, gpointer user_data)
{
  BamfView *view = user_data;
  g_return_if_fail (BAMF_IS_VIEW (view));

  view->priv->checked_flags = 0x0;
  view->priv->proxy = NULL;

  g_free (view->priv->path);
  view->priv->path = NULL;
}

void
_bamf_view_set_closed (BamfView *view, gboolean closed)
{
  BamfViewPrivate *priv;
  g_return_if_fail (BAMF_IS_VIEW (view));

  priv = view->priv;

  if (priv->is_closed != closed)
    {
      priv->is_closed = closed;

      if (closed && priv->cached_children)
        {
          g_list_free_full (priv->cached_children, g_object_unref);
          priv->cached_children = NULL;
        }
    }
}

static void
bamf_view_on_closed (DBusGProxy *proxy, BamfView *self)
{
  _bamf_view_set_closed (self, TRUE);

  g_object_ref (self);
  g_signal_emit (G_OBJECT (self), view_signals[CLOSED], 0);
  g_object_unref (self);
}

static void
bamf_view_set_property (GObject *object, guint property_id, const GValue *value, GParamSpec *pspec)
{
  switch (property_id)
    {
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
        g_value_set_string (value, self->priv->is_closed ? NULL : self->priv->path);
        break;
      
      case PROP_ACTIVE:
        g_value_set_boolean (value, bamf_view_is_active (self));
        break;
      
      case PROP_RUNNING:
        g_value_set_boolean (value, bamf_view_is_running (self));
        break;
      
      case PROP_URGENT:
        g_value_set_boolean (value, bamf_view_is_urgent (self));
        break;
      
      case PROP_USER_VISIBLE:
        g_value_set_boolean (value, bamf_view_user_visible (self));
        break;
      
      default:
        G_OBJECT_WARN_INVALID_PROPERTY_ID(object, property_id, pspec);
    }
}

static void
bamf_view_unset_proxy (BamfView *self)
{
  BamfViewPrivate *priv;

  g_return_if_fail (BAMF_IS_VIEW (self));
  priv = self->priv;

  if (!priv->proxy)
    return;

  dbus_g_proxy_disconnect_signal (priv->proxy,
                                      "ActiveChanged",
                                      (GCallback) bamf_view_on_active_changed,
                                      self);

      dbus_g_proxy_disconnect_signal (priv->proxy,
                                      "Closed",
                                      (GCallback) bamf_view_on_closed,
                                      self);

      dbus_g_proxy_disconnect_signal (priv->proxy,
                                      "ChildAdded",
                                      (GCallback) bamf_view_on_child_added,
                                      self);

      dbus_g_proxy_disconnect_signal (priv->proxy,
                                      "ChildRemoved",
                                      (GCallback) bamf_view_on_child_removed,
                                      self);

      dbus_g_proxy_disconnect_signal (priv->proxy,
                                      "RunningChanged",
                                      (GCallback) bamf_view_on_running_changed,
                                      self);

      dbus_g_proxy_disconnect_signal (priv->proxy,
                                     "UrgentChanged",
                                     (GCallback) bamf_view_on_urgent_changed,
                                     self);
  
      dbus_g_proxy_disconnect_signal (priv->proxy,
                                     "UserVisibleChanged",
                                     (GCallback) bamf_view_on_user_visible_changed,
                                     self);

      g_signal_handlers_disconnect_by_func (priv->proxy, on_view_proxy_destroyed, self);
      g_object_unref (priv->proxy);
      priv->proxy = NULL;
}

static void
bamf_view_dispose (GObject *object)
{
  BamfView *view;
  BamfViewPrivate *priv;

  view = BAMF_VIEW (object);

  priv = view->priv;

  if (priv->path)
    {
      g_free (priv->path);
      priv->path = NULL;
    }

  if (priv->type)
    {
      g_free (priv->type);
      priv->type = NULL;
    }
 
  if (priv->local_icon)
    {
      g_free (priv->local_icon);
      priv->local_icon = NULL;
    }

  if (priv->local_name)
    {
      g_free (priv->local_name);
      priv->local_name = NULL;
    }

  if (priv->cached_children)
    {
      g_list_free_full (priv->cached_children, g_object_unref);
      priv->cached_children = NULL;
    }

  bamf_view_unset_proxy (view);

  G_OBJECT_CLASS (bamf_view_parent_class)->dispose (object);
}

const char *
_bamf_view_get_path (BamfView *view)
{
  g_return_val_if_fail (BAMF_IS_VIEW (view), NULL);

  return view->priv->path;
}

void
_bamf_view_reset_flags (BamfView *view)
{
  BamfViewPrivate *priv;
  g_return_if_fail (BAMF_IS_VIEW (view));

  priv = view->priv;
  priv->checked_flags = 0x0;

  if (bamf_view_user_visible (view))
    {
      g_signal_emit (G_OBJECT(view), view_signals[VISIBLE_CHANGED], 0, TRUE);
      g_object_notify (G_OBJECT (view), "user-visible");
    }
  
  if (bamf_view_is_active (view))
    {
      g_signal_emit (G_OBJECT(view), view_signals[ACTIVE_CHANGED], 0, TRUE);
      g_object_notify (G_OBJECT (view), "active");
    }
  
  if (bamf_view_is_running (view))
    {
      g_signal_emit (G_OBJECT(view), view_signals[RUNNING_CHANGED], 0, TRUE);
      g_object_notify (G_OBJECT (view), "running");
    }
    
  if (bamf_view_is_urgent (view))
    {
      g_signal_emit (G_OBJECT(view), view_signals[URGENT_CHANGED], 0, TRUE);
      g_object_notify (G_OBJECT (view), "urgent");
    }
}

void
_bamf_view_set_path (BamfView *view, const char *path)
{
  BamfViewPrivate *priv;

  g_return_if_fail (BAMF_IS_VIEW (view));
  g_return_if_fail (path);

  priv = view->priv;

  _bamf_view_set_closed (view, FALSE);

  if (priv->proxy && g_strcmp0 (priv->path, path) == 0)
    {
      // The proxy path has not been changed, no need to unset and re-set it again
      return;
    }

  g_free (priv->path);
  bamf_view_unset_proxy (view);

  priv->path = g_strdup (path);
  priv->proxy = dbus_g_proxy_new_for_name (priv->connection,
                                           "org.ayatana.bamf",
                                           priv->path,
                                           "org.ayatana.bamf.view");

  if (priv->proxy == NULL)
    {
      g_critical ("Unable to get org.ayatana.bamf.view view");
      return;
    }

  g_signal_connect (priv->proxy, "destroy", G_CALLBACK (on_view_proxy_destroyed), view);

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

  dbus_g_proxy_add_signal (priv->proxy,
                           "UrgentChanged",
                           G_TYPE_BOOLEAN,
                           G_TYPE_INVALID);

  dbus_g_proxy_add_signal (priv->proxy,
                           "UserVisibleChanged",
                           G_TYPE_BOOLEAN,
                           G_TYPE_INVALID);

  dbus_g_proxy_add_signal (priv->proxy,
                           "NameChanged",
                           G_TYPE_STRING,
                           G_TYPE_STRING,
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

  dbus_g_proxy_connect_signal (priv->proxy,
                               "UrgentChanged",
                               (GCallback) bamf_view_on_urgent_changed,
                               view,
                               NULL);

  dbus_g_proxy_connect_signal (priv->proxy,
                               "UserVisibleChanged",
                               (GCallback) bamf_view_on_user_visible_changed,
                               view,
                               NULL);

  dbus_g_proxy_connect_signal (priv->proxy,
                               "NameChanged",
                               (GCallback) bamf_view_on_name_changed,
                               view,
                               NULL);

  if (bamf_view_is_sticky (view))
    {
      _bamf_view_reset_flags (view);
    }

  if (BAMF_VIEW_GET_CLASS (view)->set_path)
    BAMF_VIEW_GET_CLASS (view)->set_path (view, path);
}

static void
bamf_view_constructed (GObject *object)
{
  if (G_OBJECT_CLASS (bamf_view_parent_class)->constructed)
    G_OBJECT_CLASS (bamf_view_parent_class)->constructed (object);
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

  pspec = g_param_spec_string ("path", "path", "path", NULL, G_PARAM_READABLE);
  g_object_class_install_property (obj_class, PROP_PATH, pspec);
  
  pspec = g_param_spec_boolean ("active", "active", "active", FALSE, G_PARAM_READABLE);
  g_object_class_install_property (obj_class, PROP_ACTIVE, pspec);

  pspec = g_param_spec_boolean ("urgent", "urgent", "urgent", FALSE, G_PARAM_READABLE);
  g_object_class_install_property (obj_class, PROP_URGENT, pspec);
  
  pspec = g_param_spec_boolean ("running", "running", "running", FALSE, G_PARAM_READABLE);
  g_object_class_install_property (obj_class, PROP_RUNNING, pspec);
  
  pspec = g_param_spec_boolean ("user-visible", "user-visible", "user-visible", FALSE, G_PARAM_READABLE);
  g_object_class_install_property (obj_class, PROP_USER_VISIBLE, pspec);

  g_type_class_add_private (obj_class, sizeof (BamfViewPrivate));

  view_signals [ACTIVE_CHANGED] = 
        g_signal_new ("active-changed",
                      G_OBJECT_CLASS_TYPE (klass),
                      G_SIGNAL_RUN_FIRST,
                      G_STRUCT_OFFSET (BamfViewClass, active_changed), 
                      NULL, NULL,
                      g_cclosure_marshal_VOID__BOOLEAN,
                      G_TYPE_NONE, 1, 
                      G_TYPE_BOOLEAN);

  view_signals [CLOSED] = 
        g_signal_new ("closed",
                      G_OBJECT_CLASS_TYPE (klass),
                      G_SIGNAL_RUN_FIRST,
                      G_STRUCT_OFFSET (BamfViewClass, closed), 
                      NULL, NULL,
                      g_cclosure_marshal_VOID__VOID,
                      G_TYPE_NONE, 0);

  view_signals [CHILD_ADDED] = 
        g_signal_new ("child-added",
                      G_OBJECT_CLASS_TYPE (klass),
                      G_SIGNAL_RUN_FIRST,
                      G_STRUCT_OFFSET (BamfViewClass, child_added), 
                      NULL, NULL,
                      g_cclosure_marshal_VOID__OBJECT,
                      G_TYPE_NONE, 1, 
                      BAMF_TYPE_VIEW);

  view_signals [CHILD_REMOVED] = 
        g_signal_new ("child-removed",
                      G_OBJECT_CLASS_TYPE (klass),
                      G_SIGNAL_RUN_FIRST,
                      G_STRUCT_OFFSET (BamfViewClass, child_removed), 
                      NULL, NULL,
                      g_cclosure_marshal_VOID__OBJECT,
                      G_TYPE_NONE, 1, 
                      BAMF_TYPE_VIEW);
  
  view_signals [CHILD_MOVED] = 
    g_signal_new ("child-moved",
                  G_OBJECT_CLASS_TYPE (klass),
                  G_SIGNAL_RUN_FIRST,
                  G_STRUCT_OFFSET (BamfViewClass, child_moved),
                  NULL, NULL,
                  g_cclosure_marshal_VOID__OBJECT,
                  G_TYPE_NONE, 1,
                  BAMF_TYPE_VIEW);

  view_signals [RUNNING_CHANGED] = 
        g_signal_new ("running-changed",
                      G_OBJECT_CLASS_TYPE (klass),
                      G_SIGNAL_RUN_FIRST,
                      G_STRUCT_OFFSET (BamfViewClass, running_changed), 
                      NULL, NULL,
                      g_cclosure_marshal_VOID__BOOLEAN,
                      G_TYPE_NONE, 1, 
                      G_TYPE_BOOLEAN);

  view_signals [URGENT_CHANGED] = 
        g_signal_new ("urgent-changed",
                      G_OBJECT_CLASS_TYPE (klass),
                      G_SIGNAL_RUN_FIRST,
                      G_STRUCT_OFFSET (BamfViewClass, urgent_changed), 
                      NULL, NULL,
                      g_cclosure_marshal_VOID__BOOLEAN,
                      G_TYPE_NONE, 1, 
                      G_TYPE_BOOLEAN);
  
  view_signals [VISIBLE_CHANGED] = 
        g_signal_new ("user-visible-changed",
                      G_OBJECT_CLASS_TYPE (klass),
                      G_SIGNAL_RUN_FIRST,
                      G_STRUCT_OFFSET (BamfViewClass, user_visible_changed), 
                      NULL, NULL,
                      g_cclosure_marshal_VOID__BOOLEAN,
                      G_TYPE_NONE, 1, 
                      G_TYPE_BOOLEAN);

  view_signals [NAME_CHANGED] =
        g_signal_new ("name-changed",
                      G_OBJECT_CLASS_TYPE (klass),
                      0,
                      G_STRUCT_OFFSET (BamfViewClass, name_changed),
                      NULL, NULL,
                      _bamf_marshal_VOID__STRING_STRING,
                      G_TYPE_NONE, 2,
                      G_TYPE_STRING,
                      G_TYPE_STRING);
}


static void
bamf_view_init (BamfView *self)
{
  BamfViewPrivate *priv;
  GError *error = NULL;

  priv = self->priv = BAMF_VIEW_GET_PRIVATE (self);
  _bamf_view_set_closed (self, TRUE);

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
