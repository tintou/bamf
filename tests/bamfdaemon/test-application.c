/*
 * Copyright (C) 2009 Canonical Ltd
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
static void test_urgent              (void);
static void test_get_xids            (void);
static void test_manages_xid         (void);
static void test_user_visible        (void);
static void test_urgent              (void);

static gboolean signal_seen = FALSE;
static gboolean signal_result = FALSE;

void
test_application_create_suite (void)
{
#define DOMAIN "/Application"

  g_test_add_func (DOMAIN"/Allocation", test_allocation);
  g_test_add_func (DOMAIN"/DesktopFile", test_desktop_file);
  g_test_add_func (DOMAIN"/ManagesXid", test_manages_xid);
  g_test_add_func (DOMAIN"/Urgent", test_urgent);
  g_test_add_func (DOMAIN"/UserVisible", test_user_visible);
  g_test_add_func (DOMAIN"/Xids", test_get_xids);
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
  g_assert (!bamf_application_is_urgent (application));
  
  bamf_view_add_child (BAMF_VIEW (application), BAMF_VIEW (window1));
  
  // Test that when added, we signaled properly
  g_assert (!bamf_application_is_urgent (application));
  g_assert (!signal_seen);
  
  bamf_view_remove_child (BAMF_VIEW (application), BAMF_VIEW (window1));
  
  // Test that we unset and signal properly
  g_assert (!bamf_application_is_urgent (application));
  g_assert (!signal_seen);
  
  bamf_legacy_window_test_set_attention (test1, TRUE);
  bamf_view_add_child (BAMF_VIEW (application), BAMF_VIEW (window1));
  
  // Ensure that when adding a skip-tasklist window, we dont set this to visible 
  g_assert (bamf_application_is_urgent (application));
  g_assert (signal_seen);
  g_assert (signal_result);
  
  signal_seen = FALSE;

  bamf_view_add_child (BAMF_VIEW (application), BAMF_VIEW (window2));
  
  g_assert (bamf_application_is_urgent (application));
  g_assert (!signal_seen);
  
  bamf_legacy_window_test_set_attention (test1, FALSE);
  
  g_assert (!bamf_application_is_urgent (application));
  g_assert (signal_seen);
  g_assert (!signal_result);
}

static void
test_get_xids (void)
{
  BamfApplication *application;
  BamfWindow *window1, *window2;
  GArray *xids;
  gboolean found;
  guint32 xid;
  int i;

  application = bamf_application_new ();
  
  // Leaks memory
  window1 = bamf_window_new (BAMF_LEGACY_WINDOW (bamf_legacy_window_test_new (25, "window1", "class", "exec")));
  window2 = bamf_window_new (BAMF_LEGACY_WINDOW (bamf_legacy_window_test_new (50, "window2", "class", "exec")));

  xids = bamf_application_get_xids (application);
  g_assert (xids->len == 0);
  g_array_free (xids, TRUE);

  bamf_view_add_child (BAMF_VIEW (application), BAMF_VIEW (window1));
  bamf_view_add_child (BAMF_VIEW (application), BAMF_VIEW (window2));

  xids = bamf_application_get_xids (application);
  g_assert (xids->len == 2);

  found = FALSE;
  for (i = 0; i < xids->len; i++)
    {
      xid = g_array_index (xids, guint32, i);
      if (xid == 25)
        {
          found = TRUE;
          break;
        }
    }

  g_assert (found);

  found = FALSE;
  for (i = 0; i < xids->len; i++)
    {
      xid = g_array_index (xids, guint32, i);
      if (xid == 50)
        {
          found = TRUE;
          break;
        }
    }

  g_assert (found);
 
  g_object_unref (application);
}

static void
test_manages_xid (void)
{
  BamfApplication *application;
  BamfWindow *test;

  application = bamf_application_new ();
  test = bamf_window_new (BAMF_LEGACY_WINDOW (bamf_legacy_window_test_new (20, "window", "class", "exec")));

  bamf_view_add_child (BAMF_VIEW (application), BAMF_VIEW (test));

  g_assert (bamf_application_manages_xid (application, 20));

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
  g_assert (!bamf_application_user_visible (application));
  
  bamf_view_add_child (BAMF_VIEW (application), BAMF_VIEW (window1));
  
  // Test that when added, we signaled properly
  g_assert (bamf_application_user_visible (application));
  g_assert (signal_seen);
  g_assert (signal_result);
  
  signal_seen = FALSE;
  
  bamf_view_remove_child (BAMF_VIEW (application), BAMF_VIEW (window1));
  
  // Test that we unset and signal properly
  g_assert (!bamf_application_user_visible (application));
  g_assert (signal_seen);
  g_assert (!signal_result);
  
  signal_seen = FALSE;
  
  bamf_legacy_window_test_set_skip (test1, TRUE);
  bamf_view_add_child (BAMF_VIEW (application), BAMF_VIEW (window1));
  
  // Ensure that when adding a skip-tasklist window, we dont set this to visible 
  g_assert (!bamf_application_user_visible (application));
  g_assert (!signal_seen);
  
  bamf_view_add_child (BAMF_VIEW (application), BAMF_VIEW (window2));
  
  g_assert (bamf_application_user_visible (application));
  g_assert (signal_seen);
  g_assert (signal_result);
  
  signal_seen = FALSE;
  
  bamf_legacy_window_test_set_skip (test2, TRUE);
  
  g_assert (!bamf_window_user_visible (window1));
  g_assert (!bamf_application_user_visible (application));
  g_assert (signal_seen);
  g_assert (!signal_result);
}
