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

static GtkWidget *window;
static BamfMatcher *matcher;
static BamfControl *control;

//static gboolean application_seen;

static void destroy (GtkWidget *widget, gpointer data)
{
  gtk_main_quit ();
}

static void handle_view_opened (BamfMatcher *matcher, BamfView *view, gpointer data)
{
    return;
  
  
}

static void handle_view_closed (BamfMatcher *matcher, BamfView *view, gpointer data)
{
}

//static void test_window_match_without_registration ()
//{
//  gtk_widget_show_all (window);
//}

//static void test_window_match_with_registration ()
//{

//}

static void begin_testing ()
{

}

int main (int argc, char **argv)
{
  gtk_init (&argc, &argv);

  matcher = bamf_matcher_get_default ();
  control = bamf_control_get_default ();

  bamf_control_insert_desktop_file (control, g_build_filename (g_get_current_dir (), "interact-test.desktop", NULL));

  g_signal_connect (G_OBJECT (matcher), "view-opened",
	            (GCallback) handle_view_opened, NULL);
  
  g_signal_connect (G_OBJECT (matcher), "view-closed",
	            (GCallback) handle_view_closed, NULL);

  window = gtk_window_new (GTK_WINDOW_TOPLEVEL);
  gtk_window_set_wmclass (GTK_WINDOW (window), "interact-test", "interact-test");

  g_signal_connect (G_OBJECT (window), "destroy", G_CALLBACK (destroy), NULL);
  
  g_timeout_add (5000, (GSourceFunc) begin_testing, NULL);

  gtk_main ();
  return 0;
}







