/*
 * Copyright (C) 2010-2011 Canonical Ltd
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 3 as
 * published by the Free Software Foundation.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANAPPLICATIONILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 *
 * Authored by: 
 *              Robert Carr <racarr@canonical.com>
 *
 */

#include "bamf-unity-webapps-application.h"
#include "bamf-unity-webapps-tab.h"

#include <unity-webapps-context.h>

#define BAMF_UNITY_WEBAPPS_APPLICATION_GET_PRIVATE(obj) (G_TYPE_INSTANCE_GET_PRIVATE(obj, \
BAMF_TYPE_UNITY_WEBAPPS_APPLICATION, BamfUnityWebappsApplicationPrivate))

G_DEFINE_TYPE(BamfUnityWebappsApplication, bamf_unity_webapps_application, BAMF_TYPE_APPLICATION);
	
enum
{
  PROP_0,
  PROP_CONTEXT,
};

/*enum
{
  VANISHED,
  LAST_SIGNAL
};

static guint unity_webapps_application_signals[LAST_SIGNAL] = { 0 };*/

struct _BamfUnityWebappsApplicationPrivate
{
  UnityWebappsContext *context;
};




static void
bamf_unity_webapps_application_get_property (GObject *object, guint property_id, GValue *gvalue, GParamSpec *pspec)
{
  BamfUnityWebappsApplication *self;
  
  self = BAMF_UNITY_WEBAPPS_APPLICATION (object);
  
  switch (property_id)
    {
    case PROP_CONTEXT:
      g_value_set_object (gvalue, self->priv->context);
      break;
    default:
      G_OBJECT_WARN_INVALID_PROPERTY_ID (object, property_id, pspec);
    }
}

static void
bamf_unity_webapps_application_interest_appeared (UnityWebappsContext *context,
						  gint interest_id,
						  gpointer user_data)
{
  BamfUnityWebappsApplication *self;
  BamfView *interest_view;
  
  self = (BamfUnityWebappsApplication *)user_data;
  
  interest_view = (BamfView *)bamf_unity_webapps_tab_new (context, interest_id);
  
  bamf_view_add_child (BAMF_VIEW (self), interest_view);
  
  bamf_view_set_running (BAMF_VIEW (self), TRUE);
}

static void
bamf_unity_webapps_application_interest_vanished (UnityWebappsContext *context,
						  gint interest_id,
						  gpointer user_data)
{
  BamfUnityWebappsApplication *self;
  BamfUnityWebappsTab *child;
  GList *children, *walk;
  int i = 0;
  
  self = (BamfUnityWebappsApplication *)user_data;
  
  children = bamf_view_get_children (BAMF_VIEW (self));
  
  for (walk = children; walk != NULL; walk = walk->next)
    {
      child = BAMF_UNITY_WEBAPPS_TAB (walk->data);
      i++;
      if (interest_id == bamf_unity_webapps_tab_get_interest_id (child))
	{
	  bamf_view_close (BAMF_VIEW (child));
	  g_object_unref (BAMF_VIEW (child));
	}
    }
  
  if (i == 1)
    bamf_view_close (BAMF_VIEW (self));
}

static void
bamf_unity_webapps_application_add_existing_interests (BamfUnityWebappsApplication *self)
{
  GVariant *interests, *interest_variant;
  GVariantIter *variant_iter;
  
  interests = unity_webapps_context_list_interests (self->priv->context);
  
  if (interests == NULL)
    {
      return;
    }
  
  variant_iter = g_variant_iter_new (interests);
  
  while ((interest_variant = g_variant_iter_next_value (variant_iter)))
    {
      gint interest_id;
      
      interest_id = g_variant_get_int32 (interest_variant);
      
      bamf_unity_webapps_application_interest_appeared (self->priv->context, interest_id, self);
    }
}

