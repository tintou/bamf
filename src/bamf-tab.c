/*
 * Copyright (C) 2012 Canonical Ltd
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
 * Authored by: 
 *              Robert Carr <racarr@canonical.com>
 *
 */

#include "bamf-tab.h"
#include "bamf-gdbus-view-generated.h"

#define BAMF_TAB_GET_PRIVATE(obj) (G_TYPE_INSTANCE_GET_PRIVATE(obj, \
BAMF_TYPE_TAB, BamfTabPrivate))

static void bamf_tab_dbus_iface_init (BamfDBusItemTabIface *iface);
G_DEFINE_ABSTRACT_TYPE_WITH_CODE (BamfTab, bamf_tab, BAMF_TYPE_VIEW,
				  G_IMPLEMENT_INTERFACE (BAMF_DBUS_ITEM_TYPE_TAB,
							 bamf_tab_dbus_iface_init));

enum
{
  PROP_0,
  PROP_LOCATION,
  PROP_XID,
  PROP_DESKTOP_ID,
  PROP_IS_FOREGROUND_TAB
};

struct _BamfTabPrivate
{
  BamfDBusItemTab *dbus_iface;

  char *location;
  gchar *desktop_id;

  guint64 xid;
  gboolean is_foreground;
};

static const gchar *
bamf_tab_get_view_type (BamfView *view)
{
  return "tab";
}

static void
bamf_tab_get_property (GObject *object, guint property_id, GValue *value, GParamSpec *pspec)
{
  BamfTab *self;
  
  self = BAMF_TAB (object);
  
  switch (property_id)
    {
    case PROP_LOCATION:
      g_value_set_string (value, self->priv->location);
      break;
    case PROP_XID:
      g_value_set_uint64  (value, self->priv->xid);
      break;
    case PROP_DESKTOP_ID:
      g_value_set_string (value, self->priv->desktop_id);
      break;
    case PROP_IS_FOREGROUND_TAB:
      g_value_set_boolean (value, self->priv->is_foreground);
      break;
    default:
      G_OBJECT_WARN_INVALID_PROPERTY_ID (object, property_id, pspec);
    }
}

static void
bamf_tab_set_property (GObject *object, guint property_id, const GValue *value, GParamSpec *pspec)
{
  BamfTab *self;
  
  self = BAMF_TAB (object);
  
  switch (property_id)
    {
    case PROP_LOCATION:
      if (self->priv->location != NULL)
	{
	  g_free (self->priv->location);
	}
      self->priv->location = g_value_dup_string (value);
      bamf_dbus_item_tab_set_location (self->priv->dbus_iface, self->priv->location);

      break;
    case PROP_DESKTOP_ID:
      if (self->priv->desktop_id != NULL)
	{
	  g_free (self->priv->desktop_id);
	}
      self->priv->desktop_id = g_value_dup_string (value);
      bamf_dbus_item_tab_set_desktop_id (self->priv->dbus_iface, self->priv->desktop_id);
      
      break;
    case PROP_XID:
      self->priv->xid = g_value_get_uint64 (value);
      bamf_dbus_item_tab_set_xid (self->priv->dbus_iface, self->priv->xid);

      break;
    case PROP_IS_FOREGROUND_TAB:
      self->priv->is_foreground = g_value_get_boolean (value);
      bamf_dbus_item_tab_set_is_foreground_tab (self->priv->dbus_iface, self->priv->is_foreground);
    default:
      G_OBJECT_WARN_INVALID_PROPERTY_ID(object, property_id, pspec);
    }
}

static void
bamf_tab_finalize (GObject *object)
{
  BamfTab *self = BAMF_TAB (object);

  if (self->priv->location)
    {
      g_free (self->priv->location);
    }
  
  if (self->priv->desktop_id)
    {
      g_free (self->priv->desktop_id);
    }
  
  g_object_unref (self->priv->dbus_iface);
  
  G_OBJECT_CLASS (bamf_tab_parent_class)->finalize (object);
}


static gboolean
on_dbus_handle_raise (BamfDBusItemView *interface,
		      GDBusMethodInvocation *invocation,
		      BamfTab *self)
{
  bamf_tab_raise (self);
  
  g_dbus_method_invocation_return_value (invocation, NULL);
  
  return TRUE;
}

static gboolean
on_dbus_handle_close (BamfDBusItemView *interface,
		      GDBusMethodInvocation *invocation,
		      BamfTab *self)
{
  bamf_tab_close (self);
  
  g_dbus_method_invocation_return_value (invocation, NULL);
  
  return TRUE;
}

