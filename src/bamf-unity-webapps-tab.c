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
 * Authored by: 
 *              Robert Carr <racarr@canonical.com>
 *
 */

#include "bamf-unity-webapps-tab.h"
#include "bamf-matcher.h"

#include "bamf-xutils.h"
#include "bamf-legacy-window.h"
#include "bamf-legacy-screen.h"

#include <unity-webapps-service.h>
#include <unity-webapps-context.h>

#define BAMF_UNITY_WEBAPPS_TAB_GET_PRIVATE(obj) (G_TYPE_INSTANCE_GET_PRIVATE(obj, \
BAMF_TYPE_UNITY_WEBAPPS_TAB, BamfUnityWebappsTabPrivate))

G_DEFINE_TYPE(BamfUnityWebappsTab, bamf_unity_webapps_tab, BAMF_TYPE_TAB);
	
enum
{
  PROP_0,
  PROP_CONTEXT,
  PROP_INTEREST_ID
};

enum
{
  VANISHED,
  LAST_SIGNAL
};

static guint unity_webapps_tab_signals[LAST_SIGNAL] = { 0 };

struct _BamfUnityWebappsTabPrivate
{
  UnityWebappsContext *context;
  
  gint interest_id;
  
  BamfLegacyWindow *legacy_window;
  
  gboolean tab_active;
};

static void
bamf_unity_webapps_tab_ensure_flags (BamfUnityWebappsTab *self)
{
  gboolean window_active;
  
  window_active = bamf_legacy_window_is_active (self->priv->legacy_window);
  
  bamf_view_set_active (BAMF_VIEW (self), window_active && self->priv->tab_active);
}

static void
bamf_unity_webapps_tab_active_window_changed (BamfLegacyScreen *screen, BamfUnityWebappsTab *tab)
{
  bamf_unity_webapps_tab_ensure_flags (tab);
}

static void
bamf_unity_webapps_tab_create_bamf_window (BamfUnityWebappsTab *self,
					   gulong xid)
{
  self->priv->legacy_window = bamf_legacy_window_new (wnck_window_get (xid));
}


static void
bamf_unity_webapps_tab_vanished (BamfUnityWebappsTab *self,
					  gpointer user_data)
{
  self->priv->interest_id = -1;
  self->priv->context = NULL;
}

static void
bamf_unity_webapps_tab_location_changed (UnityWebappsContext *context,
					 gint interest_id,
					 const gchar *location,
					 gpointer user_data)
{
  BamfUnityWebappsTab *self;
  
  self = (BamfUnityWebappsTab *)user_data;
  
  if ((self->priv->interest_id != interest_id) || (self->priv->interest_id == -1))
    {
      return;
    }
  
  g_object_set (self, "location", location, NULL);
}

static void
bamf_unity_webapps_tab_window_changed (UnityWebappsContext *context,
				       gint interest_id,
				       guint64 xid,
				       gpointer user_data)
{
  BamfUnityWebappsTab *self;
  
  self = (BamfUnityWebappsTab *)user_data;
  
  if ((self->priv->interest_id != interest_id) || (self->priv->interest_id == -1))
    {
      return;
    }
  
  g_object_set (self, "xid", xid, NULL);
  
  bamf_unity_webapps_tab_create_bamf_window (self, xid);
  bamf_unity_webapps_tab_ensure_flags (self);
}

static void
bamf_unity_webapps_tab_active_changed (UnityWebappsContext *context,
				       gint interest_id,
				       gboolean is_active,
				       gpointer user_data)
{
  BamfUnityWebappsTab *self;
  
  self = (BamfUnityWebappsTab *)user_data;
  
  if ((self->priv->interest_id != interest_id) || (self->priv->interest_id == -1))
    {
      return;
    }
  
  self->priv->tab_active = is_active;
  bamf_unity_webapps_tab_ensure_flags (self);
}

static void
bamf_unity_webapps_tab_initialize_properties (BamfUnityWebappsTab *self)
{
  gchar *location;
  guint64 xid;
  gboolean is_active;
  
  location = unity_webapps_context_get_view_location (self->priv->context, self->priv->interest_id);
  xid = unity_webapps_context_get_view_window (self->priv->context, self->priv->interest_id);
  is_active = unity_webapps_context_get_view_is_active (self->priv->context, self->priv->interest_id);
  
  g_object_set (self, "location", location, "xid", xid, NULL);
  
  self->priv->tab_active = is_active;
  bamf_unity_webapps_tab_create_bamf_window (self, xid);
  
  bamf_unity_webapps_tab_ensure_flags (self);
  
  g_free (location);
}


