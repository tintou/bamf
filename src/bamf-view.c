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

#include "bamf-marshal.h"
#include "bamf-view.h"
#include "bamf-view-glue.h"

G_DEFINE_TYPE (BamfView, bamf_view, G_TYPE_OBJECT);
#define BAMF_VIEW_GET_PRIVATE(obj) (G_TYPE_INSTANCE_GET_PRIVATE(obj, \
BAMF_TYPE_VIEW, BamfViewPrivate))

enum
{
  PROP_0,
  
  PROP_ACTIVE,
  PROP_RUNNING,
  PROP_URGENT,
  PROP_USER_VISIBLE,
};

enum
{
  ACTIVE_CHANGED,
  CLOSED,
  CLOSED_INTERNAL,
  CHILD_ADDED,
  CHILD_REMOVED,
  EXPORTED,
  RUNNING_CHANGED,
  URGENT_CHANGED,
  USER_VISIBLE_CHANGED,
  NAME_CHANGED,

  LAST_SIGNAL,
};

static guint view_signals[LAST_SIGNAL] = { 0 };

struct _BamfViewPrivate
{
  char * name;
  char * path;
  GList * children;
  GList * parents;
  gboolean closed;
  gboolean is_active;
  gboolean is_running;
  gboolean is_urgent;
  gboolean user_visible;
};

static void
bamf_view_active_changed (BamfView *view, gboolean active)
{
  gboolean emit = TRUE;
  if (BAMF_VIEW_GET_CLASS (view)->active_changed)
    {
      emit = !BAMF_VIEW_GET_CLASS (view)->active_changed (view, active);
    }

  if (emit)
    g_signal_emit (view, view_signals[ACTIVE_CHANGED], 0, active);

}

static void
bamf_view_name_changed (BamfView*    view,
                        const gchar* old_name,
                        const gchar* new_name)
{
  g_signal_emit (view, view_signals[NAME_CHANGED], 0, old_name, new_name);

  if (view->priv->name)
    g_free (view->priv->name);

  view->priv->name = g_strdup (new_name);
}

static void
bamf_view_user_visible_changed (BamfView *view, gboolean user_visible)
{
  gboolean emit = TRUE;
  if (BAMF_VIEW_GET_CLASS (view)->user_visible_changed)
    {
      emit = !BAMF_VIEW_GET_CLASS (view)->user_visible_changed (view, user_visible);
    }

  if (emit)
    g_signal_emit (view, view_signals[USER_VISIBLE_CHANGED], 0, user_visible);
}

static void
bamf_view_running_changed (BamfView *view, gboolean running)
{
  gboolean emit = TRUE;
  if (BAMF_VIEW_GET_CLASS (view)->running_changed)
    {
      emit = !BAMF_VIEW_GET_CLASS (view)->running_changed (view, running);
    }

  if (emit)
    g_signal_emit (view, view_signals[RUNNING_CHANGED], 0, running);
}

static void
bamf_view_urgent_changed (BamfView *view, gboolean urgent)
{
  gboolean emit = TRUE;
  if (BAMF_VIEW_GET_CLASS (view)->urgent_changed)
    {
      emit = !BAMF_VIEW_GET_CLASS (view)->urgent_changed (view, urgent);
    }

  if (emit)
    g_signal_emit (view, view_signals[URGENT_CHANGED], 0, urgent);
}

void
bamf_view_close (BamfView *view)
{
  BamfViewPrivate *priv;
  gboolean emit = TRUE;
  GList *l;
  
  priv = view->priv;

  if (priv->closed)
    return;
    
  priv->closed = TRUE;

  if (BAMF_VIEW_GET_CLASS (view)->closed)
    {
      emit = !BAMF_VIEW_GET_CLASS (view)->closed (view);
    }
  
  if (priv->children)
    {
      for (l = priv->children; l; l = l->next)
        {
          if (BAMF_IS_VIEW (l->data))
            bamf_view_remove_child (view, l->data);
        }
      g_list_free (priv->children);
      priv->children = NULL;
    }

  if (emit)
    {
      g_object_ref (G_OBJECT (view));
      g_signal_emit (view, view_signals[CLOSED_INTERNAL], 0);
      g_signal_emit (view, view_signals[CLOSED], 0);
      g_object_unref (G_OBJECT (view));
    }
}

const char *
bamf_view_get_path (BamfView *view)
{
  g_return_val_if_fail (BAMF_IS_VIEW (view), NULL);

  return view->priv->path;
}

