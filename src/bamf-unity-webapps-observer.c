/*
 * bamf-unity-webapps-observer.c
 * Copyright (C) Canonical LTD 2011
 *
 * Author: Robert Carr <racarr@canonical.com>
 * 
 unity-webapps is free software: you can redistribute it and/or modify it
 * under the terms of the GNU Lesser General Public License as published
 * by the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 * 
 * unity-webapps is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
 * See the GNU Lesser General Public License for more details.
 * 
 * You should have received a copy of the GNU Lesser General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.";
 */

#include "bamf-unity-webapps-tab.h"
#include "bamf-unity-webapps-observer.h"

struct _BamfUnityWebappsObserverPrivate {
  UnityWebappsService *service;

  guint service_watch_id;
  
  GHashTable *contexts_by_name;
};

typedef struct _ContextHashData {
  GHashTable *interests_by_id;
  
  UnityWebappsContext *context;
} ContextHashData;

typedef struct _InterestHashData {
  ContextHashData *context_data;
  
  BamfUnityWebappsTab *tab;
} InterestHashData;

enum {
  TAB_APPEARED, 
  LAST_SIGNAL
};

static guint signals[LAST_SIGNAL] = { 0 };

G_DEFINE_TYPE(BamfUnityWebappsObserver, bamf_unity_webapps_observer, G_TYPE_OBJECT)


#define BAMF_UNITY_WEBAPPS_OBSERVER_GET_PRIVATE(object) (G_TYPE_INSTANCE_GET_PRIVATE ((object), BAMF_TYPE_UNITY_WEBAPPS_OBSERVER, BamfUnityWebappsObserverPrivate))

static void
interest_hash_data_free (InterestHashData *data)
{
  
  g_signal_emit_by_name (data->tab, "vanished", NULL);
  
  g_object_unref (G_OBJECT (data->tab));

  g_slice_free1 (sizeof (InterestHashData), data);
}

static InterestHashData *
interest_hash_data_new (ContextHashData *context_data,
			BamfUnityWebappsTab *tab)
{
  InterestHashData *data;
  
  data = g_slice_alloc0 (sizeof (InterestHashData));
  
  data->tab = g_object_ref (tab);
  data->context_data = context_data;

  return data;
}

static void
bamf_unity_webapps_observer_interest_vanished (UnityWebappsContext *context,
						  gint interest_id,
						  gpointer user_data)
{
  BamfUnityWebappsObserver *observer;
  ContextHashData *data;
  
  observer = (BamfUnityWebappsObserver *)user_data;
  
  data = g_hash_table_lookup (observer->priv->contexts_by_name, unity_webapps_context_get_context_name (context));
  
  g_hash_table_remove (data->interests_by_id, GINT_TO_POINTER (interest_id));
}

static void 
bamf_unity_webapps_observer_interest_appeared (UnityWebappsContext *context,
						   gint interest_id,
						   gpointer user_data)
{
  BamfUnityWebappsObserver *observer;
  BamfUnityWebappsTab *tab;
  ContextHashData *context_data;
  InterestHashData *data;
  
  observer = (BamfUnityWebappsObserver *)user_data;
  
  context_data = g_hash_table_lookup (observer->priv->contexts_by_name,
				      unity_webapps_context_get_context_name (context));
  
  tab = (BamfUnityWebappsTab *)bamf_unity_webapps_tab_new (context, interest_id);
  
  data = interest_hash_data_new (context_data, tab);
  
  g_hash_table_insert (context_data->interests_by_id, GINT_TO_POINTER (interest_id), data);
  
  g_signal_emit (observer, signals[TAB_APPEARED], 0, tab);
  
  g_object_unref (G_OBJECT (tab));

}

static void
bamf_unity_webapps_observer_register_existing_interests (BamfUnityWebappsObserver *observer,
							 ContextHashData *data)
{
  GVariant *interests, *interest_variant;
  GVariantIter *variant_iter;
  
  interests = unity_webapps_context_list_interests (data->context);
  
  if (interests == NULL)
    {
      return;
    }
  
  variant_iter = g_variant_iter_new (interests);
  
  while ((interest_variant = g_variant_iter_next_value (variant_iter)))
    {
      gint interest_id;
      
      interest_id = g_variant_get_int32 (interest_variant);
      
      bamf_unity_webapps_observer_interest_appeared (data->context, interest_id, observer);
    }

  g_variant_iter_free (variant_iter);
  g_variant_unref (interests);
}

static void
context_hash_data_free (ContextHashData *data)
{
  g_hash_table_destroy (data->interests_by_id);
  
  g_object_unref (G_OBJECT (data->context));

  g_slice_free1 (sizeof (ContextHashData), data);
}

static ContextHashData *
context_hash_data_new (BamfUnityWebappsObserver *observer, UnityWebappsContext *context)
{
  ContextHashData *data;
  
  data = g_slice_alloc0 (sizeof (ContextHashData));
  
  data->interests_by_id = g_hash_table_new_full (g_direct_hash, g_direct_equal, NULL, (GDestroyNotify)interest_hash_data_free);
  data->context = (UnityWebappsContext *)(g_object_ref (G_OBJECT (context)));
  
  unity_webapps_context_on_interest_appeared (context, bamf_unity_webapps_observer_interest_appeared, observer);
  unity_webapps_context_on_interest_vanished (context, bamf_unity_webapps_observer_interest_vanished, observer);
  bamf_unity_webapps_observer_register_existing_interests (observer, data);
  
  return data;
}

