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
#include <libbamf-private/bamf-private.h>

G_DEFINE_TYPE (BamfView, bamf_view, G_TYPE_INITIALLY_UNOWNED);

#define BAMF_VIEW_GET_PRIVATE(o) \
  (G_TYPE_INSTANCE_GET_PRIVATE ((o), BAMF_TYPE_VIEW, BamfViewPrivate))

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
  gchar            *path;
  gchar            *type;
  gchar            *local_icon;
  gchar            *local_name;
  gboolean          is_closed;
  gboolean          sticky;
  GList            *cached_children;
};

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

  if (!bamf_view_remote_ready (view))
    return NULL;

  priv = view->priv;

  if (priv->cached_children)
    return g_list_copy (priv->cached_children);

  if (!bamf_dbus_item_view_call_children_sync (priv->proxy, &children, NULL, &error))
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
      view = bamf_factory_view_for_path (bamf_factory_get_default (), children[i]);
      results = g_list_prepend (results, g_object_ref (view));
    }

  priv->cached_children = results;
  return g_list_copy (priv->cached_children);
}

gboolean
bamf_view_is_closed (BamfView *view)
{
  g_return_val_if_fail (BAMF_IS_VIEW (view), TRUE);

  return view->priv->is_closed;
}

gboolean
bamf_view_is_active (BamfView *view)
{
  g_return_val_if_fail (BAMF_IS_VIEW (view), FALSE);

  if (BAMF_VIEW_GET_CLASS (view)->is_active)
    return BAMF_VIEW_GET_CLASS (view)->is_active (view);

  return bamf_dbus_item_view_get_active (view->priv->proxy);

}

gboolean          
bamf_view_user_visible (BamfView *self)
{
  g_return_val_if_fail (BAMF_IS_VIEW (self), FALSE);
  
  return bamf_dbus_item_view_get_user_visible (self->priv->proxy);
}

gboolean
bamf_view_is_running (BamfView *self)
{
  g_return_val_if_fail (BAMF_IS_VIEW (self), FALSE);
  
  if (BAMF_VIEW_GET_CLASS (self)->is_running)
    return BAMF_VIEW_GET_CLASS (self)->is_running (self);

  return bamf_dbus_item_view_get_running (self->priv->proxy);
}

gboolean
bamf_view_is_urgent (BamfView *self)
{
  g_return_val_if_fail (BAMF_IS_VIEW (self), FALSE);
  
  if (BAMF_VIEW_GET_CLASS (self)->is_urgent)
    return BAMF_VIEW_GET_CLASS (self)->is_urgent (self);

  return bamf_dbus_item_view_get_urgent (self->priv->proxy);
}

