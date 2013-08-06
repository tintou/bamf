/*
 * bamf-unity-webapps-observer.c
 * Copyright (C) Canonical LTD 2012
 *
 * Author: Robert Carr <racarr@canonical.com>
 *
 * This program is free software: you can redistribute it and/or modify it
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
#include "bamf-unity-webapps-application.h"
#include "bamf-matcher.h"

struct _BamfUnityWebappsObserverPrivate {
  UnityWebappsService *service;

  GHashTable *applications_by_context_name;

  guint service_watch_id;
};

G_DEFINE_TYPE(BamfUnityWebappsObserver, bamf_unity_webapps_observer, G_TYPE_OBJECT)

enum
{
  APPLICATION_APPEARED,
  LAST_SIGNAL
};

static guint webapps_observer_signals[LAST_SIGNAL] = { 0 };


#define BAMF_UNITY_WEBAPPS_OBSERVER_GET_PRIVATE(object) (G_TYPE_INSTANCE_GET_PRIVATE ((object), BAMF_TYPE_UNITY_WEBAPPS_OBSERVER, BamfUnityWebappsObserverPrivate))

static void
bamf_unity_webapps_observer_context_vanished (UnityWebappsService *service,
                                             const gchar *name,
                                             gpointer user_data)
{
  BamfUnityWebappsObserver *observer;
  BamfApplication *application;

  observer = (BamfUnityWebappsObserver *)user_data;

  application = g_hash_table_lookup (observer->priv->applications_by_context_name, name);

  if (application == NULL)
    return;

  bamf_view_set_running (BAMF_VIEW (application), FALSE);
  bamf_view_close (BAMF_VIEW (application));

  g_hash_table_remove (observer->priv->applications_by_context_name, name);

}

static void
bamf_unity_webapps_application_closed (BamfView *view,
                                       gpointer user_data)
{
  BamfUnityWebappsObserver *observer;
  BamfUnityWebappsApplication *application;
  UnityWebappsContext *context;
  const gchar *context_name;

  observer = (BamfUnityWebappsObserver *)user_data;

  application = BAMF_UNITY_WEBAPPS_APPLICATION (view);

  context = bamf_unity_webapps_application_get_context  (application);
  context_name = unity_webapps_context_get_context_name (context);

  g_hash_table_remove (observer->priv->applications_by_context_name, context_name);
}

static void
bamf_unity_webapps_observer_context_appeared (UnityWebappsService *service,
                                              const gchar *name,
                                              gpointer user_data)
{
  BamfUnityWebappsObserver *observer;
  UnityWebappsContext *context;
  BamfApplication *application;

  if (name == NULL || name[0] == '\0')
    return;

  observer = (BamfUnityWebappsObserver *)user_data;

  if (g_hash_table_lookup (observer->priv->applications_by_context_name, name) != NULL)
    return;

  context = unity_webapps_context_new_for_context_name (observer->priv->service, name);

  application = bamf_unity_webapps_application_new (context);

  g_signal_connect (G_OBJECT (application), "closed-internal", G_CALLBACK (bamf_unity_webapps_application_closed),
                    observer);

  g_hash_table_insert (observer->priv->applications_by_context_name, g_strdup (name), application);

  g_signal_emit (observer, webapps_observer_signals[APPLICATION_APPEARED], 0, application);
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
bamf_unity_webapps_observer_close_all (BamfUnityWebappsObserver *observer)
{
  GList *names, *walk;

  names = g_hash_table_get_keys (observer->priv->applications_by_context_name);

  for (walk = names; walk != NULL; walk = walk->next)
    {
      bamf_unity_webapps_observer_context_vanished (observer->priv->service, (const gchar *)walk->data,
                                                    observer);
    }

  g_list_free (names);


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

  bamf_unity_webapps_observer_close_all (observer);

  g_object_unref (G_OBJECT (observer->priv->service));
  observer->priv->service = NULL;
}

static void
bamf_unity_webapps_observer_finalize (GObject *object)
{
  BamfUnityWebappsObserver *observer;

  observer = BAMF_UNITY_WEBAPPS_OBSERVER (object);

  g_hash_table_destroy (observer->priv->applications_by_context_name);

  if (observer->priv->service_watch_id)
    {
      g_bus_unwatch_name (observer->priv->service_watch_id);
    }

  if (observer->priv->service)
    {
      g_object_unref (G_OBJECT (observer->priv->service));
    }

  G_OBJECT_CLASS (bamf_unity_webapps_observer_parent_class)->finalize (object);
}

static void
bamf_unity_webapps_observer_constructed (GObject *object)
{
  BamfUnityWebappsObserver *observer;

  observer = (BamfUnityWebappsObserver *)object;
  if (G_OBJECT_CLASS (bamf_unity_webapps_observer_parent_class)->constructed)
    {
      G_OBJECT_CLASS (bamf_unity_webapps_observer_parent_class)->constructed (object);
    }

  if (g_strcmp0 (g_getenv ("BAMF_TEST_MODE"), "TRUE") == 0)
    return;

  observer->priv->service_watch_id = g_bus_watch_name (G_BUS_TYPE_SESSION,
                                                       "com.canonical.Unity.Webapps.Service",
                                                       G_BUS_NAME_WATCHER_FLAGS_NONE,
                                                       bamf_unity_webapps_observer_service_appeared,
                                                       bamf_unity_webapps_observer_service_vanished,
                                                       observer, NULL /* User data free func */);

}

static void
bamf_unity_webapps_observer_class_init (BamfUnityWebappsObserverClass *klass)
{
  GObjectClass *object_class = G_OBJECT_CLASS (klass);

  object_class->finalize = bamf_unity_webapps_observer_finalize;
  object_class->constructed = bamf_unity_webapps_observer_constructed;

  g_type_class_add_private (object_class, sizeof(BamfUnityWebappsObserverPrivate));

  webapps_observer_signals [APPLICATION_APPEARED] =
    g_signal_new ("application-appeared",
                  G_OBJECT_CLASS_TYPE (klass),
                  0, 0, NULL, NULL, NULL,
                  G_TYPE_NONE, 1,
                  BAMF_TYPE_APPLICATION);
}



static void
bamf_unity_webapps_observer_init (BamfUnityWebappsObserver *observer)
{
  observer->priv = BAMF_UNITY_WEBAPPS_OBSERVER_GET_PRIVATE (observer);

  observer->priv->service = NULL;

  observer->priv->applications_by_context_name = g_hash_table_new_full (g_str_hash, g_str_equal,
                                                                        g_free, NULL);
}

BamfUnityWebappsObserver *
bamf_unity_webapps_observer_new ()
{
  return g_object_new (BAMF_TYPE_UNITY_WEBAPPS_OBSERVER, NULL);
}

