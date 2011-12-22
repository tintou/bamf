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

#include "bamf-tab.h"
#include "bamf-tab-source.h"

#define BAMF_TAB_GET_PRIVATE(obj) (G_TYPE_INSTANCE_GET_PRIVATE(obj, \
BAMF_TYPE_TAB, BamfTabPrivate))

static void bamf_tab_dbus_iface_init (BamfDBusTabIface *iface);
G_DEFINE_TYPE_WITH_CODE (BamfTab, bamf_tab, BAMF_TYPE_VIEW,
                         G_IMPLEMENT_INTERFACE (BAMF_DBUS_TYPE_TAB,
                                                bamf_tab_dbus_iface_init));

enum
{
  PROP_0,

  PROP_ID,
  PROP_SOURCE,
};

struct _BamfTabPrivate
{
  BamfDBusTab *dbus_iface;
  char *id;
  char *uri;
  BamfTabSource *source;
};

char *
bamf_tab_current_uri (BamfTab *self)
{
  g_return_val_if_fail (BAMF_IS_TAB (self), NULL);

  return g_strdup (self->priv->uri);
}

void
bamf_tab_show (BamfTab *tab)
{
  g_return_if_fail (BAMF_IS_TAB (tab));

  bamf_tab_source_show_tab (tab->priv->source, tab->priv->id);
}

guint32 bamf_tab_parent_xid (BamfTab *tab)
{
  g_return_val_if_fail (BAMF_IS_TAB (tab), 0);

  return bamf_tab_source_get_tab_xid (tab->priv->source, tab->priv->id);
}

gchar *
bamf_tab_get_preview (BamfTab *tab)
{
  return bamf_tab_source_get_tab_preview (tab->priv->source, tab->priv->id);
}

static const char *
bamf_tab_get_view_type (BamfView *view)
{
  return "tab";
}

static void
on_tab_source_uri_changed (BamfTabSource *source, char *id,
                           char *old_uri, char *new_uri, BamfTab *self)
{
  if (g_strcmp0 (id, self->priv->id) != 0)
    return;

  g_free (self->priv->uri);
  self->priv->uri = g_strdup (new_uri);

  g_signal_emit_by_name (self, "uri-changed", old_uri, new_uri);
}

static void
on_uri_changed (BamfTab *self, const gchar *old_uri, const gchar *new_uri, gpointer _not_used)
{
  g_return_if_fail (BAMF_IS_TAB (self));
  g_signal_emit_by_name (self->priv->dbus_iface, "uri-changed", old_uri, new_uri);
}

static gboolean
on_dbus_handle_show_tab (BamfDBusTab *interface,
                         GDBusMethodInvocation *invocation,
                         BamfTab *self)
{
  bamf_tab_show (self);
  g_dbus_method_invocation_return_value (invocation, NULL);

  return TRUE;
}

static gboolean
on_dbus_handle_parent_xid (BamfDBusTab *interface,
                           GDBusMethodInvocation *invocation,
                           BamfTab *self)
{
  guint32 parent_xid = bamf_tab_parent_xid (self);
  g_dbus_method_invocation_return_value (invocation,
                                         g_variant_new ("(u)", parent_xid));

  return TRUE;
}

static gboolean
on_dbus_handle_current_uri (BamfDBusTab *interface,
                            GDBusMethodInvocation *invocation,
                            BamfTab *self)
{
  char *current_uri = self->priv->uri ? self->priv->uri : "";
  g_dbus_method_invocation_return_value (invocation,
                                         g_variant_new ("(s)", current_uri));

  return TRUE;
}

static gboolean
on_dbus_handle_preview (BamfDBusTab *interface,
                        GDBusMethodInvocation *invocation,
                        BamfTab *self)
{
  gchar *preview = bamf_tab_get_preview (self);

  if (preview)
    {
      bamf_dbus_tab_complete_preview (interface, invocation, preview);
      g_free (preview);
    }
  else
    {
      g_dbus_method_invocation_return_value (invocation,
                                             g_variant_new ("(ay)", NULL));
    }

  return TRUE;
}

static void
bamf_tab_set_property (GObject *object, guint property_id, const GValue *value, GParamSpec *pspec)
{
  BamfTab *self;

  self = BAMF_TAB (object);

  switch (property_id)
    {
      case PROP_ID:
        self->priv->id = g_value_dup_string (value);
        break;

      case PROP_SOURCE:
        self->priv->source = g_value_get_object (value);
        break;

      default:
        G_OBJECT_WARN_INVALID_PROPERTY_ID(object, property_id, pspec);
    }
}

