/*
 * Copyright (C) 2010-2011 Canonical Ltd
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
 *              Marco Trevisan (Treviño) <3v1n0@ubuntu.com>
 *
 */

#include "config.h"
#include "bamf-control.h"
#include "bamf-matcher.h"
#include "bamf-legacy-screen.h"
#include "bamf-indicator-source.h"

#include "main.h"

BamfControl *control;
BamfMatcher *matcher;
BamfIndicatorSource *approver;

static void
on_bus_acquired (GDBusConnection *connection,
                 const gchar     *name,
                 gpointer user_data)
{
  g_debug ("Acquired a message bus connection");
  GError *error = NULL;

  matcher = bamf_matcher_get_default ();
  control = bamf_control_get_default ();
  approver = bamf_indicator_source_get_default ();

  g_dbus_interface_skeleton_export (G_DBUS_INTERFACE_SKELETON (matcher),
                                    connection,
                                    BAMF_MATCHER_PATH,
                                    &error);

  if (error)
    {
      g_critical ("Can't register BAMF matcher at path %s: %s", BAMF_MATCHER_PATH, error->message);
      g_error_free (error);
    }

  g_dbus_interface_skeleton_export (G_DBUS_INTERFACE_SKELETON (control),
                                    connection,
                                    BAMF_CONTROL_PATH,
                                    &error);

  if (error)
    {
      g_critical ("Can't register BAMF control at path %s: %s", BAMF_CONTROL_PATH, error->message);
      g_error_free (error);
    }

  g_dbus_interface_skeleton_export (G_DBUS_INTERFACE_SKELETON (approver),
                                    connection,
                                    BAMF_INDICATOR_SOURCE_PATH,
                                    &error);

  if (error)
    {
      g_critical ("Can't register BAMF approver at path %s: %s\n", BAMF_INDICATOR_SOURCE_PATH, error->message);
      g_error_free (error);
    }
}

static void
on_name_acquired (GDBusConnection *connection,
                  const gchar     *name,
                  gpointer         user_data)
{
  g_debug ("Acquired the name %s", name);
}

static void
on_name_lost (GDBusConnection *connection,
              const gchar     *name,
              gpointer         user_data)
{
  g_critical ("Lost the name %s, another BAMF daemon is currently running", name);
  gtk_main_quit ();
}

int
main (int argc, char **argv)
{
  GOptionContext *options;
  GError *error = NULL;
  char *state_file = NULL;

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

  g_bus_own_name (G_BUS_TYPE_SESSION, BAMF_DBUS_SERVICE,
                  G_BUS_NAME_OWNER_FLAGS_NONE,
                  on_bus_acquired,
                  on_name_acquired,
                  on_name_lost,
                  NULL,
                  NULL);

  if (state_file)
    {
      bamf_legacy_screen_set_state_file (bamf_legacy_screen_get_default (), state_file);
    }

  gtk_main ();

  g_object_unref (matcher);
  g_object_unref (control);
  g_object_unref (approver);

  return 0;
}
