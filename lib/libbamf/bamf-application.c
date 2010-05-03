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
 * SECTION:bamf-application
 * @short_description: The base class for all applications
 *
 * #BamfApplication is the base class that all applications need to derive from.
 */

#if HAVE_CONFIG_H
#include <config.h>
#endif

#include "bamf-application.h"

#include <dbus/dbus.h>
#include <dbus/dbus-glib.h>
#include <dbus/dbus-glib-lowlevel.h>

G_DEFINE_TYPE (BamfApplication, bamf_application, BAMF_TYPE_VIEW);

#define BAMF_APPLICATION_GET_PRIVATE(o) \
  (G_TYPE_INSTANCE_GET_PRIVATE ((o), BAMF_TYPE_APPLICATION, BamfApplicationPrivate))

struct _BamfApplicationPrivate
{
  DBusGConnection *connection;
  DBusGProxy      *proxy;
};

/* Globals */

/* Forwards */

/*
 * GObject stuff
 */

static void
bamf_application_class_init (BamfApplicationClass *klass)
{
  GObjectClass *obj_class = G_OBJECT_CLASS (klass);

  g_type_class_add_private (obj_class, sizeof (BamfApplicationPrivate));
}


static void
bamf_application_init (BamfApplication *self)
{
  BamfApplicationPrivate *priv;
  GError           *error = NULL;

  priv = self->priv = BAMF_APPLICATION_GET_PRIVATE (self);

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

BamfApplication *
bamf_application_new (const char * path)
{
  BamfApplication *self;
  BamfApplicationPrivate *priv;

  self = g_object_new (BAMF_TYPE_APPLICATION, NULL);

  priv = self->priv;

  bamf_view_set_path (BAMF_VIEW (self), path);

  priv->proxy = dbus_g_proxy_new_for_name (priv->connection,
                                           "org.ayatana.bamf",
                                           path,
                                           "org.ayatana.bamf.application");
  if (priv->proxy == NULL)
    {
      g_error ("Unable to get org.ayatana.bamf.application application");
    }

  return self;
}

const gchar *
bamf_application_get_desktop_file (BamfApplication *application)
{
  return NULL;
}

const gchar *
bamf_application_get_application_type (BamfApplication *application)
{
  return NULL;
}

gboolean
bamf_application_is_urgent (BamfApplication *application)
{
  return FALSE;
}

GList *
bamf_application_get_windows (BamfApplication *application)
{
  return NULL;
}
