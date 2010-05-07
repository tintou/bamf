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

#include "test-mock-window.h"

G_DEFINE_TYPE (BamfLegacyWindowTest, bamf_legacy_window_test, BAMF_TYPE_LEGACY_WINDOW);

gint
bamf_legacy_window_test_get_pid (BamfLegacyWindow *legacy_window)
{
  BamfLegacyWindowTest *self;
  
  self = BAMF_LEGACY_WINDOW_TEST (legacy_window);
  
  return  self->pid;
}

guint32
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

gboolean
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

gboolean
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

gboolean
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

char *
bamf_legacy_window_test_get_name (BamfLegacyWindow *legacy_window)
{
  BamfLegacyWindowTest *self;
  
  self = BAMF_LEGACY_WINDOW_TEST (legacy_window);
  
  return  self->name;
}

void
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


void
bamf_legacy_window_test_init (BamfLegacyWindowTest *self)
{
  self->pid = g_random_int_range (1, 100000);
}


BamfLegacyWindowTest *
bamf_legacy_window_test_new (guint32 xid, gchar *name)
{
  BamfLegacyWindowTest *self;

  self = g_object_new (BAMF_TYPE_LEGACY_WINDOW_TEST, NULL);
  self->xid = xid;
  self->name = name;

  return self;
}
