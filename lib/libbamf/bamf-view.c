/*
 * Copyright 2010-2012 Canonical Ltd.
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
 * SECTION:bamf-view
 * @short_description: The base class for all views
 *
 * #BamfView is the base class that all views need to derive from.
 */

#if HAVE_CONFIG_H
#include <config.h>
#endif

#include <libbamf-private/bamf-private.h>

#include "bamf-view.h"
#include "bamf-view-private.h"
#include "bamf-factory.h"
#include "bamf-tab.h"
#include "bamf-window.h"
#include "bamf-marshal.h"

G_DEFINE_TYPE (BamfView, bamf_view, G_TYPE_INITIALLY_UNOWNED);

#define BAMF_VIEW_GET_PRIVATE(o) \
  (G_TYPE_INSTANCE_GET_PRIVATE ((o), BAMF_TYPE_VIEW, BamfViewPrivate))

enum
{
  ACTIVE_CHANGED,
  CLOSED,
  CHILD_ADDED,
  CHILD_REMOVED,
  CHILD_MOVED,
  RUNNING_CHANGED,
  URGENT_CHANGED,
  VISIBLE_CHANGED,
  NAME_CHANGED,
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
  BamfDBusItemView *proxy;
  GCancellable     *cancellable;
  gchar            *type;
  gchar            *local_icon;
  gchar            *local_name;
  gboolean          is_closed;
  gboolean          sticky;
  GList            *cached_children;};

static void bamf_view_unset_proxy (BamfView *self);

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

  if (!_bamf_dbus_item_view_call_children_sync (priv->proxy, &children, CANCELLABLE (view), &error))
    {
      g_warning ("Unable to fetch children: %s\n", error ? error->message : "");
      g_error_free (error);
      return NULL;
    }

  if (!children)
    return NULL;

  len = g_strv_length (children);

  for (i = len-1; i >= 0; --i)
    {
      view = _bamf_factory_view_for_path (_bamf_factory_get_default (), children[i]);
      results = g_list_prepend (results, g_object_ref (view));
    }

  priv->cached_children = results;
  return g_list_copy (priv->cached_children);
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

  if (!_bamf_view_remote_ready (view))
    return FALSE;

  return _bamf_dbus_item_view_get_active (view->priv->proxy);
}

/**
 * bamf_view_is_user_visible:
 * @view: a #BamfView
 *
 * Returns: a boolean useful for determining if a particular view is "user visible". User visible
 * is a concept relating to whether or not a window should be shown in a launcher tasklist.
 *
 * Since: 0.4.0
 */
gboolean
bamf_view_is_user_visible (BamfView *self)
{
  g_return_val_if_fail (BAMF_IS_VIEW (self), FALSE);

  if (BAMF_VIEW_GET_CLASS (self)->is_user_visible)
    return BAMF_VIEW_GET_CLASS (self)->is_user_visible (self);

  if (!_bamf_view_remote_ready (self))
    return FALSE;

  return _bamf_dbus_item_view_get_user_visible (self->priv->proxy);
}

