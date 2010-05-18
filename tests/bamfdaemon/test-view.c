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
#include "bamf-view.h"

static void test_active              (void);
static void test_active_event        (void);
static void test_allocation          (void);
static void test_child_added_event   (void);
static void test_child_removed_event (void);
static void test_children            (void);
static void test_children_paths      (void);
static void test_closed_event        (void);
static void test_name                (void);
static void test_path                (void);
static void test_path_collision      (void);
static void test_running             (void);
static void test_running_event       (void);
static void test_parent_child_out_of_order_unref (void);

void
test_view_create_suite (void)
{
#define DOMAIN "/View"

  g_test_add_func (DOMAIN"/Allocation", test_allocation);
  g_test_add_func (DOMAIN"/Name", test_name);
  g_test_add_func (DOMAIN"/Active", test_active);
  g_test_add_func (DOMAIN"/Running", test_running);
  g_test_add_func (DOMAIN"/Path", test_path);
  g_test_add_func (DOMAIN"/Path/Collision", test_path_collision);
  g_test_add_func (DOMAIN"/Events/Close", test_closed_event);
  g_test_add_func (DOMAIN"/Events/Active", test_active_event);
  g_test_add_func (DOMAIN"/Events/Running", test_running_event);
  g_test_add_func (DOMAIN"/Events/ChildAdded", test_child_added_event);
  g_test_add_func (DOMAIN"/Events/ChildRemoved", test_child_removed_event);
  g_test_add_func (DOMAIN"/Children", test_children);
  g_test_add_func (DOMAIN"/Children/Paths", test_children_paths);
  g_test_add_func (DOMAIN"/Children/UnrefOrder", test_parent_child_out_of_order_unref);
}

static void
test_allocation (void)
{
  BamfView    *view;

  view = g_object_new (BAMF_TYPE_VIEW, NULL);
  g_assert (BAMF_IS_VIEW (view));

  g_object_unref (G_OBJECT (view));
  g_assert (!BAMF_IS_VIEW (view));
}

static void
test_name (void)
{
  BamfView *view;

  view = g_object_new (BAMF_TYPE_VIEW, NULL);

  g_assert (bamf_view_get_name (view) == NULL);

  bamf_view_set_name (view, "SomeName");

  g_assert (g_strcmp0 (bamf_view_get_name (view), "SomeName") == 0);

  g_object_unref (view);
  g_assert (!BAMF_IS_VIEW (view));
}

static void
test_active (void)
{
  BamfView *view;

  view = g_object_new (BAMF_TYPE_VIEW, NULL);
  g_assert (!bamf_view_is_active (view));

  bamf_view_set_active (view, TRUE);
  g_assert (bamf_view_is_active (view));

  bamf_view_set_active (view, FALSE);
  g_assert (!bamf_view_is_active (view));

  g_object_unref (view);
  g_assert (!BAMF_IS_VIEW (view));
}

static void
test_running (void)
{
  BamfView *view;

  view = g_object_new (BAMF_TYPE_VIEW, NULL);
  g_assert (!bamf_view_is_running (view));

  bamf_view_set_running (view, TRUE);
  g_assert (bamf_view_is_running (view));

  bamf_view_set_running (view, FALSE);
  g_assert (!bamf_view_is_running (view));

  g_object_unref (view);
  g_assert (!BAMF_IS_VIEW (view));
}

static void
test_path (void)
{
  BamfView *view;
  char *path;

  view = g_object_new (BAMF_TYPE_VIEW, NULL);
  g_assert (bamf_view_get_path (view) == NULL);

  path = bamf_view_export_on_bus (view);
  g_assert (path);
  g_assert (g_strcmp0 (path, bamf_view_get_path (view)) == 0);

  g_object_unref (view);
  g_assert (!BAMF_IS_VIEW (view));
}

static void
test_path_collision (void)
{
  BamfView *view;
  GList *views, *l;
  int i, j;
  
  for (i = 0; i < 20; i++)
    {
      for (j = 0; j < 2000; j++)
        {
          view = g_object_new (BAMF_TYPE_VIEW, NULL);
          g_assert (BAMF_IS_VIEW (view));

          views = g_list_prepend (views, view);

          bamf_view_export_on_bus (view);
        }

      for (l = views; l; l = l->next)
        {
          g_object_unref (l->data);
        }

      g_list_free (views);
      views = NULL;
    }
}

