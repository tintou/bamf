/*
 * Copyright 2009 Canonical Ltd.
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
 *
 */
/**
 * SECTION:bamf-matcher
 * @short_description: The base class for all matchers
 *
 * #BamfMatcher is the base class that all matchers need to derive from.
 */

#if HAVE_CONFIG_H
#include <config.h>
#endif

#include "bamf-matcher.h"
#include "bamf-view.h"
#include "bamf-factory.h"
#include "bamf-marshal.h"

#include <dbus/dbus.h>
#include <dbus/dbus-glib.h>
#include <dbus/dbus-glib-lowlevel.h>

G_DEFINE_TYPE (BamfMatcher, bamf_matcher, G_TYPE_OBJECT);

#define BAMF_MATCHER_GET_PRIVATE(o) \
  (G_TYPE_INSTANCE_GET_PRIVATE ((o), BAMF_TYPE_MATCHER, BamfMatcherPrivate))

enum
{
  VIEW_OPENED,
  VIEW_CLOSED,
  
  LAST_SIGNAL,
};

static guint matcher_signals[LAST_SIGNAL] = { 0 };

struct _BamfMatcherPrivate
{
  DBusGConnection *connection;
  DBusGProxy      *proxy;
  GList           *views;
};

static BamfMatcher * default_matcher = NULL;

static void
bamf_matcher_class_init (BamfMatcherClass *klass)
{
  GObjectClass *obj_class = G_OBJECT_CLASS (klass);

  g_type_class_add_private (obj_class, sizeof (BamfMatcherPrivate));

  matcher_signals [VIEW_OPENED] = 
  	g_signal_new ("view-opened",
  	              G_OBJECT_CLASS_TYPE (klass),
  	              0,
  	              0, NULL, NULL,
  	              g_cclosure_marshal_VOID__POINTER,
  	              G_TYPE_NONE, 1, 
  	              G_TYPE_OBJECT);

  matcher_signals [VIEW_CLOSED] = 
  	g_signal_new ("view-closed",
  	              G_OBJECT_CLASS_TYPE (klass),
  	              0,
  	              0, NULL, NULL,
  	              g_cclosure_marshal_VOID__POINTER,
  	              G_TYPE_NONE, 1, 
  	              G_TYPE_OBJECT, G_TYPE_STRING);
}


static void
bamf_matcher_on_view_opened (DBusGProxy *proxy, 
                             char *path, 
                             char *type, 
                             BamfMatcher *matcher)
{
  BamfView *view;

  view = bamf_factory_view_for_path (bamf_factory_get_default (), path);
  g_object_ref (view);

  g_signal_emit (matcher, matcher_signals[VIEW_OPENED],0, view);
}

static void
bamf_matcher_on_view_closed (DBusGProxy *proxy, 
                             char *path, 
                             char *type, 
                             BamfMatcher *matcher)
{
  BamfView *view;

  view = bamf_factory_view_for_path (bamf_factory_get_default (), path);

  g_signal_emit (matcher, matcher_signals[VIEW_CLOSED],0, view);
  g_object_unref (view);
}

static void
bamf_matcher_init (BamfMatcher *self)
{
  BamfMatcherPrivate *priv;
  GError           *error = NULL;

  priv = self->priv = BAMF_MATCHER_GET_PRIVATE (self);

  priv->connection = dbus_g_bus_get (DBUS_BUS_SESSION, &error);
  if (priv->connection == NULL)
    {
      g_error ("Failed to open connection to bus: %s",
               error != NULL ? error->message : "Unknown");
      if (error)
        g_error_free (error);
      return;
    }

  priv->proxy = dbus_g_proxy_new_for_name (priv->connection,
                                           "org.ayatana.bamf",
                                           "/org/ayatana/bamf/matcher",
                                           "org.ayatana.bamf.matcher");
  if (priv->proxy == NULL)
    {
      g_error ("Unable to get org.ayatana.bamf.matcher matcher");
    }

  dbus_g_object_register_marshaller ((GClosureMarshal) bamf_marshal_VOID__STRING_STRING,
                                     G_TYPE_NONE, 
                                     G_TYPE_STRING, G_TYPE_STRING,
                                     G_TYPE_INVALID);

  dbus_g_proxy_add_signal (priv->proxy,
                           "ViewOpened",
                           G_TYPE_STRING, 
                           G_TYPE_STRING,
                           G_TYPE_INVALID);

  dbus_g_proxy_connect_signal (priv->proxy,
                               "ViewOpened",
                               (GCallback) bamf_matcher_on_view_opened,
                               self, NULL);

  dbus_g_proxy_add_signal (priv->proxy,
                           "ViewClosed",
                           G_TYPE_STRING, 
                           G_TYPE_STRING,
                           G_TYPE_INVALID);

  dbus_g_proxy_connect_signal (priv->proxy,
                               "ViewClosed",
                               (GCallback) bamf_matcher_on_view_closed,
                               self, NULL);
}

