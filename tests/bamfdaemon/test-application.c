/*
 * Copyright (C) 2009-2011 Canonical Ltd
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
 * Authored by Jason Smith <jason.smith@canonical.com>
 *             Marco Trevisan (Treviño) <3v1n0@ubuntu.com>
 *
 */

#include <stdlib.h>
#include <glib.h>
#include <glib-object.h>
#include "bamf-application.h"
#include "bamf-window.h"
#include "bamf-legacy-window.h"
#include "bamf-legacy-window-test.h"

#define DESKTOP_FILE "/usr/share/applications/gnome-terminal.desktop"

static void test_allocation          (void);
static void test_desktop_file        (void);
static void test_desktop_no_icon     (void);
static void test_get_mime_types      (void);
static void test_get_mime_types_none (void);
static void test_urgent              (void);
static void test_active              (void);
static void test_get_xids            (void);
static void test_manages_xid         (void);
static void test_user_visible        (void);
static void test_urgent              (void);
static void test_window_added        (void);
static void test_window_removed      (void);

static gboolean          signal_seen   = FALSE;
static gboolean          signal_result = FALSE;
static char *            signal_window = NULL;
static GDBusConnection * gdbus_connection = NULL;

void
test_application_create_suite (GDBusConnection *connection)
{
#define DOMAIN "/Application"

  gdbus_connection = connection;

  g_test_add_func (DOMAIN"/Allocation", test_allocation);
  g_test_add_func (DOMAIN"/DesktopFile", test_desktop_file);
  g_test_add_func (DOMAIN"/DesktopFile/NoIcon", test_desktop_no_icon);
  g_test_add_func (DOMAIN"/DesktopFile/MimeTypes/Valid", test_get_mime_types);
  g_test_add_func (DOMAIN"/DesktopFile/MimeTypes/None", test_get_mime_types_none);
  g_test_add_func (DOMAIN"/ManagesXid", test_manages_xid);
  g_test_add_func (DOMAIN"/Xids", test_get_xids);
  g_test_add_func (DOMAIN"/Events/Active", test_active);
  g_test_add_func (DOMAIN"/Events/Urgent", test_urgent);
  g_test_add_func (DOMAIN"/Events/UserVisible", test_user_visible);
  g_test_add_func (DOMAIN"/Events/WindowAdded", test_window_added);
  g_test_add_func (DOMAIN"/Events/WindowRemoved", test_window_removed);
}

static void
test_allocation (void)
{
  BamfApplication    *application;

  /* Check it allocates */
  application = bamf_application_new ();
  g_assert (BAMF_IS_APPLICATION (application));

  g_object_unref (application);
  g_assert (!BAMF_IS_APPLICATION (application));

  application = bamf_application_new_from_desktop_file (DESKTOP_FILE);
  g_assert (BAMF_IS_APPLICATION (application));

  g_object_unref (application);
  g_assert (!BAMF_IS_APPLICATION (application));
}

static void
test_desktop_file (void)
{
  BamfApplication    *application;

  /* Check it allocates */
  application = bamf_application_new ();
  g_assert (bamf_application_get_desktop_file (application) == NULL);

  bamf_application_set_desktop_file (application, DESKTOP_FILE);
  g_assert (g_strcmp0 (bamf_application_get_desktop_file (application), DESKTOP_FILE) == 0);

  g_object_unref (application);

  application = bamf_application_new_from_desktop_file (DESKTOP_FILE);
  g_assert (g_strcmp0 (bamf_application_get_desktop_file (application), DESKTOP_FILE) == 0);

  g_object_unref (application);
}

static void
test_desktop_no_icon (void)
{
  BamfApplication    *application;
  const char no_icon_desktop[] = TESTDIR"/bamfdaemon/data/no-icon.desktop";

  application = bamf_application_new_from_desktop_file (no_icon_desktop);
  g_assert (g_strcmp0 (bamf_application_get_desktop_file (application), no_icon_desktop) == 0);

  g_assert (g_strcmp0(bamf_view_get_icon(BAMF_VIEW(application)), "application-default-icon") == 0);
  g_object_unref (application);
}

