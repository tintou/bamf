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

GtkWidget *window;
GtkWidget *treeView;
guint timer;

void populate_tree_store (GtkTreeStore *store)
{
  GtkTreeIter position; 
  GtkTreeIter child;
  GList *windows, *w; 
  GList *apps, *l;
  BamfApplication *app;
  BamfWindow *window;
  const char *filename;
  
  apps = bamf_matcher_get_applications (bamf_matcher_get_default ());

  if (apps == NULL)
    g_print ("FAIL\n");

  for (l = apps; l; l = l->next) 
  {
    app = BAMF_APPLICATION (l->data);
    
    if (!bamf_view_user_visible (BAMF_VIEW (app)))
      continue;

    gtk_tree_store_append (store, &position, NULL);
    filename = bamf_application_get_desktop_file (app);
    gtk_tree_store_set (store, &position, 0, filename, -1);

    windows = bamf_application_get_windows (app);
    
    g_print("%i    %s\n", g_list_length (windows), filename);

    for (w = windows; w; w = w->next) 
    {
      window = BAMF_WINDOW (w->data);
      
      if (!bamf_view_user_visible (BAMF_VIEW (window)))
        continue;
      
      const gchar *name = bamf_view_get_name (BAMF_VIEW (window));
      gtk_tree_store_append (store, &child, &position);
      gtk_tree_store_set (store, &child, 0, name, -1);
    }
  }
}

GtkWidget * make_tree_view ()
{
  GtkWidget *treeView;
  GtkTreeStore *treeStore;
  GtkTreeViewColumn *column;
  GtkCellRenderer *renderer;

  treeView = gtk_tree_view_new ();
  treeStore = gtk_tree_store_new (1, G_TYPE_STRING);

  column = gtk_tree_view_column_new ();
  gtk_tree_view_column_set_title (column, "windows");
  gtk_tree_view_append_column (GTK_TREE_VIEW (treeView), column);

  renderer = gtk_cell_renderer_text_new ();
  gtk_tree_view_column_pack_start (column, renderer, TRUE);
  gtk_tree_view_column_add_attribute(column, renderer, "text", 0);

  gtk_tree_view_set_model (GTK_TREE_VIEW (treeView), GTK_TREE_MODEL (treeStore));

  populate_tree_store (treeStore);
  
  g_object_unref (treeStore);

  return treeView;
}

static void destroy (GtkWidget *widget, gpointer data)
{
  gtk_main_quit ();
}

static void handle_view_opened (BamfMatcher *matcher, BamfView *view, gpointer data)
{
  g_print ("VIEW OPENED\n");
}

/* This entire program is intentionally implemented in a slightly niave manner to allow it to show
 * the function of both of the primary calls to libbamf. There is usually no reason to use
 * both as one or the other can generally get the job done. Experienced coders should recognize how
 * to do this right away, however it is noted where it could be performed in the source.
 */
int main (int argc, char **argv)
{
  BamfMatcher *matcher;
  gtk_init (&argc, &argv);

  matcher = bamf_matcher_get_default ();

  g_signal_connect (G_OBJECT (matcher), "view-opened",
	            (GCallback) handle_view_opened, NULL);

  window = gtk_window_new (GTK_WINDOW_TOPLEVEL);

  g_signal_connect (G_OBJECT (window), "destroy", G_CALLBACK (destroy), NULL);

  treeView = make_tree_view ();
  gtk_container_add (GTK_CONTAINER (window), treeView);

  gtk_widget_show_all (window);
  gtk_main ();
  return 0;
}