/*
 * Private Methods
 */

/*
 * Public Methods
 */
BamfMatcher *
bamf_matcher_get_default (void)
{
  if (BAMF_IS_MATCHER (default_matcher))
    return g_object_ref (default_matcher);

  return (default_matcher = g_object_new (BAMF_TYPE_MATCHER, NULL));
}

BamfApplication *
bamf_matcher_get_application_for_xid (BamfMatcher  *matcher,
                                      guint32       xid)
{
  BamfMatcherPrivate *priv;
  BamfView *view;
  char *app = NULL;
  GError *error = NULL;

  g_return_val_if_fail (BAMF_IS_MATCHER (matcher), NULL);
  priv = matcher->priv;

  if (!dbus_g_proxy_call (priv->proxy,
                          "ApplicationForXid",
                          &error,
                          G_TYPE_UINT, xid,
                          G_TYPE_INVALID,
                          G_TYPE_STRING, &app,
                          G_TYPE_INVALID))
    {
      g_warning ("Failed to fetch path: %s", error->message);
      g_error_free (error);
      return NULL;
    }

  if (!app)
    return NULL;

  view = bamf_factory_view_for_path (bamf_factory_get_default (), app);

  if (!BAMF_IS_APPLICATION (view))
    return NULL;

  return BAMF_APPLICATION (view);
}

gboolean
bamf_matcher_application_is_running (BamfMatcher *matcher,
                                     const gchar *application)
{
  BamfMatcherPrivate *priv;
  gboolean running = FALSE;
  GError *error = NULL;

  g_return_val_if_fail (BAMF_IS_MATCHER (matcher), FALSE);
  priv = matcher->priv;

  if (!dbus_g_proxy_call (priv->proxy,
                          "ApplicationIsRunning",
                          &error,
                          G_TYPE_STRING, application,
                          G_TYPE_INVALID,
                          G_TYPE_BOOLEAN, &running,
                          G_TYPE_INVALID))
    {
      g_warning ("Failed to fetch path: %s", error->message);
      g_error_free (error);
      
      return FALSE;
    }

  return running;
}

GList *
bamf_matcher_get_applications (BamfMatcher *matcher)
{
  BamfMatcherPrivate *priv;
  BamfView *view;
  char **array = NULL;
  int i, len;
  GList *result = NULL;
  GError *error = NULL;

  g_return_val_if_fail (BAMF_IS_MATCHER (matcher), NULL);
  priv = matcher->priv;

  if (!dbus_g_proxy_call (priv->proxy,
                          "ApplicationPaths",
                          &error,
                          G_TYPE_INVALID,
                          G_TYPE_STRV, &array,
                          G_TYPE_INVALID))
    {
      g_warning ("Failed to fetch paths: %s", error->message);
      g_error_free (error);
      
      return FALSE;
    }

  g_return_val_if_fail (array, NULL);

  len = g_strv_length (array);
  for (i = 0; i < len; i++)
    {
      view = bamf_factory_view_for_path (bamf_factory_get_default (), array[i]);

      if (view)
        result = g_list_prepend (result, view);
      else
        g_print ("DOUBLE FAIL\n");
    }

  return result;
}

GList *
bamf_matcher_get_running_applications (BamfMatcher *matcher)
{
  BamfMatcherPrivate *priv;
  BamfView *view;
  char **array = NULL;
  int i, len;
  GList *result = NULL;
  GError *error = NULL;

  g_return_val_if_fail (BAMF_IS_MATCHER (matcher), NULL);
  priv = matcher->priv;

  if (!dbus_g_proxy_call (priv->proxy,
                          "RunningApplications",
                          &error,
                          G_TYPE_INVALID,
                          G_TYPE_STRV, &array,
                          G_TYPE_INVALID))
    {
      g_warning ("Failed to fetch paths: %s", error->message);
      g_error_free (error);
      
      return NULL;
    }

  g_return_val_if_fail (array, NULL);

  len = g_strv_length (array);
  for (i = 0; i < len; i++)
    {
      view = bamf_factory_view_for_path (bamf_factory_get_default (), array[i]);

      if (view)
        result = g_list_prepend (result, view);
      else
        g_print ("DOUBLE FAIL\n");
    }

  return result;
}

GList *
bamf_matcher_get_tabs (BamfMatcher *matcher)
{
  /* FIXME */
  return NULL;
}

GArray *
bamf_matcher_get_xids_for_application (BamfMatcher *matcher,
                                       const gchar *application)
{
  /* FIXME */
  return NULL;
}

