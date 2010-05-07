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

#include <stdlib.h>
#include <glib.h>
#include <glib-object.h>
#include "bamf-application.h"
#include "bamf-window.h"
#include "bamf-legacy-window.h"

#define BAMF_TYPE_LEGACY_WINDOW_TEST (bamf_legacy_window_test_get_type ())

#define BAMF_LEGACY_WINDOW_TEST(obj) (G_TYPE_CHECK_INSTANCE_CAST ((obj),\
	BAMF_TYPE_LEGACY_WINDOW_TEST, BamfLegacyWindowTest))

#define BAMF_LEGACY_WINDOW_TEST_CLASS(klass) (G_TYPE_CHECK_CLASS_CAST ((klass),\
	BAMF_TYPE_LEGACY_WINDOW_TEST, BamfLegacyWindowTestClass))

#define BAMF_IS_LEGACY_WINDOW_TEST(obj) (G_TYPE_CHECK_INSTANCE_TYPE ((obj),\
	BAMF_TYPE_LEGACY_WINDOW_TEST))

#define BAMF_IS_LEGACY_WINDOW_TEST_CLASS(klass) (G_TYPE_CHECK_CLASS_TYPE ((klass),\
	BAMF_TYPE_LEGACY_WINDOW_TEST))

#define BAMF_LEGACY_WINDOW_TEST_GET_CLASS(obj) (G_TYPE_INSTANCE_GET_CLASS ((obj),\
	BAMF_TYPE_LEGACY_WINDOW_TEST, BamfLegacyWindowTestClass))

typedef struct _BamfLegacyWindowTest        BamfLegacyWindowTest;
typedef struct _BamfLegacyWindowTestClass   BamfLegacyWindowTestClass;
typedef struct _BamfLegacyWindowTestPrivate BamfLegacyWindowTestPrivate;

struct _BamfLegacyWindowTest
{
  BamfLegacyWindow parent;
  guint32 xid;
  gint    pid;
  char * name;
  gboolean needs_attention;
  gboolean is_desktop;
  gboolean is_skip;
};

struct _BamfLegacyWindowTestClass
{
  /*< private >*/
  BamfLegacyWindowClass parent_class;
  
  void (*_test_padding1) (void);
  void (*_test_padding2) (void);
  void (*_test_padding3) (void);
  void (*_test_padding4) (void);
  void (*_test_padding5) (void);
  void (*_test_padding6) (void);
};

GType       bamf_legacy_window_test_get_type (void) G_GNUC_CONST;

G_DEFINE_TYPE (BamfLegacyWindowTest, bamf_legacy_window_test, BAMF_TYPE_LEGACY_WINDOW);

static gint
bamf_legacy_window_test_get_pid (BamfLegacyWindow *legacy_window)
{
  BamfLegacyWindowTest *self;
  
  self = BAMF_LEGACY_WINDOW_TEST (legacy_window);
  
  return  self->pid;
}

static guint32
bamf_legacy_window_test_get_xid (BamfLegacyWindow *legacy_window)
{
  BamfLegacyWindowTest *self;

  self = BAMF_LEGACY_WINDOW_TEST (legacy_window);

  return self->xid;
}

void
bamf_legacy_window_test_set_attention (BamfLegacyWindowTest *self, gboolean val)
{
  if (self->needs_attention == val)
    return;
    
  self->needs_attention = val;

  g_signal_emit_by_name (self, "state-changed");
}

static gboolean
bamf_legacy_window_test_needs_attention (BamfLegacyWindow *legacy_window)
{
  BamfLegacyWindowTest *self;

  self = BAMF_LEGACY_WINDOW_TEST (legacy_window);

  return self->needs_attention;
}

void
bamf_legacy_window_test_set_desktop (BamfLegacyWindowTest *self, gboolean val)
{
  if (self->is_desktop == val)
    return;
    
  self->is_desktop = val;

  g_signal_emit_by_name (self, "state-changed");
}

static gboolean
bamf_legacy_window_test_is_desktop (BamfLegacyWindow *legacy_window)
{
  BamfLegacyWindowTest *self;

  self = BAMF_LEGACY_WINDOW_TEST (legacy_window);

  return self->is_desktop;
}

void
bamf_legacy_window_test_set_skip (BamfLegacyWindowTest *self, gboolean val)
{
  if (self->is_skip == val)
    return;
    
  self->is_skip = val;

  g_signal_emit_by_name (self, "state-changed");
}

static gboolean
bamf_legacy_window_test_is_skip_tasklist (BamfLegacyWindow *legacy_window)
{
  BamfLegacyWindowTest *self;

  self = BAMF_LEGACY_WINDOW_TEST (legacy_window);

  return self->is_skip;
}

void
bamf_legacy_window_test_set_name (BamfLegacyWindowTest *self, char *val)
{
  if (g_strcmp0 (self->name, val) == 0)
    return;
    
  self->name = val;

  g_signal_emit_by_name (self, "name-changed");
}

static char *
bamf_legacy_window_test_get_name (BamfLegacyWindow *legacy_window)
{
  BamfLegacyWindowTest *self;
  
  self = BAMF_LEGACY_WINDOW_TEST (legacy_window);
  
  return  self->name;
}

static void
bamf_legacy_window_test_class_init (BamfLegacyWindowTestClass *klass)
{
  BamfLegacyWindowClass *win_class = BAMF_LEGACY_WINDOW_CLASS (klass);

  win_class->get_name         = bamf_legacy_window_test_get_name;
  win_class->get_xid          = bamf_legacy_window_test_get_xid;
  win_class->get_pid          = bamf_legacy_window_test_get_pid;
  win_class->needs_attention  = bamf_legacy_window_test_needs_attention;
  win_class->is_skip_tasklist = bamf_legacy_window_test_is_skip_tasklist;
  win_class->is_desktop       = bamf_legacy_window_test_is_desktop;
}


static void
bamf_legacy_window_test_init (BamfLegacyWindowTest *self)
{
  self->pid = g_random_int_range (1, 100000);
}


static BamfLegacyWindowTest *
bamf_legacy_window_test_new (guint32 xid, gchar *name)
{
  BamfLegacyWindowTest *self;

  self = g_object_new (BAMF_TYPE_LEGACY_WINDOW_TEST, NULL);
  self->xid = xid;
  self->name = name;

  return self;
}






#define DESKTOP_FILE "usr/share/applications/gnome-terminal.desktop"

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
  
  test1 = bamf_legacy_window_test_new (20, "Window X");
  test2 = bamf_legacy_window_test_new (20, "Window Y");
  
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
  window1 = bamf_window_new (BAMF_LEGACY_WINDOW (bamf_legacy_window_test_new (25, "window1")));
  window2 = bamf_window_new (BAMF_LEGACY_WINDOW (bamf_legacy_window_test_new (50, "window2")));

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
  test = bamf_window_new (BAMF_LEGACY_WINDOW (bamf_legacy_window_test_new (20, "window")));

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
  
  test1 = bamf_legacy_window_test_new (20, "Window X");
  test2 = bamf_legacy_window_test_new (20, "Window Y");
  
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
