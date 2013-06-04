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
 * SECTION:bamf-factory
 * @short_description: The base class for all factorys
 *
 * #BamfFactory is the base class that all factorys need to derive from.
 */

#if HAVE_CONFIG_H
#include <config.h>
#endif

#include <libbamf-private/bamf-private.h>
#include "bamf-factory.h"
#include "bamf-application-private.h"
#include "bamf-view-private.h"

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

static void
bamf_factory_dispose (GObject *object)
{
  BamfFactory *self = (BamfFactory *) object;

  if (self->priv->views)
    {
      g_hash_table_destroy (self->priv->views);
      self->priv->views = NULL;
    }

  if (self->priv->registered_views)
    {
      g_list_free (self->priv->registered_views);
      self->priv->registered_views = NULL;
    }

  if (self->priv->local_views)
    {
      g_list_free_full (self->priv->local_views, g_object_unref);
      self->priv->local_views = NULL;
    }

  G_OBJECT_CLASS (bamf_factory_parent_class)->dispose (object);
}

static void
bamf_factory_finalize (GObject *object)
{
  factory = NULL;

  G_OBJECT_CLASS (bamf_factory_parent_class)->finalize (object);
}

static void
bamf_factory_class_init (BamfFactoryClass *klass)
{
  GObjectClass *obj_class = G_OBJECT_CLASS (klass);

  obj_class->dispose = bamf_factory_dispose;
  obj_class->finalize = bamf_factory_finalize;

  g_type_class_add_private (obj_class, sizeof (BamfFactoryPrivate));
}


static void
bamf_factory_init (BamfFactory *self)
{
  BamfFactoryPrivate *priv;

  priv = self->priv = BAMF_FACTORY_GET_PRIVATE (self);

  priv->views = g_hash_table_new_full (g_str_hash, g_str_equal, g_free, NULL);
}

static void
on_view_closed (BamfView *view, BamfFactory *self)
{
  const char *path;

  g_return_if_fail (BAMF_IS_VIEW (view));

  path = _bamf_view_get_path (view);

  if (path)
    g_hash_table_remove (self->priv->views, path);

  g_object_unref (view);
}

static void
on_view_weak_unref (BamfFactory *self, BamfView *view)
{
  g_return_if_fail (BAMF_IS_FACTORY (self));

  self->priv->local_views = g_list_remove (self->priv->local_views, view);
  self->priv->registered_views = g_list_remove (self->priv->registered_views, view);
}

static void
bamf_factory_register_view (BamfFactory *self, BamfView *view, const char *path)
{
  GHashTable *views;
  views = self->priv->views;

  g_hash_table_insert (views, g_strdup (path), view);

  if (g_list_find (self->priv->registered_views, view))
    return;

  g_signal_connect (G_OBJECT (view), "closed", (GCallback) on_view_closed, self);
  g_object_weak_ref (G_OBJECT (view), (GWeakNotify) on_view_weak_unref, self);

  self->priv->registered_views = g_list_prepend (self->priv->registered_views, view);
}

BamfApplication *
_bamf_factory_app_for_file (BamfFactory * factory,
                            const char * path,
                            gboolean create)
{
  BamfApplication *result = NULL, *app;
  GList *l;

  /* check if result is available in known local_views */
  for (l = factory->priv->local_views; l; l = l->next)
    {
      if (!BAMF_IS_APPLICATION (l->data))
        continue;

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
      if (result && !g_list_find (factory->priv->local_views, result))
        {
          factory->priv->local_views = g_list_prepend (factory->priv->local_views, result);
        }
    }

  return result;
}

static
BamfFactoryViewType compute_factory_type_by_str (const char *type)
{
  BamfFactoryViewType factory_type = BAMF_FACTORY_NONE;

  if (type && type[0] != '\0')
    {
      if (g_strcmp0 (type, "window") == 0)
        {
          factory_type = BAMF_FACTORY_WINDOW;
        }
      else if (g_strcmp0 (type, "application") == 0)
        {
          factory_type = BAMF_FACTORY_APPLICATION;
        }
      else if (g_strcmp0 (type, "tab") == 0)
        {
          factory_type = BAMF_FACTORY_TAB;
        }
      else if (g_strcmp0 (type, "view") == 0)
        {
          factory_type = BAMF_FACTORY_VIEW;
        }
    }

  return factory_type;
}

BamfView *
_bamf_factory_view_for_path (BamfFactory * factory, const char * path)
{
  return _bamf_factory_view_for_path_type (factory, path, BAMF_FACTORY_NONE);
}

BamfView *
_bamf_factory_view_for_path_type_str (BamfFactory * factory, const char * path,
                                                             const char * type)
{
  g_return_val_if_fail (BAMF_IS_FACTORY (factory), NULL);
  BamfFactoryViewType factory_type = compute_factory_type_by_str (type);

  return _bamf_factory_view_for_path_type (factory, path, factory_type);
}

