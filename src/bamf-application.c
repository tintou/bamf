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


#include "bamf-application.h"
#include "bamf-application-glue.h"
#include "bamf-window.h"
#include "bamf-legacy-window.h"
#include "bamf-legacy-screen.h"
#include <string.h>
#include <gio/gdesktopappinfo.h>

G_DEFINE_TYPE (BamfApplication, bamf_application, BAMF_TYPE_VIEW);
#define BAMF_APPLICATION_GET_PRIVATE(obj) (G_TYPE_INSTANCE_GET_PRIVATE(obj, \
BAMF_TYPE_APPLICATION, BamfApplicationPrivate))

enum
{
  WINDOW_ADDED,
  WINDOW_REMOVED,

  LAST_SIGNAL,
};

static guint application_signals[LAST_SIGNAL] = { 0 };

struct _BamfApplicationPrivate
{
  char * desktop_file;
  char * app_type;
  char * icon;
  gboolean is_tab_container;
};

static char *
bamf_application_get_icon (BamfView *view)
{
  g_return_val_if_fail (BAMF_IS_APPLICATION (view), NULL);

  return BAMF_APPLICATION (view)->priv->icon;
}

char *
bamf_application_get_application_type (BamfApplication *application)
{
  g_return_val_if_fail (BAMF_IS_APPLICATION (application), NULL);

  return g_strdup (application->priv->app_type);
}

char *
bamf_application_get_desktop_file (BamfApplication *application)
{
  g_return_val_if_fail (BAMF_IS_APPLICATION (application), NULL);

  return g_strdup (application->priv->desktop_file);
}

static gboolean
icon_name_is_valid (char *name)
{
  GtkIconTheme *icon_theme;
  
  g_return_val_if_fail (name, FALSE);
  
  icon_theme = gtk_icon_theme_get_default ();
  return gtk_icon_theme_has_icon (icon_theme, name);
}

static void
bamf_application_setup_icon_and_name (BamfApplication *self)
{
  BamfView *view;
  GDesktopAppInfo *desktop;
  GIcon *gicon;
  GList *children, *l;
  const char *class;
  char *icon = NULL, *name = NULL;

  g_return_if_fail (BAMF_IS_APPLICATION (self));
  
  if (self->priv->icon && bamf_view_get_name (BAMF_VIEW (self)))
    return;

  if (self->priv->desktop_file)
    {
      desktop = g_desktop_app_info_new_from_filename (self->priv->desktop_file);

      if (!G_IS_APP_INFO (desktop))
        return;

      gicon = g_app_info_get_icon (G_APP_INFO (desktop));

      name = g_strdup (g_app_info_get_display_name (G_APP_INFO (desktop)));
      icon = g_icon_to_string (gicon);

      g_object_unref (desktop);
    }
  else if ((children = bamf_view_get_children (BAMF_VIEW (self))) != NULL)
    {
      for (l = children; l; l = l->next)
        {
          view = l->data;
          if (BAMF_IS_WINDOW (view))
            {
              do
                {
                  class = bamf_legacy_window_get_class_name (bamf_window_get_window (BAMF_WINDOW (view)));
                  icon = g_utf8_strdown (class, -1);
                  
                  if (icon_name_is_valid (icon))
                    break;
                  
                  g_free (icon);
                  icon = bamf_legacy_window_get_exec_string (bamf_window_get_window (BAMF_WINDOW (view)));
                  
                  if (icon_name_is_valid (icon))
                    break;
                  
                  g_free (icon);
                  icon = g_strdup ("application-default-icon");
                }
              while (FALSE);
              
              name = g_strdup (bamf_legacy_window_get_class_name (bamf_window_get_window (BAMF_WINDOW (view))));
              break;
            }
        }
    }
  else
    {
      /* we do nothing as we have nothing to go on */
    }
  
  if (icon)
    self->priv->icon = icon;
  
  if (name)
    bamf_view_set_name (BAMF_VIEW (self), name);
  
  g_free (name);
}

void
bamf_application_set_desktop_file (BamfApplication *application,
                                   char * desktop_file)
{
  g_return_if_fail (BAMF_IS_APPLICATION (application));

  application->priv->desktop_file = g_strdup (desktop_file);
  
  bamf_application_setup_icon_and_name (application);
}