void
bamf_view_set_name (BamfView *view, const char *name)
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
bamf_view_set_icon (BamfView *view, const char *icon)
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

  if (!bamf_view_remote_ready (self))
    return g_strdup (priv->local_icon);

  if (!bamf_dbus_item_view_call_icon_sync (priv->proxy, &icon, NULL, &error))
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

  if (!bamf_view_remote_ready (self))
    return g_strdup (priv->local_name);
    
  if (!bamf_dbus_item_view_call_name_sync (priv->proxy, &name, NULL, &error))
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
bamf_view_remote_ready (BamfView *view)
{
  return BAMF_IS_VIEW (view) && view->priv->proxy;
}

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

  if (!bamf_dbus_item_view_call_view_type_sync (priv->proxy, &type, NULL, &error))
    {
      //g_warning ("Failed to fetch view type at %s: %s", dbus_g_proxy_get_path (priv->proxy), error ? error->message : "");
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
bamf_view_on_child_added (BamfDBusItemView *proxy, char *path, BamfView *self)
{
  BamfView *view;
  BamfViewPrivate *priv;

  view = bamf_factory_view_for_path (bamf_factory_get_default (), path);
  priv = self->priv;

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
  view = bamf_factory_view_for_path (bamf_factory_get_default (), path);
  priv = self->priv;

  if (priv->cached_children)
    {
      g_list_free_full (priv->cached_children, g_object_unref);
      priv->cached_children = NULL;
    }

  g_signal_emit (G_OBJECT (self), view_signals[CHILD_REMOVED], 0, view);
}

static void
bamf_view_on_active_changed (BamfDBusItemView *proxy, gboolean active, BamfView *self)
{
  g_signal_emit (G_OBJECT(self), view_signals[ACTIVE_CHANGED], 0, active);
  g_object_notify (G_OBJECT (self), "active");
}

static void
bamf_view_on_name_changed (BamfDBusItemView *proxy,
                           const gchar *old_name,
                           const gchar *new_name,
                           BamfView *self)
{
  g_free (self->priv->local_name);
  self->priv->local_name = g_strdup (new_name);

  g_signal_emit (self, view_signals[NAME_CHANGED], 0, old_name, new_name);
}

static void
bamf_view_on_running_changed (BamfDBusItemView *proxy, gboolean running, BamfView *self)
{
  g_signal_emit (G_OBJECT (self), view_signals[RUNNING_CHANGED], 0, running);
  g_object_notify (G_OBJECT (self), "running");
}

static void
bamf_view_on_urgent_changed (BamfDBusItemView *proxy, gboolean urgent, BamfView *self)
{
  g_signal_emit (G_OBJECT (self), view_signals[URGENT_CHANGED], 0, urgent);
  g_object_notify (G_OBJECT (self), "urgent");
}

static void
bamf_view_on_user_visible_changed (BamfDBusItemView *proxy, gboolean visible, BamfView *self)
{
  g_signal_emit (G_OBJECT (self), view_signals[VISIBLE_CHANGED], 0, visible);
  g_object_notify (G_OBJECT (self), "user-visible");
}

static void
bamf_view_on_closed (BamfDBusItemView *proxy, BamfView *self)
{
  BamfViewPrivate *priv;

  priv = self->priv;
  priv->is_closed = TRUE;

  if (priv->cached_children)
    {
      g_list_free_full (priv->cached_children, g_object_unref);
      priv->cached_children = NULL;
    }

  if (priv->sticky && priv->proxy)
    {
      g_signal_handlers_disconnect_by_func (priv->proxy, bamf_view_on_active_changed, self);
      g_signal_handlers_disconnect_by_func (priv->proxy, bamf_view_on_closed, self);
      g_signal_handlers_disconnect_by_func (priv->proxy, bamf_view_on_child_added, self);
      g_signal_handlers_disconnect_by_func (priv->proxy, bamf_view_on_child_removed, self);
      g_signal_handlers_disconnect_by_func (priv->proxy, bamf_view_on_running_changed, self);
      g_signal_handlers_disconnect_by_func (priv->proxy, bamf_view_on_urgent_changed, self);
      g_signal_handlers_disconnect_by_func (priv->proxy, bamf_view_on_user_visible_changed, self);

      g_object_unref (priv->proxy);
      priv->proxy = NULL;
    }
  
  g_object_ref (self);
  
  // must be emitted before path is cleared as path is utilized in cleanup
  g_signal_emit (G_OBJECT (self), view_signals[CLOSED], 0);

  if (priv->path)
    {
      g_free (priv->path);
      priv->path = NULL;
    }
    
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
        g_value_set_string (value, self->priv->path);
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

  if (priv->proxy)
    {
      g_signal_handlers_disconnect_by_func (priv->proxy, bamf_view_on_active_changed, view);
      g_signal_handlers_disconnect_by_func (priv->proxy, bamf_view_on_closed, view);
      g_signal_handlers_disconnect_by_func (priv->proxy, bamf_view_on_child_added, view);
      g_signal_handlers_disconnect_by_func (priv->proxy, bamf_view_on_child_removed, view);
      g_signal_handlers_disconnect_by_func (priv->proxy, bamf_view_on_running_changed, view);
      g_signal_handlers_disconnect_by_func (priv->proxy, bamf_view_on_urgent_changed, view);
      g_signal_handlers_disconnect_by_func (priv->proxy, bamf_view_on_user_visible_changed, view);

      g_object_unref (priv->proxy);
      priv->proxy = NULL;
    }

  G_OBJECT_CLASS (bamf_view_parent_class)->dispose (object);
}

const char * 
bamf_view_get_path (BamfView *view)
{
  g_return_val_if_fail (BAMF_IS_VIEW (view), NULL);
  
  return view->priv->path;
}

void
bamf_view_reset_flags (BamfView *view)
{
  g_return_if_fail (BAMF_IS_VIEW (view));

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
bamf_view_set_path (BamfView *view, const char *path)
{
  BamfViewPrivate *priv;
  GError *error = NULL;

  g_return_if_fail (BAMF_IS_VIEW (view));
  g_return_if_fail (path);

  priv = view->priv;

  if (g_strcmp0 (priv->path, path) == 0)
    return;

  priv->is_closed = FALSE;

  if (priv->path)
    {
      g_free (priv->path);
    }

  if (priv->proxy)
    {
      g_object_unref (priv->proxy);
    }

  priv->proxy = bamf_dbus_item_view_proxy_new_for_bus_sync (G_BUS_TYPE_SESSION,
                                                            G_DBUS_PROXY_FLAGS_NONE,
                                                            "org.ayatana.bamf",
                                                            path, NULL, &error);

  if (priv->proxy == NULL)
    {
      g_critical ("Unable to get org.ayatana.bamf.view view: %s", error->message);
      g_error_free (error);
      return;
    }

  g_signal_connect (priv->proxy, "active-changed",
                    G_CALLBACK (bamf_view_on_active_changed), view);

  g_signal_connect (priv->proxy, "running-changed",
                    G_CALLBACK (bamf_view_on_running_changed), view);

  g_signal_connect (priv->proxy, "urgent-changed",
                    G_CALLBACK (bamf_view_on_urgent_changed), view);

  g_signal_connect (priv->proxy, "user-visible-changed",
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
                  0, NULL, NULL,
                  bamf_marshal_VOID__STRING_STRING,
                  G_TYPE_NONE, 2,
                  G_TYPE_STRING,
                  G_TYPE_STRING);
}


static void
bamf_view_init (BamfView *self)
{
  g_debug("Making new view: USING MY LIBRARY");
  BamfViewPrivate *priv;

  priv = self->priv = BAMF_VIEW_GET_PRIVATE (self);
  priv->is_closed = TRUE;
}