static void
bamf_tab_preview_ready (BamfTab *self,
			const gchar *preview_data,
			gpointer user_data)
{
  GDBusMethodInvocation *invocation;
  
  invocation = (GDBusMethodInvocation *)user_data;
  
  g_dbus_method_invocation_return_value (invocation, g_variant_new ("(s)", preview_data));
}

static gboolean
on_dbus_handle_request_preview (BamfDBusItemView *interface,
				GDBusMethodInvocation *invocation,
				BamfTab *self)
{
  bamf_tab_request_preview (self, bamf_tab_preview_ready, invocation);
  
  return TRUE;
}

static void
bamf_tab_init (BamfTab *self)
{
  self->priv = BAMF_TAB_GET_PRIVATE (self);
  
  self->priv->dbus_iface = bamf_dbus_item_tab_skeleton_new ();
  
  g_signal_connect (self->priv->dbus_iface, "handle-raise",
		    G_CALLBACK (on_dbus_handle_raise), self);
  g_signal_connect (self->priv->dbus_iface, "handle-close",
		    G_CALLBACK (on_dbus_handle_close), self);
  g_signal_connect (self->priv->dbus_iface, "handle-request-preview",
		    G_CALLBACK (on_dbus_handle_request_preview), self);
  
  bamf_dbus_item_object_skeleton_set_tab (BAMF_DBUS_ITEM_OBJECT_SKELETON (self),
					  self->priv->dbus_iface);
}


static void
bamf_tab_dbus_iface_init (BamfDBusItemTabIface *iface)
{
}

static void
bamf_tab_class_init (BamfTabClass * klass)
{
  GParamSpec *pspec;
  GObjectClass *object_class = G_OBJECT_CLASS (klass);
  BamfViewClass *view_class = BAMF_VIEW_CLASS (klass);
  
  object_class->get_property = bamf_tab_get_property;
  object_class->set_property = bamf_tab_set_property;
  object_class->finalize = bamf_tab_finalize;
  view_class->view_type = bamf_tab_get_view_type;
  
  pspec = g_param_spec_string("location", "Location", "The Current location of the remote Tab",
			      NULL, G_PARAM_READWRITE);
  g_object_class_install_property (object_class, PROP_LOCATION, pspec);
  
  pspec = g_param_spec_string("desktop-id", "Desktop ID", "The Desktop ID assosciated with the application hosted in the remote Tab",
			      NULL, G_PARAM_READWRITE);
  g_object_class_install_property (object_class, PROP_DESKTOP_ID, pspec);
  
  pspec = g_param_spec_uint64("xid", "xid", "XID for the toplevel window containing the remote Tab",
			      0, G_MAXUINT64, 0, G_PARAM_READWRITE);
  g_object_class_install_property (object_class, PROP_XID, pspec);
  
  pspec = g_param_spec_boolean ("is-foreground-tab", "Foreground Tab", "Is this tab the foreground tab in it's toplevel container",
			     FALSE, G_PARAM_READWRITE);
  g_object_class_install_property (object_class, PROP_IS_FOREGROUND_TAB, pspec);
  
  
  g_type_class_add_private (klass, sizeof (BamfTabPrivate));
}


const gchar *
bamf_tab_get_location (BamfTab *self)
{
  g_return_val_if_fail (BAMF_IS_TAB (self), NULL);

  return self->priv->location;
}

const gchar *
bamf_tab_get_desktop_id (BamfTab *self)
{
  g_return_val_if_fail (BAMF_IS_TAB (self), NULL);
  
  return self->priv->desktop_id;
}

guint64
bamf_tab_get_xid (BamfTab *self)
{
  g_return_val_if_fail (BAMF_IS_TAB (self), 0);
  
  return self->priv->xid;
}

gboolean
bamf_tab_get_is_foreground_tab (BamfTab *self)
{
  g_return_val_if_fail (BAMF_IS_TAB (self), 0);
  
  return self->priv->is_foreground;
}

void
bamf_tab_raise (BamfTab *self)
{
  g_return_if_fail (BAMF_IS_TAB (self));
  
  BAMF_TAB_GET_CLASS (self)->raise (self);
}

void 
bamf_tab_close (BamfTab *self)
{
  g_return_if_fail (BAMF_IS_TAB (self));
  
  BAMF_TAB_GET_CLASS (self)->close (self);
}

void 
bamf_tab_request_preview (BamfTab *self, BamfTabPreviewReadyCallback callback, gpointer user_data)
{
  g_return_if_fail (BAMF_IS_TAB (self));
  
  BAMF_TAB_GET_CLASS (self)->request_preview (self, callback, user_data);
}
