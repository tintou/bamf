/*
 * Copyright (C) 2010-2011 Canonical Ltd
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
 *              Marco Trevisan (Treviño) <3v1n0@ubuntu.com>
 *
 */

#include "bamf-application.h"
#include "bamf-window.h"
#include "bamf-matcher.h"
#include "bamf-indicator.h"
#include "bamf-legacy-window.h"
#include "bamf-legacy-screen.h"
#include <string.h>
#include <gio/gdesktopappinfo.h>

#define BAMF_APPLICATION_GET_PRIVATE(obj) (G_TYPE_INSTANCE_GET_PRIVATE(obj, \
BAMF_TYPE_APPLICATION, BamfApplicationPrivate))

static void bamf_application_dbus_application_iface_init (BamfDBusItemApplicationIface *iface);
G_DEFINE_TYPE_WITH_CODE (BamfApplication, bamf_application, BAMF_TYPE_VIEW,
                         G_IMPLEMENT_INTERFACE (BAMF_DBUS_ITEM_TYPE_APPLICATION,
                                                bamf_application_dbus_application_iface_init));

struct _BamfApplicationPrivate
{
  BamfDBusItemApplication *dbus_iface;
  char * desktop_file;
  GList * desktop_file_list;
  char * app_type;
  char * icon;
  char * wmclass;
  gboolean is_tab_container;
  gboolean show_stubs;
};

#define STUB_KEY  "X-Ayatana-Appmenu-Show-Stubs"

static char *
bamf_application_get_icon (BamfView *view)
{
  g_return_val_if_fail (BAMF_IS_APPLICATION (view), NULL);

  return g_strdup (BAMF_APPLICATION (view)->priv->icon);
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
  BamfApplicationPrivate *priv;
  char *result = NULL;

  g_return_val_if_fail (BAMF_IS_APPLICATION (application), NULL);
  priv = application->priv;

  result = g_strdup (priv->desktop_file);
  return result;
}

char *
bamf_application_get_wmclass (BamfApplication *application)
{
  BamfApplicationPrivate *priv;
  char *result = NULL;

  g_return_val_if_fail (BAMF_IS_APPLICATION (application), NULL);
  priv = application->priv;

  result = g_strdup (priv->wmclass);
  return result;
}

static gboolean
icon_name_is_valid (char *name)
{
  GtkIconTheme *icon_theme;

  if (name == NULL)
    return FALSE;

  icon_theme = gtk_icon_theme_get_default ();
  return gtk_icon_theme_has_icon (icon_theme, name);
}

