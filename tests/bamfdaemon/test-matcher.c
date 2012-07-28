/*
 * Copyright (C) 2009-2012 Canonical Ltd
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
 * Authored by: Neil Jagdish Patel <neil.patel@canonical.com>
 *              Marco Trevisan (Treviño) <marco.trevisan@canonical.com>
 *
 */

#include <glib.h>
#include <stdlib.h>
#include "bamf-matcher.h"
#include "bamf-matcher-private.h"
#include "bamf-legacy-screen-private.h"
#include "bamf-legacy-window.h"
#include "bamf-legacy-window-test.h"

static void test_allocation (void);
static void test_load_desktop_file (void);
static void test_open_windows (void);
static void test_match_desktopless_application (void);
static void test_match_desktop_application (void);

static GDBusConnection *gdbus_connection = NULL;

#define DOMAIN "/Matcher"
#define DATA_DIR "bamfdaemon/data"
#define TEST_BAMF_APP_DESKTOP TESTDIR "/" DATA_DIR "/test-bamf-app.desktop"

void
test_matcher_create_suite (GDBusConnection *connection)
{
  gdbus_connection = connection;

  g_test_add_func (DOMAIN"/Allocation", test_allocation);
  g_test_add_func (DOMAIN"/LoadDesktopFile", test_load_desktop_file);
  g_test_add_func (DOMAIN"/OpenWindows", test_open_windows);
  g_test_add_func (DOMAIN"/MatchDesktopLessApplication", test_match_desktopless_application);
  g_test_add_func (DOMAIN"/MatchDesktopApplication", test_match_desktop_application);
}

static void
export_matcher_on_bus (BamfMatcher *matcher)
{
  GError *error = NULL;
  g_return_if_fail (BAMF_IS_MATCHER (matcher));

  g_dbus_interface_skeleton_export (G_DBUS_INTERFACE_SKELETON (matcher),
                                    gdbus_connection, BAMF_MATCHER_PATH"/Test",
                                    &error);

  g_assert (!error);
  g_clear_error (&error);
}

static BamfWindow *
find_window_in_matcher (BamfMatcher *matcher, BamfLegacyWindow *legacy)
{
  GList *l;
  BamfWindow *found_window = NULL;

  for (l = matcher->priv->views; l; l = l->next)
    {
      if (!BAMF_IS_WINDOW (l->data))
        continue;

      if (bamf_window_get_window (BAMF_WINDOW (l->data)) == legacy)
      {
        g_assert (!found_window);
        found_window = l->data;
      }
    }

  return found_window;
}

static BamfWindow *
find_window_in_app (BamfApplication *app, BamfLegacyWindow *legacy)
{
  GList *l;
  BamfWindow *found_window = NULL;

  for (l = bamf_view_get_children (BAMF_VIEW (app)); l; l = l->next)
    {
      if (!BAMF_IS_WINDOW (l->data))
        continue;

      if (bamf_window_get_window (BAMF_WINDOW (l->data)) == legacy)
      {
        g_assert (!found_window);
        found_window = l->data;
      }
    }

  return found_window;
}

static void
test_allocation (void)
{
  BamfMatcher *matcher;

  /* Check it allocates */
  matcher = bamf_matcher_get_default ();
  g_assert (BAMF_IS_MATCHER (matcher));
  g_object_unref (matcher);
}

static void
test_load_desktop_file (void)
{
  BamfMatcher *matcher = bamf_matcher_get_default ();
  BamfMatcherPrivate *priv = matcher->priv;

  bamf_matcher_load_desktop_file (matcher, TEST_BAMF_APP_DESKTOP);

  GList *l = g_hash_table_lookup (priv->desktop_file_table, "testbamfapp");
  g_assert_cmpstr (l->data, ==, TEST_BAMF_APP_DESKTOP);

  l = g_hash_table_lookup (priv->desktop_id_table, "test-bamf-app");
  g_assert_cmpstr (l->data, ==, TEST_BAMF_APP_DESKTOP);

  const char *desktop = g_hash_table_lookup (priv->desktop_class_table, TEST_BAMF_APP_DESKTOP);
  g_assert_cmpstr (desktop, ==, "test_bamf_app");
}

