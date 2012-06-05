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

#include "bamf-tab-source.h"
#include "bamf-tab-source-glue.h"
#include "bamf-marshal.h"

#define BAMF_TAB_SOURCE_PATH "/org/bamf/tabsource"

G_DEFINE_TYPE (BamfTabSource, bamf_tab_source, G_TYPE_OBJECT)
#define BAMF_TAB_SOURCE_GET_PRIVATE(object)(G_TYPE_INSTANCE_GET_PRIVATE((object), BAMF_TYPE_TAB_SOURCE, BamfTabSourcePrivate))

enum
{
  TAB_URI_CHANGED,
  TAB_OPENED,
  TAB_CLOSED,

  LAST_SIGNAL,
};

enum
{
  PROP_0,

  PROP_ID,
};

static guint bamf_tab_source_signals[LAST_SIGNAL] = { 0 };

struct _BamfTabSourcePrivate
{
  char *id;
};

gboolean
bamf_tab_source_show_tab        (BamfTabSource *source,
                                 char *tab_id,
                                 GError *error)
{
  g_return_val_if_fail (BAMF_IS_TAB_SOURCE (source), TRUE);

  if (BAMF_TAB_SOURCE_GET_CLASS (source)->show_tab)
    BAMF_TAB_SOURCE_GET_CLASS (source)->show_tab (source, tab_id);

  return TRUE;
}

char **
bamf_tab_source_get_tab_ids     (BamfTabSource *source)
{
  g_return_val_if_fail (BAMF_IS_TAB_SOURCE (source), NULL);

  if (BAMF_TAB_SOURCE_GET_CLASS (source)->tab_ids)
    return BAMF_TAB_SOURCE_GET_CLASS (source)->tab_ids (source);

  return NULL;
}

GArray *
bamf_tab_source_get_tab_preview (BamfTabSource *source,
                                 char *tab_id)
{
  g_return_val_if_fail (BAMF_IS_TAB_SOURCE (source), NULL);

  if (BAMF_TAB_SOURCE_GET_CLASS (source)->tab_preview)
    return BAMF_TAB_SOURCE_GET_CLASS (source)->tab_preview (source, tab_id);

  return NULL;
}

char *
bamf_tab_source_get_tab_uri     (BamfTabSource *source,
                                 char *tab_id)
{
  g_return_val_if_fail (BAMF_IS_TAB_SOURCE (source), NULL);

  if (BAMF_TAB_SOURCE_GET_CLASS (source)->tab_uri)
    return BAMF_TAB_SOURCE_GET_CLASS (source)->tab_uri (source, tab_id);

  return NULL;
}

guint32
bamf_tab_source_get_tab_xid     (BamfTabSource *source,
                                 char *tab_id)
{
  g_return_val_if_fail (BAMF_IS_TAB_SOURCE (source), 0);

  if (BAMF_TAB_SOURCE_GET_CLASS (source)->tab_xid)
    return BAMF_TAB_SOURCE_GET_CLASS (source)->tab_xid (source, tab_id);

  return 0;
}

static void
bamf_tab_source_set_property (GObject *object, guint property_id, const GValue *value, GParamSpec *pspec)
{
  BamfTabSource *self;

  self = BAMF_TAB_SOURCE (object);

  switch (property_id)
    {
      case PROP_ID:
        self->priv->id = g_strdup (g_value_get_string (value));
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
      case PROP_ID:
        g_value_set_string (value, self->priv->id);
        break;
      default:
        G_OBJECT_WARN_INVALID_PROPERTY_ID(object, property_id, pspec);
    }

}

static void
bamf_tab_source_dispose (GObject *object)
{
  G_OBJECT_CLASS (bamf_tab_source_parent_class)->dispose (object);
}

static void
bamf_tab_source_constructed (GObject *object)
{
  char *path;
  DBusGConnection *bus;
  DBusGProxy      *proxy;
  GError *error = NULL;

  if (G_OBJECT_CLASS (bamf_tab_source_parent_class)->constructed)
    G_OBJECT_CLASS (bamf_tab_source_parent_class)->constructed (object);

  bus = dbus_g_bus_get (DBUS_BUS_SESSION, &error);
  g_return_if_fail (bus);

  path = g_strdup_printf ("%s%s", BAMF_TAB_SOURCE_PATH, BAMF_TAB_SOURCE (object)->priv->id);

  dbus_g_connection_register_g_object (bus, path, object);

  proxy = dbus_g_proxy_new_for_name (bus,
                                     "org.ayatana.bamf",
                                     "/org/ayatana/bamf/control",
                                     "org.ayatana.bamf.control");

  error = NULL;
  if (!dbus_g_proxy_call (proxy,
                          "RegisterTabProvider",
                          &error,
                          G_TYPE_STRING, path,
                          G_TYPE_INVALID,
                          G_TYPE_INVALID))
    {
      g_warning ("Could not register tab source: %s", error->message);
      g_error_free (error);
    }

  g_object_unref (proxy);
}

static void
bamf_tab_source_class_init (BamfTabSourceClass *klass)
{
  GParamSpec *pspec;
  GObjectClass *object_class = G_OBJECT_CLASS (klass);

  object_class->constructed  = bamf_tab_source_constructed;
  object_class->dispose      = bamf_tab_source_dispose;
  object_class->get_property = bamf_tab_source_get_property;
  object_class->set_property = bamf_tab_source_set_property;

  pspec = g_param_spec_string ("id", "id", "id", NULL, G_PARAM_READWRITE | G_PARAM_CONSTRUCT);
  g_object_class_install_property (object_class, PROP_ID, pspec);

  g_type_class_add_private (object_class, sizeof (BamfTabSourcePrivate));

  dbus_g_object_type_install_info (BAMF_TYPE_TAB_SOURCE,
				   &dbus_glib_bamf_tab_source_object_info);

  bamf_tab_source_signals [TAB_URI_CHANGED] =
    g_signal_new (BAMF_TAB_SOURCE_SIGNAL_TAB_URI_CHANGED,
                  G_OBJECT_CLASS_TYPE (klass),
                  G_SIGNAL_RUN_FIRST,
                  0,
                  NULL, NULL,
                  _bamf_marshal_VOID__STRING_STRING_STRING,
                  G_TYPE_NONE, 3,
                  G_TYPE_STRING, G_TYPE_STRING, G_TYPE_STRING);

  bamf_tab_source_signals [TAB_OPENED] =
    g_signal_new (BAMF_TAB_SOURCE_SIGNAL_TAB_OPENED,
                  G_OBJECT_CLASS_TYPE (klass),
                  G_SIGNAL_RUN_FIRST,
                  0,
                  NULL, NULL,
                  g_cclosure_marshal_VOID__STRING,
                  G_TYPE_NONE, 1,
                  G_TYPE_STRING);

  bamf_tab_source_signals [TAB_CLOSED] =
    g_signal_new (BAMF_TAB_SOURCE_SIGNAL_TAB_CLOSED,
                  G_OBJECT_CLASS_TYPE (klass),
                  G_SIGNAL_RUN_FIRST,
                  0,
                  NULL, NULL,
                  g_cclosure_marshal_VOID__STRING,
                  G_TYPE_NONE, 1,
                  G_TYPE_STRING);
}

static void
bamf_tab_source_init (BamfTabSource *self)
{
  self->priv = BAMF_TAB_SOURCE_GET_PRIVATE (self);
}