static void
bamf_application_setup_icon_and_name (BamfApplication *self)
{
  BamfView *view;
  BamfWindow *window = NULL;
  GDesktopAppInfo *desktop;
  GKeyFile * keyfile;
  GIcon *gicon;
  const GList *children, *l;
  const char *class;
  char *icon = NULL, *name = NULL;
  GError *error;

  g_return_if_fail (BAMF_IS_APPLICATION (self));

  if (self->priv->icon && bamf_view_get_name (BAMF_VIEW (self)))
    return;

  if (self->priv->desktop_file)
    {
      keyfile = g_key_file_new();
      if (!g_key_file_load_from_file(keyfile, self->priv->desktop_file, G_KEY_FILE_NONE, NULL)) {
          g_key_file_free(keyfile);
        return;
      }

      desktop = g_desktop_app_info_new_from_keyfile (keyfile);

      if (!G_IS_APP_INFO (desktop)) {
        g_key_file_free(keyfile);
        return;
      }

      gicon = g_app_info_get_icon (G_APP_INFO (desktop));

      name = g_strdup (g_app_info_get_display_name (G_APP_INFO (desktop)));

      if (gicon)
        icon = g_icon_to_string (gicon);

      if (g_key_file_has_key(keyfile, G_KEY_FILE_DESKTOP_GROUP, STUB_KEY, NULL)) {
        /* This will error to return false, which is okay as it seems
           unlikely anyone will want to set this flag except to turn
           off the stub menus. */
        self->priv->show_stubs = g_key_file_get_boolean (keyfile,
                                                         G_KEY_FILE_DESKTOP_GROUP,
                                                         STUB_KEY, NULL);
      }
      
      if (g_key_file_has_key (keyfile, G_KEY_FILE_DESKTOP_GROUP, "X-GNOME-FullName", NULL))
        {
          /* Grab the better name if its available */
          gchar *fullname = NULL;
          error = NULL; 
          fullname = g_key_file_get_locale_string (keyfile,
                                                   G_KEY_FILE_DESKTOP_GROUP,
                                                   "X-GNOME-FullName", NULL,
                                                   &error);
          if (error != NULL)
            {
              g_error_free (error);
              if (fullname)
                g_free (fullname);
            }
          else
            {
              g_free (name);
              name = fullname;
            }
        }

      g_object_unref (desktop);
      g_key_file_free(keyfile);
    }
  else if ((children = bamf_view_get_children (BAMF_VIEW (self))) != NULL)
    {
      for (l = children; l && !icon; l = l->next)
        {
          view = l->data;
          if (!BAMF_IS_WINDOW (view))
            continue;
          
          window = BAMF_WINDOW (view);
          
          do
            {
              class = bamf_legacy_window_get_class_name (bamf_window_get_window (window));
              icon = g_utf8_strdown (class, -1);

              if (icon_name_is_valid (icon))
                break;

              g_free (icon);
              icon = bamf_legacy_window_get_exec_string (bamf_window_get_window (window));

              if (icon_name_is_valid (icon))
                break;

              g_free (icon);
              icon = NULL;
            }
          while (FALSE);

          name = g_strdup (bamf_legacy_window_get_name (bamf_window_get_window (window)));
        }
        
      if (!icon)
        {
          if (window)
            {
              icon = g_strdup (bamf_legacy_window_save_mini_icon (bamf_window_get_window (window)));
            }
          
          if (!icon)
            {
              icon = g_strdup ("application-default-icon");
            }
        }
    }
  else
    {
      /* we do nothing as we have nothing to go on */
    }

  if (icon)
    {
      if (self->priv->icon)
        g_free (self->priv->icon);

      self->priv->icon = icon;
    }

  if (name)
    bamf_view_set_name (BAMF_VIEW (self), name);

  g_free (name);
}

void
bamf_application_set_desktop_file (BamfApplication *application,
                                   const char * desktop_file)
{
  g_return_if_fail (BAMF_IS_APPLICATION (application));

  if (application->priv->desktop_file)
    g_free (application->priv->desktop_file);

  if (desktop_file && desktop_file[0] != '\0')
    application->priv->desktop_file = g_strdup (desktop_file);
  else
    application->priv->desktop_file = NULL;

  bamf_application_setup_icon_and_name (application);
}

void
bamf_application_set_wmclass (BamfApplication *application,
                              const char *wmclass)
{
  g_return_if_fail (BAMF_IS_APPLICATION (application));

  if (application->priv->wmclass)
    g_free (application->priv->wmclass);

  if (wmclass && wmclass[0] != '\0')
    application->priv->wmclass = g_strdup (wmclass);
  else
    application->priv->wmclass = NULL;
}

GVariant *
bamf_application_get_xids (BamfApplication *application)
{
  const GList *l;
  GVariantBuilder b;
  BamfView *view;
  guint32 xid;

  g_return_val_if_fail (BAMF_IS_APPLICATION (application), NULL);

  g_variant_builder_init (&b, G_VARIANT_TYPE ("(au)"));
  g_variant_builder_open (&b, G_VARIANT_TYPE ("au"));

  for (l = bamf_view_get_children (BAMF_VIEW (application)); l; l = l->next)
    {
      view = l->data;

      if (!BAMF_IS_WINDOW (view))
        continue;

      xid = bamf_window_get_xid (BAMF_WINDOW (view));
      g_variant_builder_add (&b, "u", xid);
    }

  g_variant_builder_close (&b);

  return g_variant_builder_end (&b);
}

