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

#include <stdio.h>
#include <gtk/gtk.h>
#include <gdk/gdk.h>
#include <dbus/dbus.h>
#include <dbus/dbus-glib.h>
#include <dbus/dbus-glib-lowlevel.h>
#include <glib.h>
#include <sys/types.h>
#include <unistd.h>
#include <glibtop.h>


void test_matcher_create_suite (void);
void test_view_create_suite (void);
void test_application_create_suite (void);
 
gint
main (gint argc, gchar *argv[])
{
  DBusGConnection *bus;
  DBusGProxy *bus_proxy;
  GError *error = NULL;
  guint request_name_result;

  g_type_init ();
  g_test_init (&argc, &argv, NULL);

  gtk_init (&argc, &argv);
  glibtop_init ();

  dbus_g_thread_init ();

  bus = dbus_g_bus_get (DBUS_BUS_SESSION, &error);
      
  if (!bus)
    g_error ("Could not get session bus");

  bus_proxy =
    dbus_g_proxy_new_for_name (bus, "org.freedesktop.DBus",
   		                    "/org/freedesktop/DBus",
    		                    "org.freedesktop.DBus");

  error = NULL;
  if (!dbus_g_proxy_call (bus_proxy, "RequestName", &error,
  		  G_TYPE_STRING, "org.ayatana.bamftest",
    		  G_TYPE_UINT, 0,
    		  G_TYPE_INVALID,
    		  G_TYPE_UINT, &request_name_result, G_TYPE_INVALID))
    g_error ("Could not grab service");

  g_object_unref (G_OBJECT (bus_proxy));

  test_matcher_create_suite ();
  test_view_create_suite ();
  test_application_create_suite ();
  
  return g_test_run ();
}