/**
 * bamf_view_user_visible: (skip)
 * @view: a #BamfView
 *
 * Returns: a boolean useful for determining if a particular view is "user visible". User visible
 * is a concept relating to whether or not a window should be shown in a launcher tasklist.
 *
 * Deprecated: 0.4.0
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

  if (!_bamf_view_remote_ready (self))
    return FALSE;

  return _bamf_dbus_item_view_get_running (self->priv->proxy);
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

  if (!_bamf_view_remote_ready (self))
    return FALSE;

  return _bamf_dbus_item_view_get_urgent (self->priv->proxy);
}

void
_bamf_view_set_name (BamfView *view, const char *name)
{
  g_return_if_fail (BAMF_IS_VIEW (view));

  if (g_strcmp0 (name, view->priv->local_name) == 0)
    return;

  g_free (view->priv->local_name);
  view->priv->local_name = NULL;

  if (name && name[0] != '\0')
    {
      view->priv->local_name = g_strdup (name);
    }
}

void
_bamf_view_set_icon (BamfView *view, const char *icon)
{
  g_return_if_fail (BAMF_IS_VIEW (view));

  g_free (view->priv->local_icon);
  view->priv->local_icon = NULL;

  if (icon && icon[0] != '\0')
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

  if (BAMF_VIEW_GET_CLASS (view)->set_sticky)
    return BAMF_VIEW_GET_CLASS (view)->set_sticky (view, value);
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

  if (!_bamf_dbus_item_view_call_icon_sync (priv->proxy, &icon, CANCELLABLE (self), &error))
    {
      g_warning ("Failed to fetch icon: %s", error ? error->message : "");
      g_error_free (error);

      return NULL;
    }

  _bamf_view_set_icon (self, icon);
  g_free (icon);

  return g_strdup (priv->local_icon);
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

  if (!_bamf_view_remote_ready (self) || priv->local_name)
    return g_strdup (priv->local_name);

  if (!_bamf_dbus_item_view_call_name_sync (priv->proxy, &name, CANCELLABLE (self), &error))
    {
      g_warning ("Failed to fetch name: %s", error ? error->message : "");
      g_error_free (error);

      return NULL;
    }

  _bamf_view_set_name (self, name);
  g_free (name);

  return g_strdup (priv->local_name);
}

gboolean
_bamf_view_remote_ready (BamfView *view)
{
  if (BAMF_IS_VIEW (view) && G_IS_DBUS_PROXY (view->priv->proxy))
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

  if (!_bamf_view_remote_ready (self))
    return NULL;

  if (!_bamf_dbus_item_view_call_view_type_sync (priv->proxy, &type, CANCELLABLE (self), &error))
    {
      const gchar *path = g_dbus_proxy_get_object_path (G_DBUS_PROXY (priv->proxy));
      g_warning ("Failed to fetch view type at %s: %s", path, error ? error->message : "");
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
bamf_view_on_child_added (BamfDBusItemView *proxy, const char *path, BamfView *self)
{
  BamfView *view;
  BamfViewPrivate *priv;

  view = _bamf_factory_view_for_path (_bamf_factory_get_default (), path);
  priv = self->priv;

  if (BAMF_IS_TAB (view))
    {
      g_signal_connect (view, "notify::xid",
                        G_CALLBACK (bamf_view_child_xid_changed), self);
    }

  if (priv->cached_children)
    {
      g_list_free_full (priv->cached_children, g_object_unref);
      priv->cached_children = NULL;
    }

  g_signal_emit (G_OBJECT (self), view_signals[CHILD_ADDED], 0, view);
}

static void
bamf_view_on_child_removed (BamfDBusItemView *proxy, char *path, BamfView *self)
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
bamf_view_on_active_changed (BamfDBusItemView *proxy, GParamSpec *param, BamfView *self)
{
  gboolean active = _bamf_dbus_item_view_get_active (proxy);
  g_signal_emit (G_OBJECT (self), view_signals[ACTIVE_CHANGED], 0, active);
  g_object_notify (G_OBJECT (self), "active");
}

static void
bamf_view_on_name_changed (BamfDBusItemView *proxy,
                           const gchar *old_name,
                           const gchar *new_name,
                           BamfView *self)
{
  _bamf_view_set_name (self, new_name);
  g_signal_emit (self, view_signals[NAME_CHANGED], 0, old_name, new_name);
}

static void
bamf_view_on_running_changed (BamfDBusItemView *proxy, GParamSpec *param, BamfView *self)
{
  gboolean running = _bamf_dbus_item_view_get_running (proxy);
  g_signal_emit (G_OBJECT (self), view_signals[RUNNING_CHANGED], 0, running);
  g_object_notify (G_OBJECT (self), "running");
}

static void
bamf_view_on_urgent_changed (BamfDBusItemView *proxy, GParamSpec *param, BamfView *self)
{
  gboolean urgent = _bamf_dbus_item_view_get_urgent (proxy);
  g_signal_emit (G_OBJECT (self), view_signals[URGENT_CHANGED], 0, urgent);
  g_object_notify (G_OBJECT (self), "urgent");
}

static void
bamf_view_on_user_visible_changed (BamfDBusItemView *proxy, GParamSpec *param, BamfView *self)
{
  gboolean user_visible = _bamf_dbus_item_view_get_user_visible (proxy);
  g_signal_emit (G_OBJECT (self), view_signals[VISIBLE_CHANGED], 0, user_visible);
  g_object_notify (G_OBJECT (self), "user-visible");
}

GCancellable *
_bamf_view_get_cancellable (BamfView *view)
{
  g_return_val_if_fail (BAMF_IS_VIEW (view), NULL);

  return view->priv->cancellable;
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

      if (closed)
        {
          g_cancellable_cancel (priv->cancellable);
          g_list_free_full (priv->cached_children, g_object_unref);
          priv->cached_children = NULL;
        }
      else
        {
          g_cancellable_reset (priv->cancellable);
        }
    }
}

static void
bamf_view_on_closed (BamfDBusItemView *proxy, BamfView *self)
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
        G_OBJECT_WARN_INVALID_PROPERTY_ID (object, property_id, pspec);
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
        g_value_set_string (value, bamf_view_is_closed (self) ? NULL : _bamf_view_get_path (self));
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
        g_value_set_boolean (value, bamf_view_is_user_visible (self));
        break;

      default:
        G_OBJECT_WARN_INVALID_PROPERTY_ID (object, property_id, pspec);
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

  g_signal_handlers_disconnect_by_data (priv->proxy, self);

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

  if (priv->cancellable)
    {
      g_cancellable_cancel (priv->cancellable);
      g_object_unref (priv->cancellable);
      priv->cancellable = NULL;
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

  if (G_IS_DBUS_PROXY (view->priv->proxy))
    return g_dbus_proxy_get_object_path (G_DBUS_PROXY (view->priv->proxy));

  return NULL;
}

void
_bamf_view_reset_flags (BamfView *view)
{
  gboolean user_visible;
  gboolean active;
  gboolean running;
  gboolean urgent;

  g_return_if_fail (BAMF_IS_VIEW (view));

  user_visible = bamf_view_is_user_visible (view);
  g_signal_emit (G_OBJECT (view), view_signals[VISIBLE_CHANGED], 0, user_visible);
  g_object_notify (G_OBJECT (view), "user-visible");

  active = bamf_view_is_active (view);
  g_signal_emit (G_OBJECT (view), view_signals[ACTIVE_CHANGED], 0, active);
  g_object_notify (G_OBJECT (view), "active");

  running = bamf_view_is_running (view);
  g_signal_emit (G_OBJECT (view), view_signals[RUNNING_CHANGED], 0, running);
  g_object_notify (G_OBJECT (view), "running");

  urgent = bamf_view_is_urgent (view);
  g_signal_emit (G_OBJECT (view), view_signals[URGENT_CHANGED], 0, urgent);
  g_object_notify (G_OBJECT (view), "urgent");
}

void
_bamf_view_set_path (BamfView *view, const char *path)
{
  BamfViewPrivate *priv;
  GError *error = NULL;

  g_return_if_fail (BAMF_IS_VIEW (view));
  g_return_if_fail (path);

  _bamf_view_set_closed (view, FALSE);

  if (g_strcmp0 (_bamf_view_get_path (view), path) == 0)
    {
      // The proxy path has not been changed, no need to unset and re-set it again
      _bamf_view_reset_flags (view);
      return;
    }

  priv = view->priv;
  bamf_view_unset_proxy (view);
  priv->proxy = _bamf_dbus_item_view_proxy_new_for_bus_sync (G_BUS_TYPE_SESSION,
                                                             G_DBUS_PROXY_FLAGS_NONE,
                                                             BAMF_DBUS_SERVICE_NAME,
                                                             path, NULL, &error);
  if (!G_IS_DBUS_PROXY (priv->proxy))
    {
      g_critical ("Unable to get "BAMF_DBUS_SERVICE_NAME" view: %s", error ? error ? error->message : "" : "");
      g_error_free (error);
      return;
    }

  _bamf_view_reset_flags (view);

  g_signal_connect (priv->proxy, "notify::active",
                    G_CALLBACK (bamf_view_on_active_changed), view);

  g_signal_connect (priv->proxy, "notify::running",
                    G_CALLBACK (bamf_view_on_running_changed), view);

  g_signal_connect (priv->proxy, "notify::urgent",
                    G_CALLBACK (bamf_view_on_urgent_changed), view);

  g_signal_connect (priv->proxy, "notify::user-visible",
                    G_CALLBACK (bamf_view_on_user_visible_changed), view);

  g_signal_connect (priv->proxy, "name-changed",
                    G_CALLBACK (bamf_view_on_name_changed), view);

  g_signal_connect (priv->proxy, "child-added",
                    G_CALLBACK (bamf_view_on_child_added), view);

  g_signal_connect (priv->proxy, "child-removed",
                    G_CALLBACK (bamf_view_on_child_removed), view);

  g_signal_connect (priv->proxy, "closed",
                    G_CALLBACK (bamf_view_on_closed), view);

  if (BAMF_VIEW_GET_CLASS (view)->set_path)
    BAMF_VIEW_GET_CLASS (view)->set_path (view, path);
}

static void
bamf_view_class_init (BamfViewClass *klass)
{
  GParamSpec *pspec;
  GObjectClass *obj_class = G_OBJECT_CLASS (klass);

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
    g_signal_new (BAMF_VIEW_SIGNAL_ACTIVE_CHANGED,
                  G_OBJECT_CLASS_TYPE (klass),
                  G_SIGNAL_RUN_FIRST,
                  G_STRUCT_OFFSET (BamfViewClass, active_changed),
                  NULL, NULL,
                  g_cclosure_marshal_VOID__BOOLEAN,
                  G_TYPE_NONE, 1,
                  G_TYPE_BOOLEAN);

  view_signals [CLOSED] =
    g_signal_new (BAMF_VIEW_SIGNAL_CLOSED,
                  G_OBJECT_CLASS_TYPE (klass),
                  G_SIGNAL_RUN_FIRST,
                  G_STRUCT_OFFSET (BamfViewClass, closed),
                  NULL, NULL,
                  g_cclosure_marshal_VOID__VOID,
                  G_TYPE_NONE, 0);

  view_signals [CHILD_ADDED] =
    g_signal_new (BAMF_VIEW_SIGNAL_CHILD_ADDED,
                  G_OBJECT_CLASS_TYPE (klass),
                  G_SIGNAL_RUN_FIRST,
                  G_STRUCT_OFFSET (BamfViewClass, child_added),
                  NULL, NULL,
                  g_cclosure_marshal_VOID__OBJECT,
                  G_TYPE_NONE, 1,
                  BAMF_TYPE_VIEW);

  view_signals [CHILD_REMOVED] =
    g_signal_new (BAMF_VIEW_SIGNAL_CHILD_REMOVED,
                  G_OBJECT_CLASS_TYPE (klass),
                  G_SIGNAL_RUN_FIRST,
                  G_STRUCT_OFFSET (BamfViewClass, child_removed),
                  NULL, NULL,
                  g_cclosure_marshal_VOID__OBJECT,
                  G_TYPE_NONE, 1,
                  BAMF_TYPE_VIEW);

  view_signals [CHILD_MOVED] =
    g_signal_new (BAMF_VIEW_SIGNAL_CHILD_MOVED,
                  G_OBJECT_CLASS_TYPE (klass),
                  G_SIGNAL_RUN_FIRST,
                  G_STRUCT_OFFSET (BamfViewClass, child_moved),
                  NULL, NULL,
                  g_cclosure_marshal_VOID__OBJECT,
                  G_TYPE_NONE, 1,
                  BAMF_TYPE_VIEW);

  view_signals [RUNNING_CHANGED] =
    g_signal_new (BAMF_VIEW_SIGNAL_RUNNING_CHANGED,
                  G_OBJECT_CLASS_TYPE (klass),
                  G_SIGNAL_RUN_FIRST,
                  G_STRUCT_OFFSET (BamfViewClass, running_changed),
                  NULL, NULL,
                  g_cclosure_marshal_VOID__BOOLEAN,
                  G_TYPE_NONE, 1,
                  G_TYPE_BOOLEAN);

  view_signals [URGENT_CHANGED] =
    g_signal_new (BAMF_VIEW_SIGNAL_URGENT_CHANGED,
                  G_OBJECT_CLASS_TYPE (klass),
                  G_SIGNAL_RUN_FIRST,
                  G_STRUCT_OFFSET (BamfViewClass, urgent_changed),
                  NULL, NULL,
                  g_cclosure_marshal_VOID__BOOLEAN,
                  G_TYPE_NONE, 1,
                  G_TYPE_BOOLEAN);

  view_signals [VISIBLE_CHANGED] =
    g_signal_new (BAMF_VIEW_SIGNAL_USER_VISIBLE_CHANGED,
                  G_OBJECT_CLASS_TYPE (klass),
                  G_SIGNAL_RUN_FIRST,
                  G_STRUCT_OFFSET (BamfViewClass, user_visible_changed),
                  NULL, NULL,
                  g_cclosure_marshal_VOID__BOOLEAN,
                  G_TYPE_NONE, 1,
                  G_TYPE_BOOLEAN);

  view_signals [NAME_CHANGED] =
    g_signal_new (BAMF_VIEW_SIGNAL_NAME_CHANGED,
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

  priv = self->priv = BAMF_VIEW_GET_PRIVATE (self);
  priv->cancellable = g_cancellable_new ();
  priv->is_closed = TRUE;
}
