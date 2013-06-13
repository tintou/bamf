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
  GHashTable *open_views;
  GList *local_views;
  GList *allocated_views;
};

static BamfFactory *static_factory = NULL;

static void
bamf_factory_dispose (GObject *object)
{
  BamfFactory *self = (BamfFactory *) object;

  if (self->priv->open_views)
    {
      g_hash_table_destroy (self->priv->open_views);
      self->priv->open_views = NULL;
    }

  if (self->priv->allocated_views)
    {
      g_list_free (self->priv->allocated_views);
      self->priv->allocated_views = NULL;
    }

  if (self->priv->local_views)
    {
      g_list_free (self->priv->local_views);
      self->priv->local_views = NULL;
    }

  G_OBJECT_CLASS (bamf_factory_parent_class)->dispose (object);
}

static void
bamf_factory_finalize (GObject *object)
{
  static_factory = NULL;

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

  priv->open_views = g_hash_table_new_full (g_str_hash, g_str_equal, g_free, g_object_unref);
}

static void
on_view_closed (BamfView *view, BamfFactory *self)
{
  const char *path;

  g_return_if_fail (BAMF_IS_VIEW (view));

  path = _bamf_view_get_path (view);

  if (path)
    g_hash_table_remove (self->priv->open_views, path);
}

static void
on_view_weak_unref (BamfFactory *self, BamfView *view)
{
  self->priv->local_views = g_list_remove (self->priv->local_views, view);
  self->priv->allocated_views = g_list_remove (self->priv->allocated_views, view);
}

static void
bamf_factory_track_view (BamfFactory *self, BamfView *view)
{
  g_return_if_fail (BAMF_IS_VIEW (view));

  if (g_list_find (self->priv->allocated_views, view))
    return;

  g_object_weak_ref (G_OBJECT (view), (GWeakNotify) on_view_weak_unref, self);
  self->priv->allocated_views = g_list_prepend (self->priv->allocated_views, view);
}

static void
bamf_factory_register_view (BamfFactory *self, BamfView *view, const char *path)
{
  g_return_if_fail (BAMF_IS_VIEW (view));
  g_return_if_fail (path != NULL);

  g_object_ref_sink (view);
  g_hash_table_insert (self->priv->open_views, g_strdup (path), view);
  g_signal_connect_after (G_OBJECT (view), BAMF_VIEW_SIGNAL_CLOSED,
                          G_CALLBACK (on_view_closed), self);
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

      if (BAMF_IS_APPLICATION (result))
        {
          factory->priv->local_views = g_list_prepend (factory->priv->local_views, result);
          bamf_factory_track_view (factory, BAMF_VIEW (result));
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

  g_return_val_if_fail (BAMF_IS_FACTORY (factory), NULL);

  if (!path || path[0] == '\0')
    return NULL;

  views = factory->priv->open_views;
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

  gboolean created = TRUE;
  BamfView *matched_view = NULL;

  if (BAMF_IS_APPLICATION (view))
    {
      /* handle case where another allocated view exists and the new one matches it */
      const char *local_desktop_file = bamf_application_get_desktop_file (BAMF_APPLICATION (view));
      GList *local_children = _bamf_application_get_cached_xids (BAMF_APPLICATION (view));
      char *local_name = bamf_view_get_name (view);
      gboolean matched_by_name = FALSE;

      for (l = factory->priv->allocated_views; l; l = l->next)
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
          if (!list_desktop_file)
            {
              GList *list_children, *ll;
              list_children = _bamf_application_get_cached_xids (list_app);

              for (ll = local_children; ll; ll = ll->next)
                {
                  if (g_list_find (list_children, ll->data))
                    {
                      /* Not stopping the parent loop here is intended, as we
                       * can still find a better result in next iterations */
                      matched_view = list_view;
                      break;
                    }
                }

              if ((!matched_view || matched_by_name) && local_name && local_name[0] != '\0')
                {
                  char *list_name = bamf_view_get_name (list_view);
                  if (g_strcmp0 (local_name, list_name) == 0)
                    {
                      if (!matched_by_name)
                        {
                          matched_view = list_view;
                          matched_by_name = TRUE;
                        }
                      else
                        {
                          /* We have already matched an app by its name, this
                           * means that there are two apps with the same name.
                           * It's safer to ignore this, then. */
                          matched_view = NULL;
                        }
                    }

                  g_free (list_name);
                }
            }
        }

      g_free (local_name);
    }
  else if (BAMF_IS_WINDOW (view))
    {
      guint32 local_xid = bamf_window_get_xid (BAMF_WINDOW (view));

      for (l = factory->priv->allocated_views; l; l = l->next)
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

      if (view)
        g_object_unref (view);

      view = matched_view;
      _bamf_view_set_path (view, path);
    }

  if (view)
    {
      bamf_factory_register_view (factory, view, path);

      if (created)
        {
          /* It's the first time we register this view, we have also to track it */
          bamf_factory_track_view (factory, view);
        }
    }

  return view;
}

BamfFactory *
_bamf_factory_get_default (void)
{
  if (BAMF_IS_FACTORY (static_factory))
    return static_factory;

  static_factory = (BamfFactory *) g_object_new (BAMF_TYPE_FACTORY, NULL);
  return static_factory;
}