static void
test_get_mime_types (void)
{
  BamfApplication *application;
  const char* mime_types_desktop = TESTDIR"/bamfdaemon/data/mime-types.desktop";

  application = bamf_application_new_from_desktop_file (mime_types_desktop);
  g_assert_cmpstr (bamf_application_get_desktop_file (application), ==, mime_types_desktop);

  gchar** mimes = bamf_application_get_supported_mime_types (application);

  g_assert_cmpuint (g_strv_length (mimes), ==, 7);
  g_assert_cmpstr (mimes[0], ==, "text/plain");
  g_assert_cmpstr (mimes[1], ==, "text/x-chdr");
  g_assert_cmpstr (mimes[2], ==, "text/x-csrc");
  g_assert_cmpstr (mimes[3], ==, "text/html");
  g_assert_cmpstr (mimes[4], ==, "text/css");
  g_assert_cmpstr (mimes[5], ==, "text/x-diff");
  g_assert_cmpstr (mimes[6], ==, "application/xml");

  g_strfreev (mimes);
  g_object_unref (application);
}

static void
test_get_mime_types_none (void)
{
  BamfApplication *application;
  const char* mime_types_desktop = TESTDIR"/bamfdaemon/data/test-bamf-app.desktop";

  application = bamf_application_new_from_desktop_file (mime_types_desktop);
  g_assert_cmpstr (bamf_application_get_desktop_file (application), ==, mime_types_desktop);

  gchar** mimes = bamf_application_get_supported_mime_types (application);
  g_assert (!mimes);

  g_object_unref (application);
}

static void
on_urgent_changed (BamfApplication *application, gboolean result, gpointer data)
{
  signal_seen = TRUE;
  signal_result = result;
}

static void
test_urgent (void)
{
  signal_seen = FALSE;

  BamfApplication *application;
  BamfWindow *window1, *window2;
  BamfLegacyWindowTest *test1, *test2;

  application = bamf_application_new ();

  g_signal_connect (G_OBJECT (application), "urgent-changed", (GCallback) on_urgent_changed, NULL);

  test1 = bamf_legacy_window_test_new (20, "Window X", "class", "exec");
  test2 = bamf_legacy_window_test_new (20, "Window Y", "class", "exec");

  window1 = bamf_window_new (BAMF_LEGACY_WINDOW (test1));
  window2 = bamf_window_new (BAMF_LEGACY_WINDOW (test2));

  // Ensure we are not visible with no windows
  g_assert (!bamf_view_is_urgent (BAMF_VIEW (application)));

  bamf_view_add_child (BAMF_VIEW (application), BAMF_VIEW (window1));

  // Test that when added, we signaled properly
  g_assert (!bamf_view_is_urgent (BAMF_VIEW (application)));
  g_assert (!signal_seen);

  bamf_view_remove_child (BAMF_VIEW (application), BAMF_VIEW (window1));

  // Test that we unset and signal properly
  g_assert (!bamf_view_is_urgent (BAMF_VIEW (application)));
  g_assert (!signal_seen);

  bamf_legacy_window_test_set_attention (test1, TRUE);
  bamf_view_add_child (BAMF_VIEW (application), BAMF_VIEW (window1));

  // Ensure that when adding a skip-tasklist window, we dont set this to visible
  g_assert (bamf_view_is_urgent (BAMF_VIEW (application)));
  g_assert (signal_seen);
  g_assert (signal_result);

  signal_seen = FALSE;

  bamf_view_add_child (BAMF_VIEW (application), BAMF_VIEW (window2));

  g_assert (bamf_view_is_urgent (BAMF_VIEW (application)));
  g_assert (!signal_seen);

  bamf_legacy_window_test_set_attention (test1, FALSE);

  g_assert (!bamf_view_is_urgent (BAMF_VIEW (application)));
  g_assert (signal_seen);
  g_assert (!signal_result);
}

static void
on_active_changed (BamfApplication *application, gboolean result, gpointer data)
{
  signal_seen = TRUE;
  signal_result = result;
}