GArray *
bamf_application_get_xids (BamfApplication *application)
{
  GList *l;
  GArray *xids;
  BamfView *view;
  guint32 xid;

  g_return_val_if_fail (BAMF_IS_APPLICATION (application), NULL);

  xids = g_array_new (FALSE, TRUE, sizeof (guint32));

  for (l = bamf_view_get_children (BAMF_VIEW (application)); l; l = l->next)
    {
      view = l->data;

      if (!BAMF_IS_WINDOW (view))
        continue;

      xid = bamf_window_get_xid (BAMF_WINDOW (view));

      g_array_append_val (xids, xid);
    }

  return xids;
}

gboolean          
bamf_application_contains_similar_to_window (BamfApplication *self, 
                                             BamfWindow *window)
{
  gboolean result = FALSE;
  const char *class, *owned_class;
  GList *children, *l;
  BamfView *child;
  
  g_return_val_if_fail (BAMF_IS_APPLICATION (self), FALSE);
  g_return_val_if_fail (BAMF_IS_WINDOW (window), FALSE);
  
  class = bamf_legacy_window_get_class_name (bamf_window_get_window (window));
  
  children = bamf_view_get_children (BAMF_VIEW (self));
  for (l = children; l; l = l->next)
    {
      child = l->data;
      
      if (!BAMF_IS_WINDOW (child))
        continue;
      
      owned_class = bamf_legacy_window_get_class_name (bamf_window_get_window (BAMF_WINDOW (child)));
      
      if (g_strcmp0 (class, owned_class) == 0)
        {
          result = TRUE;
          break;
        }
    }
  
  return result;
}

gboolean
bamf_application_manages_xid (BamfApplication *application,
                              guint32 xid)
{
  GArray *xids;
  int i;
  gboolean result = FALSE;

  g_return_val_if_fail (BAMF_IS_APPLICATION (application), FALSE);

  xids = bamf_application_get_xids (application);

  for (i = 0; i < xids->len; i++)
    {
      if (g_array_index (xids, guint32, i) == xid)
        {
          result = TRUE;
          break;
        }
    }

  g_array_free (xids, TRUE);

  return result;
}

static char *
bamf_application_get_view_type (BamfView *view)
{
  return g_strdup ("application");
}

static void
bamf_application_ensure_flags (BamfApplication *self)
{
  gboolean urgent = FALSE, visible = FALSE, running = FALSE, active = FALSE;
  GList *l;
  BamfView *view;

  for (l = bamf_view_get_children (BAMF_VIEW (self)); l; l = l->next)
    {
      view = l->data;

      if (!BAMF_IS_WINDOW (view))
        continue;

      running = TRUE;

      if (bamf_view_is_urgent (view))
        urgent = TRUE;
      if (bamf_view_user_visible (view))
        visible = TRUE;
      if (bamf_view_is_active (view))
        active = TRUE;

      if (urgent && visible && active)
        break;
    }

  bamf_view_set_urgent       (BAMF_VIEW (self), urgent);
  bamf_view_set_user_visible (BAMF_VIEW (self), visible);
  bamf_view_set_running      (BAMF_VIEW (self), running);
  bamf_view_set_active       (BAMF_VIEW (self), active);
}

static void
view_active_changed (BamfView *view, gboolean active, BamfApplication *self)
{
  bamf_application_ensure_flags (self);
}

static void
view_urgent_changed (BamfView *view, gboolean urgent, BamfApplication *self)
{
  bamf_application_ensure_flags (self);
}

static void
view_visible_changed (BamfView *view, gboolean visible, BamfApplication *self)
{
  bamf_application_ensure_flags (self);
}

static void
view_exported (BamfView *view, BamfApplication *self)
{
  g_signal_emit (self, application_signals[WINDOW_ADDED], 0, bamf_view_get_path (view));
}

