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

#include <lib/libbamf-private/bamf-gdbus-browser-generated.h>

#include "bamf-view.h"
#include "bamf-tab-source.h"
#include "bamf-tab.h"
#include "bamf-marshal.h"

G_DEFINE_TYPE (BamfTabSource, bamf_tab_source, G_TYPE_OBJECT);
#define BAMF_TAB_SOURCE_GET_PRIVATE(obj) (G_TYPE_INSTANCE_GET_PRIVATE(obj, \
BAMF_TYPE_TAB_SOURCE, BamfTabSourcePrivate))

enum
{
  PROP_0,

  PROP_BUS,
  PROP_PATH,
};

enum
{
  REMOTE_TAB_URI_CHANGED,
  REMOTE_TAB_OPENED,
  REMOTE_TAB_CLOSED,
  TAB_OPENED,
  TAB_CLOSED,

  LAST_SIGNAL,
};

static guint bamf_tab_source_signals[LAST_SIGNAL] = { 0 };

struct _BamfTabSourcePrivate
{
  BamfDBusBrowser *proxy;
  char       *bus;
  char       *path;
  GHashTable *tabs;
};

char **
bamf_tab_source_tab_ids (BamfTabSource *self)
{
  BamfTabSourcePrivate *priv;
  GError *error = NULL;
  char **ids = NULL;

  g_return_val_if_fail (BAMF_IS_TAB_SOURCE (self), NULL);
  priv = self->priv;

  if (!bamf_dbus_browser__call_tab_ids_sync (priv->proxy, &ids, NULL, &error))
    {
      g_warning ("Failed to get tab ids: %s", error->message);
      g_error_free (error);
      return NULL;
    }

  return ids;
}

void
bamf_tab_source_show_tab (BamfTabSource *self, char *id)
{
  BamfTabSourcePrivate *priv;
  GError *error = NULL;

  g_return_if_fail (BAMF_IS_TAB_SOURCE (self));
  priv = self->priv;

  if (!bamf_dbus_browser__call_show_tab_sync (priv->proxy, id, NULL, &error))
    {
      g_warning ("Failed to show tab: %s", error->message);
      g_error_free (error);
    }
}

gchar *
bamf_tab_source_get_tab_preview (BamfTabSource *self,
                                 char *id)
{
  BamfTabSourcePrivate *priv;
  GError *error = NULL;
  gchar *preview_data = NULL;

  g_return_val_if_fail (BAMF_IS_TAB_SOURCE (self), NULL);
  priv = self->priv;

  if (!bamf_dbus_browser__call_tab_preview_sync (priv->proxy, id, &preview_data,
                                                 NULL, &error))
    {
      g_warning ("Failed to get tab preview data: %s", error->message);
      g_error_free (error);
      return NULL;
    }

  return preview_data;
}

char *
bamf_tab_source_get_tab_uri (BamfTabSource *self,
                             char *id)
{
  BamfTabSourcePrivate *priv;
  GError *error = NULL;
  char *uri = NULL;

  g_return_val_if_fail (BAMF_IS_TAB_SOURCE (self), NULL);
  priv = self->priv;

  if (!bamf_dbus_browser__call_tab_uri_sync (priv->proxy, id, &uri, NULL, &error))
    {
      g_warning ("Failed to get tab URI: %s", error->message);
      g_error_free (error);
      return NULL;
    }

  return uri;
}

guint32
bamf_tab_source_get_tab_xid (BamfTabSource *self,
                             char *id)
{
  BamfTabSourcePrivate *priv;
  GError *error = NULL;
  guint32 xid = 0;

  g_return_val_if_fail (BAMF_IS_TAB_SOURCE (self), 0);
  priv = self->priv;

  if (!bamf_dbus_browser__call_tab_xid_sync (priv->proxy, id, &xid, NULL, &error))
    {
      g_warning ("Failed to get tab XID: %s", error->message);
      g_error_free (error);
      return 0;
    }

  return xid;
}

static void
bamf_tab_source_on_tab_opened (BamfDBusBrowser *proxy, const char *id,
                               BamfTabSource *source)
{
  BamfTab *tab;

  g_return_if_fail (BAMF_IS_TAB_SOURCE (source));

  g_signal_emit (source, REMOTE_TAB_OPENED, 0, id);

  tab = bamf_tab_new (source, id);
  g_hash_table_insert (source->priv->tabs, g_strdup (id), tab);

  g_signal_emit (source, TAB_OPENED, 0, tab);
}