gboolean
bamf_application_contains_similar_to_window (BamfApplication *self,
                                             BamfWindow *window)
{
  gboolean result = FALSE;
  const char *class, *owned_class;
  const GList *children, *l;
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
  const GList *l;
  gboolean result = FALSE;

  g_return_val_if_fail (BAMF_IS_APPLICATION (application), FALSE);

  for (l = bamf_view_get_children (BAMF_VIEW (application)); l; l = l->next)
    {
      BamfView *view = l->data;

      if (!BAMF_IS_WINDOW (view))
        continue;

      if (bamf_window_get_xid (BAMF_WINDOW (view)) == xid)
        {
          result = TRUE;
          break;
        }
    }

  return result;
}

static const char *
bamf_application_get_view_type (BamfView *view)
{
  return "application";
}

static char *
bamf_application_get_stable_bus_name (BamfView *view)
{
  BamfApplication *self;
  const GList *children, *l;
  BamfView *child;

  g_return_val_if_fail (BAMF_IS_APPLICATION (view), NULL);
  self = BAMF_APPLICATION (view);

  if (self->priv->desktop_file)
    return g_strdup_printf ("application%i", abs (g_str_hash (self->priv->desktop_file)));

  children = bamf_view_get_children (BAMF_VIEW (self));
  for (l = children; l; l = l->next)
    {
      child = l->data;

      if (!BAMF_IS_WINDOW (child))
        continue;

      return g_strdup_printf ("application%s", 
                              bamf_legacy_window_get_class_name (bamf_window_get_window (BAMF_WINDOW (child))));
    }
  
  return g_strdup_printf ("application%p", view);
}

