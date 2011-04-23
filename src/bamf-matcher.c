/*
 * Copyright (C) 2010 Canonical Ltd
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 3 as
 * published by the Free Software Foundation.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 *
 * Authored by: Jason Smith <jason.smith@canonical.com>
 *
 */

#include <gdk/gdkx.h>

#include "bamf-marshal.h"
#include "bamf-matcher.h"
#include "bamf-matcher-glue.h"
#include "bamf-application.h"
#include "bamf-window.h"
#include "bamf-legacy-window.h"
#include "bamf-legacy-window-test.h"
#include "bamf-legacy-screen.h"
#include "bamf-indicator-source.h"

G_DEFINE_TYPE (BamfMatcher, bamf_matcher, G_TYPE_OBJECT);
#define BAMF_MATCHER_GET_PRIVATE(obj) (G_TYPE_INSTANCE_GET_PRIVATE(obj, \
BAMF_TYPE_MATCHER, BamfMatcherPrivate))

enum
{
  VIEW_OPENED,
  VIEW_CLOSED,
  ACTIVE_APPLICATION_CHANGED,
  ACTIVE_WINDOW_CHANGED,
  FAVORITES_CHANGED,

  LAST_SIGNAL,
};

static guint matcher_signals[LAST_SIGNAL] = { 0 };

typedef struct _OpenOfficeTimeoutArgs OpenOfficeTimeoutArgs;

struct _OpenOfficeTimeoutArgs
{
  BamfMatcher *matcher;
  BamfLegacyWindow *window;
  int count;
};


struct _BamfMatcherPrivate
{
  GArray          * bad_prefixes;
  GArray          * known_pids;
  GHashTable      * desktop_id_table;
  GHashTable      * desktop_file_table;
  GHashTable      * desktop_class_table;
  GHashTable      * exec_list;
  GHashTable      * registered_pids;
  GList           * views;
  GList           * monitors;
  GList           * favorites;
  BamfView        * active_app;
  BamfView        * active_win;
};

static void
on_view_active_changed (BamfView *view, gboolean active, BamfMatcher *matcher)
{
  BamfMatcherPrivate *priv;
  BamfView *last;

  g_return_if_fail (BAMF_IS_MATCHER (matcher));
  g_return_if_fail (BAMF_IS_VIEW (view));

  priv = matcher->priv;

  if (BAMF_IS_APPLICATION (view))
    {
      /* Do some handy short circuiting so we can assume a signal
       * will be generated at the end of this
       */
      if (!active && priv->active_app != view)
        return;

      if (active && priv->active_app == view)
        return;

      last = priv->active_app;

      if (active)
        priv->active_app = view;
      else
        priv->active_app = NULL;

      g_signal_emit (matcher,
                     matcher_signals[ACTIVE_APPLICATION_CHANGED],
                     0,
                     BAMF_IS_VIEW (last) ? bamf_view_get_path (BAMF_VIEW (last)) : NULL,
                     BAMF_IS_VIEW (priv->active_app) ? bamf_view_get_path (BAMF_VIEW (priv->active_app)) : NULL);
    }
  else if (BAMF_IS_WINDOW (view))
    {
      /* Do some handy short circuiting so we can assume a signal
       * will be generated at the end of this
       */
      if (!active && priv->active_win != view)
        return;

      if (active && priv->active_win == view)
        return;

      last = priv->active_win;

      if (active)
        priv->active_win = view;
      else
        priv->active_win = NULL;

      g_signal_emit (matcher,
                     matcher_signals[ACTIVE_WINDOW_CHANGED],
                     0,
                     BAMF_IS_VIEW (last) ? bamf_view_get_path (BAMF_VIEW (last)) : NULL,
                     BAMF_IS_VIEW (priv->active_win) ? bamf_view_get_path (BAMF_VIEW (priv->active_win)) : NULL);
    }
}

static void bamf_matcher_unregister_view (BamfMatcher *self, BamfView *view);

static void
on_view_closed (BamfView *view, BamfMatcher *self)
{
  bamf_matcher_unregister_view (self, view);
}

static void
bamf_matcher_register_view (BamfMatcher *self, BamfView *view)
{
  char * path, * type;

  path = bamf_view_export_on_bus (view);
  type = bamf_view_get_view_type (view);

  g_signal_connect (G_OBJECT (view), "closed-internal",
	      	    (GCallback) on_view_closed, self);
  g_signal_connect (G_OBJECT (view), "active-changed",
                    (GCallback) on_view_active_changed, self);

  self->priv->views = g_list_prepend (self->priv->views, view);
  g_object_ref (view);

  g_signal_emit (self, matcher_signals[VIEW_OPENED],0, path, type);
  g_free (type);
}

static void
bamf_matcher_unregister_view (BamfMatcher *self, BamfView *view)
{
  const char * path;
  char * type;

  path = bamf_view_get_path (view);
  type = bamf_view_get_view_type (view);

  g_signal_emit (self, matcher_signals[VIEW_CLOSED],0, path, type);

  g_signal_handlers_disconnect_by_func (G_OBJECT (view), on_view_closed, self);
  g_signal_handlers_disconnect_by_func (G_OBJECT (view), on_view_active_changed, self);

  self->priv->views = g_list_remove (self->priv->views, view);
  g_object_unref (view);

  g_free (type);
}

static char *
get_open_office_window_hint (BamfMatcher * self, BamfLegacyWindow * window)
{
  const gchar *name;
  char *exec;
  GHashTable *desktopFileTable;
  GList *list;

  g_return_val_if_fail (BAMF_IS_MATCHER (self), NULL);
  g_return_val_if_fail (BAMF_IS_LEGACY_WINDOW (window), NULL);

  name = bamf_legacy_window_get_name (window);

  if (name == NULL)
    return NULL;

  if (g_str_has_suffix (name, "OpenOffice.org Writer"))
    {
      exec = "ooffice -writer %F";
    }
  else if (g_str_has_suffix (name, "OpenOffice.org Math"))
    {
      exec = "ooffice -math %F";
    }
  else if (g_str_has_suffix (name, "OpenOffice.org Calc"))
    {
      exec = "ooffice -calc %F";
    }
  else if (g_str_has_suffix (name, "OpenOffice.org Impress"))
    {
      exec = "ooffice -impress %F";
    }
  else if (g_str_has_suffix (name, "OpenOffice.org Draw"))
    {
      exec = "ooffice -draw %F";
    }
  else if (g_str_has_suffix (name, "LibreOffice Writer"))
    {
      exec = "libreoffice -writer %U";
    }
  else if (g_str_has_suffix (name, "LibreOffice Math"))
    {
      exec = "libreoffice -math %U";
    }
  else if (g_str_has_suffix (name, "LibreOffice Calc"))
    {
      exec = "libreoffice -calc %U";
    }
  else if (g_str_has_suffix (name, "LibreOffice Impress"))
    {
      exec = "libreoffice -impress %U";
    }
  else if (g_str_has_suffix (name, "LibreOffice Draw"))
    {
      exec = "libreoffice -draw %U";
    }
  else
    {
      return NULL;
    }

  desktopFileTable = self->priv->desktop_file_table;
  list = g_hash_table_lookup (desktopFileTable, exec);

  g_return_val_if_fail (list, NULL);

  return (char *) list->data;
}

