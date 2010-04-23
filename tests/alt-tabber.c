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

#define WNCK_I_KNOW_THIS_IS_UNSTABLE

#include <stdio.h>
#include <gtk/gtk.h>
#include <gdk/gdk.h>
#include <glib.h>
#include <libwnck/libwnck.h>
#include <libbamf/libbamf.h>

GtkWidget *window;
GtkWidget *treeView;
guint timer;

void file_callback (BamfProxy *proxy,
                    gchar *file,
                    WnckWindow *data)
{
  g_return_if_fail (WNCK_IS_WINDOW (data));

  if (file)
    {
      g_print ("%s -- %s\n", file, wnck_window_get_name (data));
    }
}

GArray * get_desktop_files ()
{
	WnckScreen *screen = wnck_screen_get_default ();
	GList *windows = wnck_screen_get_windows (screen);

	GArray *desktopFiles = g_array_new (FALSE, TRUE, sizeof (GString*));

	GList *window;
	for (window = windows; window != NULL; window = window->next) {
		/* Doing just a little bit of tracking right here can save a lot of dbus calls.
		 * This is not done hwoever to allow us to use as many of the features of libbamf
		 * as possible.
		 */
		gchar *file = bamf_proxy_get_desktop_file (bamf_proxy_get_default (), wnck_window_get_xid (window->data));
		bamf_proxy_get_desktop_file_async (bamf_proxy_get_default (),
		                                       wnck_window_get_xid (window->data),
		                                       (BamfFileCallback) file_callback,
		                                       window->data);
		                                       
		if (file != NULL && *file != 0) {
			GString *desktopFile = g_string_new (file);

			gboolean equal = FALSE;
			int i;
			for (i = 0; i < desktopFiles->len; i++) {
				if (g_string_equal (g_array_index (desktopFiles, GString*, i), desktopFile)) {
					equal = TRUE;
					break;
				}
			}

			if (!equal)
				g_array_append_val (desktopFiles, desktopFile);
		}

		g_free (file);
	}

	return desktopFiles;
}

void array_callback (BamfProxy *proxy,
                     GArray *xids,
                     gchar *string)
{
  if (string)
    {
      g_print ("%s --", string);
      int i;
      for (i = 0; i < xids->len; i++)
        {
          g_print (" %u", g_array_index (xids, guint32, i));
        }
      g_print ("\n");
    }
}

void populate_tree_store (GtkTreeStore *store)
{
	GtkTreeIter position, child;

	GArray *desktopFiles = get_desktop_files ();

	int i;
	for (i = 0; i < desktopFiles->len; i++) {
		gtk_tree_store_append (store, &position, NULL);

		gchar *string = g_array_index (desktopFiles, GString*, i)->str;

		gtk_tree_store_set (store, &position, 0, string, -1);

		GArray * windows = bamf_proxy_get_xids (bamf_proxy_get_default (), string);
		bamf_proxy_get_xids_async (bamf_proxy_get_default (),
		                               string,
		                               (BamfArrayCallback) array_callback,
		                               string);
		int j;
		for (j = 0; j < windows->len; j++) {
			WnckWindow *window = wnck_window_get (g_array_index (windows, gulong, j));
			const gchar *name = wnck_window_get_name (window);
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

	g_object_unref (treeStore);

	return treeView;
}

static gboolean on_timer_elapsed (gpointer callback_data)
{
	GtkTreeStore *store = GTK_TREE_STORE (gtk_tree_view_get_model (GTK_TREE_VIEW (treeView)));

	gtk_tree_store_clear (store);
	populate_tree_store (store);

	timer = 0;

	return FALSE;
}


/* libwnck has a nice habit of sending up tons of these events when it is first started.
 * For this reason we use the timer to buffer out some of these events, and save ourselves
 * from murdering the dbus server.
 */
static void handle_window_opened_closed (WnckScreen *screen, WnckWindow *window, gpointer data)
{
	if (timer > 0)
		g_source_remove (timer);

	timer = g_timeout_add (300, (GSourceFunc) on_timer_elapsed, NULL);
}

static void destroy (GtkWidget *widget, gpointer data)
{
	gtk_main_quit ();
}

/* This entire program is intentionally implemented in a slightly niave manner to allow it to show
 * the function of both of the primary calls to libbamf. There is usually no reason to use
 * both as one or the other can generally get the job done. Experienced coders should recognize how
 * to do this right away, however it is noted where it could be performed in the source.
 */
int main (int argc, char **argv)
{
	gtk_init (&argc, &argv);

	window = gtk_window_new (GTK_WINDOW_TOPLEVEL);

	g_signal_connect (G_OBJECT (window), "destroy", G_CALLBACK (destroy), NULL);

	treeView = make_tree_view ();
	gtk_container_add (GTK_CONTAINER (window), treeView);

	WnckScreen *screen = wnck_screen_get_default ();

	g_signal_connect (G_OBJECT (screen), "window-opened", (GCallback) handle_window_opened_closed, window);
	g_signal_connect (G_OBJECT (screen), "window-closed", (GCallback) handle_window_opened_closed, window);

	gtk_widget_show_all (window);
	gtk_main ();
	return 0;
}