static void
test_open_windows (void)
{
  BamfMatcher *matcher;
  BamfLegacyScreen *screen;
  BamfLegacyWindow *window;
  BamfLegacyWindowTest *test_win;
  guint xid;
  const int window_count = 500;

  screen = bamf_legacy_screen_get_default();
  matcher = bamf_matcher_get_default ();

  export_matcher_on_bus (matcher);

  for (xid = G_MAXUINT; xid > G_MAXUINT-window_count; xid--)
    {
      gchar *name = g_strdup_printf ("Test Window %u", xid);
      gchar *class = g_strdup_printf ("test-class-%u", xid);
      gchar *exec = g_strdup_printf ("test-class-%u", xid);

      test_win = bamf_legacy_window_test_new (xid, name, class, exec);
      window = BAMF_LEGACY_WINDOW (test_win);

      _bamf_legacy_screen_open_test_window (screen, test_win);
      g_assert (g_list_find (bamf_legacy_screen_get_windows (screen), test_win));
      g_assert (find_window_in_matcher (matcher, window));
      g_assert (bamf_matcher_get_application_by_xid (matcher, xid));

      _bamf_legacy_screen_close_test_window (screen, test_win);
      g_assert (!g_list_find (bamf_legacy_screen_get_windows (screen), test_win));
      g_assert (!find_window_in_matcher (matcher, window));
      g_assert (!bamf_matcher_get_application_by_xid (matcher, xid));

      g_free (name);
      g_free (class);
      g_free (exec);
    }

  g_object_unref (matcher);
  g_object_unref (screen);
}

static void
test_match_desktopless_application (void)
{
  BamfMatcher *matcher;
  BamfLegacyScreen *screen;
  BamfLegacyWindow *window;
  BamfLegacyWindowTest *test_win;
  BamfApplication *app;
  GList *test_windows = NULL, *l, *app_children;
  guint xid;
  const int window_count = 5;

  screen = bamf_legacy_screen_get_default();
  matcher = bamf_matcher_get_default ();
  char *exec = "test-bamf-app";
  char *class = "test-bamf-app";

  export_matcher_on_bus (matcher);

  for (xid = G_MAXUINT; xid > G_MAXUINT-window_count; xid--)
    {
      gchar *name = g_strdup_printf ("Test Window %u", xid);

      test_win = bamf_legacy_window_test_new (xid, name, class, exec);
      window = BAMF_LEGACY_WINDOW (test_win);
      test_windows = g_list_prepend (test_windows, window);

      _bamf_legacy_screen_open_test_window (screen, test_win);
      g_free (name);
    }

  app = bamf_matcher_get_application_by_xid (matcher, G_MAXUINT);
  g_assert (app);

  app_children = bamf_view_get_children (BAMF_VIEW (app));
  g_assert_cmpuint (g_list_length (app_children), ==, window_count);

  for (l = test_windows; l; l = l->next)
    {
      g_assert (find_window_in_app (app, BAMF_LEGACY_WINDOW (l->data)));
    }

  g_list_free (test_windows);
  g_object_unref (matcher);
  g_object_unref (screen);
}

static void
test_match_desktop_application (void)
{
  BamfMatcher *matcher;
  BamfLegacyScreen *screen;
  BamfLegacyWindow *window;
  BamfLegacyWindowTest *test_win;
  BamfApplication *app;
  GList *test_windows = NULL, *l, *app_children;
  guint xid;
  const int window_count = 5;

  screen = bamf_legacy_screen_get_default();
  matcher = bamf_matcher_get_default ();
  char *exec = "testbamfapp";
  char *class = "test_bamf_app";

  export_matcher_on_bus (matcher);
  bamf_matcher_load_desktop_file (matcher, TEST_BAMF_APP_DESKTOP);

  for (xid = G_MAXUINT; xid > G_MAXUINT-window_count; xid--)
    {
      gchar *name = g_strdup_printf ("Test Window %u", xid);

      test_win = bamf_legacy_window_test_new (xid, name, class, exec);
      window = BAMF_LEGACY_WINDOW (test_win);
      test_windows = g_list_prepend (test_windows, window);

      _bamf_legacy_screen_open_test_window (screen, test_win);
      g_free (name);
    }

  app = bamf_matcher_get_application_by_desktop_file (matcher, TEST_BAMF_APP_DESKTOP);
  g_assert (app);

  g_assert (bamf_matcher_get_application_by_xid (matcher, G_MAXUINT) == app);

  app_children = bamf_view_get_children (BAMF_VIEW (app));
  g_assert_cmpuint (g_list_length (app_children), ==, window_count);

  for (l = test_windows; l; l = l->next)
    {
      g_assert (find_window_in_app (app, BAMF_LEGACY_WINDOW (l->data)));
    }

  g_list_free (test_windows);
  g_object_unref (matcher);
  g_object_unref (screen);
}

