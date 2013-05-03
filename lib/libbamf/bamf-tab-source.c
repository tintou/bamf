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

#include <libbamf-private/bamf-private.h>
#include "bamf-tab-source.h"
#include "bamf-marshal.h"
#include "bamf-control.h"
#include "bamf-gdbus-tab-source-generated.h"

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
  BamfDBusTabTabsource *dbus_iface;
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

/**
 * bamf_tab_source_get_tab_ids:
 * @source: a #BamfTabSource
 *
 * Returns: (transfer none) (allow-none) (array zero-terminated=1): A string array containing the IDs of this #BamfTabSource.
 */
char **
bamf_tab_source_get_tab_ids     (BamfTabSource *source)
{
  g_return_val_if_fail (BAMF_IS_TAB_SOURCE (source), NULL);

  if (BAMF_TAB_SOURCE_GET_CLASS (source)->tab_ids)
    return BAMF_TAB_SOURCE_GET_CLASS (source)->tab_ids (source);

  return NULL;
}

/**
 * bamf_tab_source_get_tab_preview:
 * @source: a #BamfTabSource
 * @tab_id: an ID
 *
 * Returns: (transfer none) (allow-none): A #GArray containing the preview for the given ID of this #BamfTabSource.
 */
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

static gboolean
on_dbus_handle_show_tab (BamfDBusItemView *interface,
                         GDBusMethodInvocation *invocation,
                         gchar *tab_id,
                         BamfTabSource *self)
{
  bamf_tab_source_show_tab (self, tab_id, NULL);
  g_dbus_method_invocation_return_value (invocation, NULL);

  return TRUE;
}

static gboolean
on_dbus_handle_tab_ids (BamfDBusItemView *interface,
                        GDBusMethodInvocation *invocation,
                        BamfTabSource *self)
{
  GVariantBuilder b;
  int i = 0;
  char **tabs = bamf_tab_source_get_tab_ids (self);

  g_variant_builder_init (&b, G_VARIANT_TYPE ("(as)"));
  g_variant_builder_open (&b, G_VARIANT_TYPE ("as"));

  if (tabs)
    {
      while (tabs[i])
        {
          g_variant_builder_add (&b, "s", tabs[i]);
          ++i;
        }
    }

  g_variant_builder_close (&b);
  g_dbus_method_invocation_return_value (invocation, g_variant_builder_end (&b));

  return TRUE;
}

static gboolean
on_dbus_handle_tab_preview (BamfDBusItemView *interface,
                         GDBusMethodInvocation *invocation,
                         gchar *tab_id,
                         BamfTabSource *self)
{
  GVariantBuilder b;
  int i = 0;
  GArray *preview = bamf_tab_source_get_tab_preview (self, tab_id);

  g_variant_builder_init (&b, G_VARIANT_TYPE ("(ay)"));
  g_variant_builder_open (&b, G_VARIANT_TYPE ("ay"));

  if (preview)
    {
      for (i = 0; i < preview->len; ++i)
        {
          g_variant_builder_add (&b, "y", g_array_index (preview, guchar, i));
          ++i;
        }
    }

  g_variant_builder_close (&b);
  g_dbus_method_invocation_return_value (invocation, g_variant_builder_end (&b));

  return TRUE;
}

static gboolean
on_dbus_handle_tab_uri (BamfDBusItemView *interface,
                         GDBusMethodInvocation *invocation,
                         gchar *tab_id,
                         BamfTabSource *self)
{
  char *uri = bamf_tab_source_get_tab_uri (self, tab_id);
  g_dbus_method_invocation_return_value (invocation,
                                         g_variant_new ("(s)", uri ? uri : ""));
  g_free (uri);

  return TRUE;
}

static gboolean
on_dbus_handle_tab_xid (BamfDBusItemView *interface,
                         GDBusMethodInvocation *invocation,
                         gchar *tab_id,
                         BamfTabSource *self)
{
  guint32 xid = bamf_tab_source_get_tab_xid (self, tab_id);
  g_dbus_method_invocation_return_value (invocation, g_variant_new ("(u)", xid));

  return TRUE;
}

