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

#define _POSIX_C_SOURCE 199309L
#include "windowmatcher.h"

#define GMENU_I_KNOW_THIS_IS_UNSTABLE
#include <gmenu-tree.h>

static GHashTable * create_desktop_file_table (WindowMatcher *self);

static GArray * prefix_strings (WindowMatcher *self);
static GArray * pid_parent_tree (gint pid);

static GString * desktop_file_hint_for_window (WindowMatcher *self, WnckWindow *window);
static GString * exec_string_for_window (WindowMatcher *self, WnckWindow *window);
static GString * exec_string_for_desktop_file (WindowMatcher *self, GString *desktopFile);
static GString * get_open_office_window_hint (WindowMatcher *self, WnckWindow *window);

static gboolean is_open_office_window (WindowMatcher *self, WnckWindow *window);

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

#ifdef PROFILING
#include <time.h>

#define delta_t(start,stop) (stop.tv_sec - start.tv_sec + (stop.tv_nsec-start.tv_nsec)/1000000000.0)

struct timespec _profile_start_real;
struct timespec _profile_start_cpu;

static void profile_start (const char* description)
{
	printf("start benchmarking %s\n", description);
	clock_gettime (CLOCK_REALTIME, &_profile_start_real);
	clock_gettime (CLOCK_PROCESS_CPUTIME_ID, &_profile_start_cpu);
}

static void profile_stop (const char* description)
{
    struct timespec stop_real;
    struct timespec stop_cpu;
    struct timespec res_real;
    struct timespec res_cpu;

    clock_gettime (CLOCK_REALTIME, &stop_real);
    clock_gettime (CLOCK_PROCESS_CPUTIME_ID, &stop_cpu);
    clock_getres (CLOCK_REALTIME, &res_real);
    clock_getres (CLOCK_PROCESS_CPUTIME_ID, &res_cpu);

    printf("%s: real %g (precision: %g) cpu %g (precision: %g)\n",
	    description,
	    delta_t (_profile_start_real, stop_real),
	    res_real.tv_sec + res_real.tv_nsec/1000000000.0,
	    delta_t (_profile_start_cpu, stop_cpu),
	    res_cpu.tv_sec + res_cpu.tv_nsec/1000000000.0);

}
#endif

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
	
	WindowMatcher *self = (WindowMatcher*) data;
	GString *hint = desktop_file_hint_for_window (self, window);
	
	if (hint == NULL || hint->len == 0) {
		set_window_hint ((WindowMatcher*) data, window);
	} else {
		// this is useful in the case of crash recovery
		gint pid = wnck_window_get_pid (window);
		GArray *ppids = pid_parent_tree (pid);
		
		gint i;
		gboolean found = FALSE;
		for (i = 0; i < ppids->len; i++) {
			gint *key;
			key = g_new (gint, 1);
			*key = g_array_index (ppids, gint, i);
		
			if (g_hash_table_lookup (self->priv->registered_pids, key)) {
				found = TRUE;
				break;
			}
		}
		
		g_array_free (ppids, TRUE);
		
		// we have no clue what this one is, lets register it
		if (!found) {
			gint *key;
			key = g_new (gint, 1);
			*key = pid;
		
			g_hash_table_insert (self->priv->registered_pids, key, hint);
		}
	}
}
	

static void handle_window_closed (WnckScreen *screen, WnckWindow *window, gpointer data)
{
	
}

// ------------------------------------------------------------------------------
// End Wnck Signal Handlers
// ------------------------------------------------------------------------------

gboolean window_matcher_window_is_match_ready (WindowMatcher *self, WnckWindow *window)
{
	GString *file;

	if (!is_open_office_window (self, window))
		return TRUE;

	file = get_open_office_window_hint (self, window);
	
	if (file) {
		g_string_free (file, TRUE);
		return TRUE;
	}
	return FALSE;
}

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
	
	GList *glist_item;
	for (glist_item = windows; glist_item != NULL; glist_item = glist_item->next) {
		set_window_hint (self, glist_item->data);
	}
}

