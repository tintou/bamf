/*
 * Copyright (C) 2011 Canonical Ltd
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
 * Authored by Marco Trevisan (Treviño) <3v1n0@ubuntu.com>
 *
 */

#include <stdio.h>
#include <gtk/gtk.h>
#include <gdk/gdk.h>
#include <glib.h>
#include <gio/gio.h>
#include <sys/types.h>
#include <unistd.h>
#include <glibtop.h>
#include "bamf.h"

void test_application_create_suite (GDBusConnection *connection);
void test_matcher_create_suite (GDBusConnection *connection);
void test_view_create_suite (GDBusConnection *connection);
void test_window_create_suite (void);

static int result = 1;

static void
on_bus_acquired (GDBusConnection *connection, const gchar *name, gpointer data)
{
  GMainLoop *loop = data;

  test_matcher_create_suite (connection);
  test_view_create_suite (connection);
  test_window_create_suite ();
  test_application_create_suite (connection);
  result = g_test_run ();

  g_main_loop_quit (loop);
}

static void
on_name_lost (GDBusConnection *connection, const gchar *name, gpointer data)
{
  GMainLoop *loop = data;
  g_main_loop_quit (loop);
}

gint
main (gint argc, gchar *argv[])
{
  GMainLoop *loop;

  gtk_init (&argc, &argv);
  g_test_init (&argc, &argv, NULL);
  glibtop_init ();

  g_setenv("PATH", TESTDIR"/bamfdaemon/data/bin", TRUE);
  g_setenv("BAMF_TEST_MODE", "TRUE", TRUE);

  loop = g_main_loop_new (NULL, FALSE);

  g_bus_own_name (G_BUS_TYPE_SESSION,
                  BAMF_DBUS_SERVICE".test",
                  G_BUS_NAME_OWNER_FLAGS_NONE,
                  on_bus_acquired,
                  NULL,
                  on_name_lost,
                  loop,
                  NULL);

  g_main_loop_run (loop);

  return result;
}
