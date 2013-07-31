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

#include <stdlib.h>


#include "bamf-unity-webapps-application.h"
#include "bamf-unity-webapps-tab.h"
#include "bamf-matcher.h"

#include <unity-webapps-context.h>

#define BAMF_UNITY_WEBAPPS_APPLICATION_GET_PRIVATE(obj) (G_TYPE_INSTANCE_GET_PRIVATE(obj, \
BAMF_TYPE_UNITY_WEBAPPS_APPLICATION, BamfUnityWebappsApplicationPrivate))

G_DEFINE_TYPE(BamfUnityWebappsApplication, bamf_unity_webapps_application, BAMF_TYPE_APPLICATION);

enum
{
  PROP_0,
  PROP_CONTEXT,
};

struct _BamfUnityWebappsApplicationPrivate
{
  UnityWebappsContext *context;
};


static void
bamf_unity_webapps_application_get_application_menu (BamfApplication *application,
                                                     gchar **name,
                                                     gchar **path)
{
  BamfUnityWebappsApplication *self;

  self = (BamfUnityWebappsApplication *)application;

  *name = g_strdup (unity_webapps_context_get_context_name (self->priv->context));
  *path = g_strdup (UNITY_WEBAPPS_CONTEXT_MENU_PATH);
}

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

static BamfUnityWebappsTab *
bamf_unity_webapps_application_find_child_by_interest (BamfUnityWebappsApplication *application,
                                                       gint interest_id)
{
  GList *children, *walk;
  BamfUnityWebappsTab *child;

  children = bamf_view_get_children (BAMF_VIEW (application));

  for (walk = children; walk != NULL; walk = walk->next)
    {
      child = BAMF_UNITY_WEBAPPS_TAB (walk->data);

      if (interest_id == bamf_unity_webapps_tab_get_interest_id (child))
        {
          return child;
        }
    }

  return NULL;
}

static BamfView *
bamf_unity_webapps_application_get_focusable_child (BamfApplication *application)
{
  BamfUnityWebappsApplication *self;
  gint focus_interest;

  self = BAMF_UNITY_WEBAPPS_APPLICATION (application);

  focus_interest = unity_webapps_context_get_focus_interest (self->priv->context);

  if (focus_interest == -1)
    return NULL;

  return (BamfView *)bamf_unity_webapps_application_find_child_by_interest (self, focus_interest);
}


static void
bamf_unity_webapps_application_interest_appeared (UnityWebappsContext *context,
                                                  gint interest_id,
                                                  gpointer user_data)
{
  BamfUnityWebappsApplication *self;
  BamfUnityWebappsTab *child;

  self = BAMF_UNITY_WEBAPPS_APPLICATION (user_data);

  child = bamf_unity_webapps_application_find_child_by_interest (self, interest_id);

  if (child != NULL)
    {
      return;
    }

  child = bamf_unity_webapps_tab_new (context, interest_id);
  bamf_view_add_child (BAMF_VIEW (self), BAMF_VIEW (child));

  // It's possible that the context had become lonely (i.e. no children) but not yet shut down.
  // however, if we gain an interest we are always running and "mapped".
  bamf_view_set_running (BAMF_VIEW (self), TRUE);
  bamf_view_set_user_visible (BAMF_VIEW (self), TRUE);
}

static void
bamf_unity_webapps_application_interest_vanished (UnityWebappsContext *context,
                                                  gint interest_id,
                                                  gpointer user_data)
{
  BamfUnityWebappsApplication *self;
  BamfUnityWebappsTab *child;

  self = (BamfUnityWebappsApplication *)user_data;

  child = bamf_unity_webapps_application_find_child_by_interest (self, interest_id);

  if (child == NULL)
    {
      return;
    }

  bamf_view_remove_child (BAMF_VIEW (self), BAMF_VIEW (child));
}

/* It doesn't make any sense for a BamfUnityWebappsTab to live without it's assosciated context.
 * so when our children are removed, dispose of them. */
static void
bamf_unity_webapps_application_child_removed (BamfView *view, BamfView *child)
{
  // Chain up first before we destroy the object.
  BAMF_VIEW_CLASS (bamf_unity_webapps_application_parent_class)->child_removed (view, child);

  bamf_view_set_running (child, FALSE);
  bamf_view_close (BAMF_VIEW (child));
}