static void
bamf_tab_source_on_tab_closed (BamfDBusBrowser *proxy, const char *id,
                               BamfTabSource *source)
{
  BamfTab *tab;

  g_return_if_fail (BAMF_IS_TAB_SOURCE (source));

  g_signal_emit (source, REMOTE_TAB_CLOSED, 0, id);

  tab = g_hash_table_lookup (source->priv->tabs, id);

  if (!BAMF_IS_TAB (tab))
    return;

  g_hash_table_remove (source->priv->tabs, id);

  g_signal_emit (source, TAB_CLOSED, 0, tab);
  g_object_unref (tab);
}

static void
bamf_tab_source_on_uri_changed (BamfDBusBrowser *proxy, const char *id,
                                const char *old_uri, char *new_uri, BamfTabSource *source)
{
  g_signal_emit (source, REMOTE_TAB_URI_CHANGED, 0, id, old_uri, new_uri);
}

static void
bamf_tab_source_set_property (GObject *object, guint property_id, const GValue *value, GParamSpec *pspec)
{
  BamfTabSource *self;

  self = BAMF_TAB_SOURCE (object);

  switch (property_id)
    {
      case PROP_PATH:
        self->priv->path = g_strdup (g_value_get_string (value));
        break;
      case PROP_BUS:
        self->priv->bus = g_strdup (g_value_get_string (value));
        break;
      default:
        G_OBJECT_WARN_INVALID_PROPERTY_ID(object, property_id, pspec);
    }
}

static void
bamf_tab_source_get_property (GObject *object, guint property_id, GValue *value, GParamSpec *pspec)
{
  BamfTabSource *self;

  self = BAMF_TAB_SOURCE (object);

  switch (property_id)
    {
      case PROP_PATH:
        g_value_set_string (value, self->priv->path);
        break;
      case PROP_BUS:
        g_value_set_string (value, self->priv->path);
        break;
      default:
        G_OBJECT_WARN_INVALID_PROPERTY_ID(object, property_id, pspec);
    }
}

static void bamf_tab_source_reconnect (GObject *object, GParamSpec *pspec, BamfTabSource *self);

static void
on_proxy_ready_cb (GDBusProxy *proxy, GAsyncResult *res, BamfTabSource *self)
{
  BamfDBusBrowser *bproxy;
  GError *error = NULL;
  g_return_if_fail (BAMF_IS_TAB_SOURCE (self));

  bproxy = bamf_dbus_browser__proxy_new_for_bus_finish (res, &error);

  if (error)
    {
      g_critical ("Unable to get org.ayatana.bamf.browser object from bus %s: %s",
                  self->priv->bus, error->message);
      g_error_free (error);
    }
  else
    {
      gchar *owner = g_dbus_proxy_get_name_owner (G_DBUS_PROXY (bproxy));

      if (owner)
        {
          g_free (owner);

          if (self->priv->proxy)
            g_object_unref (self->priv->proxy);

          self->priv->proxy = bproxy;

          g_signal_connect (self->priv->proxy, "tab-uri-changed",
                            (GCallback) bamf_tab_source_on_uri_changed, self);

          g_signal_connect (self->priv->proxy, "tab-opened",
                            (GCallback) bamf_tab_source_on_tab_opened, self);

          g_signal_connect (self->priv->proxy, "tab-closed",
                            (GCallback) bamf_tab_source_on_tab_closed, self);

          g_signal_connect (self->priv->proxy, "notify::g-name-owner",
                            G_CALLBACK (bamf_tab_source_reconnect), self);

        }
      else
        {
           g_debug ("Failed to get notification approver proxy: no owner available");
           g_object_unref (proxy);
        }
    }
}

static void
bamf_tab_source_reconnect (GObject *object, GParamSpec *pspec, BamfTabSource *self)
{
  g_return_if_fail (BAMF_IS_TAB_SOURCE (self));

  if (self->priv->proxy)
    {
      g_object_unref (self->priv->proxy);
      self->priv->proxy = NULL;
    }

  bamf_dbus_browser__proxy_new_for_bus (G_BUS_TYPE_SESSION,
                                        G_DBUS_PROXY_FLAGS_NONE,
                                        self->priv->bus,
                                        self->priv->path,
                                        NULL,
                                        (GAsyncReadyCallback) on_proxy_ready_cb,
                                        self);
}

static void
bamf_tab_source_constructed (GObject *object)
{
  BamfTabSource *source;
  BamfTabSourcePrivate *priv;

  if (G_OBJECT_CLASS (bamf_tab_source_parent_class)->constructed)
    G_OBJECT_CLASS (bamf_tab_source_parent_class)->constructed (object);

  source = BAMF_TAB_SOURCE (object);
  priv = source->priv;

  bamf_dbus_browser__proxy_new_for_bus (G_BUS_TYPE_SESSION,
                                        G_DBUS_PROXY_FLAGS_NONE,
                                        priv->bus,
                                        priv->path,
                                        NULL,
                                        (GAsyncReadyCallback) on_proxy_ready_cb,
                                        source);
}