static void
test_active (void)
{
  signal_seen = FALSE;

  BamfApplication *application;
  BamfWindow *window1, *window2;
  BamfLegacyWindowTest *test1, *test2;

  application = bamf_application_new ();

  g_signal_connect (G_OBJECT (application), "active-changed", (GCallback) on_active_changed, NULL);

  test1 = bamf_legacy_window_test_new (20, "Window X", "class", "exec");
  test2 = bamf_legacy_window_test_new (20, "Window Y", "class", "exec");

  window1 = bamf_window_new (BAMF_LEGACY_WINDOW (test1));
  window2 = bamf_window_new (BAMF_LEGACY_WINDOW (test2));

  // Ensure we are not active with no windows
  g_assert (!bamf_view_is_active (BAMF_VIEW (application)));

  bamf_view_add_child (BAMF_VIEW (application), BAMF_VIEW (window1));

  // Test that when added, we signaled properly
  g_assert (!bamf_view_is_active (BAMF_VIEW (application)));
  g_assert (!signal_seen);

  bamf_view_remove_child (BAMF_VIEW (application), BAMF_VIEW (window1));

  // Test that we unset and signal properly
  g_assert (!bamf_view_is_active (BAMF_VIEW (application)));
  g_assert (!signal_seen);

  bamf_legacy_window_test_set_active (test1, TRUE);
  bamf_view_add_child (BAMF_VIEW (application), BAMF_VIEW (window1));

  // Ensure that when adding a skip-tasklist window, we dont set this to visible
  g_assert (bamf_view_is_active (BAMF_VIEW (application)));
  g_assert (!signal_seen);
  while (g_main_context_pending (NULL)) g_main_context_iteration(NULL, TRUE);
  g_assert (signal_seen);
  g_assert (signal_result);

  signal_seen = FALSE;

  bamf_view_add_child (BAMF_VIEW (application), BAMF_VIEW (window2));

  g_assert (bamf_view_is_active (BAMF_VIEW (application)));
  g_assert (!signal_seen);

  bamf_legacy_window_test_set_active (test1, FALSE);
  g_assert (!signal_seen);
  g_assert (bamf_view_is_active (BAMF_VIEW (application)));
  while (g_main_context_pending (NULL)) g_main_context_iteration(NULL, TRUE);

  g_assert (!bamf_view_is_active (BAMF_VIEW (application)));
  g_assert (signal_seen);
  g_assert (!signal_result);
}

static void
test_get_xids (void)
{
  BamfApplication *application;
  BamfWindow *window1, *window2;
  BamfLegacyWindowTest *lwin1, *lwin2;
  GVariant *container;
  GVariantIter *xids;
  gboolean found;
  guint32 xid;

  application = bamf_application_new ();

  lwin1 = bamf_legacy_window_test_new (25, "window1", "class", "exec");
  lwin2 = bamf_legacy_window_test_new (50, "window2", "class", "exec");
  window1 = bamf_window_new (BAMF_LEGACY_WINDOW (lwin1));
  window2 = bamf_window_new (BAMF_LEGACY_WINDOW (lwin2));

  container = bamf_application_get_xids (application);
  g_assert (g_variant_type_equal (g_variant_get_type (container),
                                  G_VARIANT_TYPE ("(au)")));
  g_assert (g_variant_n_children (container) == 1);
  g_variant_get (container, "(au)", &xids);
  g_assert (g_variant_iter_n_children (xids) == 0);
  g_variant_iter_free (xids);
  g_variant_unref (container);

  bamf_view_add_child (BAMF_VIEW (application), BAMF_VIEW (window1));
  bamf_view_add_child (BAMF_VIEW (application), BAMF_VIEW (window2));

  container = bamf_application_get_xids (application);
  g_assert (g_variant_n_children (container) == 1);
  g_variant_get (container, "(au)", &xids);
  g_assert (g_variant_iter_n_children (xids) == 2);

  found = FALSE;
  while (g_variant_iter_loop (xids, "u", &xid))
    {
      if (xid == 25)
        {
          found = TRUE;
          break;
        }
    }

  g_assert (found);

  found = FALSE;
  g_variant_get (container, "(au)", &xids);
  while (g_variant_iter_loop (xids, "u", &xid))
    {
      if (xid == 50)
        {
          found = TRUE;
          break;
        }
    }

  g_assert (found);

  g_variant_iter_free (xids);
  g_variant_unref (container);

  g_object_unref (lwin1);
  g_object_unref (lwin2);
  g_object_unref (window1);
  g_object_unref (window2);
  g_object_unref (application);
}

static void
test_manages_xid (void)
{
  BamfApplication *application;
  BamfLegacyWindowTest *lwin;
  BamfWindow *test;

  application = bamf_application_new ();
  lwin = bamf_legacy_window_test_new (20, "window", "class", "exec");
  test = bamf_window_new (BAMF_LEGACY_WINDOW (lwin));

  bamf_view_add_child (BAMF_VIEW (application), BAMF_VIEW (test));

  g_assert (bamf_application_manages_xid (application, 20));

  g_object_unref (lwin);
  g_object_unref (test);
  g_object_unref (application);
}

static void
on_user_visible_changed (BamfApplication *application, gboolean result, gpointer data)
{
  signal_seen = TRUE;
  signal_result = result;
}

