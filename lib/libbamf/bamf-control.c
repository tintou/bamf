/*
 * Copyright 2010-2011 Canonical Ltd.
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
 *              Marco Trevisan (Treviño) <3v1n0@ubuntu.com>
 *
 */
/**
 * SECTION:bamf-control
 * @short_description: The base class for all controls
 *
 * #BamfControl is the base class that all controls need to derive from.
 */

#if HAVE_CONFIG_H
#include <config.h>
#endif

#include "bamf-control.h"

#include <dbus/dbus.h>
#include <dbus/dbus-glib.h>

G_DEFINE_TYPE (BamfControl, bamf_control, G_TYPE_OBJECT);

#define BAMF_CONTROL_GET_PRIVATE(o) \
  (G_TYPE_INSTANCE_GET_PRIVATE ((o), BAMF_TYPE_CONTROL, BamfControlPrivate))

struct _BamfControlPrivate
{
  DBusGConnection *connection;
  DBusGProxy      *proxy;
};

/* Globals */
static BamfControl * default_control = NULL;

/* Forwards */

/*
 * GObject stuff
 */

static void
bamf_control_class_init (BamfControlClass *klass)
{
  GObjectClass *obj_class = G_OBJECT_CLASS (klass);

  g_type_class_add_private (obj_class, sizeof (BamfControlPrivate));
}


static void
bamf_control_init (BamfControl *self)
{
  BamfControlPrivate *priv;
  GError           *error = NULL;

  priv = self->priv = BAMF_CONTROL_GET_PRIVATE (self);

  priv->connection = dbus_g_bus_get (DBUS_BUS_SESSION, &error);
  if (priv->connection == NULL)
    {
      g_warning ("Failed to open connection to bus: %s",
               error != NULL ? error->message : "Unknown");
      if (error)
        g_error_free (error);
      return;
    }

  priv->proxy = dbus_g_proxy_new_for_name (priv->connection,
                                           "org.ayatana.bamf",
                                           "/org/ayatana/bamf/control",
                                           "org.ayatana.bamf.control");
  if (priv->proxy == NULL)
    {
      g_error ("Unable to get org.bamf.Control control");
    }
}

BamfControl *
bamf_control_get_default (void)
{
  if (BAMF_IS_CONTROL (default_control))
    return g_object_ref (default_control);

  return (default_control = g_object_new (BAMF_TYPE_CONTROL, NULL));
}

void
bamf_control_set_approver_behavior (BamfControl *control,
                                    gint32       behavior)
{
  BamfControlPrivate *priv;
  GError *error = NULL;

  g_return_if_fail (BAMF_IS_CONTROL (control));
  priv = control->priv;

  if (!dbus_g_proxy_call (priv->proxy,
                          "SetApproverBehavior",
                          &error,
                          G_TYPE_INT, behavior,
                          G_TYPE_INVALID,
                          G_TYPE_INVALID))
    {
      g_warning ("Failed to register application: %s", error->message);
      g_error_free (error);
    }
}

void
bamf_control_insert_desktop_file (BamfControl *control,
                                   const gchar *desktop_file)
{
  BamfControlPrivate *priv;
  GError *error = NULL;

  g_return_if_fail (BAMF_IS_CONTROL (control));
  priv = control->priv;

  if (!dbus_g_proxy_call (priv->proxy,
                          "OmNomNomDesktopFile",
                          &error,
                          G_TYPE_STRING, desktop_file,
                          G_TYPE_INVALID,
                          G_TYPE_INVALID))
    {
      g_warning ("Failed to insert desktop file: %s", error->message);
      g_error_free (error);
    }
}

void
bamf_control_register_application_for_pid (BamfControl  *control,
                                           const gchar  *application,
                                           gint32        pid)
{
  BamfControlPrivate *priv;
  GError *error = NULL;

  g_return_if_fail (BAMF_IS_CONTROL (control));
  priv = control->priv;

  if (!dbus_g_proxy_call (priv->proxy,
                          "RegisterApplicationForPid",
                          &error,
                          G_TYPE_STRING, application,
                          G_TYPE_UINT, pid,
                          G_TYPE_INVALID,
                          G_TYPE_INVALID))
    {
      g_warning ("Failed to register application: %s", error->message);
      g_error_free (error);
    }
}

void
bamf_control_register_tab_provider (BamfControl *control,
                                    const char  *path)
{
}
