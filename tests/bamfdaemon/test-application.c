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

#define BAMF_TYPE_WINDOW_TEST (bamf_window_test_get_type ())

#define BAMF_WINDOW_TEST(obj) (G_TYPE_CHECK_INSTANCE_CAST ((obj),\
	BAMF_TYPE_WINDOW_TEST, BamfWindowTest))

#define BAMF_WINDOW_TEST_CLASS(klass) (G_TYPE_CHECK_CLASS_CAST ((klass),\
	BAMF_TYPE_WINDOW_TEST, BamfWindowTestClass))

#define BAMF_IS_WINDOW_TEST(obj) (G_TYPE_CHECK_INSTANCE_TYPE ((obj),\
	BAMF_TYPE_WINDOW_TEST))

#define BAMF_IS_WINDOW_TEST_CLASS(klass) (G_TYPE_CHECK_CLASS_TYPE ((klass),\
	BAMF_TYPE_WINDOW_TEST))

#define BAMF_WINDOW_TEST_GET_CLASS(obj) (G_TYPE_INSTANCE_GET_CLASS ((obj),\
	BAMF_TYPE_WINDOW_TEST, BamfWindowTestClass))

typedef struct _BamfWindowTest        BamfWindowTest;
typedef struct _BamfWindowTestClass   BamfWindowTestClass;
typedef struct _BamfWindowTestPrivate BamfWindowTestPrivate;

struct _BamfWindowTest
{
  BamfWindow parent;
  guint32 xid;
};

struct _BamfWindowTestClass
{
  /*< private >*/
  BamfWindowClass parent_class;
  
  void (*_test_padding1) (void);
  void (*_test_padding2) (void);
  void (*_test_padding3) (void);
  void (*_test_padding4) (void);
  void (*_test_padding5) (void);
  void (*_test_padding6) (void);
};

GType       bamf_window_test_get_type (void) G_GNUC_CONST;

G_DEFINE_TYPE (BamfWindowTest, bamf_window_test, BAMF_TYPE_WINDOW);


static guint32
bamf_window_test_get_xid (BamfWindow *window)
{
  BamfWindowTest *self;

  self = BAMF_WINDOW_TEST (window);

  return self->xid;
}

static void
bamf_window_test_class_init (BamfWindowTestClass *klass)
{
  BamfWindowClass *win_class = BAMF_WINDOW_CLASS (klass);

  win_class->get_xid = bamf_window_test_get_xid;
}


static void
bamf_window_test_init (BamfWindowTest *self)
{
  ;
}


static BamfWindowTest *
bamf_window_test_new (guint32 xid)
{
  BamfWindowTest *self;

  self = g_object_new (BAMF_TYPE_WINDOW_TEST, NULL);
  self->xid = xid;

  return self;
}






#define DESKTOP_FILE "usr/share/applications/gnome-terminal.desktop"

static void test_allocation          (void);
static void test_desktop_file        (void);
static void test_urgent              (void);
static void test_urgent_event        (void);
static void test_get_xids            (void);
static void test_manages_xid         (void);

void
test_application_create_suite (void)
{
#define DOMAIN "/Application"

  g_test_add_func (DOMAIN"/Allocation", test_allocation);
  g_test_add_func (DOMAIN"/DesktopFile", test_desktop_file);
  g_test_add_func (DOMAIN"/Urgent", test_urgent);
  g_test_add_func (DOMAIN"/Urgent/ChangedEvent", test_urgent_event);
  g_test_add_func (DOMAIN"/Xids", test_get_xids);
  g_test_add_func (DOMAIN"/ManagesXid", test_manages_xid);
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

  application = bamf_application_new ();
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

  application = bamf_application_new ();
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

static void
test_get_xids (void)
{
  BamfApplication *application;
  BamfWindowTest *window1, *window2;
  GArray *xids;
  gboolean found;
  guint32 xid;
  int i;

  application = bamf_application_new ();
  window1 = bamf_window_test_new (25);
  window2 = bamf_window_test_new (50);

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
  BamfWindowTest *test;

  application = bamf_application_new ();
  test = bamf_window_test_new (20);

  bamf_view_add_child (BAMF_VIEW (application), BAMF_VIEW (test));

  g_assert (bamf_application_manages_xid (application, 20));

  g_object_unref (test);
  g_object_unref (application);
}
