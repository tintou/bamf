/*
 *
 * Copyright (C) 2009 - Canonical Ltd.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 3 of the License
 *
 * Bamf is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin St, Fifth Floor,
 * Boston, MA  02110-1301  USA
 */

#include <stdio.h>
#include <gtk/gtk.h>
#include <gdk/gdk.h>
#include <glib.h>
#include <libbamf/libbamf.h>

int main (int argc, char **argv)
{
  BamfMatcher *matcher;
  BamfControl *control;
  BamfTabSource *source;
  
  gtk_init (&argc, &argv);

  matcher = bamf_matcher_get_default ();
  control = bamf_control_get_default ();
  source = g_object_new (BAMF_TYPE_TAB_SOURCE, "id", "testingsource", NULL);
  
  gtk_main ();
  return 0;
}
