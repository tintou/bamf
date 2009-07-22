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

static GHashTable * create_desktop_file_table (WindowMatcher *self);

static GArray * prefix_strings (WindowMatcher *self);
static GArray * xdg_data_dirs (WindowMatcher *self);
static GArray * list_desktop_file_in_dir (WindowMatcher *self, GFile *dir);
static GArray * pid_parent_tree (gint pid);
static GArray * window_list_for_desktop_file_hint (WindowMatcher *self, GString *hint);

static GString * desktop_file_hint_for_window (WindowMatcher *self, WnckWindow *window);
static GString * exec_string_for_window (WindowMatcher *self, WnckWindow *window);
static GString * exec_string_for_desktop_file (WindowMatcher *self, GString *desktopFile);

static void process_exec_string (WindowMatcher *self, GString *execString);
static void handle_window_opened (WnckScreen *screen, WnckWindow *window, gpointer data);
static void handle_window_closed (WnckScreen *screen, WnckWindow *window, gpointer data);
static void set_window_hint (WindowMatcher *self, WnckWindow *window);

static void window_matcher_class_init (WindowMatcherClass *klass);
static void window_matcher_init (WindowMatcher *self);

#define WINDOW_MATCHER_GET_PRIVATE(obj) (G_TYPE_INSTANCE_GET_PRIVATE ((obj), WINDOW_MATCHER_TYPE, WindowMatcherPrivate))

struct _WindowMatcherPrivate
{
	GArray     *bad_prefixes;
	GHashTable *desktop_file_table;
	GHashTable *exec_list;
	GHashTable *registered_pids;
};

G_DEFINE_TYPE (WindowMatcher, window_matcher, G_TYPE_OBJECT);

static void window_matcher_class_init (WindowMatcherClass *klass)
{
	g_type_class_add_private (klass, sizeof (WindowMatcherPrivate));
}

static void window_matcher_init (WindowMatcher *self)
{
	WindowMatcherPrivate *priv;

	self->priv = priv = WINDOW_MATCHER_GET_PRIVATE (self);
}

WindowMatcher * window_matcher_new ()
{
	WindowMatcher *self = (WindowMatcher*) g_object_new (WINDOW_MATCHER_TYPE, NULL);
	window_matcher_init (self);
	
	self->priv->bad_prefixes    = g_array_new (FALSE, TRUE, sizeof (GRegex*));
	self->priv->registered_pids = g_hash_table_new ((GHashFunc) g_int_hash, (GEqualFunc) g_int_equal);
	
	GArray* prefixstrings = prefix_strings (self);
	
	int i;
	for (i = 0; i < prefixstrings->len; i++) {
		GString *gstr = g_array_index (prefixstrings, GString*, i);
		GRegex* regex = g_regex_new (gstr->str, G_REGEX_OPTIMIZE, 0, NULL);
		g_array_append_val (self->priv->bad_prefixes, regex);
		g_string_free (gstr, TRUE);
	}
	
	g_array_free (prefixstrings, TRUE);
	
	self->priv->desktop_file_table = create_desktop_file_table (self);
	
	// fixme: Get the screen we are actually on somehow? Probably can use $DISPLAY
	WnckScreen *screen = wnck_screen_get_default ();
	
	g_signal_connect (G_OBJECT (screen), "window-opened", (GCallback) handle_window_opened, self);
	g_signal_connect (G_OBJECT (screen), "window-closed", (GCallback) handle_window_closed, self);
	
	return self;
}

// ------------------------------------------------------------------------------
// Wnck Signal Handlers and Related
// ------------------------------------------------------------------------------

static void handle_window_opened (WnckScreen *screen, WnckWindow *window, gpointer data)
{
	if (window == NULL) return;
	
	set_window_hint ((WindowMatcher*) data, window);
}
	

static void handle_window_closed (WnckScreen *screen, WnckWindow *window, gpointer data)
{
	
}

