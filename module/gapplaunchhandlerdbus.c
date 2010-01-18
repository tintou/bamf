/*
 * Copyright (C) 2009 Canonical, Ltd.
 *
 * This library is free software; you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License
 * version 3.0 as published by the Free Software Foundation.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License version 3.0 for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library. If not, see
 * <http://www.gnu.org/licenses/>.
 *
 * Authored by Jason Smith <jason.smith@canonical.com>
 */

#include <config.h>

#include <glib.h>
#include <gio/gio.h>
#include <dbus/dbus.h>
#include <dbus/dbus-glib.h>
#include <dbus/dbus-glib-lowlevel.h>

#include "gapplaunchhandlerdbus.h"


struct _GAppLaunchHandlerDBus {
  GObject parent;

};

static void launch_handler_iface_init (GDesktopAppInfoLaunchHandlerIface *iface);
static void g_app_launch_handler_dbus_finalize (GObject *object);

#define _G_IMPLEMENT_INTERFACE_DYNAMIC(TYPE_IFACE, iface_init)       { \
  const GInterfaceInfo g_implement_interface_info = { \
    (GInterfaceInitFunc) iface_init, NULL, NULL \
  }; \
  g_type_module_add_interface (type_module, g_define_type_id, TYPE_IFACE, &g_implement_interface_info); \
}

G_DEFINE_DYNAMIC_TYPE_EXTENDED (GAppLaunchHandlerDBus, g_app_launch_handler_dbus, G_TYPE_OBJECT, 0,
                                _G_IMPLEMENT_INTERFACE_DYNAMIC (G_TYPE_DESKTOP_APP_INFO_LAUNCH_HANDLER,
                                                                launch_handler_iface_init))

static void
g_app_launch_handler_dbus_finalize (GObject *object)
{
  if (G_OBJECT_CLASS (g_app_launch_handler_dbus_parent_class)->finalize)
    (*G_OBJECT_CLASS (g_app_launch_handler_dbus_parent_class)->finalize) (object);
}

static GObject *
g_app_launch_handler_dbus_constructor (GType                  type,
                         	       guint                  n_construct_properties,
                         	       GObjectConstructParam *construct_properties)
{
  GObject *object;
  GAppLaunchHandlerDBusClass *klass;
  GObjectClass *parent_class;

  object = NULL;

  /* Invoke parent constructor. */
  klass = G_APP_LAUNCH_HANDLER_DBUS_CLASS (g_type_class_peek (G_TYPE_APP_LAUNCH_HANDLER_DBUS));
  parent_class = G_OBJECT_CLASS (g_type_class_peek_parent (klass));
  object = parent_class->constructor (type,
                                      n_construct_properties,
                                      construct_properties);

  return object;
}

static void
g_app_launch_handler_dbus_init (GAppLaunchHandlerDBus *lookup)
{
}

static void
g_app_launch_handler_dbus_class_finalize (GAppLaunchHandlerDBusClass *klass)
{
}


static void
g_app_launch_handler_dbus_class_init (GAppLaunchHandlerDBusClass *klass)
{
  GObjectClass *gobject_class = G_OBJECT_CLASS (klass);

  gobject_class->constructor = g_app_launch_handler_dbus_constructor;
  gobject_class->finalize = g_app_launch_handler_dbus_finalize;
}

static void
on_launched (GDesktopAppInfoLaunchHandler *launch_handler,
             const char  *desktop_file_path,
             gint pid)
{
	DBusGConnection *connection;
	GError *error = NULL;
	DBusGProxy *proxy;

	connection = dbus_g_bus_get (DBUS_BUS_SESSION, &error);

	if (connection == NULL)
	{
		g_printerr ("Failed to open bus: %s\n", error->message);
		g_error_free (error);
	}

	g_return_if_fail (connection);

	error = NULL;

	proxy = dbus_g_proxy_new_for_name (connection,
					   "org.wncksync.Matcher",
					   "/org/wncksync/Matcher",
					   "org.wncksync.Matcher");

	g_return_if_fail (proxy);

	dbus_g_proxy_call (proxy,
	                   "RegisterDesktopFileForPid",
	                   &error,
	                   G_TYPE_STRING, desktop_file_path,
	                   G_TYPE_INT, pid,
	                   G_TYPE_INVALID, G_TYPE_INVALID);
	g_object_unref (proxy);
	dbus_g_connection_unref (connection);
}

static void
launch_handler_iface_init (GDesktopAppInfoLaunchHandlerIface *iface)
{
  iface->on_launched = on_launched;
}

void
g_app_launch_handler_dbus_register (GIOModule *module)
{
  g_app_launch_handler_dbus_register_type (G_TYPE_MODULE (module));
  g_io_extension_point_implement (G_DESKTOP_APP_INFO_LAUNCH_HANDLER_EXTENSION_POINT_NAME,
				  G_TYPE_APP_LAUNCH_HANDLER_DBUS,
				  "wncksync",
				  10);
}