/* Attempts to return the binary name for a particular execution string */
static char *
trim_exec_string (BamfMatcher * self, char * execString)
{
  gchar *result = NULL, *exec = NULL, *part = NULL, *tmp = NULL;
  gchar **parts;
  gint i, j;
  gboolean regexFail;
  GRegex *regex;

  g_return_val_if_fail (execString && strlen (execString) > 0, g_strdup (execString));

  exec = g_utf8_casefold (execString, -1);
  parts = g_strsplit (exec, " ", 0);

  i = 0;
  while (parts[i] != NULL)
    {
      part = parts[i];

      if (!g_str_has_prefix (part, "-"))
	{
	  tmp = g_utf8_strrchr (part, -1, '/');
	  if (tmp)
            part = tmp + 1;

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
	      result = g_strdup (part);
	      break;
	    }
	}
      i++;
    }

  if (!result)
    {
      result = g_strdup (execString);
    }
  else
    {
      tmp = result;
      
      regex = g_regex_new ("((\\.|-)bin|\\.py)$", 0, 0, NULL);
      result = g_regex_replace_literal (regex, result, -1, 0, "", 0, NULL);
      
      g_free (tmp);
      g_regex_unref (regex);
    }

  g_free (exec);
  g_strfreev (parts);

  return result;
}

static GArray *
prefix_strings (BamfMatcher * self)
{
  GArray *arr = g_array_new (FALSE, TRUE, sizeof (char *));

  char *str = "^gsku$";
  g_array_append_val (arr, str);

  str = "^sudo$";
  g_array_append_val (arr, str);

  str = "^java$";
  g_array_append_val (arr, str);

  str = "^mono$";
  g_array_append_val (arr, str);

  str = "^ruby$";
  g_array_append_val (arr, str);

  str = "^padsp$";
  g_array_append_val (arr, str);

  str = "^aoss$";
  g_array_append_val (arr, str);

  str = "^python(\\d.\\d)?$";
  g_array_append_val (arr, str);

  str = "^(ba)?sh$";
  g_array_append_val (arr, str);

  return arr;
}

static GArray *
pid_parent_tree (BamfMatcher *self, gint pid)
{
  BamfMatcherPrivate *priv;
  GArray *tree;
  gint known_pid;
  gint i;

  g_return_val_if_fail (BAMF_IS_MATCHER (self), NULL);

  priv = self->priv;

  tree = g_array_new (FALSE, TRUE, sizeof (gint));

  g_array_append_val (tree, pid);

  glibtop_proc_uid buf;
  glibtop_get_proc_uid (&buf, pid);

  pid = buf.ppid;

  while (pid > 1)
    {
      for (i = 0; i < priv->known_pids->len; i++)
        {
          /* ensure we dont match onto a terminal by mistake */
          known_pid = g_array_index (priv->known_pids, gint, i);
          if (known_pid == pid)
            return tree;
        }

      g_array_append_val (tree, pid);

      glibtop_proc_uid buf;
      glibtop_get_proc_uid (&buf, pid);

      pid = buf.ppid;
    }
  return tree;
}

static gboolean
exec_string_should_be_processed (BamfMatcher *self,
                                 char *exec)
{
  return !g_str_has_prefix (exec, "ooffice") && !g_str_has_prefix (exec, "libreoffice");
}

static void
insert_data_into_tables (BamfMatcher *self,
                         const char *data,
                         const char *exec,
                         const char *desktop_id,
                         GHashTable *desktop_file_table,
                         GHashTable *desktop_id_table)
{
  GList *file_list, *id_list;
  char *datadup;
  
  g_return_if_fail (exec);
  g_return_if_fail (desktop_id);

  file_list = g_hash_table_lookup (desktop_file_table, exec);
  id_list   = g_hash_table_lookup (desktop_id_table, desktop_id);

  datadup = g_strdup (data);
  
  /* order so that items whose desktop_id == exec string are first in the list */
  if (g_strcmp0 (exec, desktop_id) == 0)
    {
      file_list = g_list_prepend (file_list, datadup);
      id_list   = g_list_prepend (id_list,   datadup);
    }
  else
    {
      file_list = g_list_append (file_list, datadup);
      id_list   = g_list_append (id_list,   datadup);
    }

  g_hash_table_insert (desktop_file_table, g_strdup (exec),       file_list);
  g_hash_table_insert (desktop_id_table,   g_strdup (desktop_id), id_list);
}

static void
insert_desktop_file_class_into_table (BamfMatcher *self,
                                      const char *desktop_file,
                                      GHashTable *desktop_class_table)
{
  GKeyFile *desktop_keyfile;
  char *class;

  g_return_if_fail (desktop_file);

  desktop_keyfile = g_key_file_new ();

  if (g_key_file_load_from_file (desktop_keyfile, desktop_file, G_KEY_FILE_NONE,
                                 NULL))
    {
      class = g_key_file_get_string (desktop_keyfile,
                                     G_KEY_FILE_DESKTOP_GROUP,
                                     G_KEY_FILE_DESKTOP_KEY_STARTUP_WM_CLASS,
                                     NULL);
      if (class)
        g_hash_table_insert (desktop_class_table, g_strdup (desktop_file), class);

      g_key_file_free (desktop_keyfile);
    }
}

static void
load_desktop_file_to_table (BamfMatcher * self,
                            const char *file,
                            GHashTable *desktop_file_table,
                            GHashTable *desktop_id_table,
                            GHashTable *desktop_class_table)
{
  GAppInfo *desktop_file;
  char *exec;
  char *path;
  GString *desktop_id; /* is ok... really */

  g_return_if_fail (BAMF_IS_MATCHER (self));

  desktop_file = G_APP_INFO (g_desktop_app_info_new_from_filename (file));

  if (!G_IS_APP_INFO (desktop_file))
    return;

  exec = g_strdup (g_app_info_get_commandline (desktop_file));
  
  if (!exec)
    return;

  g_object_unref (desktop_file);

  if (exec_string_should_be_processed (self, exec))
    {
      /**
       * Set of nasty hacks which should be removed some day. We wish to keep the full exec
       * strings so we can do nasty matching hacks later. A very very evil thing indeed. However this
       * helps hack around applications that run in the same process cross radically different instances.
       * A better solution needs to be thought up, however at this time it is not known.
       **/
      char *tmp = trim_exec_string (self, exec);
      g_free (exec);
      exec = tmp;
    }

  path = g_path_get_basename (file);
  desktop_id = g_string_new (path);
  g_free (path);

  desktop_id = g_string_truncate (desktop_id, desktop_id->len - 8); /* remove last 8 characters for .desktop */
  
  insert_data_into_tables (self, file, exec, desktop_id->str, desktop_file_table, desktop_id_table);
  insert_desktop_file_class_into_table (self, file, desktop_class_table);

  g_free (exec);
  g_string_free (desktop_id, TRUE);
}

