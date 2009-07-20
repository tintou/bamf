/*
 *
 * Copyright (C) 2009 - Canonical Ltd.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * <program name> is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin St, Fifth Floor, 
 * Boston, MA  02110-1301  USA
 */
 
#define WNCK_I_KNOW_THIS_IS_UNSTABLE 

#include <stdio.h>
#include <gtk/gtk.h>
#include <gdk/gdk.h>
#include <glib.h>
#include <libwnck/libwnck.h>

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
	
	gtk_tree_view_set_model (GTK_TREE_VIEW (treeView), GTK_TREE_MODEL (treeStore));
	
	g_object_unref (treeStore);
	
	return treeView;
}

int main (int argc, char **argv)
{
	GtkWidget *window;
	GtkWidget *treeView;
	
	gtk_init (&argc, &argv);

	window = gtk_window_new (GTK_WINDOW_TOPLEVEL);
	gtk_widget_show (window);

	treeView = make_tree_view ();
	gtk_container_add (GTK_CONTAINER (window), treeView);
	
	gtk_widget_show (treeView);	
	
	gtk_main ();
	
	return 0;
}