static void
bamf_tab_source_dispose (GObject *object)
{
  BamfTabSource *self;

  self = BAMF_TAB_SOURCE (object);

  if (self->priv->bus)
    {
      g_free (self->priv->bus);
      self->priv->bus = NULL;
    }

  if (self->priv->path)
    {
      g_free (self->priv->path);
      self->priv->path = NULL;
    }

  if (self->priv->proxy)
    {
      g_object_unref (self->priv->proxy);
      self->priv->proxy = NULL;
    }

  G_OBJECT_CLASS (bamf_tab_source_parent_class)->dispose (object);
}

static void
bamf_tab_source_init (BamfTabSource * self)
{
  BamfTabSourcePrivate *priv;
  priv = self->priv = BAMF_TAB_SOURCE_GET_PRIVATE (self);

  priv->tabs = g_hash_table_new_full ((GHashFunc) g_str_hash,
                                      (GEqualFunc) g_str_equal,
                                      (GDestroyNotify) g_free,
                                      NULL);
}

static void
bamf_tab_source_class_init (BamfTabSourceClass * klass)
{
  GParamSpec *pspec;
  GObjectClass *object_class = G_OBJECT_CLASS (klass);

  object_class->dispose      = bamf_tab_source_dispose;
  object_class->set_property = bamf_tab_source_set_property;
  object_class->get_property = bamf_tab_source_get_property;
  object_class->constructed  = bamf_tab_source_constructed;

  g_type_class_add_private (klass, sizeof (BamfTabSourcePrivate));

  pspec = g_param_spec_string ("path", "path", "path", NULL, G_PARAM_READWRITE | G_PARAM_CONSTRUCT);
  g_object_class_install_property (object_class, PROP_PATH, pspec);

  pspec = g_param_spec_string ("bus", "bus", "bus", NULL, G_PARAM_READWRITE | G_PARAM_CONSTRUCT);
  g_object_class_install_property (object_class, PROP_BUS, pspec);

  bamf_tab_source_signals [REMOTE_TAB_URI_CHANGED] =
    g_signal_new ("remote-tab-uri-changed",
                  G_OBJECT_CLASS_TYPE (klass),
                  G_SIGNAL_RUN_FIRST,
                  0,
                  NULL, NULL,
                  bamf_marshal_VOID__STRING_STRING_STRING,
                  G_TYPE_NONE, 3,
                  G_TYPE_STRING, G_TYPE_STRING, G_TYPE_STRING);

  bamf_tab_source_signals [REMOTE_TAB_OPENED] =
    g_signal_new ("remote-tab-opened",
                  G_OBJECT_CLASS_TYPE (klass),
                  G_SIGNAL_RUN_FIRST,
                  0,
                  NULL, NULL,
                  g_cclosure_marshal_VOID__STRING,
                  G_TYPE_NONE, 1,
                  G_TYPE_STRING);

  bamf_tab_source_signals [REMOTE_TAB_CLOSED] =
    g_signal_new ("remote-tab-closed",
                  G_OBJECT_CLASS_TYPE (klass),
                  G_SIGNAL_RUN_FIRST,
                  0,
                  NULL, NULL,
                  g_cclosure_marshal_VOID__STRING,
                  G_TYPE_NONE, 1,
                  G_TYPE_STRING);

  bamf_tab_source_signals [TAB_OPENED] =
    g_signal_new ("tab-opened",
                  G_OBJECT_CLASS_TYPE (klass),
                  G_SIGNAL_RUN_FIRST,
                  0,
                  NULL, NULL,
                  g_cclosure_marshal_VOID__OBJECT,
                  G_TYPE_NONE, 1,
                  BAMF_TYPE_TAB);

  bamf_tab_source_signals [TAB_CLOSED] =
    g_signal_new ("tab-closed",
                  G_OBJECT_CLASS_TYPE (klass),
                  G_SIGNAL_RUN_FIRST,
                  0,
                  NULL, NULL,
                  g_cclosure_marshal_VOID__OBJECT,
                  G_TYPE_NONE, 1,
                  BAMF_TYPE_TAB);
}

BamfTabSource *
bamf_tab_source_new (char *bus, char *path)
{
  BamfTabSource *self;
  self = (BamfTabSource *) g_object_new (BAMF_TYPE_TAB_SOURCE,
                                         "path", path,
                                         "bus", bus,
                                         NULL);

  return self;
}