GString * window_matcher_desktop_file_for_window (WindowMatcher *self, WnckWindow *window)
{
	GString *desktopFile = desktop_file_hint_for_window (self, window);
	
	
	if (desktopFile == NULL || desktopFile->len == 0) {
		WindowMatcherPrivate *priv = self->priv;
		GString *exec = exec_string_for_window (self, window);
		
		if (exec != NULL) {
			process_exec_string (self, exec);
			if (exec->len > 0) {
				desktopFile = g_hash_table_lookup (priv->desktop_file_table, exec);
			}
			g_string_free (exec, TRUE);
		}
	}

	if (desktopFile == NULL)
		return g_string_new ("");
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

static void create_desktop_file_table_for_entry (WindowMatcher *self,
	GMenuTreeEntry *entry, GHashTable *table)
{
	GString *exec = g_string_new(gmenu_tree_entry_get_exec (entry));

	/**
	 * Set full string into database for open office apps, we use this later to help
	 * matching these files up for hint setting
	 **/
	if (!g_str_has_prefix (exec->str, "ooffice"))
		process_exec_string (self, exec);

	g_hash_table_insert (table, exec,
		g_string_new(gmenu_tree_entry_get_desktop_file_path (entry)));
}

static void create_desktop_file_table_for_directory (WindowMatcher *self,
	GMenuTreeDirectory *dir, GHashTable *table)
{
	GSList *contents, *i;
	contents = gmenu_tree_directory_get_contents (dir);

	for (i = contents; i != NULL; i = g_slist_next (i)) {
		GMenuTreeItem *item = i->data;

		switch (gmenu_tree_item_get_type (item)) {
		    case GMENU_TREE_ITEM_ENTRY:
			create_desktop_file_table_for_entry (self, GMENU_TREE_ENTRY (item), table);
			break;

		    case GMENU_TREE_ITEM_DIRECTORY:
			create_desktop_file_table_for_directory (self, GMENU_TREE_DIRECTORY (item), table);
			break;

		    default:
			/* we do not need to care about the other types */
			break;
		}
	    gmenu_tree_item_unref (item);
	}
	g_slist_free (contents);
}

static GHashTable * create_desktop_file_table (WindowMatcher *self)
{
#ifdef PROFILING
	profile_start("create_desktop_file_table()");
#endif
	GHashTable *table = g_hash_table_new ((GHashFunc) g_string_hash, (GEqualFunc) g_string_equal);

	const char* menu_trees[] = {"applications.menu", "settings.menu", NULL};
	const char** tree_name;

	for (tree_name = menu_trees; *tree_name; tree_name++) {
		/* TODO: do we need GMENU_TREE_FLAGS_INCLUDE_NODISPLAY ? */
		GMenuTree* tree = gmenu_tree_lookup (*tree_name, 0);
		GMenuTreeDirectory* dir = gmenu_tree_get_root_directory (tree);
		create_desktop_file_table_for_directory (self, dir, table);
		gmenu_tree_unref (tree);
	}
	
	/*
	GHashTableIter iter;
	gpointer key, value;

	g_hash_table_iter_init (&iter, table);
	while (g_hash_table_iter_next (&iter, &key, &value))
	    printf("key: %s val: %s\n", *((char**) key), *((char**) value));
	*/
	
#ifdef PROFILING
	profile_stop("create_desktop_file_table()");
#endif
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
	gchar *buffer;
	
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
	
	if (result != Success) {
		hint = NULL;
	} else {
		hint = g_string_new (buffer);
	}
	XFree (buffer);
	return hint;
}

static GString * exec_string_for_desktop_file (WindowMatcher *self, GString *desktopFile)
{
	GDesktopAppInfo *desktopApp = g_desktop_app_info_new_from_filename (desktopFile->str);
	
	if (desktopApp == NULL)
		return NULL;
		
	gchar const *line = g_app_info_get_commandline (G_APP_INFO (desktopApp));
	GString *execLine = g_string_new (line);
	
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

static gboolean is_open_office_window (WindowMatcher *self, WnckWindow *window)
{
	WnckClassGroup *group = wnck_window_get_class_group (window);
	
	return (group && g_str_has_prefix (wnck_class_group_get_name (group), "OpenOffice"));
}

static GString * get_open_office_window_hint (WindowMatcher *self, WnckWindow *window)
{
	gchar *name = wnck_window_get_name (window);
	
	if (name == NULL)
		return NULL;
	
	GString *exec;
	if (g_str_has_suffix (name, "OpenOffice.org Writer")) {
		exec = g_string_new ("ooffice -writer %F");
	} else if (g_str_has_suffix (name, "OpenOffice.org Math")) {
		exec = g_string_new ("ooffice -math %F");
	} else if (g_str_has_suffix (name, "OpenOffice.org Calc")) {
		exec = g_string_new ("ooffice -calc %F");
	} else if (g_str_has_suffix (name, "OpenOffice.org Impress")) {
		exec = g_string_new ("ooffice -impress %F");
	} else if (g_str_has_suffix (name, "OpenOffice.org Draw")) {
		exec = g_string_new ("ooffice -draw %F");
	} else {
		return NULL;
	}

	GHashTable *desktopFileTable = self->priv->desktop_file_table;
	GString *file = g_hash_table_lookup (desktopFileTable, exec);
	g_string_free (exec, TRUE);

	g_return_val_if_fail (file, NULL);

	return g_string_new (file->str);
}

static void set_window_hint (WindowMatcher *self, WnckWindow *window)
{
	g_return_if_fail (self && window);

	GString *desktopFile;
	if (is_open_office_window (self, window)) {
		desktopFile = get_open_office_window_hint (self, window);
	} else {
		GHashTable *registered_pids = self->priv->registered_pids;
	
		GArray *pids = pid_parent_tree (wnck_window_get_pid (window));

		g_return_if_fail (pids->len > 0);
	
		int i;
		for (i = 0; i < pids->len; i++) {
			gint pid = g_array_index (pids, gint, i);
		
			gint *key;
			key = g_new (gint, 1);
			*key = pid;
		
			desktopFile = g_hash_table_lookup (registered_pids, key);
			if (desktopFile != NULL && desktopFile->len != 0) break;
		}
		
		g_array_free (pids, TRUE);
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
			 (unsigned char *)desktopFile->str,
			 desktopFile->len);
	XCloseDisplay (XDisplay);
}