static void set_window_hint (WindowMatcher *self, WnckWindow *window)
{
	if (self == NULL || window == NULL) return;
	
	GHashTable *registered_pids = self->priv->registered_pids;
	
	GArray *pids = pid_parent_tree (wnck_window_get_pid (window));

	if (pids->len == 0) return;
	
	GString *desktopFile;
	int i;
	for (i = 0; i < pids->len; i++) {
		gint pid = g_array_index (pids, gint, i);
		
		gint *key;
		key = g_new (gint, 1);
		*key = pid;
		
		desktopFile = g_hash_table_lookup (registered_pids, key);
		if (desktopFile != NULL && desktopFile->len != 0) break;
	}
	
	if (desktopFile == NULL || desktopFile->len == 0) return;
	
	Display *XDisplay = XOpenDisplay (NULL);
	XChangeProperty (XDisplay,
	                 wnck_window_get_xid (window),
	                 XInternAtom (XDisplay,
	                              _NET_WM_DESKTOP_FILE,
				      FALSE),
			 XA_STRING,
			 8,
			 PropModeReplace,
			 desktopFile->str,
			 desktopFile->len);
	XCloseDisplay (XDisplay);
	g_array_free (pids, TRUE);
}

// ------------------------------------------------------------------------------
// End Wnck Signal Handlers
// ------------------------------------------------------------------------------

void window_matcher_register_desktop_file_for_pid (WindowMatcher *self, GString *desktopFile, gint pid)
{
	gint *key;
	key = g_new (gint, 1);
	*key = pid;
	
	GString *dup = g_string_new (desktopFile->str);
	g_hash_table_insert (self->priv->registered_pids, key, dup);
	
	//fixme, this is a bit heavy
	
	WnckScreen *screen = wnck_screen_get_default ();
	GList *windows = wnck_screen_get_windows (screen);
	
	GList *glist_item = windows;
	while (glist_item != NULL) {
		set_window_hint (self, glist_item->data);
		
		glist_item = glist_item->next;	
	}
}

GString * window_matcher_desktop_file_for_window (WindowMatcher *self, WnckWindow *window)
{
	GString *desktopFile = desktop_file_hint_for_window (self, window);
	
	if (desktopFile == NULL || desktopFile->len == 0) {
		WindowMatcherPrivate *priv = self->priv;
		GString *exec = exec_string_for_window (self, window);
		
		process_exec_string (self, exec);
		
		desktopFile = g_hash_table_lookup (priv->desktop_file_table, exec);
	}

	if (desktopFile == NULL)
		return g_string_new ("");
	else
		return g_string_new (desktopFile->str);
}

GArray * window_matcher_window_list_for_desktop_file (WindowMatcher *self, GString *desktopFile)
{
	GArray *windowList = g_array_new (FALSE, TRUE, sizeof (WnckWindow*));
	
	if (desktopFile == NULL || desktopFile->len == 0)
		return windowList;
		
	GString *exec = exec_string_for_desktop_file (self, desktopFile);
	process_exec_string (self, exec);
	
	if (exec == NULL || exec->len == 0) {
		if (exec != NULL)
			g_string_free (exec, TRUE);
		return windowList;
	}
	
	WnckScreen *screen = wnck_screen_get_default ();
	GList *windows = wnck_screen_get_windows (screen);
	
	GList *glist_item = windows;
	while (glist_item != NULL) {
		WnckWindow *window = glist_item->data;
		
		GString *windowHint = desktop_file_hint_for_window (self, window);
		if (windowHint != NULL && windowHint->len > 0) {
			if (g_string_equal (windowHint, desktopFile))
				g_array_append_val (windowList, window); 
			g_string_free (windowHint, TRUE);
		} else {
			GString *windowExec = exec_string_for_window (self, glist_item->data);
			process_exec_string (self, windowExec);
			
			if (windowExec != NULL && windowExec->len != 0) {
				if (g_string_equal (exec, windowExec)) {
					g_array_append_val (windowList, window);
				}
				g_string_free (windowExec, TRUE);
			}
			
		}
		glist_item = glist_item->next;
	}
	
	g_string_free (exec, TRUE);
	return windowList;
}

