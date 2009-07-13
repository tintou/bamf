//  
//  Copyright (C) 2009 Canonical Ltd.
// 
//  This program is free software: you can redistribute it and/or modify
//  it under the terms of the GNU General Public License as published by
//  the Free Software Foundation, either version 3 of the License, or
//  (at your option) any later version.
// 
//  This program is distributed in the hope that it will be useful,
//  but WITHOUT ANY WARRANTY; without even the implied warranty of
//  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//  GNU General Public License for more details.
// 
//  You should have received a copy of the GNU General Public License
//  along with this program.  If not, see <http://www.gnu.org/licenses/>.
// 

#include "windowmatcher.h"

static GHashTable * create_desktop_file_table ();

static GArray * prefix_strings ();
static GArray * xdg_data_dirs ();
static GArray * list_desktop_file_in_dir (GFile *dir);

static GString * exec_string_for_window (WnckWindow *window);
static GString * exec_string_for_desktop_file (GString *desktopFile);

static void process_exec_string (GString *execString);
static void handle_window_opened (WnckScreen *screen, WnckWindow *window, gpointer data);
static void handle_window_closed (WnckScreen *screen, WnckWindow *window, gpointer data);

static gboolean INITIALIZED;

static GArray *bad_prefixes;
static GHashTable *desktop_file_table;
static GHashTable *exec_list;

void windowmatcher_initialize ()
{
	if (INITIALIZED)
		return;
	
	INITIALIZED = TRUE;
	
	glibtop_init ();
	
	bad_prefixes = g_array_new (FALSE, TRUE, sizeof (GRegex*));
	
	GArray* prefixstrings = prefix_strings ();
	
	int i;
	for (i = 0; i < prefixstrings->len; i++) {
		GString *gstr = g_array_index (prefixstrings, GString*, i);
		GRegex* regex = g_regex_new (gstr->str, G_REGEX_OPTIMIZE, 0, NULL);
		g_array_append_val (bad_prefixes, regex);
		g_string_free (gstr, TRUE);
	}
	
	g_array_free (prefixstrings, TRUE);
	
	desktop_file_table = create_desktop_file_table ();
	
	// fixme: Get the screen we are actually on somehow? Probably can use $DISPLAY
	WnckScreen *screen = wnck_screen_get_default ();
	
	g_signal_connect (G_OBJECT (screen), "window-opened", (GCallback) handle_window_opened, NULL);
	g_signal_connect (G_OBJECT (screen), "window-closed", (GCallback) handle_window_closed, NULL);
}

// ------------------------------------------------------------------------------
// Wnck Signal Handlers
// ------------------------------------------------------------------------------

static void handle_window_opened (WnckScreen *screen, WnckWindow *window, gpointer data)
{
	
}
	

static void handle_window_closed (WnckScreen *screen, WnckWindow *window, gpointer data)
{
	
}

// ------------------------------------------------------------------------------
// End Wnck Signal Handlers
// ------------------------------------------------------------------------------

void windowmatcher_quit ()
{
	
}

void windowmatcher_register_desktop_file_for_pid (GString *desktopFile, gulong pid)
{
	// FIXME
	
	return;
}

GString * windowmatcher_desktop_file_for_window (WnckWindow *window)
{
	GString *exec = exec_string_for_window (window);
	
	process_exec_string (exec);
	
	return g_hash_table_lookup (desktop_file_table, exec);
}