static void
test_children (void)
{
  BamfView *parent;
  BamfView *child1, *child2, *child3;

  parent = g_object_new (BAMF_TYPE_VIEW, NULL);
  child1 = g_object_new (BAMF_TYPE_VIEW, NULL);
  child2 = g_object_new (BAMF_TYPE_VIEW, NULL);
  child3 = g_object_new (BAMF_TYPE_VIEW, NULL);

  g_assert (bamf_view_get_children (parent) == NULL);

  bamf_view_add_child (parent, child1);
  g_assert (g_list_length (bamf_view_get_children (parent)) == 1);
  g_assert (g_list_nth_data (bamf_view_get_children (parent), 0) == child1);

  bamf_view_add_child (parent, child2);
  bamf_view_add_child (parent, child3);
  g_assert (g_list_length (bamf_view_get_children (parent)) == 3);

  g_object_unref (child1);
  g_assert (g_list_length (bamf_view_get_children (parent)) == 2);

  g_object_unref (child2);
  g_assert (g_list_length (bamf_view_get_children (parent)) == 1);

  g_object_unref (child3);
  g_assert (g_list_length (bamf_view_get_children (parent)) == 0);

  g_object_unref (parent);
}

static void
test_children_paths (void)
{
  BamfView *parent;
  BamfView *child1, *child2, *child3;
  char **paths;
  char *path;
  int i; 
  guint length;
  gboolean found;

  parent = g_object_new (BAMF_TYPE_VIEW, NULL);
  child1 = g_object_new (BAMF_TYPE_VIEW, NULL);
  child2 = g_object_new (BAMF_TYPE_VIEW, NULL);
  child3 = g_object_new (BAMF_TYPE_VIEW, NULL);

  bamf_view_export_on_bus (parent);
  bamf_view_export_on_bus (child1);
  bamf_view_export_on_bus (child2);

  g_assert (bamf_view_get_children (parent) == NULL);

  bamf_view_add_child (parent, child1);
  bamf_view_add_child (parent, child2);
  bamf_view_add_child (parent, child3);
  g_assert (g_list_length (bamf_view_get_children (parent)) == 3);

  
  paths = bamf_view_get_children_paths (parent);
  length = g_strv_length (paths);
  g_assert (length == 2);

  g_strfreev (paths);
  bamf_view_export_on_bus (child3);

  paths = bamf_view_get_children_paths (parent);
  length = g_strv_length (paths);
  g_assert (length == 3);

  found = FALSE;
  for (i = 0; i < length; i++)
    {
      path = paths[i];
      if (g_strcmp0 (path, bamf_view_get_path (child1)) == 0)
        {
          found = TRUE;
          break;
        }      
    }

  g_assert (found);

  found = FALSE;
  for (i = 0; i < length; i++)
    {
      path = paths[i];
      if (g_strcmp0 (path, bamf_view_get_path (child2)) == 0)
        {
          found = TRUE;
          break;
        }      
    }

  g_assert (found);

  found = FALSE;
  for (i = 0; i < length; i++)
    {
      path = paths[i];
      if (g_strcmp0 (path, bamf_view_get_path (child3)) == 0)
        {
          found = TRUE;
          break;
        }      
    }

  g_assert (found);

  g_object_unref (child1);
  g_object_unref (child2);
  g_object_unref (child3);
  g_object_unref (parent);
}

static gboolean active_event_fired;
static gboolean active_event_result;

static void
on_active_event (BamfView *view, gboolean active, gpointer pointer)
{
  active_event_fired = TRUE;
  active_event_result = active;
}

static void
test_active_event (void)
{
  BamfView *view;

  view = g_object_new (BAMF_TYPE_VIEW, NULL);
  g_assert (!bamf_view_is_active (view));

  g_signal_connect (G_OBJECT (view), "active-changed",
		    (GCallback) on_active_event, NULL);

  active_event_fired = FALSE;
  bamf_view_set_active (view, TRUE);
  g_assert (bamf_view_is_active (view));

  g_assert (active_event_fired);
  g_assert (active_event_result);

  active_event_fired = FALSE;
  bamf_view_set_active (view, FALSE);
  g_assert (!bamf_view_is_active (view));

  g_assert (active_event_fired);
  g_assert (!active_event_result);

  g_object_unref (view);
  g_assert (!BAMF_IS_VIEW (view));
}

