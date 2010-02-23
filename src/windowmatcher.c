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

#define WINDOW_MATCHER_GET_PRIVATE(obj) (G_TYPE_INSTANCE_GET_PRIVATE ((obj), WINDOW_MATCHER_TYPE, WindowMatcherPrivate))

struct _WindowMatcherPrivate
{
  GArray *bad_prefixes;
  GHashTable *desktop_id_table;
  GHashTable *desktop_file_table;
  GHashTable *exec_list;
  GHashTable *registered_pids;
  GHashTable *window_to_desktop_files;
};

G_DEFINE_TYPE (WindowMatcher, window_matcher, G_TYPE_OBJECT);

static GString *
get_open_office_window_hint (WindowMatcher * self, WnckWindow * window)
{
  const gchar *name;
  GString *exec;
  GHashTable *desktopFileTable;
  GString *file;
  
  g_return_val_if_fail (IS_WINDOW_MATCHER (self), NULL);
  g_return_val_if_fail (WNCK_IS_WINDOW (window), NULL);

  name = wnck_window_get_name (window);

  if (name == NULL)
    return NULL;

  if (g_str_has_suffix (name, "OpenOffice.org Writer"))
    {
      exec = g_string_new ("ooffice -writer %F");
    }
  else if (g_str_has_suffix (name, "OpenOffice.org Math"))
    {
      exec = g_string_new ("ooffice -math %F");
    }
  else if (g_str_has_suffix (name, "OpenOffice.org Calc"))
    {
      exec = g_string_new ("ooffice -calc %F");
    }
  else if (g_str_has_suffix (name, "OpenOffice.org Impress"))
    {
      exec = g_string_new ("ooffice -impress %F");
    }
  else if (g_str_has_suffix (name, "OpenOffice.org Draw"))
    {
      exec = g_string_new ("ooffice -draw %F");
    }
  else
    {
      return NULL;
    }

  desktopFileTable = self->priv->desktop_file_table;
  file = g_hash_table_lookup (desktopFileTable, exec);
  g_string_free (exec, TRUE);

  g_return_val_if_fail (file, NULL);

  return g_string_new (file->str);
}

/* Attempts to return the binary name for a particular execution string */
static void
process_exec_string (WindowMatcher * self, GString * execString)
{
  gchar *exec, *part;
  gchar **parts;
  gint i, j;
  gboolean regexFail;
  GRegex *regex;
  
  g_return_if_fail (execString && execString->len > 0);

  exec = g_utf8_casefold (execString->str, -1);
  parts = g_strsplit (exec, " ", 0);

  i = 0;
  while (parts[i] != NULL)
    {
      part = parts[i];

      if (!g_str_has_prefix (part, "-"))
	{
	  part = g_utf8_strrchr (part, -1, '/');
	  if (part == NULL)
	    part = parts[i];
	  else
	    part++;

	  regexFail = FALSE;
	  for (j = 0; j < self->priv->bad_prefixes->len; j++)
	    {
	      regex = g_array_index (self->priv->bad_prefixes, GRegex *, j);
	      if (g_regex_match (regex, part, 0, NULL))
		{
		  regexFail = TRUE;
		  break;
		}
	    }

	  if (!regexFail)
	    {
	      g_string_assign (execString, part);
	      break;
	    }
	}
      i++;
    }

  i = 0;
  while (parts[i] != NULL)
    {
      g_free (parts[i]);
      i++;
    }

  g_free (exec);
  g_free (parts);
}