static void
bamf_unity_webapps_tab_interest_id_set (BamfUnityWebappsTab *self)
{
  unity_webapps_context_on_view_location_changed (self->priv->context, bamf_unity_webapps_tab_location_changed,
						  self);
  unity_webapps_context_on_view_window_changed (self->priv->context, bamf_unity_webapps_tab_window_changed,
						self);
  unity_webapps_context_on_view_is_active_changed (self->priv->context, bamf_unity_webapps_tab_active_changed,
						   self);
  
  bamf_unity_webapps_tab_initialize_properties (self);

  bamf_view_set_running (BAMF_VIEW (self), TRUE);
  bamf_view_set_user_visible (BAMF_VIEW (self), TRUE);
  
  bamf_matcher_register_view (bamf_matcher_get_default (), BAMF_VIEW (self));
}


static void
bamf_unity_webapps_tab_get_property (GObject *object, guint property_id, GValue *gvalue, GParamSpec *pspec)
{
  BamfUnityWebappsTab *self;
  
  self = BAMF_UNITY_WEBAPPS_TAB (object);
  
  switch (property_id)
    {
    case PROP_CONTEXT:
      g_value_set_object (gvalue, self->priv->context);
      break;
    case PROP_INTEREST_ID:
      g_value_set_int (gvalue, self->priv->interest_id);
      break;
    default:
      G_OBJECT_WARN_INVALID_PROPERTY_ID (object, property_id, pspec);
    }
}

static void
bamf_unity_webapps_tab_set_property (GObject *object, guint property_id, const GValue *gvalue, GParamSpec *pspec)
{
  BamfUnityWebappsTab *self;
  
  self = BAMF_UNITY_WEBAPPS_TAB (object);
  
  switch (property_id)
    {
    case PROP_CONTEXT:
      g_assert (self->priv->context == NULL);
      self->priv->context = g_value_get_object (gvalue);
      self->priv->context = unity_webapps_context_new_for_context_name (unity_webapps_context_get_service (self->priv->context), 
									unity_webapps_context_get_context_name (self->priv->context));
      break;
    case PROP_INTEREST_ID:
      g_assert (self->priv->interest_id == 0);
      self->priv->interest_id = g_value_get_int (gvalue);
      bamf_unity_webapps_tab_interest_id_set (self);
      break;
    default:
      G_OBJECT_WARN_INVALID_PROPERTY_ID(object, property_id, pspec);
    }
}


static void
bamf_unity_webapps_tab_dispose (GObject *object)
{
  //  BamfUnityWebappsTab *self = BAMF_UNITY_WEBAPPS_TAB (object);
  
    
  G_OBJECT_CLASS (bamf_unity_webapps_tab_parent_class)->dispose (object);
}

static void
bamf_unity_webapps_tab_finalize (GObject *object)
{
  BamfUnityWebappsTab *self = BAMF_UNITY_WEBAPPS_TAB (object);
  
  g_object_unref (G_OBJECT (self->priv->context));
  
  
  G_OBJECT_CLASS (bamf_unity_webapps_tab_parent_class)->finalize (object);
}

static void
bamf_unity_webapps_tab_init (BamfUnityWebappsTab *self)
{
  self->priv = BAMF_UNITY_WEBAPPS_TAB_GET_PRIVATE (self);
  
  self->priv->tab_active = FALSE;
  self->priv->legacy_window = NULL;
  
  g_signal_connect (G_OBJECT (bamf_legacy_screen_get_default ()), "active-window-changed",
		    (GCallback) bamf_unity_webapps_tab_active_window_changed, self);

}

static void
bamf_unity_webapps_tab_raise (BamfTab *tab)
{
  BamfUnityWebappsTab *self = BAMF_UNITY_WEBAPPS_TAB (tab);

  if (self->priv->interest_id == -1)
    {
      return;
    }
  
  unity_webapps_context_raise_interest (self->priv->context, self->priv->interest_id);
}