void
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
  bamf_application_set_desktop_file_from_id (BAMF_APPLICATION (self),
                                             unity_webapps_context_get_desktop_name (self->priv->context));

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

static gchar *
bamf_unity_webapps_application_get_stable_bus_name (BamfView *view)
{
  const gchar *desktop_file;

  desktop_file = bamf_application_get_desktop_file (BAMF_APPLICATION (view));

  if (desktop_file)
    {
      return g_strdup_printf ("webapp/%i", abs (g_str_hash (desktop_file)));
    }

  return g_strdup_printf ("webapp/%p", view);
}


static void
bamf_unity_webapps_application_finalize (GObject *object)
{
  BamfUnityWebappsApplication *self = BAMF_UNITY_WEBAPPS_APPLICATION (object);

  g_object_unref (self->priv->context);

  G_OBJECT_CLASS (bamf_unity_webapps_application_parent_class)->finalize (object);
}

static void
on_accept_data_changed (UnityWebappsContext *context, const gchar **file, gpointer user_data)
{
  BamfUnityWebappsApplication *self = BAMF_UNITY_WEBAPPS_APPLICATION (user_data);

  g_signal_emit_by_name (self, "supported-mimes-changed", file);
}

static void
bamf_unity_webapps_application_constructed (GObject *object)
{
  BamfUnityWebappsApplication *self;

  self = (BamfUnityWebappsApplication *)object;

  g_signal_connect (self->priv->context, "accept-data-changed", G_CALLBACK (on_accept_data_changed), self);
}


static void
bamf_unity_webapps_application_init (BamfUnityWebappsApplication *self)
{
  self->priv = BAMF_UNITY_WEBAPPS_APPLICATION_GET_PRIVATE (self);

  bamf_application_set_application_type (BAMF_APPLICATION (self), BAMF_APPLICATION_WEB);

}

static char **
bamf_unity_webapps_application_get_supported_mime_types (BamfApplication *application)
{
  BamfUnityWebappsApplication *self = BAMF_UNITY_WEBAPPS_APPLICATION (application);

  return unity_webapps_context_get_application_accept_data (self->priv->context);
}

static gboolean
bamf_unity_webapps_application_get_close_when_empty (BamfApplication *application)
{
  // Sometimes we might have no children for a short period (for example, the page is reloading), in the case
  // Unity Webapps will keep the context alive for a while. Allowing for new children to appear...before eventually
  // shutting it down. So we use this flag to ensure BAMF will not shut us down prematurely.
  return FALSE;
}

static void
bamf_unity_webapps_application_class_init (BamfUnityWebappsApplicationClass * klass)
{
  GParamSpec *pspec;
  GObjectClass *object_class = G_OBJECT_CLASS (klass);
  BamfApplicationClass *bamf_application_class = BAMF_APPLICATION_CLASS (klass);
  BamfViewClass *bamf_view_class = BAMF_VIEW_CLASS (klass);

  object_class->get_property = bamf_unity_webapps_application_get_property;
  object_class->set_property = bamf_unity_webapps_application_set_property;
  object_class->finalize = bamf_unity_webapps_application_finalize;
  object_class->constructed = bamf_unity_webapps_application_constructed;

  bamf_view_class->stable_bus_name = bamf_unity_webapps_application_get_stable_bus_name;
  bamf_view_class->child_removed = bamf_unity_webapps_application_child_removed;

  bamf_application_class->get_application_menu = bamf_unity_webapps_application_get_application_menu;
  bamf_application_class->get_focusable_child = bamf_unity_webapps_application_get_focusable_child;
  bamf_application_class->get_supported_mime_types = bamf_unity_webapps_application_get_supported_mime_types;
  bamf_application_class->get_close_when_empty = bamf_unity_webapps_application_get_close_when_empty;

  pspec = g_param_spec_object("context", "Context", "The Unity Webapps Context assosciated with the Application",
                              UNITY_WEBAPPS_TYPE_CONTEXT, G_PARAM_READWRITE | G_PARAM_CONSTRUCT_ONLY);
  g_object_class_install_property (object_class, PROP_CONTEXT, pspec);

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
