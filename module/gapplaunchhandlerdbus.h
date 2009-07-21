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

#ifndef __G_APP_LAUNCH_HANDLER_DBUS_H__
#define __G_APP_LAUNCH_HANDLER_DBUS_H__

#include <glib-object.h>
#include <gio/gio.h>
#include <gio/gdesktopappinfo.h>


G_BEGIN_DECLS

#define G_TYPE_APP_LAUNCH_HANDLER_DBUS        (g_app_launch_handler_dbus_get_type ())
#define G_APP_LAUNCH_HANDLER_DBUS(o)          (G_TYPE_CHECK_INSTANCE_CAST ((o), G_TYPE_APP_LAUNCH_HANDLER_DBUS, GAppLaunchHandlerDBus))
#define G_APP_LAUNCH_HANDLER_DBUS_CLASS(k)    (G_TYPE_CHECK_CLASS_CAST((k), G_TYPE_APP_LAUNCH_HANDLER_DBUS, GAppLaunchHandlerDBusClass))
#define G_IS_APP_LAUNCH_HANDLER_DBUS(o)       (G_TYPE_CHECK_INSTANCE_TYPE ((o), G_TYPE_APP_LAUNCH_HANDLER_DBUS))
#define G_IS_APP_LAUNCH_HANDLER_DBUS_CLASS(k) (G_TYPE_CHECK_CLASS_TYPE ((k), G_TYPE_APP_LAUNCH_HANDLER_DBUS))

typedef struct _GAppLaunchHandlerDBus GAppLaunchHandlerDBus;
typedef struct _GAppLaunchHandlerDBusClass GAppLaunchHandlerDBusClass;

struct _GAppLaunchHandlerDBusClass {
  GObjectClass parent_class;
};

GType g_app_launch_handler_dbus_get_type (void) G_GNUC_CONST;
void  g_app_launch_handler_dbus_register (GIOModule *module);

G_END_DECLS

#endif /* __G_APP_LAUNCH_HANDLER_DBUS_H__ */