static void
bamf_unity_webapps_tab_close (BamfTab *tab)
{
  BamfUnityWebappsTab *self = BAMF_UNITY_WEBAPPS_TAB (tab);
  
  if (self->priv->interest_id == -1)
    {
      return;
    }
  
  unity_webapps_context_close_interest (self->priv->context, self->priv->interest_id);
}

typedef struct _bamf_unity_webapps_preview_data {
  BamfUnityWebappsTab *tab;
  BamfTabPreviewReadyCallback callback;
  gpointer user_data;
} bamf_unity_webapps_preview_data;

static void
bamf_unity_webapps_tab_preview_ready (UnityWebappsContext *context, 
				      gint interest_id,
				      const gchar *preview_data,
				      gpointer user_data)
{
  bamf_unity_webapps_preview_data *data;
  
  data = (bamf_unity_webapps_preview_data *)user_data;

  data->callback ((BamfTab *)data->tab, preview_data, data->user_data);
  
  g_slice_free1 (sizeof (bamf_unity_webapps_preview_data), data);
}

static void
bamf_unity_webapps_tab_request_preview (BamfTab *tab, 
					BamfTabPreviewReadyCallback callback, 
					gpointer user_data)
{
  BamfUnityWebappsTab *self;
  bamf_unity_webapps_preview_data *data;
  
  self = BAMF_UNITY_WEBAPPS_TAB (tab);

  data = g_slice_alloc0 (sizeof (bamf_unity_webapps_preview_data));
  
  data->tab = self;
  data->callback = callback;
  data->user_data = user_data;
  
  unity_webapps_context_request_preview (self->priv->context,
					 self->priv->interest_id,
					 bamf_unity_webapps_tab_preview_ready,
					 data);  

}

static void
bamf_unity_webapps_tab_class_init (BamfUnityWebappsTabClass * klass)
{
  GParamSpec *pspec;
  GObjectClass *object_class = G_OBJECT_CLASS (klass);
  BamfTabClass *bamf_tab_class = BAMF_TAB_CLASS (klass);
  
  object_class->get_property = bamf_unity_webapps_tab_get_property;
  object_class->set_property = bamf_unity_webapps_tab_set_property;
  object_class->dispose = bamf_unity_webapps_tab_dispose;
  object_class->finalize = bamf_unity_webapps_tab_finalize;
  
  bamf_tab_class->raise = bamf_unity_webapps_tab_raise;
  bamf_tab_class->close = bamf_unity_webapps_tab_close;
  bamf_tab_class->request_preview = bamf_unity_webapps_tab_request_preview;
  
  pspec = g_param_spec_object("context", "Context", "The Unity Webapps Context assosciated with the Tab",
			      UNITY_WEBAPPS_TYPE_CONTEXT, G_PARAM_READWRITE | G_PARAM_CONSTRUCT_ONLY);
  g_object_class_install_property (object_class, PROP_CONTEXT, pspec);
  
  pspec = g_param_spec_int("interest-id", "Interest ID", "The Interest ID (unique to Context) for this Tab",
			   G_MININT, G_MAXINT, -1, G_PARAM_READWRITE | G_PARAM_CONSTRUCT_ONLY);
  g_object_class_install_property (object_class, PROP_INTEREST_ID, pspec);
  
  unity_webapps_tab_signals [VANISHED] =
    g_signal_new_class_handler("vanished",
			       G_OBJECT_CLASS_TYPE (klass),
			       G_SIGNAL_RUN_FIRST,
			       G_CALLBACK (bamf_unity_webapps_tab_vanished),
			       NULL, NULL,
			       g_cclosure_marshal_VOID__VOID,
			       G_TYPE_NONE, 0);
  
  g_type_class_add_private (klass, sizeof (BamfUnityWebappsTabPrivate));
}


gint 
bamf_unity_webapps_tab_get_interest_id (BamfUnityWebappsTab *tab)
{
  return tab->priv->interest_id;
}

BamfUnityWebappsTab *
bamf_unity_webapps_tab_new (UnityWebappsContext *context, gint interest_id)
{
  return (BamfUnityWebappsTab *)g_object_new (BAMF_TYPE_UNITY_WEBAPPS_TAB, "context", context, "interest-id", interest_id, NULL);
}