// ------------------------------------------------------------------------------
// Internal methods
// ------------------------------------------------------------------------------

static GHashTable * create_desktop_file_table (WindowMatcher *self)
{
	GHashTable *table = g_hash_table_new ((GHashFunc) g_string_hash, (GEqualFunc) g_string_equal);
	
	GArray *data_dirs = xdg_data_dirs (self);
	
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
				GArray *newFiles = list_desktop_file_in_dir (self, file);
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
		
		GString *execLine = exec_string_for_desktop_file (self, desktopFile);
		if (execLine == NULL)
			continue;
	
		process_exec_string (self, execLine);
		
		g_hash_table_insert (table, execLine, desktopFile);
	}
	
	return table;
}

static GString * desktop_file_hint_for_window (WindowMatcher *self, WnckWindow *window)
{
	Display *XDisplay = XOpenDisplay (NULL);
	Atom atom = XInternAtom (XDisplay, _NET_WM_DESKTOP_FILE, FALSE);
	GString *hint;
	
	Atom type;
	gint format;
	gulong numItems;
	gulong bytesAfter;
	guchar *buffer;
	
	int result = XGetWindowProperty (XDisplay,
					 wnck_window_get_xid (window),
					 atom,
					 0,
					 G_MAXINT,
					 FALSE,
					 XA_STRING,
					 &type,
					 &format,
					 &numItems,
					 &bytesAfter,
					 &buffer);
					 
	XCloseDisplay (XDisplay);
	
	if (result != Success)
		hint = NULL;
	else
		hint = g_string_new (buffer);
	
	XFree (buffer);
	return hint;
}

static GString * exec_string_for_desktop_file (WindowMatcher *self, GString *desktopFile)
{
	GDesktopAppInfo *desktopApp = g_desktop_app_info_new_from_filename (desktopFile->str);
	
	if (desktopApp == NULL)
		return NULL;
		
	gchar *line = g_app_info_get_commandline (G_APP_INFO (desktopApp));
	GString *execLine = g_string_new (line);
	
	g_free (line);
	
	return execLine;
}

static GString * exec_string_for_window (WindowMatcher *self, WnckWindow *window)
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

static GArray * list_desktop_file_in_dir (WindowMatcher *self, GFile *dir)
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
			GArray *recursFiles = list_desktop_file_in_dir (self, g_file_new_for_path (filename->str));
		} else if (g_file_info_get_file_type (info) == G_FILE_TYPE_REGULAR) {
			if (g_str_has_suffix (filename->str, ".desktop")) {
				g_array_append_val (files, filename);
			}
		}
	}
	
	return files;
}

static GArray * pid_parent_tree (gint pid)
{
	GArray *tree = g_array_new (FALSE, TRUE, sizeof (gint));
	
	while (pid > 1) {
		g_array_append_val (tree, pid);
	
		glibtop_proc_uid buf;
		glibtop_get_proc_uid (&buf, pid);
		
		pid = buf.ppid;
	}
	
	return tree;
}

static GArray * prefix_strings (WindowMatcher *self)
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

static void process_exec_string (WindowMatcher *self, GString *execString)
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
		 	for (j = 0; j < self->priv->bad_prefixes->len; j++) {
				GRegex* regex = g_array_index (self->priv->bad_prefixes, GRegex*, j);
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

static GArray * window_list_for_desktop_file_hint (WindowMatcher *self, GString *hint)
{
	GArray *arr = g_array_new (FALSE, TRUE, sizeof (WnckWindow*));
	
	WnckScreen *screen = wnck_screen_get_default ();
	GList *windows = wnck_screen_get_windows (screen);
	
	GList *window = windows;
	for (window = windows; window != NULL; window = window->next) {
		WnckWindow *win = window->data;
		GString *windowHint = desktop_file_hint_for_window (self, win);
		
		if (windowHint != NULL && g_string_equal (windowHint, hint))
			g_array_append_val (arr, win);
	}
}

static GArray * xdg_data_dirs (WindowMatcher *self)
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
