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


#include "bamf-view.h"
#include "bamf-tab-source.h"
#include "bamf-tab.h"
#include "bamf-marshal.h"
#include <dbus/dbus-glib.h>

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
  char       *bus;
  char       *path;
  DBusGProxy *proxy;
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

  if (!dbus_g_proxy_call (priv->proxy,
                          "TabIds",
                          &error,
                          G_TYPE_INVALID,
                          G_TYPE_STRV, &ids,
                          G_TYPE_INVALID))
    {
      g_warning ("Failed to get tab ids: %s", error->message);
      g_error_free (error);
      return NULL;
    }

  return ids;
}

void
bamf_tab_source_show_tab (BamfTabSource *self,
                          char *id)
{
  BamfTabSourcePrivate *priv;
  GError *error = NULL;

  g_return_if_fail (BAMF_IS_TAB_SOURCE (self));
  priv = self->priv;

  if (!dbus_g_proxy_call (priv->proxy,
                          "ShowTab",
                          &error,
                          G_TYPE_STRING, id,
                          G_TYPE_INVALID,
                          G_TYPE_INVALID))
    {
      g_warning ("Failed to show tab: %s", error->message);
      g_error_free (error);
    }
}

GArray *
bamf_tab_source_get_tab_preview (BamfTabSource *self,
                                 char *id)
{
  BamfTabSourcePrivate *priv;
  GError *error = NULL;
  GArray *preview_data = NULL;

  g_return_val_if_fail (BAMF_IS_TAB_SOURCE (self), NULL);
  priv = self->priv;

  if (!dbus_g_proxy_call (priv->proxy,
                          "TabUri",
                          &error,
                          G_TYPE_STRING, id,
                          G_TYPE_INVALID,
                          G_TYPE_ARRAY, &preview_data,
                          G_TYPE_INVALID))
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

  if (!dbus_g_proxy_call (priv->proxy,
                          "TabUri",
                          &error,
                          G_TYPE_STRING, id,
                          G_TYPE_INVALID,
                          G_TYPE_STRING, &uri,
                          G_TYPE_INVALID))
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

  if (!dbus_g_proxy_call (priv->proxy,
                          "TabXid",
                          &error,
                          G_TYPE_STRING, id,
                          G_TYPE_INVALID,
                          G_TYPE_UINT, &xid,
                          G_TYPE_INVALID))
    {
      g_warning ("Failed to get tab XID: %s", error->message);
      g_error_free (error);
      return 0;
    }

  return xid;
}

static void
bamf_tab_source_on_tab_opened (DBusGProxy *proxy, char *id, BamfTabSource *source)
{
  BamfTab *tab;

  g_return_if_fail (BAMF_IS_TAB_SOURCE (source));

  g_signal_emit (source, REMOTE_TAB_OPENED, 0, id);

  tab = bamf_tab_new (source, id);
  g_hash_table_insert (source->priv->tabs, g_strdup (id), tab);

  g_signal_emit (source, TAB_OPENED, 0, tab);
}

static void
bamf_tab_source_on_tab_closed (DBusGProxy *proxy, char *id, BamfTabSource *source)
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
bamf_tab_source_on_uri_changed (DBusGProxy *proxy, char *id, char *old_uri, char *new_uri, BamfTabSource *source)
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

static void
bamf_tab_source_constructed (GObject *object)
{
  BamfTabSource *source;
  BamfTabSourcePrivate *priv;
  DBusGConnection *connection;
  GError *error = NULL;

  if (G_OBJECT_CLASS (bamf_tab_source_parent_class)->constructed)
    G_OBJECT_CLASS (bamf_tab_source_parent_class)->constructed (object);

  source = BAMF_TAB_SOURCE (object);
  priv = source->priv;

  connection = dbus_g_bus_get (DBUS_BUS_SESSION, &error);
  if (connection == NULL)
    {
      g_critical ("Failed to open connection to bus: %s",
               error != NULL ? error->message : "Unknown");
      if (error)
        g_error_free (error);
      return;
    }

  priv->proxy = dbus_g_proxy_new_for_name (connection,
                                           priv->bus,
                                           priv->path,
                                           "org.ayatana.bamf.browser");

  if (priv->proxy == NULL)
    {
      g_critical ("Unable to get org.ayatana.bamf.browser object from bus");
    }

  dbus_g_proxy_add_signal (priv->proxy,
                           "TabUriChanged",
                           G_TYPE_STRING,
                           G_TYPE_STRING,
                           G_TYPE_STRING,
                           G_TYPE_INVALID);

  dbus_g_proxy_add_signal (priv->proxy,
                           "TabOpened",
                           G_TYPE_STRING,
                           G_TYPE_INVALID);

  dbus_g_proxy_add_signal (priv->proxy,
                           "TabClosed",
                           G_TYPE_STRING,
                           G_TYPE_INVALID);

  dbus_g_proxy_connect_signal (priv->proxy,
                               "TabUriChanged",
                               (GCallback) bamf_tab_source_on_uri_changed,
                               source,
                               NULL);

  dbus_g_proxy_connect_signal (priv->proxy,
                               "TabOpened",
                               (GCallback) bamf_tab_source_on_tab_opened,
                               source,
                               NULL);

  dbus_g_proxy_connect_signal (priv->proxy,
                               "TabClosed",
                               (GCallback) bamf_tab_source_on_tab_closed,
                               source,
                               NULL);
}

static void
bamf_tab_source_dispose (GObject *object)
{
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