GArray * windowmatcher_window_list_for_desktop_file (GString *desktopFile)
{
	GArray *windowList = g_array_new (FALSE, TRUE, sizeof (WnckWindow*));
	
	if (desktopFile == NULL || desktopFile->len == 0)
		return windowList;
		
	GString *exec = exec_string_for_desktop_file (desktopFile);
	process_exec_string (exec);
	
	if (exec == NULL || exec->len == 0) {
		if (exec != NULL)
			g_string_free (exec, TRUE);
		return windowList;
	}
	
	WnckScreen *screen = wnck_screen_get_default ();
	GList *windows = wnck_screen_get_windows (screen);
	
	GList *glist_item = windows;
	while (glist_item != NULL) {
		GString *windowExec = exec_string_for_window (glist_item->data);
		process_exec_string (windowExec);
		if (windowExec != NULL && windowExec->len != 0) {
			if (g_string_equal (exec, windowExec)) {
				WnckWindow *window = glist_item->data;
				g_array_append_val (windowList, window);
			}
	
			g_string_free (windowExec, TRUE);
		}
		glist_item = glist_item->next;	
	}
	
	g_string_free (exec, TRUE);
	return windowList;
}

// ------------------------------------------------------------------------------
// Internal methods
// ------------------------------------------------------------------------------

static GArray * prefix_strings ()
{
	GArray *arr = g_array_new (FALSE, TRUE, sizeof (GString*));
	
	GString *gstr = g_string_new ("^gsku$");
	g_array_append_val (arr, gstr);
	
	gstr = g_string_new ("^sudo$");
	g_array_append_val (arr, gstr);
	
	gstr = g_string_new ("^java$");
	g_array_append_val (arr, gstr);
	
	gstr = g_string_new ("^mono$");
	g_array_append_val (arr, gstr);
	
	gstr = g_string_new ("^ruby$");
	g_array_append_val (arr, gstr);
	
	gstr = g_string_new ("^padsp$");
	g_array_append_val (arr, gstr);
	
	gstr = g_string_new ("^aoss$");
	g_array_append_val (arr, gstr);
	
	gstr = g_string_new ("^python(\\d.\\d)?$");
	g_array_append_val (arr, gstr);
	
	gstr = g_string_new ("^(ba)?sh$");
	g_array_append_val (arr, gstr);
	
	return arr;
}

static GArray * xdg_data_dirs ()
{
	    char *env = getenv ("XDG_DATA_DIRS");
        gchar **dirs;
        
        GArray *arr = g_array_new (FALSE, TRUE, sizeof (GString*));

        if (!env)
        	env = "/usr/share/";
        
        dirs = g_strsplit(env, ":", 0);
        
        int i=0;
        while (dirs [i] != NULL) {
	        GString* dir = g_string_new (dirs [i]);
			g_array_append_val (arr, dir);
			g_free (dirs [i]);
			i++;        
        }
        g_free (dirs);
        
        return arr;
}

static GHashTable * create_desktop_file_table ()
{
	GHashTable *table = g_hash_table_new ((GHashFunc) g_string_hash, (GEqualFunc) g_string_equal);
	
	GArray *data_dirs = xdg_data_dirs ();
	
	GArray *desktop_files = g_array_new (FALSE, TRUE, sizeof (GString*));
	
	int i;
	for (i = 0; i < data_dirs->len; i++) {
		GString* dir = g_array_index (data_dirs, GString*, i);
		g_string_append (dir, "applications/");
		
		GFile *file = g_file_new_for_path (dir->str);
		
		if (g_file_query_exists (file, NULL)) {
			GError *error = NULL;
			GFileInfo *fileInfo = g_file_query_info (file, G_FILE_ATTRIBUTE_STANDARD_TYPE, 0, NULL, &error); 
			if (g_file_info_get_file_type (fileInfo) == G_FILE_TYPE_DIRECTORY) {
				GArray *newFiles = list_desktop_file_in_dir (file);
				if (newFiles->len > 0)
					g_array_append_vals (desktop_files, newFiles->data, newFiles->len);
				g_array_free (newFiles, TRUE);
			}
			
			g_object_unref (fileInfo);
		}
		
		g_object_unref (file);
	}
	
	for (i = 0; i < desktop_files->len; i++) {
		GString *desktopFile = g_array_index (desktop_files, GString*, i);
		
		GString *execLine = exec_string_for_desktop_file (desktopFile);
		if (execLine == NULL)
			continue;
	
		process_exec_string (execLine);
		
		g_hash_table_insert (table, execLine, desktopFile);
	}
	
	return table;
}