static GArray *
prefix_strings (WindowMatcher * self)
{
  GArray *arr = g_array_new (FALSE, TRUE, sizeof (GString *));

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

static GArray *
pid_parent_tree (gint pid)
{
  GArray *tree = g_array_new (FALSE, TRUE, sizeof (gint));

  while (pid > 1)
    {
      g_array_append_val (tree, pid);

      glibtop_proc_uid buf;
      glibtop_get_proc_uid (&buf, pid);

      pid = buf.ppid;
    }

  return tree;
}

static GString *
exec_string_for_window (WindowMatcher * self, WnckWindow * window)
{
  gint pid = 0, i = 0;
  gchar **argv = NULL;
  GString *exec = NULL;

  g_return_val_if_fail (WNCK_IS_WINDOW (window), NULL);
  g_return_val_if_fail (IS_WINDOW_MATCHER (self), NULL);

  pid = wnck_window_get_pid (window);

  if (pid == 0)
    return NULL;

  glibtop_proc_args buffer;
  argv = glibtop_get_proc_argv (&buffer, pid, 0);
  exec = g_string_new ("");

  while (argv[i] != NULL)
    {
      g_string_append (exec, argv[i]);
      if (argv[i + 1] != NULL)
	g_string_append (exec, " ");
      g_free (argv[i]);
      i++;
    }

  g_free (argv);

  return exec;
}

static GString *
exec_string_for_desktop_file (WindowMatcher * self, GString * desktopFile)
{
  GDesktopAppInfo *desktopApp;
  const gchar *line;
  
  g_return_val_if_fail (desktopFile, NULL);
  g_return_val_if_fail (IS_WINDOW_MATCHER (self), NULL);

  desktopApp = g_desktop_app_info_new_from_filename (desktopFile->str);

  if (desktopApp == NULL)
    return NULL;

  line = g_app_info_get_commandline (G_APP_INFO (desktopApp));

  return g_string_new (line);
}

static void
create_desktop_file_table_for_entry (WindowMatcher * self,
				     GMenuTreeEntry * entry,
				     GHashTable * desktop_file_table,
				     GHashTable * desktop_id_table)
{
  GString *exec;
  
  g_return_if_fail (IS_WINDOW_MATCHER (self));
  
  exec = g_string_new (gmenu_tree_entry_get_exec (entry));

  if (!(g_str_has_prefix (exec->str, "chromium-browser") && g_strrstr (exec->str, "--app")) && 
      !g_str_has_prefix (exec->str, "ooffice"))
    {
      /**
       * Set of nasty hacks which should be removed some day. We wish to keep the full exec
       * strings so we can do nasty matching hacks later. A very very evil thing indeed. However this
       * helps hack around applications that run in the same process cross radically different instances.
       * A better solution needs to be thought up, however at this time it is not known.
       **/
      process_exec_string (self, exec);
    }
  
  GString* filename = g_string_new (gmenu_tree_entry_get_desktop_file_path (entry));
  g_hash_table_insert (desktop_file_table, exec, filename);

  GString* desktop_id = g_string_new (g_path_get_basename (filename->str));
  desktop_id = g_string_truncate (desktop_id, desktop_id->len - 8); /* remove last 8 characters for .desktop */
  
  g_hash_table_insert (desktop_id_table, desktop_id, filename); 
}

static void
create_desktop_file_table_for_directory (WindowMatcher * self,
					 GMenuTreeDirectory * dir,
					 GHashTable * desktop_file_table,
					 GHashTable * desktop_id_table)
{
  GSList *contents, *i;
  contents = gmenu_tree_directory_get_contents (dir);

  for (i = contents; i != NULL; i = g_slist_next (i))
    {
      GMenuTreeItem *item = i->data;

      switch (gmenu_tree_item_get_type (item))
	{
	case GMENU_TREE_ITEM_ENTRY:
	  create_desktop_file_table_for_entry (self, GMENU_TREE_ENTRY (item),
					       desktop_file_table, desktop_id_table);
	  break;

	case GMENU_TREE_ITEM_DIRECTORY:
	  create_desktop_file_table_for_directory (self,
						   GMENU_TREE_DIRECTORY
						   (item), desktop_file_table, desktop_id_table);
	  break;

	default:
	  /* we do not need to care about the other types */
	  break;
	}
      gmenu_tree_item_unref (item);
    }
  g_slist_free (contents);
}

static void
create_desktop_file_table (WindowMatcher * self, GHashTable **desktop_file_table, GHashTable **desktop_id_table)
{
  *desktop_file_table =
    g_hash_table_new ((GHashFunc) g_string_hash, (GEqualFunc) g_string_equal);
  
  *desktop_id_table =
    g_hash_table_new ((GHashFunc) g_string_hash, (GEqualFunc) g_string_equal);
  
  const char *menu_trees[] = { "applications.menu", "settings.menu", NULL };
  const char **tree_name;

  for (tree_name = menu_trees; *tree_name; tree_name++)
    {
      /* TODO: do we need GMENU_TREE_FLAGS_INCLUDE_NODISPLAY ? */
      GMenuTree *tree = gmenu_tree_lookup (*tree_name, 0);
      GMenuTreeDirectory *dir = gmenu_tree_get_root_directory (tree);
      create_desktop_file_table_for_directory (self, dir, *desktop_file_table, *desktop_id_table);
      gmenu_tree_unref (tree);
    }

  /*
     GHashTableIter iter;
     gpointer key, value;

     g_hash_table_iter_init (&iter, table);
     while (g_hash_table_iter_next (&iter, &key, &value))
     printf("key: %s val: %s\n", *((char**) key), *((char**) value));
   */
}

static gboolean
is_open_office_window (WindowMatcher * self, WnckWindow * window)
{
  WnckClassGroup *group;

  g_return_val_if_fail (WNCK_IS_WINDOW (window), FALSE);

  group = wnck_window_get_class_group (window);
  return group && g_str_has_prefix (wnck_class_group_get_name (group), "OpenOffice");
}

static GString *
get_window_hint (WindowMatcher *self,
                 WnckWindow *window,
                 const char *atom_name)
{
  Display *XDisplay;
  Atom atom;
  GString *hint;
  Atom type;
  gint format;
  gulong numItems;
  gulong bytesAfter;
  unsigned char *buffer;

  g_return_val_if_fail (IS_WINDOW_MATCHER (self), NULL);
  g_return_val_if_fail (WNCK_IS_WINDOW (window), NULL);
  g_return_val_if_fail (atom_name, NULL);
  
  XDisplay = XOpenDisplay (NULL);
  atom = XInternAtom (XDisplay, atom_name, FALSE);
  hint = NULL;


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

  if (result == Success && numItems > 0)
    hint = g_string_new ((char*) buffer);

  XFree (buffer);
  return hint;
}

static void
set_window_hint (WindowMatcher * self, 
                 WnckWindow * window,
                 const char *atom_name,
                 const char *data)
{
  Display *XDisplay;
  
  g_return_if_fail (IS_WINDOW_MATCHER (self));
  g_return_if_fail (WNCK_IS_WINDOW (window));
  g_return_if_fail (atom_name);
  g_return_if_fail (data);

  XDisplay = XOpenDisplay (NULL);
  XChangeProperty (XDisplay,
		   wnck_window_get_xid (window),
		   XInternAtom (XDisplay,
				atom_name,
				FALSE),
		   XA_STRING,
		   8,
		   PropModeReplace,
		   (unsigned char *) data,
		   strlen (data));
  XCloseDisplay (XDisplay);
}

static void
clean_window_memory (WindowMatcher *self,
                     WnckWindow *window)
{
  WindowMatcherPrivate *priv;
  GArray *desktop_files = NULL;
  GString *file;
  int i;
  
  g_return_if_fail (WNCK_IS_WINDOW (window));
  g_return_if_fail (IS_WINDOW_MATCHER (self));
  
  priv = self->priv;

  desktop_files = g_hash_table_lookup (priv->window_to_desktop_files, window);
  if (desktop_files)
    {
      /* Array already exists, lets make it empty */
      for (i = 0; i < desktop_files->len; i++)
        {
          file = g_array_index (desktop_files, GString*, i);
          g_array_remove_index_fast (desktop_files, i);
          g_string_free (file, TRUE);
        }
        
      g_array_free (desktop_files, TRUE);
      g_hash_table_remove (priv->window_to_desktop_files, window);
    }
}

static GString*
window_class_name (WnckWindow *window)
{
  WnckClassGroup *group;
  
  g_return_val_if_fail (WNCK_IS_WINDOW (window), NULL);

  group = wnck_window_get_class_group (window);

  if (!group)
    return NULL;
  
  return g_string_new (wnck_class_group_get_res_class (group));
}

static void
ensure_window_matching_state (WindowMatcher *self,
                              WnckWindow *window)
{
  GString *hint = NULL, *chrome_app_url = NULL, *file = NULL, *exec = NULL;
  WindowMatcherPrivate *priv;
  GArray *desktop_files = NULL;
  GHashTableIter iter;
  gpointer key, value;
  int i;
  
  g_return_if_fail (WNCK_IS_WINDOW (window));
  g_return_if_fail (IS_WINDOW_MATCHER (self));

  priv = self->priv;

  desktop_files = g_hash_table_lookup (priv->window_to_desktop_files, window);
  if (desktop_files)
    {
      /* Array already exists, lets make it empty */
      for (i = 0; i < desktop_files->len; i++)
        {
          file = g_array_index (desktop_files, GString*, i);
          g_array_remove_index_fast (desktop_files, i);
          g_string_free (file, TRUE);
        }
    }
  else
    {
      /* Make a new one and insert */
      desktop_files = g_array_new (FALSE, TRUE, sizeof (GString*));
      g_hash_table_insert (priv->window_to_desktop_files, window, desktop_files);
    }
  

  hint = get_window_hint (self, window, _NET_WM_DESKTOP_FILE);
  chrome_app_url = get_window_hint (self, window, _CHROME_APP_URL);
  if (chrome_app_url && chrome_app_url->len > 2)
    {
      /* We have a chrome app, lets find its desktop file */
      
      /* We need to replace _/ with / as chromium will signal the "base name" of the url
       * by placing a _ after it. So www.google.com/reader becomes www.google.com_/reader to
       * chromium. We are restoring the original URL.
       */
      GString *tmp = chrome_app_url;
      GRegex *regex = g_regex_new ("_/", 0, 0, NULL);
      
      chrome_app_url = g_string_new (g_regex_replace_literal (regex, 
                                                              chrome_app_url->str, 
                                                              -1, 0, 
                                                              "/", 
                                                              0, NULL));
      
      g_string_free (tmp, TRUE);
      g_regex_unref (regex);
      
      /* remove trailing slash */
      if (g_str_has_suffix (chrome_app_url->str, "/"))
        chrome_app_url = g_string_truncate (chrome_app_url, chrome_app_url->len - 1);
      
      /* Do a slow iteration over every .desktop file in our hash table, we are looking
       * for a chromium instance which contains --app and chrome_app_url
       */
      
      regex = g_regex_new (g_strdup_printf ("chromium-browser (.* )?--app( |=)\"?(http://|https://)?%s/?\"?( |$)", 
                                            chrome_app_url->str), 0, 0, NULL);
      
      g_hash_table_iter_init (&iter, priv->desktop_file_table);
      while (g_hash_table_iter_next (&iter, &key, &value))
        {
          GString *exec_line = (GString*) key;
          GString *file_path = (GString*) value;
          
          if (g_regex_match (regex, exec_line->str, 0, NULL))
            {
              /* We have found a match */
              file = g_string_new (file_path->str);
              g_array_append_val (desktop_files, file);
            }
        }
     
      g_regex_unref (regex);
    }
  else if (hint && hint->len > 0)
    {
      /* We have a window that has a hint that is NOT a chrome app */
      g_array_append_val (desktop_files, hint);
      /* whew, hard work, didn't even have to make a copy! */
    }
  else
    {
      GString *class_name;
      class_name = g_string_ascii_down (window_class_name (window));
      
      if (class_name)
        {
          file = g_hash_table_lookup (priv->desktop_id_table, class_name);
      
          if (file)
            {
              file = g_string_new (file->str);
              g_array_append_val (desktop_files, file);
            }
         
         g_string_free (class_name, TRUE);
       }
      
      /* Make a fracking guess */
      exec = exec_string_for_window (self, window);

      if (exec)
        process_exec_string (self, exec);
      
      if (exec && exec->len > 0)
        {
          file = g_hash_table_lookup (priv->desktop_file_table, exec);
          if (file)
            {
              /* Make sure we make a copy of the string */
              file = g_string_new (file->str);
              g_array_append_val (desktop_files, file);
            }
            
          g_string_free (exec, TRUE);
        }
      /* sub-optimal guesswork ends */
    }
}

static void
ensure_window_hint_set (WindowMatcher *self,
                        WnckWindow *window)
{
  GString *window_hint;
  gint i, pid;
  gint *key;
  
  g_return_if_fail (IS_WINDOW_MATCHER (self));
  g_return_if_fail (WNCK_IS_WINDOW (window));
  
  window_hint = get_window_hint (self, window, _NET_WM_DESKTOP_FILE);
  
  if (window_hint && window_hint->len > 0)
    return; /* already set */
  
  if (is_open_office_window (self, window))
    {
      window_hint = get_open_office_window_hint (self, window);
    }
  else
    {  
      GHashTable *registered_pids = self->priv->registered_pids;

      GArray *pids = pid_parent_tree (wnck_window_get_pid (window));

      for (i = 0; i < pids->len; i++)
        {
          pid = g_array_index (pids, gint, i);

          key = g_new (gint, 1);
          *key = pid;

          window_hint = g_hash_table_lookup (registered_pids, key);
          if (window_hint != NULL && window_hint->len != 0)
            break;
        }
      g_array_free (pids, TRUE);
    }
      
  if (window_hint)
    {
      set_window_hint (self, window, _NET_WM_DESKTOP_FILE, window_hint->str);
    }
}

static void
handle_window_opened (WnckScreen * screen, WnckWindow * window, gpointer data)
{
  WindowMatcher *self;
  self = (WindowMatcher *) data;
  
  g_return_if_fail (IS_WINDOW_MATCHER (self));
  g_return_if_fail (WNCK_IS_WINDOW (window));
  
  /* technically this round-trips to the Xorg server up to three times, however this
   * is relatively infrequent and keeps the code nicely compartmentalized */
  ensure_window_hint_set (self, window);
  ensure_window_matching_state (self, window);
}

static void
handle_window_closed (WnckScreen * screen, WnckWindow * window, gpointer data)
{
  WindowMatcher *self;
  self = (WindowMatcher *) data;
  
  g_return_if_fail (IS_WINDOW_MATCHER (self));
  g_return_if_fail (WNCK_IS_WINDOW (window));
  
  clean_window_memory (self, window);
}

gboolean
window_matcher_window_is_match_ready (WindowMatcher * self,
				      WnckWindow * window)
{
  GString *file;

  if (!is_open_office_window (self, window))
    return TRUE;

  file = get_open_office_window_hint (self, window);

  if (file)
    {
      g_string_free (file, TRUE);
      return TRUE;
    }
  return FALSE;
}

void
window_matcher_register_desktop_file_for_pid (WindowMatcher * self,
					      GString * desktopFile, gint pid)
{
  gint *key;
  WnckScreen *screen;
  GList *glist_item;
  GList *windows;
  GString *dup;
  
  g_return_if_fail (IS_WINDOW_MATCHER (self));
  g_return_if_fail (desktopFile && desktopFile->len > 0);
  
  key = g_new (gint, 1);
  *key = pid;

  dup = g_string_new (desktopFile->str);
  g_hash_table_insert (self->priv->registered_pids, key, dup);

  /* fixme, this is a bit heavy */

  screen = wnck_screen_get_default ();
  windows = wnck_screen_get_windows (screen);

  for (glist_item = windows; glist_item != NULL;
       glist_item = glist_item->next)
    {
      ensure_window_hint_set (self, glist_item->data);
    }
}

GString *
window_matcher_desktop_file_for_window (WindowMatcher * self,
					WnckWindow * window)
{
  WindowMatcherPrivate *priv;
  GArray *desktop_files;
  GString *result;
  
  result = g_string_new ("");
  
  g_return_val_if_fail (IS_WINDOW_MATCHER (self), result);
  g_return_val_if_fail (WNCK_IS_WINDOW (window), result);
  
  priv = self->priv;
  
  desktop_files = g_hash_table_lookup (priv->window_to_desktop_files, window);
  
  g_return_val_if_fail (desktop_files, result);
  
  /* Dont warn */
  if (desktop_files->len == 0)
    return result;
  
  g_string_free (result, TRUE);
  result = g_string_new (g_array_index (desktop_files, GString*, 0)->str);
  
  return result;
}

GArray *
window_matcher_window_list_for_desktop_file (WindowMatcher * self,
					     GString * desktopFile)
{
  WnckWindow *window;
  WindowMatcherPrivate *priv;
  GArray *desktop_files, *result;
  GString *desktop_file;
  GHashTableIter iter;
  gpointer key, value;
  int i;
  
  g_return_val_if_fail (IS_WINDOW_MATCHER (self), NULL);
  g_return_val_if_fail (desktopFile, NULL);
  
  priv = self->priv;
  
  result = g_array_new (FALSE, TRUE, sizeof (WnckWindow*));
  
  /* walk over our hashtable looking for hits, should not be too bad
   * as at any one time the user should have a reasonable number of windows
   * open.
   */
  g_hash_table_iter_init (&iter, priv->window_to_desktop_files);
  while (g_hash_table_iter_next (&iter, &key, &value))
    {
      window = (WnckWindow*) key;
      desktop_files = (GArray*) value;
      
      for (i = 0; i < desktop_files->len; i++)
        {
          desktop_file = g_array_index (desktop_files, GString*, i);
          if (g_string_equal (desktop_file, desktopFile))
            {
              g_array_append_val (result, window);
              break;
            }
        }
    }
  
  return result;
}

static void
window_matcher_class_init (WindowMatcherClass * klass)
{
  g_type_class_add_private (klass, sizeof (WindowMatcherPrivate));
}

static void
window_matcher_init (WindowMatcher * self)
{
  WindowMatcherPrivate *priv;

  self->priv = priv = WINDOW_MATCHER_GET_PRIVATE (self);
}

WindowMatcher *
window_matcher_new ()
{
  WindowMatcher *self;
  WindowMatcherPrivate *priv;
  GArray *prefixstrings;
  int i;
  
  self = (WindowMatcher *) g_object_new (WINDOW_MATCHER_TYPE, NULL);
  priv = self->priv;

  priv->bad_prefixes = g_array_new (FALSE, TRUE, sizeof (GRegex *));
  priv->registered_pids =
    g_hash_table_new ((GHashFunc) g_int_hash, (GEqualFunc) g_int_equal);
  
  priv->window_to_desktop_files =
    g_hash_table_new ((GHashFunc) g_direct_hash, (GEqualFunc) g_direct_equal);
  
  prefixstrings = prefix_strings (self);
  for (i = 0; i < prefixstrings->len; i++)
    {
      GString *gstr = g_array_index (prefixstrings, GString *, i);
      GRegex *regex = g_regex_new (gstr->str, G_REGEX_OPTIMIZE, 0, NULL);
      g_array_append_val (priv->bad_prefixes, regex);
      g_string_free (gstr, TRUE);
    }

  g_array_free (prefixstrings, TRUE);

  create_desktop_file_table (self, &(priv->desktop_file_table), &(priv->desktop_id_table));

  WnckScreen *screen = wnck_screen_get_default ();

  g_signal_connect (G_OBJECT (screen), "window-opened",
		    (GCallback) handle_window_opened, self);
  g_signal_connect (G_OBJECT (screen), "window-closed",
		    (GCallback) handle_window_closed, self);

  return self;
}

