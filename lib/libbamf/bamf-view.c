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
 * SECTION:bamf-view
 * @short_description: The base class for all views
 *
 * #BamfView is the base class that all views need to derive from.
 */

#if HAVE_CONFIG_H
#include <config.h>
#endif

#include "bamf-view.h"

#include <dbus/dbus.h>
#include <dbus/dbus-glib.h>
#include <dbus/dbus-glib-lowlevel.h>

G_DEFINE_TYPE (BamfView, bamf_view, G_TYPE_OBJECT);

#define BAMF_VIEW_GET_PRIVATE(o) \
  (G_TYPE_INSTANCE_GET_PRIVATE ((o), BAMF_TYPE_VIEW, BamfViewPrivate))

struct _BamfViewPrivate
{
  DBusGConnection *connection;
  DBusGProxy      *proxy;
};

/* Globals */

/* Forwards */

/*
 * GObject stuff
 */

void bamf_view_set_path (BamfView *view, 
                         const char *path)
{
  BamfViewPrivate *priv;

  g_return_if_fail (BAMF_IS_VIEW (view));

  priv = view->priv;

  if (priv->proxy)
    {
      g_warning ("bamf_view_set_path called multiple times\n");
      return;
    }
  
  priv->proxy = dbus_g_proxy_new_for_name (priv->connection,
                                           "org.ayatana.bamf",
                                           path,
                                           "org.ayatana.bamf.view");
  if (priv->proxy == NULL)
    {
      g_error ("Unable to get org.ayatana.bamf.view view");
    }
}

static void
bamf_view_class_init (BamfViewClass *klass)
{
  GObjectClass *obj_class = G_OBJECT_CLASS (klass);

  g_type_class_add_private (obj_class, sizeof (BamfViewPrivate));
}


static void
bamf_view_init (BamfView *self)
{
  BamfViewPrivate *priv;
  GError           *error = NULL;


  

  priv = self->priv = BAMF_VIEW_GET_PRIVATE (self);

  priv->connection = dbus_g_bus_get (DBUS_BUS_SESSION, &error);
  if (priv->connection == NULL)
    {
      g_error ("Failed to open connection to bus: %s",
               error != NULL ? error->message : "Unknown");
      if (error)
        g_error_free (error);
      return;
    }
}

/*
 * Private Methods
 */

/*
 * Public Methods
 */
GList *
bamf_view_get_children (BamfView *view)
{
  /*char ** children;
  GError *error;
  BamfViewPrivate *priv;

  g_return_val_if_fail (BAMF_IS_VIEW (view), NULL);

  priv = view->priv;

  if (!dbus_g_proxy_call (priv->proxy,
                          "Children",
                          &error,
                          G_TYPE_NONE,
                          G_TYPE_INVALID,
                          G_TYPE_STRV, &children,
                          G_TYPE_INVALID))
    {
      g_error ("Unable to fetch children: %s\n", error->message);
    }*/

  
  return NULL;
}

gboolean
bamf_view_is_active (BamfView *view)
{
  return FALSE;
}

gboolean
bamf_view_is_running (BamfView *view)
{
  return FALSE;
}

gchar *
bamf_view_get_name (BamfView *view)
{
  return NULL;
}

BamfView *
bamf_view_get_parent (BamfView *view)
{
  return NULL;
}

gchar *
bamf_view_get_view_type (BamfView *view)
{
  return NULL;
}