BamfView *
_bamf_factory_view_for_path_type (BamfFactory * factory, const char * path,
                                                         BamfFactoryViewType type)
{
  GHashTable *views;
  BamfView *view;
  BamfDBusItemView *vproxy;
  GList *l;
  gboolean created = FALSE;

  g_return_val_if_fail (BAMF_IS_FACTORY (factory), NULL);

  if (!path || path[0] == '\0')
    return NULL;

  views = factory->priv->views;
  view = g_hash_table_lookup (views, path);

  if (BAMF_IS_VIEW (view))
    return view;

  if (type == BAMF_FACTORY_NONE)
    {
      vproxy = _bamf_dbus_item_view_proxy_new_for_bus_sync (G_BUS_TYPE_SESSION,
                                                            G_DBUS_PROXY_FLAGS_NONE,
                                                            BAMF_DBUS_SERVICE_NAME,
                                                            path, NULL, NULL);
      if (G_IS_DBUS_PROXY (vproxy))
        {
          char *type_str = NULL;
          g_dbus_proxy_set_default_timeout (G_DBUS_PROXY (vproxy), BAMF_DBUS_DEFAULT_TIMEOUT);
          _bamf_dbus_item_view_call_view_type_sync (vproxy, &type_str, NULL, NULL);
          type = compute_factory_type_by_str (type_str);
          g_free (type_str);
          g_object_unref (vproxy);
        }
    }

  switch (type)
  {
    case BAMF_FACTORY_VIEW:
      view = g_object_new (BAMF_TYPE_VIEW, NULL);
      break;
    case BAMF_FACTORY_WINDOW:
      view = BAMF_VIEW (bamf_window_new (path));
      break;
    case BAMF_FACTORY_APPLICATION:
      view = BAMF_VIEW (bamf_application_new (path));
      break;
    case BAMF_FACTORY_TAB:
      view = BAMF_VIEW (bamf_tab_new (path));
      break;
    case BAMF_FACTORY_NONE:
      view = NULL;
      break;
  }

  created = TRUE;
  BamfView *matched_view = NULL;

  if (BAMF_IS_APPLICATION (view))
    {
      /* handle case where a favorite exists and this matches it */
      const char *local_desktop_file = bamf_application_get_desktop_file (BAMF_APPLICATION (view));
      GList *local_children = bamf_view_get_children (view);

      for (l = factory->priv->local_views; l; l = l->next)
        {
          if (!BAMF_IS_APPLICATION (l->data))
            continue;

          BamfView *list_view = BAMF_VIEW (l->data);
          BamfApplication *list_app = BAMF_APPLICATION (l->data);

          const char *list_desktop_file = bamf_application_get_desktop_file (list_app);

          /* We try to match applications by desktop files */
          if (local_desktop_file && g_strcmp0 (local_desktop_file, list_desktop_file) == 0)
            {
              matched_view = list_view;
              break;
            }

          /* If the primary search doesn't give out any result, we fallback
           * to children window comparison */
          if (!list_desktop_file && !matched_view)
            {
              GList *list_children, *ll;
              list_children = _bamf_application_get_cached_xids (list_app);

              for (ll = local_children; ll; ll = ll->next)
                {
                  if (!BAMF_IS_WINDOW (ll->data))
                    continue;

                  guint32 local_xid = bamf_window_get_xid (BAMF_WINDOW (ll->data));

                  if (g_list_find (list_children, GUINT_TO_POINTER (local_xid)))
                    {
                      matched_view = list_view;
                      break;
                    }
                }
            }
        }

      g_list_free (local_children);
    }
  else if (BAMF_IS_WINDOW (view))
    {
      guint32 local_xid = bamf_window_get_xid (BAMF_WINDOW (view));

      for (l = factory->priv->local_views; l; l = l->next)
        {
          if (!BAMF_IS_WINDOW (l->data))
            continue;

          BamfView *list_view = BAMF_VIEW (l->data);
          BamfWindow *list_win = BAMF_WINDOW (l->data);

          guint32 list_xid = bamf_window_get_xid (list_win);

          /* We try to match windows by xid */
          if (local_xid != 0 && local_xid == list_xid)
            {
              matched_view = list_view;
              break;
            }
        }
    }

  if (matched_view)
    {
      created = FALSE;
      g_object_unref (view);

      view = matched_view;
      _bamf_view_set_path (view, path);
      g_object_ref_sink (view);
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

  return view;
}

BamfFactory *
_bamf_factory_get_default (void)
{
  if (BAMF_IS_FACTORY (factory))
    return factory;

  factory = (BamfFactory *) g_object_new (BAMF_TYPE_FACTORY, NULL);
  return factory;
}