static void
bamf_application_child_added (BamfView *view, BamfView *child)
{
  BamfApplication *application;

  application = BAMF_APPLICATION (view);

  if (BAMF_IS_WINDOW (child))
    {
      if (bamf_view_is_on_bus (child))
        g_signal_emit (BAMF_APPLICATION (view), application_signals[WINDOW_ADDED], 0, bamf_view_get_path (child));
      else
        g_signal_connect (G_OBJECT (child), "exported",
                          (GCallback) view_exported, view);
    }

  // If we're not a real application, get some properties from our first child
  if (application->priv->desktop_file == NULL)
    {
      if (bamf_view_get_name (view) == NULL)
        bamf_view_set_name (view, bamf_view_get_name (child));
    }

  g_signal_connect (G_OBJECT (child), "active-changed",
                    (GCallback) view_active_changed, view);
  g_signal_connect (G_OBJECT (child), "urgent-changed",
                    (GCallback) view_urgent_changed, view);
  g_signal_connect (G_OBJECT (child), "user-visible-changed",
                    (GCallback) view_visible_changed, view);

  bamf_application_ensure_flags (BAMF_APPLICATION (view));
  
  bamf_application_setup_icon_and_name (application);
}

static gboolean
on_empty (GObject *object)
{
  g_object_unref (object);
  return FALSE;
}

static void
bamf_application_child_removed (BamfView *view, BamfView *child)
{
  BamfApplication *application;

  application = BAMF_APPLICATION (view);

  if (BAMF_IS_WINDOW (child))
    {
      if (bamf_view_is_on_bus (child))
        g_signal_emit (BAMF_APPLICATION (view), application_signals[WINDOW_REMOVED],0, bamf_view_get_path (child));
    }

  g_signal_handlers_disconnect_by_func (G_OBJECT (child), view_active_changed, view);
  g_signal_handlers_disconnect_by_func (G_OBJECT (child), view_urgent_changed, view);
  g_signal_handlers_disconnect_by_func (G_OBJECT (child), view_visible_changed, view);

  bamf_application_ensure_flags (BAMF_APPLICATION (view));

  if (g_list_length (bamf_view_get_children (view)) == 0)
    {
      g_idle_add ((GSourceFunc) on_empty, view);
    }
}

static void
bamf_application_dispose (GObject *object)
{
  BamfApplication *app;
  BamfApplicationPrivate *priv;

  app = BAMF_APPLICATION (object);
  priv = app->priv;

  if (priv->desktop_file)
    {
      g_free (priv->desktop_file);
      priv->desktop_file = NULL;
    }

  if (priv->app_type)
    {
      g_free (priv->app_type);
      priv->app_type = NULL;
    }

  G_OBJECT_CLASS (bamf_application_parent_class)->dispose (object);

}

static void
bamf_application_init (BamfApplication * self)
{
  BamfApplicationPrivate *priv;
  priv = self->priv = BAMF_APPLICATION_GET_PRIVATE (self);

  priv->is_tab_container = FALSE;
  priv->app_type = g_strdup ("system");
}

static void
bamf_application_class_init (BamfApplicationClass * klass)
{
  GObjectClass *object_class = G_OBJECT_CLASS (klass);
  BamfViewClass *view_class = BAMF_VIEW_CLASS (klass);

  object_class->dispose = bamf_application_dispose;

  view_class->view_type = bamf_application_get_view_type;
  view_class->child_added = bamf_application_child_added;
  view_class->child_removed = bamf_application_child_removed;
  view_class->get_icon = bamf_application_get_icon;

  g_type_class_add_private (klass, sizeof (BamfApplicationPrivate));

  dbus_g_object_type_install_info (BAMF_TYPE_APPLICATION,
				   &dbus_glib_bamf_application_object_info);

  application_signals [WINDOW_ADDED] =
  	g_signal_new ("window-added",
  	              G_OBJECT_CLASS_TYPE (klass),
  	              0,
  	              0, NULL, NULL,
  	              g_cclosure_marshal_VOID__STRING,
  	              G_TYPE_NONE, 1,
  	              G_TYPE_STRING);

  application_signals [WINDOW_REMOVED] =
  	g_signal_new ("window-removed",
  	              G_OBJECT_CLASS_TYPE (klass),
  	              0,
  	              0, NULL, NULL,
  	              g_cclosure_marshal_VOID__STRING,
  	              G_TYPE_NONE, 1,
  	              G_TYPE_STRING);
}

BamfApplication *
bamf_application_new (void)
{
  BamfApplication *application;
  application = (BamfApplication *) g_object_new (BAMF_TYPE_APPLICATION, NULL);

  return application;
}

BamfApplication *
bamf_application_new_from_desktop_file (char * desktop_file)
{
  BamfApplication *application;
  application = (BamfApplication *) g_object_new (BAMF_TYPE_APPLICATION, NULL);

  bamf_application_set_desktop_file (application, desktop_file);

  return application;
}