static gboolean running_event_fired;
static gboolean running_event_result;

static void
on_running_event (BamfView *view, gboolean running, gpointer pointer)
{
  running_event_fired = TRUE;
  running_event_result = running;
}

static void
test_running_event (void)
{
  BamfView *view;

  view = g_object_new (BAMF_TYPE_VIEW, NULL);
  g_assert (!bamf_view_is_running (view));

  g_signal_connect (G_OBJECT (view), "running-changed",
		    (GCallback) on_running_event, NULL);

  running_event_fired = FALSE;
  bamf_view_set_running (view, TRUE);
  g_assert (bamf_view_is_running (view));

  g_assert (running_event_fired);
  g_assert (running_event_result);

  running_event_fired = FALSE;
  bamf_view_set_running (view, FALSE);
  g_assert (!bamf_view_is_running (view));

  g_assert (running_event_fired);
  g_assert (!running_event_result);

  g_object_unref (view);
  g_assert (!BAMF_IS_VIEW (view));
}

static gboolean child_added_event_fired;
static char * child_added_event_result;

static void
on_child_added (BamfView *view, char *path, gpointer pointer)
{
  child_added_event_fired = TRUE;
  child_added_event_result = g_strdup (path);
}

static void
test_child_added_event (void)
{
  BamfView *parent;
  BamfView *child;

  parent = g_object_new (BAMF_TYPE_VIEW, NULL);
  child = g_object_new (BAMF_TYPE_VIEW, NULL);

  bamf_view_export_on_bus (parent);
  bamf_view_export_on_bus (child);

  g_signal_connect (G_OBJECT (parent), "child-added",
                    (GCallback) on_child_added, NULL);

  child_added_event_fired = FALSE;
  bamf_view_add_child (parent, child);

  g_assert (child_added_event_fired);
  g_assert (g_strcmp0 (bamf_view_get_path (child), child_added_event_result) == 0);

  g_object_unref (child);
  g_object_unref (parent);
}

static gboolean child_removed_event_fired;
static char * child_removed_event_result;

static void
on_child_removed (BamfView *view, char *path, gpointer pointer)
{
  child_removed_event_fired = TRUE;
  child_removed_event_result = g_strdup (path);
}

static void
test_child_removed_event (void)
{
  BamfView *parent;
  BamfView *child;

  parent = g_object_new (BAMF_TYPE_VIEW, NULL);
  child = g_object_new (BAMF_TYPE_VIEW, NULL);

  bamf_view_export_on_bus (parent);
  bamf_view_export_on_bus (child);
  bamf_view_add_child (parent, child);

  g_signal_connect (G_OBJECT (parent), "child-removed",
                    (GCallback) on_child_removed, NULL);

  child_removed_event_fired = FALSE;
  bamf_view_remove_child (parent, child);

  g_assert (child_removed_event_fired);
  g_assert (g_strcmp0 (bamf_view_get_path (child), child_removed_event_result) == 0);

  g_object_unref (child);
  g_object_unref (parent);
}

static gboolean closed_event_fired;

static void
on_closed (BamfView *view, gpointer pointer)
{
  closed_event_fired = TRUE;
}

static void
test_closed_event (void)
{
  BamfView *view;

  view = g_object_new (BAMF_TYPE_VIEW, NULL);
  bamf_view_export_on_bus (view);

  g_signal_connect (G_OBJECT (view), "closed",
                    (GCallback) on_closed, NULL);

  closed_event_fired = FALSE;

  g_object_unref (view);

  g_assert (closed_event_fired);
}

static void
test_parent_child_out_of_order_unref (void)
{
  BamfView *parent, *child;

  parent = g_object_new (BAMF_TYPE_VIEW, NULL);
  child = g_object_new (BAMF_TYPE_VIEW, NULL);

  bamf_view_export_on_bus (parent);
  bamf_view_export_on_bus (child);

  bamf_view_add_child (parent, child);

  g_object_unref (parent);
  g_object_unref (child);
}