static void
on_tab_opened (BamfTabSource *self, const gchar *tab_uri, gpointer _not_used)
{
  g_return_if_fail (BAMF_IS_TAB_SOURCE (self));
  bamf_dbus_tab_tabsource_emit_tab_opened (self->priv->dbus_iface, self->priv->id, tab_uri);
}

static void
on_tab_closed (BamfTabSource *self, const gchar *tab_id, gpointer _not_used)
{
  g_return_if_fail (BAMF_IS_TAB_SOURCE (self));
  bamf_dbus_tab_tabsource_emit_tab_closed (self->priv->dbus_iface, self->priv->id);
}

static void
on_tab_uri_changed (BamfTabSource *self, const gchar *tab_id, const gchar *old_uri,
                    const gchar *new_uri, gpointer _not_used)
{
  g_return_if_fail (BAMF_IS_TAB_SOURCE (self));
  bamf_dbus_tab_tabsource_emit_tab_uri_changed (self->priv->dbus_iface, self->priv->id, old_uri, new_uri);
}

static void
bamf_tab_source_set_property (GObject *object, guint property_id, const GValue *value, GParamSpec *pspec)
{
  BamfTabSource *self;

  self = BAMF_TAB_SOURCE (object);

  switch (property_id)
    {
      case PROP_ID:
        g_free (self->priv->id);
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
  BamfTabSource *self = BAMF_TAB_SOURCE (object);

  if (self->priv->id)
    {
      g_free (self->priv->id);
      self->priv->id = NULL;
    }

  G_OBJECT_CLASS (bamf_tab_source_parent_class)->dispose (object);
}

static void
bamf_tab_source_constructed (GObject *object)
{
  BamfTabSource *self;
  BamfControl *control;
  GDBusConnection* bus;
  GDBusInterfaceSkeleton *iface;
  GError *error = NULL;
  char *path;

  self = BAMF_TAB_SOURCE (object);

  bus = g_bus_get_sync (G_BUS_TYPE_SESSION, NULL, NULL);
  g_return_if_fail (bus);

  self->priv->dbus_iface = bamf_dbus_tab_tabsource_skeleton_new ();
  path = g_strconcat (BAMF_TAB_SOURCE_PATH"/", self->priv->id, NULL);
  iface = G_DBUS_INTERFACE_SKELETON (self->priv->dbus_iface);
  g_dbus_interface_skeleton_export (iface, bus, path, &error);
  g_return_if_fail (!error);

  /* We need to connect to the object own signals to redirect them to the dbus
   * interface */
  g_signal_connect (self, "tab-opened", G_CALLBACK (on_tab_opened), NULL);
  g_signal_connect (self, "tab-closed", G_CALLBACK (on_tab_closed), NULL);
  g_signal_connect (self, "tab-uri-changed", G_CALLBACK (on_tab_uri_changed), NULL);

  /* Registering signal callbacks to reply to dbus method calls */
  g_signal_connect (self->priv->dbus_iface, "handle-show-tab",
                    G_CALLBACK (on_dbus_handle_show_tab), self);

  g_signal_connect (self->priv->dbus_iface, "handle-tab-ids",
                    G_CALLBACK (on_dbus_handle_tab_ids), self);

  g_signal_connect (self->priv->dbus_iface, "handle-tab-preview",
                    G_CALLBACK (on_dbus_handle_tab_preview), self);

  g_signal_connect (self->priv->dbus_iface, "handle-tab-uri",
                    G_CALLBACK (on_dbus_handle_tab_uri), self);

  g_signal_connect (self->priv->dbus_iface, "handle-tab-xid",
                    G_CALLBACK (on_dbus_handle_tab_xid), self);

  control = bamf_control_get_default ();
  bamf_control_register_tab_provider (control, path);

  g_object_unref (bus);
  g_object_unref (control);
  g_free (path);

  if (G_OBJECT_CLASS (bamf_tab_source_parent_class)->constructed)
    G_OBJECT_CLASS (bamf_tab_source_parent_class)->constructed (object);
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
