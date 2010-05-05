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
 * Authored by Neil Jagdish Patel <neil.patel@canonical.com>
 *
 */

#include <glib.h>
#include <stdlib.h>
#include "bamf-application.h"

#define DESKTOP_FILE "usr/share/applications/gnome-terminal.desktop"

static void test_allocation          (void);
static void test_desktop_file        (void);
static void test_urgent              (void);
static void test_urgent_event        (void);

void
test_application_create_suite (void)
{
#define DOMAIN "/Application"

  g_test_add_func (DOMAIN"/Allocation", test_allocation);
  g_test_add_func (DOMAIN"/DesktopFile", test_desktop_file);
  g_test_add_func (DOMAIN"/Urgent", test_urgent);
  g_test_add_func (DOMAIN"/Urgent/ChangedEvent", test_urgent_event);
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
test_urgent (void)
{
  BamfApplication *application;

  application = g_object_new (BAMF_TYPE_APPLICATION, NULL);
  g_assert (!bamf_application_is_urgent (application));

  bamf_application_set_urgent (application, TRUE);
  g_assert (bamf_application_is_urgent (application));

  bamf_application_set_urgent (application, FALSE);
  g_assert (!bamf_application_is_urgent (application));

  g_object_unref (application);
  g_assert (!BAMF_IS_APPLICATION (application));
}

static gboolean urgent_event_fired;
static gboolean urgent_event_result;

static void
on_urgent_event (BamfApplication *application, gboolean urgent, gpointer pointer)
{
  urgent_event_fired = TRUE;
  urgent_event_result = urgent;
}

static void
test_urgent_event (void)
{
  BamfApplication *application;

  application = g_object_new (BAMF_TYPE_APPLICATION, NULL);
  g_assert (!bamf_application_is_urgent (application));

  g_signal_connect (G_OBJECT (application), "urgent-changed",
		    (GCallback) on_urgent_event, NULL);

  urgent_event_fired = FALSE;
  bamf_application_set_urgent (application, TRUE);
  g_assert (bamf_application_is_urgent (application));

  g_assert (urgent_event_fired);
  g_assert (urgent_event_result);

  urgent_event_fired = FALSE;
  bamf_application_set_urgent (application, FALSE);
  g_assert (!bamf_application_is_urgent (application));

  g_assert (urgent_event_fired);
  g_assert (!urgent_event_result);

  g_object_unref (application);
  g_assert (!BAMF_IS_APPLICATION (application));
}