char **
bamf_view_get_children_paths (BamfView *view)
{
  char ** paths;
  const char *path;
  int n_items, i;
  GList *child;
  BamfView *cview;

  g_return_val_if_fail (BAMF_IS_VIEW (view), NULL);

  n_items = g_list_length (view->priv->children);

  paths = g_malloc0 (sizeof (char *) * (n_items + 1));

  i = 0;
  for (child = view->priv->children; child; child = child->next)
    {
      cview = child->data;
      path = bamf_view_get_path (cview);

      if (!path)
        continue;

      paths[i] = g_strdup (path);
      i++;
    }

  return paths;
}

GList *
bamf_view_get_children (BamfView *view)
{
  g_return_val_if_fail (BAMF_IS_VIEW (view), NULL);

  return view->priv->children;
}

char **
bamf_view_get_parent_paths (BamfView *view)
{
  char ** paths;
  const char *path;
  int n_items, i;
  GList *parent;
  BamfView *pview;

  g_return_val_if_fail (BAMF_IS_VIEW (view), NULL);

  n_items = g_list_length (view->priv->parents);

  paths = g_malloc0 (sizeof (char *) * (n_items + 1));

  i = 0;
  for (parent = view->priv->parents; parent; parent = parent->next)
    {
      pview = parent->data;
      path = bamf_view_get_path (pview);

      if (!path)
        continue;

      paths[i] = g_strdup (path);
      i++;
    }

  return paths;
}

GList *
bamf_view_get_parents (BamfView *view)
{
  g_return_val_if_fail (BAMF_IS_VIEW (view), NULL);

  return view->priv->parents;
}

static void
bamf_view_handle_child_closed (BamfView *child,
                               BamfView *view)
{
  bamf_view_remove_child (view, child);
}

void
bamf_view_add_child (BamfView *view,
                     BamfView *child)
{
  const char * added;

  g_return_if_fail (BAMF_IS_VIEW (view));
  g_return_if_fail (BAMF_IS_VIEW (child));

  g_signal_connect (G_OBJECT (child), "closed-internal",
		    (GCallback) bamf_view_handle_child_closed, view);

  /* Make sure our parent child lists are ok, pay attention to whose list you add parents to */
  view->priv->children = g_list_prepend (view->priv->children, child);
  child->priv->parents = g_list_prepend (child->priv->parents, view);

  // Do this by hand so we can pass and object instead of a string
  if (BAMF_VIEW_GET_CLASS (view)->child_added)
    BAMF_VIEW_GET_CLASS (view)->child_added (view, child);

  added = bamf_view_get_path (child);
  g_signal_emit (view, view_signals[CHILD_ADDED], 0, added);
}

void
bamf_view_remove_child (BamfView *view,
                        BamfView *child)
{
  const char * removed;

  g_return_if_fail (BAMF_IS_VIEW (view));
  g_return_if_fail (BAMF_IS_VIEW (child));

  g_signal_handlers_disconnect_by_func (child, bamf_view_handle_child_closed, view);

  /* Make sure our parent child lists are ok, pay attention to whose list you add parents to */
  view->priv->children = g_list_remove (view->priv->children, child);
  child->priv->parents = g_list_remove (child->priv->parents, view);

  // Do this by hand so we can pass and object instead of a string
  if (BAMF_VIEW_GET_CLASS (view)->child_removed)
    BAMF_VIEW_GET_CLASS (view)->child_removed (view, child);

  removed = bamf_view_get_path (child);
  g_signal_emit (view, view_signals[CHILD_REMOVED], 0, removed);
}

gboolean
bamf_view_is_active (BamfView *view)
{
  g_return_val_if_fail (BAMF_IS_VIEW (view), FALSE);

  return view->priv->is_active;
}

void
bamf_view_set_active (BamfView *view,
                      gboolean active)
{
  g_return_if_fail (BAMF_IS_VIEW (view));

  if (active == view->priv->is_active)
    return;

  view->priv->is_active = active;
  bamf_view_active_changed (view, active);
}

gboolean
bamf_view_is_urgent (BamfView *view)
{
  g_return_val_if_fail (BAMF_IS_VIEW (view), FALSE);

  return view->priv->is_urgent;
}

void
bamf_view_set_urgent (BamfView *view,
                       gboolean urgent)
{
  g_return_if_fail (BAMF_IS_VIEW (view));

  if (urgent == view->priv->is_urgent)
    return;

  view->priv->is_urgent = urgent;
  bamf_view_urgent_changed (view, urgent);
}

gboolean
bamf_view_is_running (BamfView *view)
{
  g_return_val_if_fail (BAMF_IS_VIEW (view), FALSE);

  return view->priv->is_running;
}

