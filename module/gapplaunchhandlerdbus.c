//  
//  Copyright (C) 2009 Canonical Ltd.
// 
//  This program is free software: you can redistribute it and/or modify
//  it under the terms of the GNU General Public License as published by
//  the Free Software Foundation, either version 3 of the License, or
//  (at your option) any later version.
// 
//  This program is distributed in the hope that it will be useful,
//  but WITHOUT ANY WARRANTY; without even the implied warranty of
//  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//  GNU General Public License for more details.
// 
//  You should have received a copy of the GNU General Public License
//  along with this program.  If not, see <http://www.gnu.org/licenses/>.
// 

#include <config.h>

#include <string.h>

#include <glib.h>
#include <gio/gio.h>

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
	g_print ("%s\n", desktop_file_path);
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
