//  
//  Copyright (C) 2009 Canonical Ltd.
// 
//  This program is free software: you can redistribute it and/or modify
//  it under the terms of the GNU General Public License as published by
//  the Free Software Foundation, either version 3 of the License, or
//  (at your option) any later version.
// 
//  This program is distributed in the hope that it will be useful,
//  but WITHOUT ANY WARRANTY; without even the implied warranty of
//  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//  GNU General Public License for more details.
// 
//  You should have received a copy of the GNU General Public License
//  along with this program.  If not, see <http://www.gnu.org/licenses/>.
// 

#include "marshal.h"
#include "bamf-view.h"
#include "bamf-view-glue.h"

G_DEFINE_TYPE (BamfView, bamf_view, G_TYPE_OBJECT);
#define BAMF_VIEW_GET_PRIVATE(obj) (G_TYPE_INSTANCE_GET_PRIVATE(obj, \
BAMF_TYPE_VIEW, BamfViewPrivate))

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
  char * name;
  char * path;
  BamfView * parent;
  GList * children;
  gboolean is_active;
  gboolean is_running;
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
bamf_view_closed (BamfView *view)
{
  gboolean emit = TRUE;
  if (BAMF_VIEW_GET_CLASS (view)->closed)
    {
      emit = !BAMF_VIEW_GET_CLASS (view)->closed (view);
    }

  if (emit)
    g_signal_emit (view, view_signals[CLOSED], 0);
}

char * 
bamf_view_get_path (BamfView *view)
{
  g_return_val_if_fail (BAMF_IS_VIEW (view), NULL);

  return view->priv->path;
}

char ** 
bamf_view_get_children_paths (BamfView *view)
{
  char ** paths;
  char *path;
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
  char * added;

  g_return_if_fail (BAMF_IS_VIEW (view));
  g_return_if_fail (BAMF_IS_VIEW (child));

  g_signal_connect (G_OBJECT (child), "closed",
		    (GCallback) bamf_view_handle_child_closed, view);

  view->priv->children = g_list_prepend (view->priv->children, child);

  if (BAMF_VIEW_GET_CLASS (view)->child_added)
    BAMF_VIEW_GET_CLASS (view)->child_added (view, child);
    
  added = bamf_view_get_path (child);
  g_signal_emit (view, view_signals[CHILD_ADDED], 0, added);
}

void
bamf_view_remove_child (BamfView *view,
                        BamfView *child)
{
  char * removed;

  g_return_if_fail (BAMF_IS_VIEW (view));
  g_return_if_fail (BAMF_IS_VIEW (child));

  g_signal_handlers_disconnect_by_func (child, bamf_view_handle_child_closed, view);

  view->priv->children = g_list_remove (view->priv->children, child);

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

  view->priv->name = g_strdup (name);
}

char * 
bamf_view_get_parent_path (BamfView *view)
{
  g_return_val_if_fail (BAMF_IS_VIEW (view), NULL);

  if (!BAMF_IS_VIEW (view->priv->parent))
    return NULL;
  
  return bamf_view_get_path (view->priv->parent);
}

char *
bamf_view_get_view_type (BamfView *view)
{
  g_return_val_if_fail (BAMF_IS_VIEW (view), NULL);
  return g_strdup (BAMF_VIEW_GET_CLASS (view)->view_type (view));
}

char * 
bamf_view_inner_get_view_type (BamfView *view)
{
  g_return_val_if_fail (BAMF_IS_VIEW (view), NULL);
  return "view";
}

char * 
bamf_view_export_on_bus (BamfView *view)
{
  char *path = NULL;
  int  num;
  DBusGConnection *bus;
  GError *error = NULL;
  GList *l = NULL;

  g_return_val_if_fail (BAMF_IS_VIEW (view), NULL);

  if (!view->priv->path)
    {  
      num = g_random_int_range (1000, 9999);
      do
        {
          if (path)
            g_free (path);
          num++;
          path = g_strdup_printf ("%s/%s%i", BAMF_DBUS_PATH, bamf_view_get_view_type (view), num);
        }
      while ((l = g_list_find_custom (BAMF_VIEW_GET_CLASS (view)->names, path, (GCompareFunc) g_strcmp0)) != NULL);

      BAMF_VIEW_GET_CLASS (view)->names = g_list_prepend (BAMF_VIEW_GET_CLASS (view)->names, path);

      view->priv->path = path;

      bus = dbus_g_bus_get (DBUS_BUS_SESSION, &error);
      
      g_return_val_if_fail (bus, NULL);

      dbus_g_connection_register_g_object (bus, path, G_OBJECT (view));
    }  
  
  return view->priv->path;
}

static void
bamf_view_dispose (GObject *object)
{
  DBusGConnection *bus;
  GError *error = NULL;
  GList *l;
  
  BamfView *view = BAMF_VIEW (object);
  BamfViewPrivate *priv = view->priv;
  
  bamf_view_closed (view);

  bus = dbus_g_bus_get (DBUS_BUS_SESSION, &error);

  if (bus && priv->path)
    {
      dbus_g_connection_unregister_g_object (bus, object);
      BAMF_VIEW_GET_CLASS (view)->names = g_list_remove (BAMF_VIEW_GET_CLASS (view)->names, priv->path);
    }

  if (priv->children)
    {
      for (l = priv->children; l; l = l->next)
        {
          bamf_view_remove_child (view, l->data);
        }
      g_list_free (priv->children);
      view->priv->children = NULL;
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
bamf_view_init (BamfView * self)
{
  BamfViewPrivate *priv;
  priv = self->priv = BAMF_VIEW_GET_PRIVATE (self);
}

static void
bamf_view_class_init (BamfViewClass * klass)
{
  GObjectClass *object_class = G_OBJECT_CLASS (klass);

  object_class->dispose = bamf_view_dispose;

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

  view_signals [CHILD_ADDED] = 
  	g_signal_new ("child-added",
  	              G_OBJECT_CLASS_TYPE (klass),
  	              0,
  	              0, NULL, NULL,
  	              g_cclosure_marshal_VOID__STRING,
  	              G_TYPE_NONE, 1,
  	              G_TYPE_STRING);

  view_signals [CHILD_REMOVED] = 
  	g_signal_new ("child-removed",
  	              G_OBJECT_CLASS_TYPE (klass),
  	              0,
  	              0, NULL, NULL,
  	              g_cclosure_marshal_VOID__STRING,
  	              G_TYPE_NONE, 1,
  	              G_TYPE_STRING);

  view_signals [RUNNING_CHANGED] = 
  	g_signal_new ("running-changed",
  	              G_OBJECT_CLASS_TYPE (klass),
  	              0,
  	              0, NULL, NULL,
  	              g_cclosure_marshal_VOID__BOOLEAN,
  	              G_TYPE_NONE, 1,
  	              G_TYPE_BOOLEAN);

  klass->view_type = bamf_view_inner_get_view_type;
}