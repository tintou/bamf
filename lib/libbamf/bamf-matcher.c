/*
 * Copyright 2010 Canonical Ltd.
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
#include "bamf-view-private.h"
#include "bamf-factory.h"
#include "bamf-marshal.h"

#include <string.h>
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
  ACTIVE_APPLICATION_CHANGED,
  ACTIVE_WINDOW_CHANGED,
  STACKING_ORDER_CHANGED,
  
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
                  g_cclosure_marshal_VOID__OBJECT,
                  G_TYPE_NONE, 1, 
                  G_TYPE_OBJECT);

  matcher_signals [VIEW_CLOSED] = 
    g_signal_new ("view-closed",
                  G_OBJECT_CLASS_TYPE (klass),
                  0,
                  0, NULL, NULL,
                  g_cclosure_marshal_VOID__OBJECT,
                  G_TYPE_NONE, 1, 
                  G_TYPE_OBJECT);


  matcher_signals [ACTIVE_APPLICATION_CHANGED] = 
    g_signal_new ("active-application-changed",
                  G_OBJECT_CLASS_TYPE (klass),
                  0,
                  0, NULL, NULL,
                  bamf_marshal_VOID__OBJECT_OBJECT,
                  G_TYPE_NONE, 2, 
                  G_TYPE_OBJECT, G_TYPE_OBJECT);

  matcher_signals [ACTIVE_WINDOW_CHANGED] = 
    g_signal_new ("active-window-changed",
                  G_OBJECT_CLASS_TYPE (klass),
                  0,
                  0, NULL, NULL,
                  bamf_marshal_VOID__OBJECT_OBJECT,
                  G_TYPE_NONE, 2, 
                  G_TYPE_OBJECT, G_TYPE_OBJECT);

  matcher_signals [STACKING_ORDER_CHANGED] = 
    g_signal_new ("stacking-order-changed",
                  G_OBJECT_CLASS_TYPE (klass),
                  0,
                  0, NULL, NULL,
                  g_cclosure_marshal_VOID__VOID,
                  G_TYPE_NONE, 0);
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
  
  g_signal_emit (matcher, matcher_signals[VIEW_OPENED], 0, view);
}

static void
bamf_matcher_on_view_closed (DBusGProxy *proxy, 
                             char *path, 
                             char *type, 
                             BamfMatcher *matcher)
{
  BamfView *view;

  view = bamf_factory_view_for_path (bamf_factory_get_default (), path);

  if (!BAMF_IS_VIEW (view))
    return;

  g_signal_emit (matcher, matcher_signals[VIEW_CLOSED], 0, view);
  
  g_object_unref (view);
}

static void
bamf_matcher_on_active_application_changed (DBusGProxy *proxy,
                                            char *old_path,
                                            char *new_path,
                                            BamfMatcher *matcher)
{
  BamfView *old_view = NULL;
  BamfView *new_view = NULL;

  if (old_path && strlen (old_path) > 0)
    old_view = bamf_factory_view_for_path (bamf_factory_get_default (), old_path);

  if (new_path && strlen (new_path) > 0)
    new_view = bamf_factory_view_for_path (bamf_factory_get_default (), new_path);

  g_signal_emit (matcher, matcher_signals[ACTIVE_APPLICATION_CHANGED], 0, old_view, new_view);
}

static void
bamf_matcher_on_active_window_changed (DBusGProxy *proxy,
                                       char *old_path,
                                       char *new_path,
                                       BamfMatcher *matcher)
{
  BamfView *old_view = NULL;
  BamfView *new_view = NULL;

  if (old_path && strlen (old_path) > 0)
    old_view = bamf_factory_view_for_path (bamf_factory_get_default (), old_path);

  if (new_path && strlen (new_path) > 0)
    new_view = bamf_factory_view_for_path (bamf_factory_get_default (), new_path);

  g_signal_emit (matcher, matcher_signals[ACTIVE_WINDOW_CHANGED], 0, old_view, new_view);
}

static void
bamf_matcher_on_stacking_order_changed (DBusGProxy *proxy, BamfMatcher *matcher)
{
  g_signal_emit (matcher, matcher_signals[STACKING_ORDER_CHANGED], 0);
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

  dbus_g_proxy_add_signal (priv->proxy,
                           "ActiveApplicationChanged",
                           G_TYPE_STRING, 
                           G_TYPE_STRING,
                           G_TYPE_INVALID);

  dbus_g_proxy_connect_signal (priv->proxy,
                               "ActiveApplicationChanged",
                               (GCallback) bamf_matcher_on_active_application_changed,
                               self, NULL);

  dbus_g_proxy_add_signal (priv->proxy,
                           "ActiveWindowChanged",
                           G_TYPE_STRING, 
                           G_TYPE_STRING,
                           G_TYPE_INVALID);

  dbus_g_proxy_connect_signal (priv->proxy,
                               "ActiveWindowChanged",
                               (GCallback) bamf_matcher_on_active_window_changed,
                               self, NULL);

  dbus_g_proxy_add_signal (priv->proxy,
                           "StackingOrderChanged",
                           G_TYPE_INVALID);

  dbus_g_proxy_connect_signal (priv->proxy,
                               "StackingOrderChanged",
                               (GCallback) bamf_matcher_on_stacking_order_changed,
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
bamf_matcher_get_active_application (BamfMatcher *matcher)
{
  BamfMatcherPrivate *priv;
  BamfView *view;
  char *app = NULL;
  GError *error = NULL;

  g_return_val_if_fail (BAMF_IS_MATCHER (matcher), NULL);
  priv = matcher->priv;

  if (!dbus_g_proxy_call (priv->proxy,
                          "ActiveApplication",
                          &error,
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
  g_free (app);

  if (!BAMF_IS_APPLICATION (view))
    return NULL;

  return BAMF_APPLICATION (view);
}

BamfWindow *
bamf_matcher_get_active_window (BamfMatcher *matcher)
{
  BamfMatcherPrivate *priv;
  BamfView *view;
  char *app = NULL;
  GError *error = NULL;

  g_return_val_if_fail (BAMF_IS_MATCHER (matcher), NULL);
  priv = matcher->priv;

  if (!dbus_g_proxy_call (priv->proxy,
                          "ActiveWindow",
                          &error,
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
  g_free (app);

  if (!BAMF_IS_WINDOW (view))
    return NULL;

  return BAMF_WINDOW (view);
}

/* Looks up the window's XID and calls the application_for_xid
   function just below here. */