static void
load_directory_to_table (BamfMatcher * self,
                         const char *directory,
                         GHashTable *desktop_file_table,
                         GHashTable *desktop_id_table,
                         GHashTable *desktop_class_table)
{
  GFile *dir;
  GFileEnumerator *enumerator;
  GFileInfo *info;
  const char* name;
  const char *path;

  dir = g_file_new_for_path (directory);

  enumerator = g_file_enumerate_children (dir,
                                          "standard::*",
                                          G_FILE_QUERY_INFO_NOFOLLOW_SYMLINKS,
                                          NULL,
                                          NULL);

  if (!enumerator)
    return;

  info = g_file_enumerator_next_file (enumerator, NULL, NULL);
  for (; info; info = g_file_enumerator_next_file (enumerator, NULL, NULL))
    {

      name = g_file_info_get_name (info);
      path = g_build_filename (directory, name, NULL);

      if (g_str_has_suffix (name, ".desktop"))
        load_desktop_file_to_table (self,
                                    path,
                                    desktop_file_table,
                                    desktop_id_table,
                                    desktop_class_table);

      g_free ((gpointer) path);
      g_object_unref (info);
    }

  g_object_unref (enumerator);
  g_object_unref (dir);
}

static void
load_index_file_to_table (BamfMatcher * self,
                          const char *index_file,
                          GHashTable *desktop_file_table,
                          GHashTable *desktop_id_table,
                          GHashTable *desktop_class_table)
{
  GFile *file;
  GFileInputStream *stream;
  GDataInputStream *input;
  char *line;
  char *directory;
  gsize length;

  file = g_file_new_for_path (index_file);

  g_return_if_fail (file);

  stream = g_file_read (file, NULL, NULL);

  if (!stream)
    {
      g_object_unref (file);
      return;
    }

  directory = g_path_get_dirname (index_file);
  input = g_data_input_stream_new (G_INPUT_STREAM (stream));


  while ((line = g_data_input_stream_read_line (input, &length, NULL, NULL)) != NULL)
    {
      char *exec;
      GString *desktop_id;
      GString *filename;

      gchar **parts = g_strsplit (line, "\t", 3);

      exec = parts[1];

      if (exec_string_should_be_processed (self, exec))
        {
          char *tmp = trim_exec_string (self, exec);
          exec = tmp;
        }

      char *name = g_build_filename (directory, parts[0], NULL);
      filename = g_string_new (name);
      g_free ((gpointer) name);

      desktop_id = g_string_new (parts[0]);
      g_string_truncate (desktop_id, desktop_id->len - 8);
      
      insert_data_into_tables (self, filename->str, exec, desktop_id->str, desktop_file_table, desktop_id_table);
      insert_desktop_file_class_into_table (self, filename->str, desktop_class_table);

      g_string_free (desktop_id, TRUE);
      length = 0;
      g_strfreev (parts);
    }
  g_free ((gpointer) directory);
}

static GList *
get_desktop_file_directories (BamfMatcher *self)
{
  GFile *file;
  GFileInfo *info;
  GFileEnumerator *enumerator;
  GList *dirs = NULL, *l;
  const char *env;
  char  *path;
  char  *subpath;
  char **data_dirs = NULL;
  char **data;
  
  env = g_getenv ("XDG_DATA_DIRS");
  
  if (env)
    {
      data_dirs = g_strsplit (env, ":", 0);
  
      for (data = data_dirs; *data; data++)
        {
          path = g_build_filename (*data, "applications", NULL);
          if (g_file_test (path, G_FILE_TEST_IS_DIR))
            dirs = g_list_prepend (dirs, path);
          else
            g_free (path);
        }
    }
    
  if (!g_list_find_custom (dirs, "/usr/share/applications", (GCompareFunc) g_strcmp0))
    dirs = g_list_prepend (dirs, g_strdup ("/usr/share/applications"));
  
  if (!g_list_find_custom (dirs, "/usr/local/share/applications", (GCompareFunc) g_strcmp0))
    dirs = g_list_prepend (dirs, g_strdup ("/usr/local/share/applications"));
  
  dirs = g_list_prepend (dirs, g_strdup (g_build_filename (g_get_home_dir (), ".local/share/applications", NULL)));
  
  if (data_dirs)
    g_strfreev (data_dirs);
  
  /* include subdirs */
  for (l = dirs; l; l = l->next)
    {
      path = l->data;
      
      file = g_file_new_for_path (path);
      
      if (!g_file_query_exists (file, NULL))
        {
          g_object_unref (file);
          continue;
        }
      
      enumerator = g_file_enumerate_children (file,
                                              "standard::*",
                                              G_FILE_QUERY_INFO_NOFOLLOW_SYMLINKS,
                                              NULL,
                                              NULL);
      
      if (!enumerator)
        continue;

      info = g_file_enumerator_next_file (enumerator, NULL, NULL);
      for (; info; info = g_file_enumerator_next_file (enumerator, NULL, NULL))
        {
          if (g_file_info_get_file_type (info) != G_FILE_TYPE_DIRECTORY)
            continue;
          
          subpath = g_build_filename (path, g_file_info_get_name (info), NULL);
          /* append for non-recursive recursion love */
          dirs = g_list_append (dirs, subpath);

          g_object_unref (info);
        }

      g_object_unref (enumerator);
      g_object_unref (file);
    }
  
  return dirs;
}

static gboolean
hash_table_remove_values (gpointer key, gpointer value, gpointer target)
{
  return g_strcmp0 ((char *) value, (char *) target) == 0;
}