static void
bamf_tab_get_property (GObject *object, guint property_id, GValue *value, GParamSpec *pspec)
{
  BamfTab *self;

  self = BAMF_TAB (object);

  switch (property_id)
    {
      case PROP_ID:
        g_value_set_string (value, self->priv->id);
        break;

      case PROP_SOURCE:
        g_value_set_object (value, self->priv->source);
        break;

      default:
        G_OBJECT_WARN_INVALID_PROPERTY_ID(object, property_id, pspec);
    }
}

static void
bamf_tab_constructed (GObject *object)
{
  BamfTab *self;

  if (G_OBJECT_CLASS (bamf_tab_parent_class)->constructed)
    G_OBJECT_CLASS (bamf_tab_parent_class)->constructed (object);

  self = BAMF_TAB (object);

  g_signal_connect (self->priv->source, "remote-tab-uri-changed",
                    G_CALLBACK (on_tab_source_uri_changed), self);
}

static void
bamf_tab_dispose (GObject *object)
{
  BamfTab *self = BAMF_TAB (object);

  if (self->priv->id)
    {
      g_free (self->priv->id);
      self->priv->id = NULL;
    }

  if (self->priv->uri)
    {
      g_free (self->priv->uri);
      self->priv->uri = NULL;
    }

  G_OBJECT_CLASS (bamf_tab_parent_class)->dispose (object);
}

static void
bamf_tab_finalize (GObject *object)
{
  BamfTab *self = BAMF_TAB (object);

  g_object_unref (self->priv->dbus_iface);

  G_OBJECT_CLASS (bamf_tab_parent_class)->finalize (object);
}

static void
bamf_tab_init (BamfTab * self)
{
  self->priv = BAMF_TAB_GET_PRIVATE (self);

  /* Initializing the dbus interface */
  self->priv->dbus_iface = bamf_dbus_tab_skeleton_new ();

  /* We need to connect to the object own signals to redirect them to the dbus
   * interface                                                                */
  g_signal_connect (self, "uri-changed", G_CALLBACK (on_uri_changed), NULL);

  /* Registering signal callbacks to reply to dbus method calls */
  g_signal_connect (self->priv->dbus_iface, "handle-show-tab",
                    G_CALLBACK (on_dbus_handle_show_tab), self);

  g_signal_connect (self->priv->dbus_iface, "handle-parent-xid",
                    G_CALLBACK (on_dbus_handle_parent_xid), self);

  g_signal_connect (self->priv->dbus_iface, "handle-current-uri",
                    G_CALLBACK (on_dbus_handle_current_uri), self);

  g_signal_connect (self->priv->dbus_iface, "handle-preview",
                    G_CALLBACK (on_dbus_handle_preview), self);

  /* Setting the interface for the dbus object */
  bamf_dbus_object_skeleton_set_tab (BAMF_DBUS_OBJECT_SKELETON (self),
                                     self->priv->dbus_iface);
}

static void bamf_tab_dbus_iface_init (BamfDBusTabIface *iface)
{
}

static void
bamf_tab_class_init (BamfTabClass * klass)
{
  GParamSpec *pspec;
  GObjectClass *object_class = G_OBJECT_CLASS (klass);
  BamfViewClass *view_class = BAMF_VIEW_CLASS (klass);

  object_class->constructed  = bamf_tab_constructed;
  object_class->get_property = bamf_tab_get_property;
  object_class->set_property = bamf_tab_set_property;
  object_class->dispose      = bamf_tab_dispose;
  object_class->finalize     = bamf_tab_finalize;
  view_class->view_type      = bamf_tab_get_view_type;

  pspec = g_param_spec_string ("id", "id", "id", NULL, G_PARAM_READWRITE | G_PARAM_CONSTRUCT);
  g_object_class_install_property (object_class, PROP_ID, pspec);

  pspec = g_param_spec_object ("source", "source", "source", BAMF_TYPE_TAB_SOURCE, G_PARAM_READWRITE | G_PARAM_CONSTRUCT);
  g_object_class_install_property (object_class, PROP_SOURCE, pspec);

  g_type_class_add_private (klass, sizeof (BamfTabPrivate));
}

BamfTab *
bamf_tab_new (BamfTabSource *source, const char *id)
{
  BamfTab *self;
  self = (BamfTab *) g_object_new (BAMF_TYPE_TAB,
                                   "source", source,
                                   "id", id,
                                   NULL);

  return self;
}
