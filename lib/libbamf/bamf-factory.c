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
 * SECTION:bamf-factory
 * @short_description: The base class for all factorys
 *
 * #BamfFactory is the base class that all factorys need to derive from.
 */

#if HAVE_CONFIG_H
#include <config.h>
#endif

#include "bamf-factory.h"
#include "bamf-view.h"
#include "bamf-view-private.h"
#include "bamf-window.h"
#include "bamf-application.h"
#include "bamf-indicator.h"

#include <dbus/dbus.h>
#include <dbus/dbus-glib.h>
#include <dbus/dbus-glib-lowlevel.h>
#include <string.h>

G_DEFINE_TYPE (BamfFactory, bamf_factory, G_TYPE_OBJECT);

#define BAMF_FACTORY_GET_PRIVATE(o) \
  (G_TYPE_INSTANCE_GET_PRIVATE ((o), BAMF_TYPE_FACTORY, BamfFactoryPrivate))

struct _BamfFactoryPrivate
{
  GHashTable *views;
  GList *local_views;
  GList *registered_views;
};

static BamfFactory *factory = NULL;

BamfApplication * bamf_application_new_favorite     (const char *favorite_path);

BamfApplication * bamf_application_new              (const char *path);

BamfWindow      * bamf_window_new                   (const char *path);

BamfIndicator   * bamf_indicator_new                (const char *path);

static void
bamf_factory_class_init (BamfFactoryClass *klass)
{
  GObjectClass *obj_class = G_OBJECT_CLASS (klass);

  g_type_class_add_private (obj_class, sizeof (BamfFactoryPrivate));
}


static void
bamf_factory_init (BamfFactory *self)
{
  BamfFactoryPrivate *priv;

  priv = self->priv = BAMF_FACTORY_GET_PRIVATE (self);

  priv->views = g_hash_table_new_full ((GHashFunc) g_str_hash, (GEqualFunc) g_str_equal, (GDestroyNotify) g_free, NULL);
}

static void
on_view_closed (BamfView *view, BamfFactory *self)
{
  const char *path;
  
  g_return_if_fail (BAMF_IS_VIEW (view));
  
  path = bamf_view_get_path (view);
  if (path)
    g_hash_table_remove (self->priv->views, path);
  
  g_object_unref (view);
}

static void
on_view_weak_unref (BamfFactory *self, BamfView *view)
{
  g_return_if_fail (BAMF_IS_FACTORY (self));
  self->priv->local_views = g_list_remove (self->priv->local_views, view);
}

static void
bamf_factory_register_view (BamfFactory *self, BamfView *view, const char *path)
{
  GHashTable *views;
  views = self->priv->views;
  
  g_hash_table_insert (views, g_strdup (path), view);

  if (g_list_find (self->priv->registered_views, view))
    return;
  
  self->priv->registered_views = g_list_prepend (self->priv->registered_views, view);
  
  g_signal_connect (G_OBJECT (view), "closed", (GCallback) on_view_closed, self);
  g_object_weak_ref (G_OBJECT (view), (GWeakNotify) on_view_weak_unref, self);
}

BamfApplication * 
bamf_factory_app_for_file (BamfFactory * factory,
                           const char * path,
                           gboolean create)
{
  BamfApplication *result = NULL, *app;
  GList *l;
  
  /* check if result is available in known local_views */
  for (l = factory->priv->local_views; l; l = l->next)
    {
      app = BAMF_APPLICATION (l->data);
      if (g_strcmp0 (bamf_application_get_desktop_file (app), path) == 0)
        {
          result = app;
          break;
        }
    }
  
  /* else create new */
  if (!result && create)
    {
      /* delay registration until match time */
      result = bamf_application_new_favorite (path);
      if (result)
        factory->priv->local_views = g_list_prepend (factory->priv->local_views, result);
    }
  
  return result;
}

BamfView * 
bamf_factory_view_for_path (BamfFactory * factory,
                            const char * path)
{
  GHashTable *views;
  BamfView *view;
  GList *l;
  gchar *type;
  gboolean created = FALSE;

  g_return_val_if_fail (BAMF_IS_FACTORY (factory), NULL);
  
  if (!path || strlen (path) == 0)
    return NULL;
  
  views = factory->priv->views;

  view = g_hash_table_lookup (views, path);
  
  if (BAMF_IS_VIEW (view))
    return view;
  
  view = g_object_new (BAMF_TYPE_VIEW, NULL);
  bamf_view_set_path (view, path);
  type = g_strdup (bamf_view_get_view_type (view));
  g_object_unref (view);
  
  view = NULL;
  if (g_strcmp0 (type, "application") == 0)
    view = BAMF_VIEW (bamf_application_new (path));
  else if (g_strcmp0 (type, "window") == 0)
    view = BAMF_VIEW (bamf_window_new (path));
  else if (g_strcmp0 (type, "indicator") == 0)
    view = BAMF_VIEW (bamf_indicator_new (path));
  
  created = TRUE;
  
  if (BAMF_IS_APPLICATION (view))
    {
      /* handle case where a favorite exists and this matches it */
      const char *local_desktop_file = bamf_application_get_desktop_file (BAMF_APPLICATION (view));
      for (l = factory->priv->local_views; l; l = l->next)
        {
          /* remote ready views are already matched */
          if (bamf_view_remote_ready (BAMF_VIEW (l->data)) || !BAMF_IS_APPLICATION (l->data))
            continue;
          
          const char *list_desktop_file = bamf_application_get_desktop_file (BAMF_APPLICATION (l->data));
          
          if (g_strcmp0 (local_desktop_file, list_desktop_file) == 0)
            {
              created = FALSE;
              g_object_unref (view);

              view = BAMF_VIEW (l->data);
              bamf_view_set_path (view, path);
              g_object_ref_sink (view);
              break;
            }
        }
    }
  
  if (view)
    {
      bamf_factory_register_view (factory, view, path);
      
      if (created)
        {
          factory->priv->local_views = g_list_prepend (factory->priv->local_views, view);
          g_object_ref_sink (view);
        }
    }
  
  g_free (type);
  return view;
}

BamfFactory * 
bamf_factory_get_default (void)
{
  
  if (BAMF_IS_FACTORY (factory))
    return factory;
  
  factory = (BamfFactory *) g_object_new (BAMF_TYPE_FACTORY, NULL);
  return factory;
}
