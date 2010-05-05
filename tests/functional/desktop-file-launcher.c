/*
 *
 * Copyright (C) 2009 - Canonical Ltd.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 3 of the License.
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
#include <glib.h>
#include <gio/gdesktopappinfo.h>

int main (int argc, char **argv)
{
	gtk_init (&argc, &argv);

	GAppInfo *appInfo = G_APP_INFO (g_desktop_app_info_new_from_filename (argv [1]));

	GError *error;
	g_app_info_launch (appInfo, NULL, NULL, &error);

	g_print ("End");

	return 0;
}