void
bamf_view_set_running (BamfView *view,
                       gboolean running)
{
  g_return_if_fail (BAMF_IS_VIEW (view));

  if (running == view->priv->is_running)
    return;

  view->priv->is_running = running;
  bamf_view_running_changed (view, running);
}

gboolean
bamf_view_user_visible (BamfView *view)
{
  g_return_val_if_fail (BAMF_IS_VIEW (view), FALSE);

  return view->priv->user_visible;
}

void
bamf_view_set_user_visible (BamfView *view, gboolean user_visible)
{
  g_return_if_fail (BAMF_IS_VIEW (view));

  if (user_visible == view->priv->user_visible)
    return;

  view->priv->user_visible = user_visible;
  bamf_view_user_visible_changed (view, user_visible);
}

char *
bamf_view_get_icon (BamfView *view)
{
  g_return_val_if_fail (BAMF_IS_VIEW (view), NULL);

  if (BAMF_VIEW_GET_CLASS (view)->get_icon)
    return BAMF_VIEW_GET_CLASS (view)->get_icon (view);

  return NULL;
}

char *
bamf_view_get_name (BamfView *view)
{
  g_return_val_if_fail (BAMF_IS_VIEW (view), NULL);

  return g_strdup (view->priv->name);
}

void
bamf_view_set_name (BamfView *view,
                    const char * name)
{
  g_return_if_fail (BAMF_IS_VIEW (view));

  if (!g_strcmp0 (name, view->priv->name))
    return;

  bamf_view_name_changed (view, view->priv->name, (gchar*) name);
}

char *
bamf_view_get_view_type (BamfView *view)
{
  g_return_val_if_fail (BAMF_IS_VIEW (view), NULL);

  if (BAMF_VIEW_GET_CLASS (view)->view_type)
    return BAMF_VIEW_GET_CLASS (view)->view_type (view);

  return g_strdup ("view");
}

char *
bamf_view_get_stable_bus_name (BamfView *view)
{
  g_return_val_if_fail (BAMF_IS_VIEW (view), NULL);

  if (BAMF_VIEW_GET_CLASS (view)->stable_bus_name)
    return BAMF_VIEW_GET_CLASS (view)->stable_bus_name (view);
    
  return g_strdup_printf ("view%p", view);
}

char *
bamf_view_export_on_bus (BamfView *view)
{
  char *path = NULL;
  DBusGConnection *bus;
  GError *error = NULL;

  g_return_val_if_fail (BAMF_IS_VIEW (view), NULL);

  if (!view->priv->path)
    {
      char *stable_name = bamf_view_get_stable_bus_name (view);
      path = g_strdup_printf ("%s/%s", BAMF_DBUS_PATH, stable_name);
      g_free (stable_name);

      BAMF_VIEW_GET_CLASS (view)->names = g_list_prepend (BAMF_VIEW_GET_CLASS (view)->names, path);

      view->priv->path = path;

      bus = dbus_g_bus_get (DBUS_BUS_SESSION, &error);

      g_return_val_if_fail (bus, NULL);

      dbus_g_connection_register_g_object (bus, path, G_OBJECT (view));

      g_signal_emit (view, view_signals[EXPORTED], 0);
    }

  return view->priv->path;
}

gboolean
bamf_view_is_on_bus (BamfView *view)
{
  g_return_val_if_fail (BAMF_IS_VIEW (view), FALSE);

  return view->priv->path != NULL;
}

static void
bamf_view_dispose (GObject *object)
{
  DBusGConnection *bus;
  GError *error = NULL;

  BamfView *view = BAMF_VIEW (object);
  BamfViewPrivate *priv = view->priv;

  bus = dbus_g_bus_get (DBUS_BUS_SESSION, &error);

  if (bus && priv->path)
    {
      dbus_g_connection_unregister_g_object (bus, object);
      BAMF_VIEW_GET_CLASS (view)->names = g_list_remove (BAMF_VIEW_GET_CLASS (view)->names, priv->path);
    }

  if (priv->name)
    {
      g_free (priv->name);
      priv->name = NULL;
    }

  if (priv->path)
    {
      g_free (priv->path);
      priv->path = NULL;
    }

  G_OBJECT_CLASS (bamf_view_parent_class)->dispose (object);
}