static void
bamf_unity_webapps_observer_context_vanished (UnityWebappsService *service,
					     const gchar *name,
					     gpointer user_data)
{
  BamfUnityWebappsObserver *observer;
  
  observer = (BamfUnityWebappsObserver *)user_data;
  
  g_hash_table_remove (observer->priv->contexts_by_name, name);
}

static void
bamf_unity_webapps_observer_context_appeared (UnityWebappsService *service,
					      const gchar *name,
					      gpointer user_data)
{
  BamfUnityWebappsObserver *observer;
  UnityWebappsContext *context;
  ContextHashData *data;

  observer = (BamfUnityWebappsObserver *)user_data;
  
  context = unity_webapps_context_new_for_context_name (service, name);

  data = context_hash_data_new (observer, context);
  g_hash_table_insert (observer->priv->contexts_by_name, g_strdup (name), data);
  
  g_object_unref (G_OBJECT (context));
}

static void
bamf_unity_webapps_observer_register_existing_contexts (BamfUnityWebappsObserver *observer,
							UnityWebappsService *service)
{
  gchar **contexts;
  gint i, len;
  
  contexts = unity_webapps_service_list_contexts (service);
  
  if (contexts == NULL)
    return;
  
  len = g_strv_length (contexts);
  
  if (len == 0)
    return;
  
  for (i = 0; i < len; i++)
    {
      bamf_unity_webapps_observer_context_appeared (service, contexts[i], observer);
    }
  
  g_strfreev (contexts);
}

static void
bamf_unity_webapps_observer_service_appeared (GDBusConnection *connection,
					      const gchar *name,
					      const gchar *name_owner,
					      gpointer user_data)
{
  BamfUnityWebappsObserver *observer;
  
  observer = (BamfUnityWebappsObserver *)user_data;
  
  observer->priv->service = unity_webapps_service_new ();
  
  unity_webapps_service_on_context_appeared (observer->priv->service, bamf_unity_webapps_observer_context_appeared, observer);
  unity_webapps_service_on_context_vanished (observer->priv->service, bamf_unity_webapps_observer_context_vanished, observer);

  bamf_unity_webapps_observer_register_existing_contexts (observer, observer->priv->service);
}

static void
bamf_unity_webapps_observer_service_vanished (GDBusConnection *connection,
					      const gchar *name,
					      gpointer user_data)
{
  BamfUnityWebappsObserver *observer;
  
  observer = (BamfUnityWebappsObserver *)user_data;

  if (observer->priv->service == NULL)
    {
      return;
    }
  
  g_hash_table_remove_all (observer->priv->contexts_by_name);
  
  g_object_unref (G_OBJECT (observer->priv->service));
  observer->priv->service = NULL;
}

static void
bamf_unity_webapps_observer_finalize (GObject *object)
{
  BamfUnityWebappsObserver *observer;
  
  observer = BAMF_UNITY_WEBAPPS_OBSERVER (object);
  
  g_bus_unwatch_name (observer->priv->service_watch_id);
  
  g_hash_table_destroy (observer->priv->contexts_by_name);
  
  g_object_unref (G_OBJECT (observer->priv->service));
  
  G_OBJECT_CLASS (bamf_unity_webapps_observer_parent_class)->finalize (object);
}

static void
bamf_unity_webapps_observer_class_init (BamfUnityWebappsObserverClass *klass)
{
  GObjectClass *object_class = G_OBJECT_CLASS (klass);
  
  object_class->finalize = bamf_unity_webapps_observer_finalize;
  
  g_type_class_add_private (object_class, sizeof(BamfUnityWebappsObserverPrivate));
  
  signals[TAB_APPEARED] = g_signal_new ("tab-appeared",
					BAMF_TYPE_UNITY_WEBAPPS_OBSERVER,
					G_SIGNAL_RUN_LAST, 0,
					NULL, NULL, NULL,
					G_TYPE_NONE,
					1, BAMF_TYPE_TAB);
}

static void
bamf_unity_webapps_observer_init (BamfUnityWebappsObserver *observer)
{
  observer->priv = BAMF_UNITY_WEBAPPS_OBSERVER_GET_PRIVATE (observer);
  
  observer->priv->service = NULL;
  observer->priv->contexts_by_name = g_hash_table_new_full (g_str_hash, g_str_equal, g_free, (GDestroyNotify)context_hash_data_free);
  
  observer->priv->service_watch_id = g_bus_watch_name (G_BUS_TYPE_SESSION,
						       "com.canonical.Unity.Webapps.Service",
						       G_BUS_NAME_WATCHER_FLAGS_NONE,
						       bamf_unity_webapps_observer_service_appeared,
						       bamf_unity_webapps_observer_service_vanished,
						       observer, NULL /* User data free func */);
}

BamfUnityWebappsObserver *
bamf_unity_webapps_observer_new ()
{
  return g_object_new (BAMF_TYPE_UNITY_WEBAPPS_OBSERVER, NULL);
}

