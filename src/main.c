/*
 * Copyright (C) 2010 Canonical Ltd
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 3 as
 * published by the Free Software Foundation.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 *
 * Authored by: Jason Smith <jason.smith@canonical.com>
 *
 */

#include "config.h"
#include "bamf-control.h"
#include "bamf-matcher.h"
#include "bamf-legacy-screen.h"
#include "bamf-indicator-source.h"
#include <dbus/dbus.h>
#include <dbus/dbus-glib.h>
#include <dbus/dbus-glib-lowlevel.h>

#include "main.h"

int
main (int argc, char **argv)
{
  GOptionContext *options;
  BamfControl *control;
  BamfMatcher *matcher;
  BamfIndicatorSource *approver;
  DBusGConnection *bus;
  DBusGProxy *bus_proxy;
  GError *error = NULL;
  char *state_file = NULL;
  guint request_name_result;

  gtk_init (&argc, &argv);
  glibtop_init ();

  options = g_option_context_new ("");
  g_option_context_set_help_enabled (options, TRUE);
  g_option_context_set_summary (options, "It's one, and so are we...");

  GOptionEntry entries[] =
  {
    {"load-file", 'l', 0, G_OPTION_ARG_STRING, &state_file, "Load bamf state from file instead of the system", NULL },
    {NULL}
  };

  g_option_context_add_main_entries (options, entries, NULL);

  g_option_context_add_group (options, gtk_get_option_group (FALSE));

  g_option_context_parse (options, &argc, &argv, &error);

  if (error)
    {
      g_error_free (error);
      error = NULL;

      g_print ("%s\n", g_option_context_get_help (options, TRUE, NULL));
      exit (1);
    }

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
  		  G_TYPE_STRING, BAMF_DBUS_SERVICE,
    		  G_TYPE_UINT, 0,
    		  G_TYPE_INVALID,
    		  G_TYPE_UINT, &request_name_result, G_TYPE_INVALID))
    g_error ("Could not grab service");

  g_object_unref (G_OBJECT (bus_proxy));

  matcher  = bamf_matcher_get_default ();
  control  = bamf_control_get_default ();
  approver = bamf_indicator_source_get_default ();

  g_print ("They make me do something with things I dont need to do stuff with %p %p %p", matcher, control, approver);

  if (state_file)
    {
      bamf_legacy_screen_set_state_file (bamf_legacy_screen_get_default (), state_file);
    }

  gtk_main ();

  return 0;
}