static void
on_monitor_changed (GFileMonitor *monitor, GFile *file, GFile *other_file, GFileMonitorEvent type, BamfMatcher *self)
{
  char *path;
  GFileType filetype;
  GFileMonitor *dirmonitor;

  g_return_if_fail (G_IS_FILE_MONITOR (monitor));
  g_return_if_fail (BAMF_IS_MATCHER (self));

  if (type != G_FILE_MONITOR_EVENT_CREATED &&
      type != G_FILE_MONITOR_EVENT_DELETED &&
      type != G_FILE_MONITOR_EVENT_CHANGES_DONE_HINT)
    return;

  g_return_if_fail (G_IS_FILE (file));
  path = g_file_get_path (file);
  filetype = g_file_query_file_type (file, G_FILE_QUERY_INFO_NONE, NULL);
  
  if (!g_str_has_suffix (path, ".desktop") &&
      filetype != G_FILE_TYPE_DIRECTORY &&
      type != G_FILE_MONITOR_EVENT_DELETED)
    goto out;

  if (type == G_FILE_MONITOR_EVENT_DELETED || type == G_FILE_MONITOR_EVENT_CHANGES_DONE_HINT)
    {
      if (g_str_has_suffix (path, ".desktop"))
        {
          g_hash_table_foreach_remove (self->priv->desktop_id_table, (GHRFunc) hash_table_remove_values, path);
          g_hash_table_foreach_remove (self->priv->desktop_file_table, (GHRFunc) hash_table_remove_values, path);
          g_hash_table_remove (self->priv->desktop_class_table, path);
        } else if (g_strcmp0 (g_object_get_data (G_OBJECT (monitor), "root"), path) == 0) {
          g_signal_handlers_disconnect_by_func (monitor, on_monitor_changed, self);
          self->priv->monitors = g_list_remove (self->priv->monitors, monitor);
          g_object_unref (monitor);
        }
    }

  if (type == G_FILE_MONITOR_EVENT_CREATED || type == G_FILE_MONITOR_EVENT_CHANGES_DONE_HINT)
    {
      if (filetype == G_FILE_TYPE_DIRECTORY)
        {
          dirmonitor = g_file_monitor_directory (file, G_FILE_MONITOR_NONE, NULL, NULL);
          g_file_monitor_set_rate_limit (dirmonitor, 1000);
          self->priv->monitors = g_list_prepend (self->priv->monitors, dirmonitor);
          g_signal_connect (dirmonitor, "changed", (GCallback) on_monitor_changed, self);
          g_object_set_data_full (G_OBJECT (dirmonitor), "root", g_strdup (path), g_free);
        }
      else if (filetype != G_FILE_TYPE_UNKNOWN)
        {
          bamf_matcher_load_desktop_file (self, path);
        }
    } 

out:
  g_free (path);
}

static void
create_desktop_file_table (BamfMatcher * self,
                           GHashTable **desktop_file_table,
                           GHashTable **desktop_id_table,
                           GHashTable **desktop_class_table)
{
  GList *directories;
  GList *l;
  char *directory;
  const char *bamf_file;
  GFile *file;
  GFileMonitor *monitor;

  *desktop_file_table =
    g_hash_table_new_full ((GHashFunc) g_str_hash,
                           (GEqualFunc) g_str_equal,
                           (GDestroyNotify) g_free,
                           NULL);

  *desktop_id_table =
    g_hash_table_new_full ((GHashFunc) g_str_hash,
                           (GEqualFunc) g_str_equal,
                           (GDestroyNotify) g_free,
                           NULL);

  *desktop_class_table =
    g_hash_table_new_full ((GHashFunc) g_str_hash,
                           (GEqualFunc) g_str_equal,
                           (GDestroyNotify) g_free,
                           (GDestroyNotify) g_free);

  g_return_if_fail (BAMF_IS_MATCHER (self));

  directories = get_desktop_file_directories (self);

  for (l = directories; l; l = l->next)
    {
      directory = l->data;
      
      if (!g_file_test (directory, G_FILE_TEST_IS_DIR))
        continue;

      file = g_file_new_for_path (directory);
      monitor = g_file_monitor_directory (file, G_FILE_MONITOR_NONE, NULL, NULL);
      g_object_set_data_full (G_OBJECT (monitor), "root", g_strdup (directory), g_free);
      g_file_monitor_set_rate_limit (monitor, 1000);
      
      self->priv->monitors = g_list_prepend (self->priv->monitors, monitor);
      
      g_signal_connect (monitor, "changed", (GCallback) on_monitor_changed, self);
      
      bamf_file = g_build_filename (directory, "bamf.index", NULL);

      if (g_file_test (bamf_file, G_FILE_TEST_EXISTS))
        {
          load_index_file_to_table (self, bamf_file, *desktop_file_table,
                                    *desktop_id_table, *desktop_class_table);
        }
      else
        {
          load_directory_to_table (self, directory, *desktop_file_table,
                                   *desktop_id_table, *desktop_class_table);
        }

      g_free (directory);
      g_free ((gpointer) bamf_file);
    }
  
  g_list_free (directories);
}

static gboolean
is_open_office_window (BamfMatcher * self, BamfLegacyWindow * window)
{
  return g_str_has_prefix (bamf_legacy_window_get_class_name (window), "OpenOffice") ||
         g_str_has_prefix (bamf_legacy_window_get_class_name (window), "LibreOffice") ||
         g_str_has_prefix (bamf_legacy_window_get_class_name (window), "libreoffice");
}