static GString * exec_string_for_desktop_file (GString *desktopFile)
{
	GDesktopAppInfo *desktopApp = g_desktop_app_info_new_from_filename (desktopFile->str);
	
	if (desktopApp == NULL)
		return NULL;
		
	gchar *line = g_app_info_get_commandline (G_APP_INFO (desktopApp));
	GString *execLine = g_string_new (line);
	
	g_free (line);
	
	return execLine;
}

static GArray * list_desktop_file_in_dir (GFile *dir)
{
	GArray *files = g_array_new (FALSE, TRUE, sizeof (GString*));
	GError *error = NULL;
	
	GFileEnumerator *enumerator = g_file_enumerate_children (dir, 
															 G_FILE_ATTRIBUTE_STANDARD_TYPE "," G_FILE_ATTRIBUTE_STANDARD_NAME,
															 0,
															 NULL,
															 &error);
	
	if (!enumerator) {
		g_print ("%s\n", error->message);
		g_error_free (error);
		return files;	
	}
	
	if (error != NULL)
		g_error_free (error);
	
	GFileInfo *info = NULL;
	error = NULL;
	while (TRUE)
	{
		info = g_file_enumerator_next_file (enumerator, NULL, &error);
		if (error != NULL) {
			g_print ("%s\n", error->message);
			g_error_free (error);
			error = NULL;		
		} else if (info  == NULL) {
			break;		
		}
		
		GString *filename = g_string_new (g_file_get_path (dir));
		g_string_append (filename, "/");
		g_string_append (filename, g_file_info_get_name (info));
		
		if (g_file_info_get_file_type (info) == G_FILE_TYPE_DIRECTORY) {
			GArray *recursFiles = list_desktop_file_in_dir (g_file_new_for_path (filename->str));
		} else if (g_file_info_get_file_type (info) == G_FILE_TYPE_REGULAR) {
			if (g_str_has_suffix (filename->str, ".desktop")) {
				g_array_append_val (files, filename);
			}
		}
	}
	
	return files;
}

static void process_exec_string (GString *execString)
{
	if (execString == NULL || execString->len == 0)
		return;
	
	gchar *exec = g_utf8_casefold (execString->str, -1);
	gchar **parts = g_strsplit (exec, " ", 0);
	
	int i = 0;
	while (parts [i] != NULL) {
		gchar *part = parts [i];
		
		if (!g_str_has_prefix (part, "-")) {
		 	
		 	part = g_utf8_strrchr (part, -1, '/');
		 	if (part == NULL)
		 		part = parts [i];
		 	else
		 		part++;
		 	
		 	int j;
		 	gboolean regexFail = FALSE;
		 	for (j = 0; j < bad_prefixes->len; j++) {
				GRegex* regex = g_array_index (bad_prefixes, GRegex*, j);
				if (g_regex_match (regex, part, 0, NULL)) {
					regexFail = TRUE;
					break;
				}
		 	}
		 	
		 	if (!regexFail) {
				g_string_assign (execString, part);
				break;
		 	}
		}
		i++;
	}
	
	i = 0;
	while (parts [i] != NULL) {
		g_free (parts [i]);
		i++;	
	}
	
	g_free (exec);
	g_free (parts);
}

static GString * exec_string_for_window (WnckWindow *window)
{
	gint pid = wnck_window_get_pid (window);
	
	if (pid == 0)
		return NULL;
	
	glibtop_proc_args buffer;
	
	gchar **argv = glibtop_get_proc_argv (&buffer, pid, 0);
	
	GString *exec = g_string_new ("");
	
	int i = 0;
	while (argv [i] != NULL) {
		g_string_append (exec, argv [i]);
		if (argv [i + 1] != NULL)
			g_string_append (exec, " ");
		g_free (argv [i]);
		i++;
	}
	
	g_free (argv);
	
	return exec;
}