static void
bamf_view_get_property (GObject *object, guint property_id, GValue *value, GParamSpec *pspec)
{
  BamfView *self;

  self = BAMF_VIEW (object);

  switch (property_id)
    {
      case PROP_ACTIVE:
        g_value_set_boolean (value, self->priv->is_active);
        break;
      case PROP_URGENT:
        g_value_set_boolean (value, self->priv->is_urgent);
        break;
      case PROP_USER_VISIBLE:
        g_value_set_boolean (value, self->priv->user_visible);
        break;
      case PROP_RUNNING:
        g_value_set_boolean (value, self->priv->is_running);
        break;
      default:
        G_OBJECT_WARN_INVALID_PROPERTY_ID(object, property_id, pspec);
    }
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
bamf_view_init (BamfView * self)
{
  self->priv = BAMF_VIEW_GET_PRIVATE (self);
}

static void
bamf_view_class_init (BamfViewClass * klass)
{
  GParamSpec *pspec;
  GObjectClass *object_class = G_OBJECT_CLASS (klass);

  object_class->dispose = bamf_view_dispose;
  object_class->get_property = bamf_view_get_property;
  object_class->set_property = bamf_view_set_property;
  
  pspec = g_param_spec_boolean ("active", "active", "active", FALSE, G_PARAM_READABLE);
  g_object_class_install_property (object_class, PROP_ACTIVE, pspec);

  pspec = g_param_spec_boolean ("urgent", "urgent", "urgent", FALSE, G_PARAM_READABLE);
  g_object_class_install_property (object_class, PROP_URGENT, pspec);
  
  pspec = g_param_spec_boolean ("running", "running", "running", FALSE, G_PARAM_READABLE);
  g_object_class_install_property (object_class, PROP_RUNNING, pspec);
  
  pspec = g_param_spec_boolean ("user-visible", "user-visible", "user-visible", FALSE, G_PARAM_READABLE);
  g_object_class_install_property (object_class, PROP_USER_VISIBLE, pspec);

  dbus_g_object_type_install_info (BAMF_TYPE_VIEW,
				   &dbus_glib_bamf_view_object_info);

  g_type_class_add_private (klass, sizeof (BamfViewPrivate));

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
  
  view_signals [CLOSED_INTERNAL] =
  	g_signal_new ("closed-internal",
  	              G_OBJECT_CLASS_TYPE (klass),
  	              0,
  	              0, NULL, NULL,
  	              g_cclosure_marshal_VOID__VOID,
  	              G_TYPE_NONE, 0);

  view_signals [CHILD_ADDED] =
  	g_signal_new ("child-added",
  	              G_OBJECT_CLASS_TYPE (klass),
  	              0,
  	              0,
  	              NULL, NULL,
  	              g_cclosure_marshal_VOID__STRING,
  	              G_TYPE_NONE, 1,
  	              G_TYPE_STRING);

  view_signals [CHILD_REMOVED] =
  	g_signal_new ("child-removed",
  	              G_OBJECT_CLASS_TYPE (klass),
  	              0,
  	              0,
  	              NULL, NULL,
  	              g_cclosure_marshal_VOID__STRING,
  	              G_TYPE_NONE, 1,
  	              G_TYPE_STRING);

  view_signals [EXPORTED] =
  	g_signal_new ("exported",
  	              G_OBJECT_CLASS_TYPE (klass),
  	              0,
  	              0, NULL, NULL,
  	              g_cclosure_marshal_VOID__VOID,
  	              G_TYPE_NONE, 0);

  view_signals [RUNNING_CHANGED] =
  	g_signal_new ("running-changed",
  	              G_OBJECT_CLASS_TYPE (klass),
  	              0,
  	              0, NULL, NULL,
  	              g_cclosure_marshal_VOID__BOOLEAN,
  	              G_TYPE_NONE, 1,
  	              G_TYPE_BOOLEAN);

  view_signals [URGENT_CHANGED] =
  	g_signal_new ("urgent-changed",
  	              G_OBJECT_CLASS_TYPE (klass),
  	              0,
  	              0, NULL, NULL,
  	              g_cclosure_marshal_VOID__BOOLEAN,
  	              G_TYPE_NONE, 1,
  	              G_TYPE_BOOLEAN);

  view_signals [USER_VISIBLE_CHANGED] =
  	g_signal_new ("user-visible-changed",
  	              G_OBJECT_CLASS_TYPE (klass),
  	              0,
  	              0, NULL, NULL,
  	              g_cclosure_marshal_VOID__BOOLEAN,
  	              G_TYPE_NONE, 1,
  	              G_TYPE_BOOLEAN);

  view_signals [NAME_CHANGED] =
  	g_signal_new ("name-changed",
  	              G_OBJECT_CLASS_TYPE (klass),
  	              0,
  	              0, NULL, NULL,
  	              bamf_marshal_VOID__STRING_STRING,
  	              G_TYPE_NONE, 2,
                      G_TYPE_STRING,
                      G_TYPE_STRING);

}