static char *
get_window_hint (BamfMatcher *self,
                 BamfLegacyWindow *window,
                 const char *atom_name)
{
  Display *XDisplay;
  Atom atom;
  char *hint = NULL;
  Atom type;
  gint format;
  gulong numItems;
  gulong bytesAfter;
  unsigned char *buffer;
  gboolean close_display = TRUE;

  g_return_val_if_fail (BAMF_IS_MATCHER (self), NULL);
  g_return_val_if_fail (BAMF_IS_LEGACY_WINDOW (window), NULL);
  g_return_val_if_fail (atom_name, NULL);

  XDisplay = XOpenDisplay (NULL);
  if (!XDisplay)
  {
    XDisplay = gdk_x11_get_default_xdisplay ();
    if (!XDisplay)
    {
      g_warning ("%s: Unable to get a valid XDisplay", G_STRFUNC);
      return hint;
    }
    
    close_display = FALSE;
  }

  atom = XInternAtom (XDisplay, atom_name, FALSE);

  int result = XGetWindowProperty (XDisplay,
				   (gulong) bamf_legacy_window_get_xid (window),
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

  if (close_display)
    XCloseDisplay (XDisplay);

  if (result == Success && numItems > 0)
    {
      hint = g_strdup ((char*) buffer);
      XFree (buffer);
    }

  return hint;
}

static void
set_window_hint (BamfMatcher * self,
                 BamfLegacyWindow * window,
                 const char *atom_name,
                 const char *data)
{
  Display *XDisplay;
  gboolean close_display = TRUE;

  g_return_if_fail (BAMF_IS_MATCHER (self));
  g_return_if_fail (BAMF_LEGACY_WINDOW (window));
  g_return_if_fail (atom_name);
  g_return_if_fail (data);
  
  XDisplay = XOpenDisplay (NULL);
  if (!XDisplay)
  {
    XDisplay = gdk_x11_get_default_xdisplay ();
    if (!XDisplay)
    {
      g_warning ("%s: Unable to get a valid XDisplay", G_STRFUNC);
      return;
    }
    close_display = FALSE;
  }

  XChangeProperty (XDisplay,
		   bamf_legacy_window_get_xid (window),
		   XInternAtom (XDisplay,
				atom_name,
				FALSE),
		   XA_STRING,
		   8,
		   PropModeReplace,
		   (unsigned char *) data,
		   strlen (data));

  if (close_display)
  XCloseDisplay (XDisplay);
}

static char*
window_class_name (BamfLegacyWindow *window)
{
  return g_strdup (bamf_legacy_window_get_class_name (window));
}

static char *
process_exec_string (gint pid)
{
  gchar *result = NULL;
  gint i = 0;
  gchar **argv = NULL;
  GString *exec = NULL;
  glibtop_proc_args buffer;

  if (pid == 0)
    return NULL;

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

  result = g_strdup (exec->str);
  g_string_free (exec, TRUE);
  return result;
}


static char *
process_name (gint pid)
{
  char *stat_path;
  char *contents;
  char **lines;
  char **sections;
  char *result = NULL;
  
  if (pid <= 0)
    return NULL;
  
  stat_path = g_strdup_printf ("/proc/%i/status", pid);
  
  if (g_file_get_contents (stat_path, &contents, NULL, NULL))
    { 
      lines = g_strsplit (contents, "\n", 2);
      
      if (lines && g_strv_length (lines) > 0)
        {
          sections = g_strsplit (lines[0], "\t", 0);
          if (sections && g_strv_length (sections) > 1)
            {
              result = g_strdup (sections[1]);
              g_strfreev (sections);
            }
          g_strfreev (lines);
        }
      g_free (contents);
    }  
  g_free (stat_path);
  
  return result;
}

static GList *
bamf_matcher_possible_applications_for_pid (BamfMatcher *self,
                                           gint pid)
{
  BamfMatcherPrivate *priv;
  GList *result = NULL, *table_list, *l;
  char *proc_name;
  char *exec_string;
  char *trimmed;
  
  g_return_val_if_fail (BAMF_IS_MATCHER (self), NULL);
  
  priv = self->priv;
  
  exec_string  = process_exec_string (pid);
  
  if (exec_string)
    {
      trimmed = trim_exec_string (self, exec_string);
      
      if (trimmed)
        {
          if (strlen (trimmed) > 0)
            {
              table_list = g_hash_table_lookup (priv->desktop_file_table, trimmed);
              
              for (l = table_list; l; l = l->next)
                {
                  result = g_list_prepend (result, g_strdup (l->data));
                }
            }
          g_free (trimmed);
        }
      
      g_free (exec_string);
    }

  if (g_list_length (result) > 0)
    {
      result = g_list_reverse (result);
      return result;
    }
    
  proc_name = process_name (pid);
  if (proc_name)
    {
      table_list = g_hash_table_lookup (priv->desktop_file_table, proc_name);
              
      for (l = table_list; l; l = l->next)
        {
          result = g_list_prepend (result, g_strdup (l->data)); 
        }
      g_free (proc_name);
    }
  
  result = g_list_reverse (result);
  return result;
}

static GList *
bamf_matcher_possible_applications_for_window (BamfMatcher *self,
                                               BamfWindow *bamf_window)
{
  char *hint = NULL;
  BamfMatcherPrivate *priv;
  gint pid;
  BamfLegacyWindow *window;
  GList *desktop_files = NULL, *pid_list = NULL, *l;

  g_return_val_if_fail (BAMF_IS_WINDOW (bamf_window), NULL);
  g_return_val_if_fail (BAMF_IS_MATCHER (self), NULL);

  priv = self->priv;

  window = bamf_window_get_window (bamf_window);

  hint = get_window_hint (self, window, _NET_WM_DESKTOP_FILE);

  if (hint && strlen (hint) > 0)
    {
      desktop_files = g_list_prepend (desktop_files, hint);
      /* whew, hard work, didn't even have to make a copy! */
    }
  else
    {
      char *window_class = window_class_name (window);
      
      char *desktop_file;
      char *desktop_class;
      
      if (window_class)
        {
          char *window_class_down = g_ascii_strdown (g_strdup(window_class), -1);
          l = g_hash_table_lookup (priv->desktop_id_table, window_class_down);
          g_free (window_class_down);

          for (; l; l = l->next)
            {
              desktop_file = l->data;
              if (desktop_file)
                {
                  desktop_class = g_hash_table_lookup (priv->desktop_class_table, desktop_file);
                  if ((desktop_class == NULL || g_strcmp0 (desktop_class, window_class) == 0) &&
                      !g_list_find_custom (desktop_files, desktop_file,
                                           (GCompareFunc) g_strcmp0))
                    {
                      desktop_files = g_list_prepend (desktop_files, g_strdup (desktop_file));
                    }
                }
            }

          desktop_files = g_list_reverse (desktop_files);
        }

      /* Iterate over the desktop class table, and add matching desktop files */
      gpointer key;
      gpointer value;
      GHashTableIter iter;
      g_hash_table_iter_init (&iter, priv->desktop_class_table);

      while (g_hash_table_iter_next (&iter, &key, &value))
        {
          desktop_file = g_strdup (key);
          desktop_class = value;
          if (g_strcmp0 (desktop_class, window_class) == 0 &&
              !g_list_find_custom (desktop_files, desktop_file, (GCompareFunc) g_strcmp0))
            {
              desktop_files = g_list_prepend (desktop_files, desktop_file);
            }
        }

      pid = bamf_legacy_window_get_pid (window);
      pid_list = bamf_matcher_possible_applications_for_pid (self, pid);
      
      /* Append these files to the end to give preference to window_class style picking.
         This style of matching is prefered and used by GNOME Shell however does not work
         very well in practice, thus requiring the fallback here */
      for (l = pid_list; l; l = l->next)
        {
          desktop_file = l->data;
          if (g_list_find_custom (desktop_files, l->data, (GCompareFunc) g_strcmp0))
            g_free (desktop_file);
          else
            {
              if (window_class)
                {
                  desktop_class = g_hash_table_lookup (priv->desktop_class_table, desktop_file);
                  if ((desktop_class == NULL || g_strcmp0 (desktop_class, window_class) == 0) &&
                      !g_list_find_custom (desktop_files, desktop_file,
                                           (GCompareFunc) g_strcmp0))
                    {
                      desktop_files = g_list_append (desktop_files, desktop_file);
                    }
                }
              else
                desktop_files = g_list_append (desktop_files, desktop_file);
            }
        }

      g_free (window_class);
      g_list_free (pid_list);
    }
  
  return desktop_files;
}

static void
bamf_matcher_setup_window_state (BamfMatcher *self,
                                 BamfWindow *bamf_window)
{
  GList *possible_apps, *l;
  BamfLegacyWindow *window;
  GList *views, *a;
  char *desktop_file;
  char *win_class;
  char *app_class;
  BamfApplication *app = NULL, *best = NULL;
  BamfView *view;

  g_return_if_fail (BAMF_IS_MATCHER (self));
  g_return_if_fail (BAMF_IS_WINDOW (bamf_window));

  window = bamf_window_get_window (bamf_window);
  views = self->priv->views;

  possible_apps = bamf_matcher_possible_applications_for_window (self, bamf_window);
  win_class = window_class_name(window);

  /* Loop over every application, inside that application see if its .desktop file
   * matches with any of our possible hits. If so we match it. If we have no possible hits
   * fall back to secondary matching. 
   */
  for (a = views; a; a = a->next)
    {
      view = a->data;

      if (!BAMF_IS_APPLICATION (view))
        continue;

      app = BAMF_APPLICATION (view);
      app_class = bamf_application_get_wmclass (app);
      desktop_file = bamf_application_get_desktop_file (app);

      if (possible_apps)
        {
          /* primary matching */

          for (l = possible_apps; l; l = l->next)
            {
              if (g_strcmp0 (desktop_file, l->data) == 0)
                {
                  if (!best || !g_strcmp0 (win_class, app_class))
                    best = app;
                  break;
                }
            }
        }
      else if (desktop_file == NULL) 
        {
          /* secondary matching */
          
          if (bamf_application_contains_similar_to_window (app, bamf_window) && (!best || !g_strcmp0 (win_class, app_class)))
            best = app;
        }

      g_free (desktop_file);
      g_free (app_class);
    }

  if (!best)
    {
      if (possible_apps)
        best = bamf_application_new_from_desktop_files (possible_apps);
      else
        best = bamf_application_new ();

      bamf_application_set_wmclass (best, win_class);

      bamf_matcher_register_view (self, BAMF_VIEW (best));
      g_object_unref (best);
    }

  g_free (win_class);

  for (l = possible_apps; l; l = l->next)
    {
      char *str = l->data;
      g_free (str);
    }

  g_list_free (possible_apps);

  bamf_view_add_child (BAMF_VIEW (best), BAMF_VIEW (bamf_window));
}

/* Ensures that the window hint is set if a registered pid matches, and that set window hints
   are already known to bamfdaemon */
static void
ensure_window_hint_set (BamfMatcher *self,
                        BamfLegacyWindow *window)
{
  GArray *pids;
  GHashTable *registered_pids;
  char *window_hint = NULL;
  gint i, pid;
  gint *key;

  g_return_if_fail (BAMF_IS_MATCHER (self));
  g_return_if_fail (BAMF_IS_LEGACY_WINDOW (window));

  registered_pids = self->priv->registered_pids;
  window_hint = get_window_hint (self, window, _NET_WM_DESKTOP_FILE);

  if (window_hint && strlen (window_hint) > 0)
    {
      /* already set, make sure we know about this
       * fact for future windows of this applications */
      pid = bamf_legacy_window_get_pid (window);

      if (pid > 0)
        {
          key = g_new (gint, 1);
          *key = pid;

          if (!g_hash_table_lookup (registered_pids, key))
            {
              g_hash_table_insert (registered_pids, key, g_strdup (window_hint));
            }
        }

      g_free (window_hint);
      return;
    }

  if (is_open_office_window (self, window))
    {
      window_hint = get_open_office_window_hint (self, window);
    }
  else
    {
      pids = pid_parent_tree (self, bamf_legacy_window_get_pid (window));

      for (i = 0; i < pids->len; i++)
        {
          pid = g_array_index (pids, gint, i);

          key = g_new (gint, 1);
          *key = pid;

          window_hint = g_hash_table_lookup (registered_pids, key);
          if (window_hint != NULL && strlen (window_hint) > 0)
            break;
        }
      g_array_free (pids, TRUE);
    }

  if (window_hint)
    set_window_hint (self, window, _NET_WM_DESKTOP_FILE, window_hint);
}

static void
handle_raw_window (BamfMatcher *self, BamfLegacyWindow *window)
{
  BamfWindow *bamfwindow;

  g_return_if_fail (BAMF_IS_MATCHER (self));
  g_return_if_fail (BAMF_IS_LEGACY_WINDOW (window));

  gint pid = bamf_legacy_window_get_pid (window);
  if (pid > 1)
    g_array_append_val (self->priv->known_pids, pid);

  ensure_window_hint_set (self, window);

  /* We need to make our objects for bus export, the quickest way to do this, though not
   * always the best is to simply go window by window creating new applications as needed.
   */

  bamfwindow = bamf_window_new (window);
  bamf_matcher_register_view (self, BAMF_VIEW (bamfwindow));
  g_object_unref (bamfwindow);

  bamf_matcher_setup_window_state (self, bamfwindow);
}

static gboolean
open_office_window_setup_timer (OpenOfficeTimeoutArgs *args)
{
  if (bamf_legacy_window_is_closed (args->window))
  {
    g_object_unref (args->window);
    return FALSE;
  }

  args->count++;
  if (args->count > 20 || get_open_office_window_hint (args->matcher, args->window))  
    {
      g_object_unref (args->window);
      handle_raw_window (args->matcher, args->window);
      return FALSE;
    }
  
  return TRUE;
}

static void
handle_window_opened (BamfLegacyScreen * screen, BamfLegacyWindow * window, BamfMatcher *self)
{
  g_return_if_fail (BAMF_IS_MATCHER (self));
  g_return_if_fail (BAMF_IS_LEGACY_WINDOW (window));

  if (bamf_legacy_window_get_window_type (window) == BAMF_WINDOW_DESKTOP)
    {
      BamfWindow *bamfwindow;
      
      bamfwindow = bamf_window_new (window);
      bamf_matcher_register_view (self, BAMF_VIEW (bamfwindow));
      g_object_unref (bamfwindow);
      
      return;    
    }

  if (is_open_office_window (self, window) && get_open_office_window_hint (self, window) == NULL)
    {
      OpenOfficeTimeoutArgs* args = (OpenOfficeTimeoutArgs*) g_malloc0 (sizeof (OpenOfficeTimeoutArgs));
      args->matcher = self;
      args->window = window;
      
      g_object_ref (window);
      /* we have an open office window who is not ready to match yet */
      g_timeout_add (100, (GSourceFunc) open_office_window_setup_timer, args);
    }
  else
    {
      /* we have a window who is ready to be matched */
      handle_raw_window (self, window); 
    }
}

static void
bamf_matcher_setup_indicator_state (BamfMatcher *self, BamfIndicator *indicator)
{
  GList *possible_apps, *l;
  GList *views, *a;
  char *desktop_file;
  BamfApplication *app = NULL, *best = NULL;
  BamfView *view;

  g_return_if_fail (BAMF_IS_MATCHER (self));
  g_return_if_fail (BAMF_IS_INDICATOR (indicator));
  
  views = self->priv->views;

  possible_apps = bamf_matcher_possible_applications_for_pid (self, bamf_indicator_get_pid (indicator));

  /* Loop over every application, inside that application see if its .desktop file
   * matches with any of our possible hits. If so we match it. If we have no possible hits
   * fall back to secondary matching. 
   */
  for (a = views; a && !best; a = a->next)
    {
      view = a->data;

      if (!BAMF_IS_APPLICATION (view))
        continue;

      app = BAMF_APPLICATION (view);
      desktop_file = bamf_application_get_desktop_file (app);
      
      if (possible_apps)
        {
          /* primary matching */

          for (l = possible_apps; l; l = l->next)
            {
              if (g_strcmp0 (desktop_file, l->data) == 0)
                {
                  best = app;
                  break;
                }
            }
        }
        
      g_free (desktop_file);
    }

  if (!best)
    {
      desktop_file = NULL;

      if (possible_apps)
        desktop_file = possible_apps->data;

      if (desktop_file)
        {
          best = bamf_application_new_from_desktop_file (desktop_file);
          bamf_matcher_register_view (self, BAMF_VIEW (best));
          g_object_unref (best);
        }
    }

 for (l = possible_apps; l; l = l->next)
  {
    char *str = l->data;
    g_free (str);
  }


  g_list_free (possible_apps);

  if (best)
    bamf_view_add_child (BAMF_VIEW (best), BAMF_VIEW (indicator));
}

static void
handle_indicator_opened (BamfIndicatorSource *approver, BamfIndicator *indicator, BamfMatcher *self)
{
  g_return_if_fail (BAMF_IS_MATCHER (self));
  g_return_if_fail (BAMF_IS_INDICATOR (indicator));
  
  bamf_matcher_register_view (self, BAMF_VIEW (indicator));
  bamf_matcher_setup_indicator_state (self, indicator);
}

void
bamf_matcher_load_desktop_file (BamfMatcher * self,
                                char * desktop_file)
{
  g_return_if_fail (BAMF_IS_MATCHER (self));

  load_desktop_file_to_table (self,
                              desktop_file,
                              self->priv->desktop_file_table,
                              self->priv->desktop_id_table,
                              self->priv->desktop_class_table);
}

void
bamf_matcher_register_desktop_file_for_pid (BamfMatcher * self,
                                            char * desktopFile,
                                            gint pid)
{
  gint *key;
  BamfLegacyScreen *screen;
  GList *glist_item;
  GList *windows;
  char *dup;

  g_return_if_fail (BAMF_IS_MATCHER (self));
  g_return_if_fail (desktopFile);

  key = g_new (gint, 1);
  *key = pid;

  dup = g_strdup (desktopFile);
  g_hash_table_insert (self->priv->registered_pids, key, dup);

  /* fixme, this is a bit heavy */

  screen = bamf_legacy_screen_get_default ();

  g_return_if_fail (BAMF_IS_LEGACY_SCREEN (screen));

  windows = bamf_legacy_screen_get_windows (screen);

  for (glist_item = windows; glist_item != NULL;
       glist_item = glist_item->next)
    {
      ensure_window_hint_set (self, glist_item->data);
    }
}

static int
x_error_handler (Display *display, XErrorEvent *event)
{
  /* We hardly care, this means one of our get or set property calls didn't work
   * while this kind of sucks, it wont effect the running of the program except to
   * perhaps reduce the efficiency of the application
   */
  return 0;
}

char *
bamf_matcher_get_active_application (BamfMatcher *matcher)
{
  GList *l;
  BamfView *view;
  BamfMatcherPrivate *priv;

  g_return_val_if_fail (BAMF_IS_MATCHER (matcher), NULL);

  priv = matcher->priv;

  for (l = priv->views; l; l = l->next)
    {
      view = l->data;

      if (!BAMF_IS_APPLICATION (view))
        continue;

      if (bamf_view_is_active (view))
        {
          return g_strdup (bamf_view_get_path (view));
        }
    }

  return NULL;
}

char *
bamf_matcher_get_active_window (BamfMatcher *matcher)
{
  GList *l;
  BamfView *view;
  BamfMatcherPrivate *priv;

  g_return_val_if_fail (BAMF_IS_MATCHER (matcher), NULL);

  priv = matcher->priv;

  for (l = priv->views; l; l = l->next)
    {
      view = l->data;

      if (!BAMF_IS_WINDOW (view))
        continue;

      if (bamf_view_is_active (view))
        {
          return g_strdup (bamf_view_get_path (view));
        }
    }

  return NULL;
}

char *
bamf_matcher_application_for_xid (BamfMatcher *matcher,
                                  guint32 xid)
{
  GList *l;
  BamfView *view;
  BamfMatcherPrivate *priv;

  g_return_val_if_fail (BAMF_IS_MATCHER (matcher), NULL);

  priv = matcher->priv;

  for (l = priv->views; l; l = l->next)
    {
      view = l->data;

      if (!BAMF_IS_APPLICATION (view))
        continue;

      if (bamf_application_manages_xid (BAMF_APPLICATION (view), xid))
        {
          return g_strdup (bamf_view_get_path (view));
        }
    }

  return NULL;
}

gboolean
bamf_matcher_application_is_running (BamfMatcher *matcher,
                                     char *application)
{
  char * desktop_file;
  GList *l;
  BamfView *view;
  BamfMatcherPrivate *priv;

  g_return_val_if_fail (BAMF_IS_MATCHER (matcher), FALSE);

  priv = matcher->priv;

  for (l = priv->views; l; l = l->next)
    {
      view = l->data;

      if (!BAMF_IS_APPLICATION (view))
        continue;

      desktop_file = bamf_application_get_desktop_file (BAMF_APPLICATION (view));
      if (g_strcmp0 (desktop_file, application) == 0)
        {
          g_free (desktop_file);
          return bamf_view_is_running (view);
        }
      g_free (desktop_file);
    }

  return FALSE;
}

char **
bamf_matcher_window_dbus_paths (BamfMatcher *matcher)
{
  char ** paths;
  int i;
  GList *l;
  BamfView *view;
  BamfMatcherPrivate *priv;

  g_return_val_if_fail (BAMF_IS_MATCHER (matcher), NULL);

  priv = matcher->priv;

  paths = g_malloc0 (sizeof (char *) * (g_list_length (priv->views) + 1));

  i = 0;
  for (l = priv->views; l; l = l->next)
    {
      view = l->data;

      if (!BAMF_IS_WINDOW (view))
        continue;

      paths[i] = g_strdup (bamf_view_get_path (view));
      i++;
    }

  return paths;
}

char **
bamf_matcher_application_dbus_paths (BamfMatcher *matcher)
{
  char ** paths;
  int i;
  GList *l;
  BamfView *view;
  BamfMatcherPrivate *priv;

  g_return_val_if_fail (BAMF_IS_MATCHER (matcher), NULL);

  priv = matcher->priv;

  paths = g_malloc0 (sizeof (char *) * (g_list_length (priv->views) + 1));

  i = 0;
  for (l = priv->views; l; l = l->next)
    {
      view = l->data;

      if (!BAMF_IS_APPLICATION (view))
        continue;

      paths[i] = g_strdup (bamf_view_get_path (view));
      i++;
    }

  return paths;
}

char *
bamf_matcher_dbus_path_for_application (BamfMatcher *matcher,
                                        char *application)
{
  char * path = NULL;
  char * desktop_file;
  GList *l;
  BamfView *view;
  BamfMatcherPrivate *priv;

  g_return_val_if_fail (BAMF_IS_MATCHER (matcher), NULL);

  priv = matcher->priv;

  for (l = priv->views; l; l = l->next)
    {
      view = l->data;

      if (!BAMF_IS_APPLICATION (view))
        continue;

      desktop_file = bamf_application_get_desktop_file (BAMF_APPLICATION (view));
      if (g_strcmp0 (desktop_file, application) == 0)
        {
          path = g_strdup (bamf_view_get_path (view));
        }
      g_free (desktop_file);
    }

  return path;
}

GList *
bamf_matcher_get_favorites (BamfMatcher *matcher)
{
  g_return_val_if_fail (BAMF_IS_MATCHER (matcher), NULL);
  
  return matcher->priv->favorites;
}

gboolean
bamf_matcher_register_favorites (BamfMatcher *matcher,
                                 char **favorites,
                                 GError *error)
{
  char *fav;
  char **favs;
  BamfMatcherPrivate *priv;
  
  g_return_val_if_fail (BAMF_IS_MATCHER (matcher), TRUE);
  g_return_val_if_fail (favorites, TRUE);
  
  priv = matcher->priv;
  
  for (favs = favorites; *favs; favs++)
    {
      fav = *favs;
      /* ignore things already in the list */
      if (g_list_find_custom (priv->favorites, fav, (GCompareFunc) g_strcmp0))
        continue;
      
      
      priv->favorites = g_list_prepend (priv->favorites, g_strdup (fav));
      bamf_matcher_load_desktop_file (matcher, fav);
    }
  
  g_signal_emit (matcher, matcher_signals[FAVORITES_CHANGED], 0);

  return TRUE;
}

char **
bamf_matcher_running_application_paths (BamfMatcher *matcher)
{
  char ** paths;
  int i;
  GList *l;
  BamfView *view;
  BamfMatcherPrivate *priv;

  g_return_val_if_fail (BAMF_IS_MATCHER (matcher), NULL);

  priv = matcher->priv;

  paths = g_malloc0 (sizeof (char *) * (g_list_length (priv->views) + 1));

  i = 0;
  for (l = priv->views; l; l = l->next)
    {
      view = l->data;

      if (!BAMF_IS_APPLICATION (view) || !bamf_view_is_running (view))
        continue;

      paths[i] = g_strdup (bamf_view_get_path (view));
      i++;
    }

  return paths;
}

char **
bamf_matcher_tab_dbus_paths (BamfMatcher *matcher)
{
  return NULL;
}

GArray *
bamf_matcher_xids_for_application (BamfMatcher *matcher,
                                   char *application)
{
  GArray * xids = NULL;
  char * desktop_file;
  GList *l;
  BamfView *view;
  BamfMatcherPrivate *priv;

  g_return_val_if_fail (BAMF_IS_MATCHER (matcher), NULL);

  priv = matcher->priv;

  for (l = priv->views; l; l = l->next)
    {
      view = l->data;

      if (!BAMF_IS_APPLICATION (view))
        continue;

      desktop_file = bamf_application_get_desktop_file (BAMF_APPLICATION (view));
      if (g_strcmp0 (desktop_file, application) == 0)
        {
          xids = bamf_application_get_xids (BAMF_APPLICATION (view));
          g_free (desktop_file);
          break;
        }
      g_free (desktop_file);
    }

  return xids;
}

static void
bamf_matcher_init (BamfMatcher * self)
{
  GArray *prefixstrings;
  int i;
  DBusGConnection *bus;
  GError *error = NULL;
  BamfMatcherPrivate *priv;
  BamfLegacyScreen *screen;
  BamfIndicatorSource *approver;

  priv = self->priv = BAMF_MATCHER_GET_PRIVATE (self);

  priv->known_pids = g_array_new (FALSE, TRUE, sizeof (gint));
  priv->bad_prefixes = g_array_new (FALSE, TRUE, sizeof (GRegex *));
  priv->registered_pids =
    g_hash_table_new ((GHashFunc) g_int_hash, (GEqualFunc) g_int_equal);

  prefixstrings = prefix_strings (self);
  for (i = 0; i < prefixstrings->len; i++)
    {
      char *str = g_array_index (prefixstrings, char *, i);
      GRegex *regex = g_regex_new (str, G_REGEX_OPTIMIZE, 0, NULL);
      g_array_append_val (priv->bad_prefixes, regex);
    }

  g_array_free (prefixstrings, TRUE);

  create_desktop_file_table (self, &(priv->desktop_file_table),
                             &(priv->desktop_id_table),
                             &(priv->desktop_class_table));

  screen = bamf_legacy_screen_get_default ();
  g_signal_connect (G_OBJECT (screen), "window-opened",
                    (GCallback) handle_window_opened, self);

  approver = bamf_indicator_source_get_default ();
  g_signal_connect (G_OBJECT (approver), "indicator-opened",
                    (GCallback) handle_indicator_opened, self);

  XSetErrorHandler (x_error_handler);

  bus = dbus_g_bus_get (DBUS_BUS_SESSION, &error);

  g_return_if_fail (bus);

  dbus_g_connection_register_g_object (bus, BAMF_MATCHER_PATH, G_OBJECT (self));
}

static void
bamf_matcher_class_init (BamfMatcherClass * klass)
{
  g_type_class_add_private (klass, sizeof (BamfMatcherPrivate));

  dbus_g_object_type_install_info (BAMF_TYPE_MATCHER,
				   &dbus_glib_bamf_matcher_object_info);

  matcher_signals [VIEW_OPENED] =
  	g_signal_new ("view-opened",
  	              G_OBJECT_CLASS_TYPE (klass),
  	              0,
  	              0, NULL, NULL,
  	              bamf_marshal_VOID__STRING_STRING,
  	              G_TYPE_NONE, 2,
  	              G_TYPE_STRING, G_TYPE_STRING);

  matcher_signals [VIEW_CLOSED] =
  	g_signal_new ("view-closed",
  	              G_OBJECT_CLASS_TYPE (klass),
  	              0,
  	              0, NULL, NULL,
  	              bamf_marshal_VOID__STRING_STRING,
  	              G_TYPE_NONE, 2,
  	              G_TYPE_STRING, G_TYPE_STRING);

  matcher_signals [ACTIVE_APPLICATION_CHANGED] =
  	g_signal_new ("active-application-changed",
  	              G_OBJECT_CLASS_TYPE (klass),
  	              0,
  	              0, NULL, NULL,
  	              bamf_marshal_VOID__STRING_STRING,
  	              G_TYPE_NONE, 2,
  	              G_TYPE_STRING, G_TYPE_STRING);

  matcher_signals [ACTIVE_WINDOW_CHANGED] =
  	g_signal_new ("active-window-changed",
  	              G_OBJECT_CLASS_TYPE (klass),
  	              0,
  	              0, NULL, NULL,
  	              bamf_marshal_VOID__STRING_STRING,
  	              G_TYPE_NONE, 2,
  	              G_TYPE_STRING, G_TYPE_STRING);
  	              
  matcher_signals [FAVORITES_CHANGED] =
  	g_signal_new ("favorites-changed",
  	              G_OBJECT_CLASS_TYPE (klass),
  	              0,
  	              0, NULL, NULL,
  	              g_cclosure_marshal_VOID__VOID,
  	              G_TYPE_NONE, 0);
}

BamfMatcher *
bamf_matcher_get_default (void)
{
  static BamfMatcher *matcher;

  if (!BAMF_IS_MATCHER (matcher))
    {
      matcher = (BamfMatcher *) g_object_new (BAMF_TYPE_MATCHER, NULL);
      return matcher;
    }

  return g_object_ref (G_OBJECT (matcher));
}
