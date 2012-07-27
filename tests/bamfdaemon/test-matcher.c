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

static void test_allocation (void);
static void test_load_desktop_file (void);

#define DATA_DIR "bamfdaemon/data"
#define TEST_BAMF_APP_DESKTOP TESTDIR "/" DATA_DIR "/test-bamf-app.desktop"

void
test_matcher_create_suite (void)
{
#define DOMAIN "/Matcher"

  g_test_add_func (DOMAIN"/Allocation", test_allocation);
  g_test_add_func (DOMAIN"/LoadDesktopFile", test_load_desktop_file);
}

static void
test_allocation (void)
{
  BamfMatcher *matcher;

  /* Check it allocates */
  matcher = bamf_matcher_get_default ();
  g_assert (BAMF_IS_MATCHER (matcher));
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