static void
bamf_unity_webapps_application_context_set (BamfUnityWebappsApplication *self)
{
  gchar *desktop_file = g_strdup_printf("%s/.local/share/applications/%s", g_get_home_dir (), unity_webapps_context_get_desktop_name (self->priv->context));
  bamf_application_set_desktop_file (BAMF_APPLICATION (self), desktop_file);
  
  bamf_unity_webapps_application_add_existing_interests (self);
  
  unity_webapps_context_on_interest_appeared (self->priv->context, bamf_unity_webapps_application_interest_appeared, self);
  unity_webapps_context_on_interest_vanished (self->priv->context, bamf_unity_webapps_application_interest_vanished, self);
}

static void
bamf_unity_webapps_application_set_property (GObject *object, guint property_id, const GValue *gvalue, GParamSpec *pspec)
{
  BamfUnityWebappsApplication *self;
  
  self = BAMF_UNITY_WEBAPPS_APPLICATION (object);
  
  switch (property_id)
    {
    case PROP_CONTEXT:
      g_assert (self->priv->context == NULL);
      self->priv->context = g_value_get_object (gvalue);
      
      bamf_unity_webapps_application_context_set (self);
      break;
    default:
      G_OBJECT_WARN_INVALID_PROPERTY_ID(object, property_id, pspec);
    }
}


static void
bamf_unity_webapps_application_dispose (GObject *object)
{
  //  BamfUnityWebappsApplication *self = BAMF_UNITY_WEBAPPS_APPLICATION (object);
  
    
  G_OBJECT_CLASS (bamf_unity_webapps_application_parent_class)->dispose (object);
}

static void
bamf_unity_webapps_application_finalize (GObject *object)
{
  BamfUnityWebappsApplication *self = BAMF_UNITY_WEBAPPS_APPLICATION (object);
  
  g_object_unref (self->priv->context);
  
  
  G_OBJECT_CLASS (bamf_unity_webapps_application_parent_class)->finalize (object);
}

static void
bamf_unity_webapps_application_init (BamfUnityWebappsApplication *self)
{
  self->priv = BAMF_UNITY_WEBAPPS_APPLICATION_GET_PRIVATE (self);
}

static void
bamf_unity_webapps_application_class_init (BamfUnityWebappsApplicationClass * klass)
{
  GParamSpec *pspec;
  GObjectClass *object_class = G_OBJECT_CLASS (klass);
  //  BamfApplicationClass *bamf_application_class = BAMF_APPLICATION_CLASS (klass);
  
  object_class->get_property = bamf_unity_webapps_application_get_property;
  object_class->set_property = bamf_unity_webapps_application_set_property;
  object_class->dispose = bamf_unity_webapps_application_dispose;
  object_class->finalize = bamf_unity_webapps_application_finalize;
  
  //  bamf_application_class->raise = bamf_unity_webapps_application_raise;
  //  bamf_application_class->request_preview = bamf_unity_webapps_application_request_preview;
  
  pspec = g_param_spec_object("context", "Context", "The Unity Webapps Context assosciated with the Application",
			      UNITY_WEBAPPS_TYPE_CONTEXT, G_PARAM_READWRITE | G_PARAM_CONSTRUCT_ONLY);
  g_object_class_install_property (object_class, PROP_CONTEXT, pspec);
  
  //  pspec = g_param_spec_int("interest-id", "Interest ID", "The Interest ID (unique to Context) for this Application",
  //			   G_MININT, G_MAXINT, -1, G_PARAM_READWRITE | G_PARAM_CONSTRUCT_ONLY);
  
  
  g_type_class_add_private (klass, sizeof (BamfUnityWebappsApplicationPrivate));
}


BamfApplication *
bamf_unity_webapps_application_new (UnityWebappsContext *context)
{
  return (BamfApplication *)g_object_new (BAMF_TYPE_UNITY_WEBAPPS_APPLICATION, "context", context, NULL);
}

UnityWebappsContext *
bamf_unity_webapps_application_get_context (BamfUnityWebappsApplication *application)
{
  return application->priv->context;
}