BamfApplication * 
bamf_matcher_get_application_for_window  (BamfMatcher *matcher,
                                          BamfWindow *window)
{
	g_return_val_if_fail(BAMF_IS_WINDOW(window), NULL);
	return bamf_matcher_get_application_for_xid (matcher, bamf_window_get_xid(window));
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
  
  g_free (app);

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
    }
  
  g_strfreev (array);
  return result;
}

GList *
bamf_matcher_get_windows (BamfMatcher *matcher)
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
                          "WindowPaths",
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
    }
  
  g_strfreev (array);
  return result;
}

GList *
bamf_matcher_get_window_stack_for_monitor (BamfMatcher *matcher, gint monitor)
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
                          "WindowStackForMonitor",
                          &error,
                          G_TYPE_INT,
                          monitor,
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
  for (i = len-1; i >= 0; i--)
    {
      view = bamf_factory_view_for_path (bamf_factory_get_default (), array[i]);

      if (view)
        result = g_list_prepend (result, view);
    }
  
  g_strfreev (array);
  return result;
}

void
bamf_matcher_register_favorites (BamfMatcher *matcher,
                                 const gchar **favorites)
{
  BamfMatcherPrivate *priv;

  g_return_if_fail (BAMF_IS_MATCHER (matcher));
  g_return_if_fail (favorites);
  
  priv = matcher->priv;

  dbus_g_proxy_call_no_reply (priv->proxy,
                              "RegisterFavorites",
                              G_TYPE_STRV, favorites,
                              G_TYPE_INVALID);
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
    }

  g_strfreev (array);
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

BamfApplication * 
bamf_matcher_get_application_for_desktop_file (BamfMatcher *matcher,
                                               const gchar *desktop_file_path,
                                               gboolean create_if_not_found)
{
  BamfApplication *app;
  const gchar ** favs;
  
  app = bamf_factory_app_for_file (bamf_factory_get_default (), desktop_file_path, create_if_not_found);
  if (app)
    {
      favs = g_malloc0 (sizeof (gchar *) * 2),
      favs[0] = desktop_file_path;      
      bamf_matcher_register_favorites (matcher, favs);
      g_free (favs);
    }
  return app;
}
