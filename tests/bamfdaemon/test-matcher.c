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
static void test_match_libreoffice_windows (void);
static void test_match_gnome_control_center_panels (void);
static void test_match_javaws_windows (void);
static void test_match_javaws_windows_hint_ignored (void);
static void test_new_desktop_matches_unmatched_windows (void);
static void test_trim_exec_string (void);

static GDBusConnection *gdbus_connection = NULL;

#define DOMAIN "/Matcher"
#define DATA_DIR TESTDIR "/bamfdaemon/data"
#define TEST_BAMF_APP_DESKTOP DATA_DIR "/test-bamf-app.desktop"

void
test_matcher_create_suite (GDBusConnection *connection)
{
  gdbus_connection = connection;

  g_test_add_func (DOMAIN"/Allocation", test_allocation);
  g_test_add_func (DOMAIN"/LoadDesktopFile", test_load_desktop_file);
  g_test_add_func (DOMAIN"/OpenWindows", test_open_windows);
  g_test_add_func (DOMAIN"/Matching/Application/DesktopLess", test_match_desktopless_application);
  g_test_add_func (DOMAIN"/Matching/Application/Desktop", test_match_desktop_application);
  g_test_add_func (DOMAIN"/Matching/Application/LibreOffice", test_match_libreoffice_windows);
  g_test_add_func (DOMAIN"/Matching/Application/GnomeControlCenter", test_match_gnome_control_center_panels);
  g_test_add_func (DOMAIN"/Matching/Application/JavaWebStart", test_match_javaws_windows);
  g_test_add_func (DOMAIN"/Matching/Application/JavaWebStartHintIngored", test_match_javaws_windows_hint_ignored);
  g_test_add_func (DOMAIN"/Matching/Windows/UnmatchedOnNewDesktop", test_new_desktop_matches_unmatched_windows);
  g_test_add_func (DOMAIN"/ExecStringTrimming", test_trim_exec_string);
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

static void
cleanup_matcher_tables (BamfMatcher *matcher)
{
  g_return_if_fail (BAMF_IS_MATCHER (matcher));

  g_hash_table_destroy (matcher->priv->desktop_file_table);
  g_hash_table_destroy (matcher->priv->desktop_id_table);
  g_hash_table_destroy (matcher->priv->desktop_class_table);

  matcher->priv->desktop_file_table =
    g_hash_table_new_full ((GHashFunc) g_str_hash,
                           (GEqualFunc) g_str_equal,
                           (GDestroyNotify) g_free,
                           NULL);

  matcher->priv->desktop_id_table =
    g_hash_table_new_full ((GHashFunc) g_str_hash,
                           (GEqualFunc) g_str_equal,
                           (GDestroyNotify) g_free,
                           NULL);

  matcher->priv->desktop_class_table =
    g_hash_table_new_full ((GHashFunc) g_str_hash,
                           (GEqualFunc) g_str_equal,
                           (GDestroyNotify) g_free,
                           (GDestroyNotify) g_free);
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

  g_return_val_if_fail (BAMF_IS_APPLICATION (app), NULL);

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

  cleanup_matcher_tables (matcher);
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

  cleanup_matcher_tables (matcher);
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
  const char *exec = "test-bamf-app";
  const char *class = "test-bamf-app";

  cleanup_matcher_tables (matcher);
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
  const char *exec = "testbamfapp";
  const char *class = "test_bamf_app";

  cleanup_matcher_tables (matcher);
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

static void
test_new_desktop_matches_unmatched_windows (void)
{
  BamfMatcher *matcher;
  BamfLegacyScreen *screen;
  BamfLegacyWindowTest *test_win;
  BamfApplication *app;
  GList *app_children;
  guint xid = 0;
  const int window_count = 5;

  screen = bamf_legacy_screen_get_default();
  matcher = bamf_matcher_get_default ();
  const char *exec = "testbamfapp";
  const char *class = "test_bamf_app";

  cleanup_matcher_tables (matcher);
  export_matcher_on_bus (matcher);
  g_assert (!bamf_matcher_get_application_by_desktop_file (matcher, TEST_BAMF_APP_DESKTOP));

  for (xid = G_MAXUINT; xid > G_MAXUINT-window_count; xid--)
    {
      gchar *name = g_strdup_printf ("Test Window %u", xid);

      test_win = bamf_legacy_window_test_new (xid, name, class, exec);
      _bamf_legacy_screen_open_test_window (screen, test_win);

      g_free (name);
    }

  bamf_matcher_load_desktop_file (matcher, TEST_BAMF_APP_DESKTOP);

  app = bamf_matcher_get_application_by_desktop_file (matcher, TEST_BAMF_APP_DESKTOP);
  g_assert (app);

  app_children = bamf_view_get_children (BAMF_VIEW (app));
  g_assert_cmpuint (g_list_length (app_children), ==, window_count);

  for (xid = G_MAXUINT; xid > G_MAXUINT-window_count; xid--)
    {
      g_assert (bamf_matcher_get_application_by_xid (matcher, xid) == app);
    }

  g_object_unref (matcher);
  g_object_unref (screen);
}

static void
test_match_libreoffice_windows (void)
{
  BamfMatcher *matcher;
  BamfWindow *window;
  BamfLegacyScreen *screen;
  BamfLegacyWindowTest *test_win;
  BamfApplication *app;
  char *hint;

  screen = bamf_legacy_screen_get_default ();
  matcher = bamf_matcher_get_default ();
  guint xid = g_random_int ();
  const char *exec = "soffice.bin";
  const char *class_instance = "VCLSalFrame.DocumentWindow";

  cleanup_matcher_tables (matcher);
  export_matcher_on_bus (matcher);

  bamf_matcher_load_desktop_file (matcher, DATA_DIR"/libreoffice-startcenter.desktop");
  bamf_matcher_load_desktop_file (matcher, DATA_DIR"/libreoffice-base.desktop");
  bamf_matcher_load_desktop_file (matcher, DATA_DIR"/libreoffice-calc.desktop");
  bamf_matcher_load_desktop_file (matcher, DATA_DIR"/libreoffice-draw.desktop");
  bamf_matcher_load_desktop_file (matcher, DATA_DIR"/libreoffice-impress.desktop");
  bamf_matcher_load_desktop_file (matcher, DATA_DIR"/libreoffice-math.desktop");
  bamf_matcher_load_desktop_file (matcher, DATA_DIR"/libreoffice-writer.desktop");

  test_win = bamf_legacy_window_test_new (xid, "LibreOffice", "libreoffice-startcenter", exec);
  bamf_legacy_window_test_set_wmclass (test_win, "libreoffice-startcenter", class_instance);
  _bamf_legacy_screen_open_test_window (screen, test_win);

  hint = bamf_legacy_window_get_hint (BAMF_LEGACY_WINDOW (test_win), _NET_WM_DESKTOP_FILE);
  g_assert_cmpstr (hint, ==, DATA_DIR"/libreoffice-startcenter.desktop");
  g_free (hint);
  app = bamf_matcher_get_application_by_desktop_file (matcher, DATA_DIR"/libreoffice-startcenter.desktop");
  g_assert (find_window_in_app (app, BAMF_LEGACY_WINDOW (test_win)));

  bamf_legacy_window_test_set_wmclass (test_win, "libreoffice-base", class_instance);
  bamf_legacy_window_test_set_name (test_win, "FooDoc.odb - LibreOffice Base");
  g_assert (!bamf_matcher_get_application_by_desktop_file (matcher, DATA_DIR"/libreoffice-startcenter.desktop"));
  app = bamf_matcher_get_application_by_desktop_file (matcher, DATA_DIR"/libreoffice-base.desktop");
  g_assert (app);
  window = BAMF_WINDOW (bamf_view_get_children (BAMF_VIEW (app))->data);
  test_win = BAMF_LEGACY_WINDOW_TEST (bamf_window_get_window (window));
  hint = bamf_legacy_window_get_hint (BAMF_LEGACY_WINDOW (test_win), _NET_WM_DESKTOP_FILE);
  g_assert_cmpstr (hint, ==, DATA_DIR"/libreoffice-base.desktop");
  g_free (hint);

  bamf_legacy_window_test_set_wmclass (test_win, "libreoffice-calc", class_instance);
  bamf_legacy_window_test_set_name (test_win, "FooDoc.ods - LibreOffice Calc");
  g_assert (!bamf_matcher_get_application_by_desktop_file (matcher, DATA_DIR"/libreoffice-base.desktop"));
  app = bamf_matcher_get_application_by_desktop_file (matcher, DATA_DIR"/libreoffice-calc.desktop");
  g_assert (app);
  window = BAMF_WINDOW (bamf_view_get_children (BAMF_VIEW (app))->data);
  test_win = BAMF_LEGACY_WINDOW_TEST (bamf_window_get_window (window));
  hint = bamf_legacy_window_get_hint (BAMF_LEGACY_WINDOW (test_win), _NET_WM_DESKTOP_FILE);
  g_assert_cmpstr (hint, ==, DATA_DIR"/libreoffice-calc.desktop");
  g_free (hint);

  bamf_legacy_window_test_set_wmclass (test_win, "libreoffice-draw", class_instance);
  bamf_legacy_window_test_set_name (test_win, "FooDoc.odg - LibreOffice Draw");
  g_assert (!bamf_matcher_get_application_by_desktop_file (matcher, DATA_DIR"/libreoffice-calc.desktop"));
  app = bamf_matcher_get_application_by_desktop_file (matcher, DATA_DIR"/libreoffice-draw.desktop");
  g_assert (app);
  window = BAMF_WINDOW (bamf_view_get_children (BAMF_VIEW (app))->data);
  test_win = BAMF_LEGACY_WINDOW_TEST (bamf_window_get_window (window));
  hint = bamf_legacy_window_get_hint (BAMF_LEGACY_WINDOW (test_win), _NET_WM_DESKTOP_FILE);
  g_assert_cmpstr (hint, ==, DATA_DIR"/libreoffice-draw.desktop");
  g_free (hint);

  bamf_legacy_window_test_set_wmclass (test_win, "libreoffice-impress", class_instance);
  bamf_legacy_window_test_set_name (test_win, "FooDoc.odp - LibreOffice Impress");
  g_assert (!bamf_matcher_get_application_by_desktop_file (matcher, DATA_DIR"/libreoffice-draw.desktop"));
  app = bamf_matcher_get_application_by_desktop_file (matcher, DATA_DIR"/libreoffice-impress.desktop");
  g_assert (app);
  window = BAMF_WINDOW (bamf_view_get_children (BAMF_VIEW (app))->data);
  test_win = BAMF_LEGACY_WINDOW_TEST (bamf_window_get_window (window));
  hint = bamf_legacy_window_get_hint (BAMF_LEGACY_WINDOW (test_win), _NET_WM_DESKTOP_FILE);
  g_assert_cmpstr (hint, ==, DATA_DIR"/libreoffice-impress.desktop");
  g_free (hint);

  bamf_legacy_window_test_set_wmclass (test_win, "libreoffice-math", class_instance);
  bamf_legacy_window_test_set_name (test_win, "FooDoc.odf - LibreOffice Math");
  g_assert (!bamf_matcher_get_application_by_desktop_file (matcher, DATA_DIR"/libreoffice-impress.desktop"));
  app = bamf_matcher_get_application_by_desktop_file (matcher, DATA_DIR"/libreoffice-math.desktop");
  g_assert (app);
  window = BAMF_WINDOW (bamf_view_get_children (BAMF_VIEW (app))->data);
  test_win = BAMF_LEGACY_WINDOW_TEST (bamf_window_get_window (window));
  hint = bamf_legacy_window_get_hint (BAMF_LEGACY_WINDOW (test_win), _NET_WM_DESKTOP_FILE);
  g_assert_cmpstr (hint, ==, DATA_DIR"/libreoffice-math.desktop");
  g_free (hint);

  bamf_legacy_window_test_set_wmclass (test_win, "libreoffice-writer", class_instance);
  bamf_legacy_window_test_set_name (test_win, "FooDoc.odt - LibreOffice Writer");
  g_assert (!bamf_matcher_get_application_by_desktop_file (matcher, DATA_DIR"/libreoffice-math.desktop"));
  app = bamf_matcher_get_application_by_desktop_file (matcher, DATA_DIR"/libreoffice-writer.desktop");
  g_assert (app);
  window = BAMF_WINDOW (bamf_view_get_children (BAMF_VIEW (app))->data);
  test_win = BAMF_LEGACY_WINDOW_TEST (bamf_window_get_window (window));
  hint = bamf_legacy_window_get_hint (BAMF_LEGACY_WINDOW (test_win), _NET_WM_DESKTOP_FILE);
  g_assert_cmpstr (hint, ==, DATA_DIR"/libreoffice-writer.desktop");
  g_free (hint);

  xid = g_random_int ();
  test_win = bamf_legacy_window_test_new (xid, "BarDoc.odt - LibreOffice Writer", "libreoffice-writer", exec);
  bamf_legacy_window_test_set_wmclass (test_win, "libreoffice-writer", class_instance);
  _bamf_legacy_screen_open_test_window (screen, test_win);

  g_assert_cmpuint (g_list_length (bamf_view_get_children (BAMF_VIEW (app))), ==, 2);

  xid = g_random_int ();
  test_win = bamf_legacy_window_test_new (xid, "BarDoc.ods - LibreOffice Calc", "libreoffice-calc", exec);
  bamf_legacy_window_test_set_wmclass (test_win, "libreoffice-calc", class_instance);
  _bamf_legacy_screen_open_test_window (screen, test_win);
  g_assert (bamf_matcher_get_application_by_desktop_file (matcher, DATA_DIR"/libreoffice-calc.desktop"));

  g_object_unref (matcher);
  g_object_unref (screen);
}

static void
test_match_gnome_control_center_panels (void)
{
  BamfMatcher *matcher;
  BamfWindow *window;
  BamfLegacyScreen *screen;
  BamfLegacyWindowTest *test_win;
  BamfApplication *app;
  char *hint;

  screen = bamf_legacy_screen_get_default ();
  matcher = bamf_matcher_get_default ();
  guint xid = g_random_int ();
  const char *exec = "gnome-control-center";
  const char *class_name = "Gnome-control-center";
  const char *class_instance = "gnome-control-center";

  cleanup_matcher_tables (matcher);
  export_matcher_on_bus (matcher);

  bamf_matcher_load_desktop_file (matcher, DATA_DIR"/gnome-control-center.desktop");
  bamf_matcher_load_desktop_file (matcher, DATA_DIR"/gnome-display-panel.desktop");
  bamf_matcher_load_desktop_file (matcher, DATA_DIR"/gnome-mouse-panel.desktop");

  test_win = bamf_legacy_window_test_new (xid, "System Settings", NULL, exec);
  bamf_legacy_window_test_set_wmclass (test_win, class_name, class_instance);
  bamf_legacy_window_set_hint (BAMF_LEGACY_WINDOW (test_win), WM_WINDOW_ROLE, NULL);
  _bamf_legacy_screen_open_test_window (screen, test_win);

  hint = bamf_legacy_window_get_hint (BAMF_LEGACY_WINDOW (test_win), _NET_WM_DESKTOP_FILE);
  g_assert_cmpstr (hint, ==, DATA_DIR"/gnome-control-center.desktop");
  g_free (hint);
  app = bamf_matcher_get_application_by_desktop_file (matcher, DATA_DIR"/gnome-control-center.desktop");
  g_assert (find_window_in_app (app, BAMF_LEGACY_WINDOW (test_win)));

  bamf_legacy_window_set_hint (BAMF_LEGACY_WINDOW (test_win), WM_WINDOW_ROLE, "display");
  bamf_legacy_window_test_set_name (test_win, "Displays");
  g_assert (!bamf_matcher_get_application_by_desktop_file (matcher, DATA_DIR"/gnome-control-center.desktop"));
  app = bamf_matcher_get_application_by_desktop_file (matcher, DATA_DIR"/gnome-display-panel.desktop");
  g_assert (app);
  window = BAMF_WINDOW (bamf_view_get_children (BAMF_VIEW (app))->data);
  test_win = BAMF_LEGACY_WINDOW_TEST (bamf_window_get_window (window));
  hint = bamf_legacy_window_get_hint (BAMF_LEGACY_WINDOW (test_win), _NET_WM_DESKTOP_FILE);
  g_assert_cmpstr (hint, ==, DATA_DIR"/gnome-display-panel.desktop");
  g_free (hint);

  bamf_legacy_window_set_hint (BAMF_LEGACY_WINDOW (test_win), WM_WINDOW_ROLE, "mouse");
  bamf_legacy_window_test_set_name (test_win, "Mouse and Touchpad");
  g_assert (!bamf_matcher_get_application_by_desktop_file (matcher, DATA_DIR"/gnome-display-panel.desktop"));
  app = bamf_matcher_get_application_by_desktop_file (matcher, DATA_DIR"/gnome-mouse-panel.desktop");
  g_assert (app);
  window = BAMF_WINDOW (bamf_view_get_children (BAMF_VIEW (app))->data);
  test_win = BAMF_LEGACY_WINDOW_TEST (bamf_window_get_window (window));
  hint = bamf_legacy_window_get_hint (BAMF_LEGACY_WINDOW (test_win), _NET_WM_DESKTOP_FILE);
  g_assert_cmpstr (hint, ==, DATA_DIR"/gnome-mouse-panel.desktop");
  g_free (hint);

  bamf_legacy_window_set_hint (BAMF_LEGACY_WINDOW (test_win), WM_WINDOW_ROLE, "invalid-role");
  bamf_legacy_window_test_set_name (test_win, "Invalid Panel");
  g_assert (!bamf_matcher_get_application_by_desktop_file (matcher, DATA_DIR"/gnome-mouse-panel.desktop"));
  app = bamf_matcher_get_application_by_desktop_file (matcher, DATA_DIR"/gnome-control-center.desktop");
  g_assert (app);
  window = BAMF_WINDOW (bamf_view_get_children (BAMF_VIEW (app))->data);
  test_win = BAMF_LEGACY_WINDOW_TEST (bamf_window_get_window (window));
  hint = bamf_legacy_window_get_hint (BAMF_LEGACY_WINDOW (test_win), _NET_WM_DESKTOP_FILE);
  g_assert_cmpstr (hint, ==, DATA_DIR"/gnome-control-center.desktop");
  g_free (hint);

  g_object_unref (matcher);
  g_object_unref (screen);
}

static void
test_match_javaws_windows (void)
{
  BamfMatcher *matcher;
  BamfLegacyScreen *screen;
  BamfLegacyWindowTest *test_win;
  BamfApplication *app1, *app2, *app3;
  GList *app_children;

  screen = bamf_legacy_screen_get_default ();
  matcher = bamf_matcher_get_default ();
  const char *exec_prefix = "/usr/lib/jvm/java-6-openjdk-amd64/jre/bin/javaws " \
                            "-Xbootclasspath/a:/usr/share/icedtea-web/netx.jar " \
                            "-Xms8m -Djava.security.manager " \
                            "-Djava.security.policy=/etc/icedtea-web/javaws.policy " \
                            "-classpath /usr/lib/jvm/java-6-openjdk-amd64/jre/lib/rt.jar " \
                            "-Dicedtea-web.bin.name=javaws " \
                            "-Dicedtea-web.bin.location=/usr/bin/javaws "\
                            "net.sourceforge.jnlp.runtime.Boot";
  const char *class_name = "net-sourceforge-jnlp-runtime-Boot";
  const char *class_instance = "sun-awt-X11-XFramePeer";

  cleanup_matcher_tables (matcher);
  export_matcher_on_bus (matcher);

  guint xid = g_random_int ();
  char *exec = g_strconcat (exec_prefix, " Notepad.jnlp", NULL);
  test_win = bamf_legacy_window_test_new (xid, "Notepad", NULL, exec);
  bamf_legacy_window_test_set_wmclass (test_win, class_name, class_instance);
  _bamf_legacy_screen_open_test_window (screen, test_win);
  g_free (exec);
  app1 = bamf_matcher_get_application_by_xid (matcher, xid);
  g_assert (BAMF_IS_APPLICATION (app1));
  app_children = bamf_view_get_children (BAMF_VIEW (app1));
  g_assert_cmpuint (g_list_length (app_children), ==, 1);
  g_assert (find_window_in_app (app1, BAMF_LEGACY_WINDOW (test_win)));

  xid = g_random_int ();
  exec = g_strconcat (exec_prefix, " Draw.jnlp", NULL);
  test_win = bamf_legacy_window_test_new (xid, "Draw", NULL, exec);
  bamf_legacy_window_test_set_wmclass (test_win, class_name, class_instance);
  _bamf_legacy_screen_open_test_window (screen, test_win);
  g_free (exec);
  app2 = bamf_matcher_get_application_by_xid (matcher, xid);
  g_assert (BAMF_IS_APPLICATION (app2));
  g_assert (app1 != app2);
  app_children = bamf_view_get_children (BAMF_VIEW (app2));
  g_assert_cmpuint (g_list_length (app_children), ==, 1);
  g_assert (find_window_in_app (app2, BAMF_LEGACY_WINDOW (test_win)));

  xid = g_random_int ();
  exec = g_strconcat (exec_prefix, " Notepad.jnlp", NULL);
  test_win = bamf_legacy_window_test_new (xid, "Notepad Subwin", NULL, exec);
  bamf_legacy_window_test_set_wmclass (test_win, class_name, class_instance);
  _bamf_legacy_screen_open_test_window (screen, test_win);
  g_free (exec);
  app3 = bamf_matcher_get_application_by_xid (matcher, xid);
  g_assert (app3 == app1);
  g_assert (BAMF_IS_APPLICATION (app3));
  app_children = bamf_view_get_children (BAMF_VIEW (app3));
  g_assert_cmpuint (g_list_length (app_children), ==, 2);
  g_assert (find_window_in_app (app3, BAMF_LEGACY_WINDOW (test_win)));

  g_object_unref (matcher);
  g_object_unref (screen);
}

static void
test_match_javaws_windows_hint_ignored (void)
{
  BamfMatcher *matcher;
  BamfLegacyScreen *screen;
  BamfLegacyWindowTest *test_win;

  screen = bamf_legacy_screen_get_default ();
  matcher = bamf_matcher_get_default ();
  const char *exec_prefix = "/usr/lib/jvm/java-6-openjdk-amd64/jre/bin/javaws " \
                            "-Xbootclasspath/a:/usr/share/icedtea-web/netx.jar " \
                            "-Xms8m -Djava.security.manager " \
                            "-Djava.security.policy=/etc/icedtea-web/javaws.policy " \
                            "-classpath /usr/lib/jvm/java-6-openjdk-amd64/jre/lib/rt.jar " \
                            "-Dicedtea-web.bin.name=javaws " \
                            "-Dicedtea-web.bin.location=/usr/bin/javaws "\
                            "net.sourceforge.jnlp.runtime.Boot ";
  const char *class_name = "net-sourceforge-jnlp-runtime-Boot";
  const char *class_instance = "sun-awt-X11-XFramePeer";

  cleanup_matcher_tables (matcher);
  export_matcher_on_bus (matcher);

  guint xid = g_random_int ();
  guint pid = g_random_int ();
  char *exec = g_strconcat (exec_prefix, "Notepad.jnlp", NULL);
  test_win = bamf_legacy_window_test_new (xid, "Notepad", NULL, exec);
  bamf_legacy_window_test_set_wmclass (test_win, class_name, class_instance);
  test_win->pid = pid;
  _bamf_legacy_screen_open_test_window (screen, test_win);
  g_free (exec);

  bamf_matcher_register_desktop_file_for_pid (matcher, DATA_DIR"/icedtea-netx-javaws.desktop", pid);

  char *hint = bamf_legacy_window_get_hint (BAMF_LEGACY_WINDOW (test_win), _NET_WM_DESKTOP_FILE);
  g_assert (hint == NULL);

  g_object_unref (matcher);
  g_object_unref (screen);
}

static void
test_trim_exec_string (void)
{
  BamfMatcher *matcher;
  char *trimmed;

  matcher = bamf_matcher_get_default ();

  // Bad prefixes
  trimmed = bamf_matcher_get_trimmed_exec (matcher, "gksudo bad-prefix-bin");
  g_assert_cmpstr (trimmed, ==, "bad-prefix-bin");
  g_free (trimmed);

  trimmed = bamf_matcher_get_trimmed_exec (matcher, "sudo --opt val=X /usr/bin/bad-prefix-bin");
  g_assert_cmpstr (trimmed, ==, "bad-prefix-bin");
  g_free (trimmed);

  trimmed = bamf_matcher_get_trimmed_exec (matcher, "python2.7 /home/foo/bad-prefix-script.py");
  g_assert_cmpstr (trimmed, ==, "bad-prefix-script");
  g_free (trimmed);

  trimmed = bamf_matcher_get_trimmed_exec (matcher, "/usr/bin/python3.1");
  g_assert_cmpstr (trimmed, ==, "python3.1");
  g_free (trimmed);

  trimmed = bamf_matcher_get_trimmed_exec (matcher, "/usr/bin/python %u --option val=/path");
  g_assert_cmpstr (trimmed, ==, "python");
  g_free (trimmed);

  trimmed = bamf_matcher_get_trimmed_exec (matcher, "sh -c \"binary --option --value %U || exec binary\"");
  g_assert_cmpstr (trimmed, ==, "binary");
  g_free (trimmed);

  // Good prefixes
  trimmed = bamf_matcher_get_trimmed_exec (matcher, "/usr/bin/libreoffice --writer %U");
  g_assert_cmpstr (trimmed, ==, "libreoffice --writer");
  g_free (trimmed);

  trimmed = bamf_matcher_get_trimmed_exec (matcher, "/usr/bin/gnome-control-center");
  g_assert_cmpstr (trimmed, ==, "gnome-control-center");
  g_free (trimmed);

  trimmed = bamf_matcher_get_trimmed_exec (matcher, "gnome-control-center foo-panel");
  g_assert_cmpstr (trimmed, ==, "gnome-control-center foo-panel");
  g_free (trimmed);

  // Other exec strings
  trimmed = bamf_matcher_get_trimmed_exec (matcher, "env FOOVAR=\"bar\" myprog");
  g_assert_cmpstr (trimmed, ==, "myprog");
  g_free (trimmed);

  trimmed = bamf_matcher_get_trimmed_exec (matcher, "/opt/path/bin/myprog --option %U --foo=daa");
  g_assert_cmpstr (trimmed, ==, "myprog");
  g_free (trimmed);

  const char *exec = "/usr/lib/jvm/java-6-openjdk-amd64/jre/bin/java " \
                     "-Xbootclasspath/a:/usr/share/icedtea-web/netx.jar " \
                     "-Xms8m -Djava.security.manager " \
                     "-Djava.security.policy=/etc/icedtea-web/javaws.policy " \
                     "-classpath /usr/lib/jvm/java-6-openjdk-amd64/jre/lib/rt.jar " \
                     "-Dicedtea-web.bin.name=javaws " \
                     "-Dicedtea-web.bin.location=/usr/bin/javaws "\
                     "net.sourceforge.jnlp.runtime.Boot Notepad.jnlp";
  trimmed = bamf_matcher_get_trimmed_exec (matcher, exec);
  g_assert_cmpstr (trimmed, ==, "notepad.jnlp");
  g_free (trimmed);

  g_object_unref (matcher);
}