static void
test_user_visible (void)
{
  signal_seen = FALSE;

  BamfApplication *application;
  BamfWindow *window1, *window2;
  BamfLegacyWindowTest *test1, *test2;

  application = bamf_application_new ();

  g_signal_connect (G_OBJECT (application), "user-visible-changed", (GCallback) on_user_visible_changed, NULL);

  test1 = bamf_legacy_window_test_new (20, "Window X", "class", "exec");
  test2 = bamf_legacy_window_test_new (20, "Window Y", "class", "exec");

  window1 = bamf_window_new (BAMF_LEGACY_WINDOW (test1));
  window2 = bamf_window_new (BAMF_LEGACY_WINDOW (test2));

  // Ensure we are not visible with no windows
  g_assert (!bamf_view_user_visible (BAMF_VIEW (application)));

  bamf_view_add_child (BAMF_VIEW (application), BAMF_VIEW (window1));

  // Test that when added, we signaled properly
  g_assert (bamf_view_user_visible (BAMF_VIEW (application)));
  g_assert (signal_seen);
  g_assert (signal_result);

  signal_seen = FALSE;

  bamf_view_remove_child (BAMF_VIEW (application), BAMF_VIEW (window1));

  // Test that we unset and signal properly
  g_assert (!bamf_view_user_visible (BAMF_VIEW (application)));
  g_assert (signal_seen);
  g_assert (!signal_result);

  signal_seen = FALSE;

  bamf_legacy_window_test_set_skip (test1, TRUE);
  bamf_view_add_child (BAMF_VIEW (application), BAMF_VIEW (window1));

  // Ensure that when adding a skip-tasklist window, we dont set this to visible
  g_assert (!bamf_view_user_visible (BAMF_VIEW (application)));
  g_assert (!signal_seen);

  bamf_view_add_child (BAMF_VIEW (application), BAMF_VIEW (window2));

  g_assert (bamf_view_user_visible (BAMF_VIEW (application)));
  g_assert (signal_seen);
  g_assert (signal_result);

  signal_seen = FALSE;

  bamf_legacy_window_test_set_skip (test2, TRUE);

  g_assert (!bamf_view_user_visible (BAMF_VIEW (window1)));
  g_assert (!bamf_view_user_visible (BAMF_VIEW (application)));
  g_assert (signal_seen);
  g_assert (!signal_result);
}

static void
on_window_added (BamfApplication *application, char *window, gpointer data)
{
  signal_seen = TRUE;
  signal_window = g_strdup (window);
}

static void
test_window_added (void)
{
  signal_seen = FALSE;

  BamfApplication *application;
  BamfWindow *window;
  BamfLegacyWindowTest *test;
  const char *path;

  application = bamf_application_new ();

  g_signal_connect (G_OBJECT (application), "window-added", (GCallback) on_window_added, NULL);

  test = bamf_legacy_window_test_new (20, "Window X", "class", "exec");
  window = bamf_window_new (BAMF_LEGACY_WINDOW (test));

  bamf_view_add_child (BAMF_VIEW (application), BAMF_VIEW (window));

  // Ensure we dont signal things that are not on the bus
  g_assert (!signal_seen);

  bamf_view_remove_child (BAMF_VIEW (application), BAMF_VIEW (window));

  path = bamf_view_export_on_bus (BAMF_VIEW (window), gdbus_connection);

  bamf_view_add_child (BAMF_VIEW (application), BAMF_VIEW (window));

  g_assert (signal_seen);
  g_assert_cmpstr (signal_window, ==, path);

  signal_seen = FALSE;

  g_object_unref (window);
  g_object_unref (test);
}

static void
on_window_removed (BamfApplication *application, char *window, gpointer data)
{
  signal_seen = TRUE;
  signal_window = g_strdup (window);
}

static void
test_window_removed (void)
{
  signal_seen = FALSE;

  BamfApplication *application;
  BamfWindow *window;
  BamfLegacyWindowTest *test;
  const char *path;

  application = bamf_application_new ();

  g_signal_connect (G_OBJECT (application), "window-removed", (GCallback) on_window_removed, NULL);

  test = bamf_legacy_window_test_new (20, "Window X", "class", "exec");
  window = bamf_window_new (BAMF_LEGACY_WINDOW (test));

  bamf_view_add_child (BAMF_VIEW (application), BAMF_VIEW (window));
  bamf_view_remove_child (BAMF_VIEW (application), BAMF_VIEW (window));

  // Ensure we dont signal things that are not on the bus
  g_assert (!signal_seen);

  path = bamf_view_export_on_bus (BAMF_VIEW (window), gdbus_connection);

  bamf_view_add_child (BAMF_VIEW (application), BAMF_VIEW (window));
  bamf_view_remove_child (BAMF_VIEW (application), BAMF_VIEW (window));

  g_assert (signal_seen);
  g_assert (g_strcmp0 (signal_window, path) == 0);

  signal_seen = FALSE;

  g_object_unref (window);
  g_object_unref (test);
}