static void
bamf_application_ensure_flags (BamfApplication *self)
{
  gboolean urgent = FALSE, visible = FALSE, running = FALSE, active = FALSE;
  const GList *l;
  BamfView *view;

  for (l = bamf_view_get_children (BAMF_VIEW (self)); l; l = l->next)
    {
      view = l->data;

      if (!BAMF_IS_VIEW (view))
        continue;

      running = TRUE;
      
      if (BAMF_IS_INDICATOR (view))
        visible = TRUE;

      if (!BAMF_IS_WINDOW (view))
        continue;

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
  g_signal_emit_by_name (self, "window-added", bamf_view_get_path (view));
}

static void
bamf_application_child_added (BamfView *view, BamfView *child)
{
  BamfApplication *application;

  application = BAMF_APPLICATION (view);

  if (BAMF_IS_WINDOW (child))
    {
      if (bamf_view_is_on_bus (child))
        g_signal_emit_by_name (BAMF_APPLICATION (view), "window-added", bamf_view_get_path (child));
      else
        g_signal_connect (G_OBJECT (child), "exported",
                          (GCallback) view_exported, view);
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

static char *
bamf_application_favorite_from_list (BamfApplication *self, GList *list)
{
  BamfMatcher *matcher;
  GList *favs, *l;
  char *result = NULL;

  g_return_val_if_fail (BAMF_IS_APPLICATION (self), NULL);

  matcher = bamf_matcher_get_default ();
  favs = bamf_matcher_get_favorites (matcher);
  
  if (favs)
    {
      for (l = favs; l; l = l->next)
        {
          if (g_list_find_custom (list, l->data, (GCompareFunc) g_strcmp0))
            {
              result = l->data;
              break;
            }
        }
    }

  return result;
}

static void
bamf_application_set_desktop_file_from_list (BamfApplication *self, GList *list)
{
  BamfApplicationPrivate *priv;
  GList *l;
  char *desktop_file;

  g_return_if_fail (BAMF_IS_APPLICATION (self));
  g_return_if_fail (list);

  priv = self->priv;

  if (priv->desktop_file_list)
    {
      g_list_free_full (priv->desktop_file_list, g_free);
      priv->desktop_file_list = NULL;
    }

  for (l = list; l; l = l->next)
    priv->desktop_file_list = g_list_prepend (priv->desktop_file_list, g_strdup (l->data));

  priv->desktop_file_list = g_list_reverse (priv->desktop_file_list);

  desktop_file = bamf_application_favorite_from_list (self, priv->desktop_file_list);
  
  /* items, after reversing them, are in priority order */
  if (!desktop_file)
    desktop_file = list->data;

  bamf_application_set_desktop_file (self, desktop_file);
}

static void
bamf_application_child_removed (BamfView *view, BamfView *child)
{
  if (BAMF_IS_WINDOW (child))
    {
      if (bamf_view_is_on_bus (child))
        g_signal_emit_by_name (BAMF_APPLICATION (view), "window-removed",
                               bamf_view_get_path (child));
    }

  g_signal_handlers_disconnect_by_func (G_OBJECT (child), view_active_changed, view);
  g_signal_handlers_disconnect_by_func (G_OBJECT (child), view_urgent_changed, view);
  g_signal_handlers_disconnect_by_func (G_OBJECT (child), view_visible_changed, view);

  bamf_application_ensure_flags (BAMF_APPLICATION (view));

  const GList *children = bamf_view_get_children (view);
  if (children == NULL)
    {
      bamf_view_close (view);
    }
}

static void
matcher_favorites_changed (BamfMatcher *matcher, BamfApplication *self)
{
  char *new_desktop_file = NULL;
  
  g_return_if_fail (BAMF_IS_APPLICATION (self));
  g_return_if_fail (BAMF_IS_MATCHER (matcher));
  
  new_desktop_file = bamf_application_favorite_from_list (self, self->priv->desktop_file_list);

  if (new_desktop_file)
    {
      bamf_application_set_desktop_file (self, new_desktop_file);
    }
}

static void
on_window_added (BamfApplication *self, const gchar *win_path, gpointer _not_used)
{
  g_return_if_fail (BAMF_IS_APPLICATION (self));
  g_signal_emit_by_name (self->priv->dbus_iface, "window-added", win_path);
}

static void
on_window_removed (BamfApplication *self, const gchar *win_path, gpointer _not_used)
{
  g_return_if_fail (BAMF_IS_APPLICATION (self));
  g_signal_emit_by_name (self->priv->dbus_iface, "window-removed", win_path);
}

static gboolean
on_dbus_handle_show_stubs (BamfDBusItemApplication *interface,
                           GDBusMethodInvocation *invocation,
                           BamfApplication *self)
{
  gboolean show_stubs = bamf_application_get_show_stubs (self);
  g_dbus_method_invocation_return_value (invocation,
                                         g_variant_new ("(b)", show_stubs));

  return TRUE;
}

static gboolean
on_dbus_handle_xids (BamfDBusItemApplication *interface,
                     GDBusMethodInvocation *invocation,
                     BamfApplication *self)
{
  GVariant *xids = bamf_application_get_xids (self);
  g_dbus_method_invocation_return_value (invocation, xids);

  return TRUE;
}

static gboolean
on_dbus_handle_desktop_file (BamfDBusItemApplication *interface,
                             GDBusMethodInvocation *invocation,
                             BamfApplication *self)
{
  const char *desktop_file = self->priv->desktop_file ? self->priv->desktop_file : "";
  g_dbus_method_invocation_return_value (invocation,
                                         g_variant_new ("(s)", desktop_file));

  return TRUE;
}

static gboolean
on_dbus_handle_application_type (BamfDBusItemApplication *interface,
                                 GDBusMethodInvocation *invocation,
                                 BamfApplication *self)
{
  const char *type = self->priv->app_type ? self->priv->app_type : "";
  g_dbus_method_invocation_return_value (invocation,
                                         g_variant_new ("(s)", type));

  return TRUE;
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
    
  if (priv->desktop_file_list)
    {
      g_list_free_full (priv->desktop_file_list, g_free);
      priv->desktop_file_list = NULL;
    }

  if (priv->app_type)
    {
      g_free (priv->app_type);
      priv->app_type = NULL;
    }

  if (priv->icon)
    {
      g_free (priv->icon);
      priv->icon = NULL;
    }

  if (priv->wmclass)
    {
      g_free (priv->wmclass);
      priv->wmclass = NULL;
    }

  g_signal_handlers_disconnect_by_func (G_OBJECT (bamf_matcher_get_default ()),
                                        matcher_favorites_changed, object);

  G_OBJECT_CLASS (bamf_application_parent_class)->dispose (object);
}

static void
bamf_application_finalize (GObject *object)
{
  BamfApplication *self;
  self = BAMF_APPLICATION (object);

  g_object_unref (self->priv->dbus_iface);

  G_OBJECT_CLASS (bamf_application_parent_class)->finalize (object);
}

static void
bamf_application_init (BamfApplication * self)
{
  BamfApplicationPrivate *priv;
  priv = self->priv = BAMF_APPLICATION_GET_PRIVATE (self);

  priv->is_tab_container = FALSE;
  priv->app_type = g_strdup ("system");
  priv->show_stubs = TRUE;
  priv->wmclass = NULL;

  /* Initializing the dbus interface */
  priv->dbus_iface = bamf_dbus_item_application_skeleton_new ();

  /* We need to connect to the object own signals to redirect them to the dbus
   * interface                                                                */
  g_signal_connect (self, "window-added", G_CALLBACK (on_window_added), NULL);
  g_signal_connect (self, "window-removed", G_CALLBACK (on_window_removed), NULL);

  /* Registering signal callbacks to reply to dbus method calls */
  g_signal_connect (priv->dbus_iface, "handle-show-stubs",
                    G_CALLBACK (on_dbus_handle_show_stubs), self);

  g_signal_connect (priv->dbus_iface, "handle-xids",
                    G_CALLBACK (on_dbus_handle_xids), self);

  g_signal_connect (priv->dbus_iface, "handle-desktop-file",
                    G_CALLBACK (on_dbus_handle_desktop_file), self);

  g_signal_connect (priv->dbus_iface, "handle-application-type",
                    G_CALLBACK (on_dbus_handle_application_type), self);

  /* Setting the interface for the dbus object */
  bamf_dbus_item_object_skeleton_set_application (BAMF_DBUS_ITEM_OBJECT_SKELETON (self),
                                                  priv->dbus_iface);

  g_signal_connect (G_OBJECT (bamf_matcher_get_default ()), "favorites-changed", 
                    (GCallback) matcher_favorites_changed, self);
}

static void
bamf_application_dbus_application_iface_init (BamfDBusItemApplicationIface *iface)
{
}

static void
bamf_application_class_init (BamfApplicationClass * klass)
{
  GObjectClass *object_class = G_OBJECT_CLASS (klass);
  BamfViewClass *view_class = BAMF_VIEW_CLASS (klass);

  object_class->dispose = bamf_application_dispose;
  object_class->finalize = bamf_application_finalize;

  view_class->view_type = bamf_application_get_view_type;
  view_class->child_added = bamf_application_child_added;
  view_class->child_removed = bamf_application_child_removed;
  view_class->get_icon = bamf_application_get_icon;
  view_class->stable_bus_name = bamf_application_get_stable_bus_name;

  g_type_class_add_private (klass, sizeof (BamfApplicationPrivate));
}

BamfApplication *
bamf_application_new (void)
{
  BamfApplication *application;
  application = (BamfApplication *) g_object_new (BAMF_TYPE_APPLICATION, NULL);

  return application;
}

BamfApplication *
bamf_application_new_from_desktop_file (const char * desktop_file)
{
  BamfApplication *application;
  application = (BamfApplication *) g_object_new (BAMF_TYPE_APPLICATION, NULL);

  bamf_application_set_desktop_file (application, desktop_file);

  return application;
}

BamfApplication *
bamf_application_new_from_desktop_files (GList *desktop_files)
{
  BamfApplication *application;
  application = (BamfApplication *) g_object_new (BAMF_TYPE_APPLICATION, NULL);
  
  bamf_application_set_desktop_file_from_list (application, desktop_files);
  
  return application;  
}

BamfApplication *
bamf_application_new_with_wmclass (const char *wmclass)
{
  BamfApplication *application;
  application = (BamfApplication *) g_object_new (BAMF_TYPE_APPLICATION, NULL);

  bamf_application_set_wmclass (application, wmclass);

  return application;
}

/**
    bamf_application_get_show_stubs:
    @application: Application to check for menu stubs

    Checks to see if the application should show menu stubs or not.
    This is specified with the "X-Ayatana-Appmenu-Show-Stubs" desktop
    file key.

    Return Value: Defaults to TRUE, else FALSE if specified in
      .desktop file.
*/
gboolean
bamf_application_get_show_stubs (BamfApplication *application)
{
    g_return_val_if_fail(BAMF_IS_APPLICATION(application), TRUE);
    return application->priv->show_stubs;
}
