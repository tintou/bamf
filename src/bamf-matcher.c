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


#include "bamf-marshal.h"
#include "bamf-matcher.h"
#include "bamf-matcher-glue.h"
#include "bamf-application.h"
#include "bamf-window.h"
#include "bamf-legacy-window.h"
#include "bamf-legacy-window-test.h"
#include "bamf-legacy-screen.h"

G_DEFINE_TYPE (BamfMatcher, bamf_matcher, G_TYPE_OBJECT);
#define BAMF_MATCHER_GET_PRIVATE(obj) (G_TYPE_INSTANCE_GET_PRIVATE(obj, \
BAMF_TYPE_MATCHER, BamfMatcherPrivate))

enum
{
  VIEW_OPENED,
  VIEW_CLOSED,
  ACTIVE_APPLICATION_CHANGED,
  ACTIVE_WINDOW_CHANGED,

  LAST_SIGNAL,
};

static guint matcher_signals[LAST_SIGNAL] = { 0 };

struct _BamfMatcherPrivate
{
  GArray          * bad_prefixes;
  GArray          * known_pids;
  GHashTable      * desktop_id_table;
  GHashTable      * desktop_file_table;
  GHashTable      * exec_list;
  GHashTable      * registered_pids;
  GList           * views;
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

  g_signal_connect (G_OBJECT (view), "closed",
	      	    (GCallback) on_view_closed, self);
  g_signal_connect (G_OBJECT (view), "active-changed",
                    (GCallback) on_view_active_changed, self);

  self->priv->views = g_list_prepend (self->priv->views, view);

  g_signal_emit (self, matcher_signals[VIEW_OPENED],0, path, type);

  g_free (type);
}

static void
bamf_matcher_unregister_view (BamfMatcher *self, BamfView *view)
{
  char * path, * type;

  path = bamf_view_get_path (view);
  type = bamf_view_get_view_type (view);

  g_signal_emit (self, matcher_signals[VIEW_CLOSED],0, path, type);

  g_signal_handlers_disconnect_by_func (G_OBJECT (view), on_view_closed, self);
  g_signal_handlers_disconnect_by_func (G_OBJECT (view), on_view_active_changed, self);

  self->priv->views = g_list_remove (self->priv->views, view);

  g_free (type);
}

/******** OLD MATCHER CODE **********/

static char *
get_open_office_window_hint (BamfMatcher * self, BamfLegacyWindow * window)
{
  const gchar *name;
  char *exec;
  GHashTable *desktopFileTable;
  char *file;

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
  else
    {
      return NULL;
    }

  desktopFileTable = self->priv->desktop_file_table;
  file = g_hash_table_lookup (desktopFileTable, exec);

  g_return_val_if_fail (file, NULL);

  return file;
}

/* Attempts to return the binary name for a particular execution string */
static char *
process_exec_string (BamfMatcher * self, char * execString)
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
    result = g_strdup (execString);

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
  return !(g_str_has_prefix (exec, "chromium-browser") &&
           g_strrstr (exec, "--app")) &&
         !g_str_has_prefix (exec, "ooffice");
}

static void
load_desktop_file_to_table (BamfMatcher * self,
                            const char *file,
                            GHashTable *desktop_file_table,
                            GHashTable *desktop_id_table)
{
  GAppInfo *desktop_file;
  char *exec;
  char *existing;
  char *path;
  GString *tmp;
  GString *desktop_id; /* is ok */

  g_return_if_fail (BAMF_IS_MATCHER (self));

  desktop_file = G_APP_INFO (g_desktop_app_info_new_from_filename (file));

  exec = g_strdup (g_app_info_get_commandline (desktop_file));

  g_object_unref (desktop_file);

  if (exec_string_should_be_processed (self, exec))
    {
      /**
       * Set of nasty hacks which should be removed some day. We wish to keep the full exec
       * strings so we can do nasty matching hacks later. A very very evil thing indeed. However this
       * helps hack around applications that run in the same process cross radically different instances.
       * A better solution needs to be thought up, however at this time it is not known.
       **/
      char *tmp = process_exec_string (self, exec);
      g_free (exec);
      exec = tmp;
    }

  path = g_path_get_basename (file);
  desktop_id = g_string_new (path);
  g_free (path);

  desktop_id = g_string_truncate (desktop_id, desktop_id->len - 8); /* remove last 8 characters for .desktop */

  existing = g_hash_table_lookup (desktop_file_table, exec);

  if (existing)
    {
      path = g_path_get_basename (existing);
      tmp = g_string_new (path);
      g_free (path);

      g_string_truncate (tmp, tmp->len - 8); /* remove last 8 characters for .desktop */

      if (g_strcmp0 (tmp->str, exec) == 0)
        {
          /* we prefer to have the desktop file where the desktop-id == exec */
          g_string_free (desktop_id, TRUE);
          g_string_free (tmp, TRUE);
          return;
        }
      g_string_free (tmp, TRUE);
    }

  path = g_strdup (file);
  g_hash_table_insert (desktop_file_table, g_strdup (exec), path);
  g_hash_table_insert (desktop_id_table, g_strdup (desktop_id->str), path);

  g_free (exec);
  g_string_free (desktop_id, TRUE);
}

static void
load_directory_to_table (BamfMatcher * self,
                         const char *directory,
                         GHashTable *desktop_file_table,
                         GHashTable *desktop_id_table)
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
                                    desktop_id_table);

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
                          GHashTable *desktop_id_table)
{
  GFile *file;
  GFileInputStream *stream;
  GDataInputStream *input;
  char *line;
  char *directory;
  char *path;
  char *filestr;
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
      char *exec, *existing;
      GString *desktop_id, *tmp;
      GString *filename;

      gchar **parts = g_strsplit (line, "\t", 3);

      exec = parts[1];

      if (exec_string_should_be_processed (self, exec))
        {
          char *tmp = process_exec_string (self, exec);
          exec = tmp;
        }

      char *name = g_build_filename (directory, parts[0], NULL);
      filename = g_string_new (name);
      g_free ((gpointer) name);

      desktop_id = g_string_new (parts[0]);
      g_string_truncate (desktop_id, desktop_id->len - 8);

      existing = g_hash_table_lookup (desktop_file_table, exec);

      if (existing)
        {
          path = g_path_get_basename (existing);
          tmp = g_string_new (path);
          g_free (path);
          tmp = g_string_truncate (tmp, tmp->len - 8); /* remove last 8 characters for .desktop */

          if (g_strcmp0 (tmp->str, exec) == 0)
            {
              /* we prefer to have the desktop file where the desktop-id == exec */
              g_string_free (filename, TRUE);
              g_string_free (desktop_id, TRUE);
              continue;
            }
          g_string_free (tmp, TRUE);
        }

      filestr = g_strdup (filename->str);
      g_hash_table_insert (desktop_file_table, g_strdup (exec), filestr);
      g_hash_table_insert (desktop_id_table, g_strdup (desktop_id->str), filestr);

      g_string_free (desktop_id, TRUE);

      length = 0;
      g_strfreev (parts);
    }
  g_free ((gpointer) directory);
}

static void
create_desktop_file_table (BamfMatcher * self, GHashTable **desktop_file_table, GHashTable **desktop_id_table)
{
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

  g_return_if_fail (BAMF_IS_MATCHER (self));

  const char *directories[] = { "/usr/share/applications",
                                "/usr/local/share/applications",
                                g_build_filename (g_get_home_dir (), ".local/share/applications", NULL),
                                NULL };
  const char **directory;
  for (directory = directories; *directory; directory++)
    {
      if (!g_file_test (*directory, G_FILE_TEST_IS_DIR))
        continue;

      const char *bamf_file = g_build_filename (*directory, "bamf.index", NULL);

      if (g_file_test (bamf_file, G_FILE_TEST_EXISTS))
        {
          load_index_file_to_table (self, bamf_file, *desktop_file_table, *desktop_id_table);
        }
      else
        {
          load_directory_to_table (self, *directory, *desktop_file_table, *desktop_id_table);
        }

      g_free ((gpointer) bamf_file);
    }
}

static gboolean
is_open_office_window (BamfMatcher * self, BamfLegacyWindow * window)
{
  return g_str_has_prefix (bamf_legacy_window_get_class_name (window), "OpenOffice");
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

  g_return_val_if_fail (BAMF_IS_MATCHER (self), NULL);
  g_return_val_if_fail (BAMF_IS_LEGACY_WINDOW (window), NULL);
  g_return_val_if_fail (atom_name, NULL);

  XDisplay = XOpenDisplay (NULL);
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

  g_return_if_fail (BAMF_IS_MATCHER (self));
  g_return_if_fail (BAMF_LEGACY_WINDOW (window));
  g_return_if_fail (atom_name);
  g_return_if_fail (data);

  XDisplay = XOpenDisplay (NULL);

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
  XCloseDisplay (XDisplay);
}

static char*
window_class_name (BamfLegacyWindow *window)
{
  return g_strdup (bamf_legacy_window_get_class_name (window));
}

static GArray *
bamf_matcher_possible_applications_for_window (BamfMatcher *self,
                                               BamfWindow *bamf_window)
{
  char *hint = NULL, *chrome_app_url = NULL, *file = NULL, *exec = NULL;
  BamfMatcherPrivate *priv;
  BamfLegacyWindow *window;
  GArray *desktop_files = NULL;
  GHashTableIter iter;
  gpointer key, value;

  g_return_val_if_fail (BAMF_IS_WINDOW (bamf_window), NULL);
  g_return_val_if_fail (BAMF_IS_MATCHER (self), NULL);

  priv = self->priv;

  desktop_files =  g_array_new (FALSE, TRUE, sizeof (char*));
  window = bamf_window_get_window (bamf_window);

  hint = get_window_hint (self, window, _NET_WM_DESKTOP_FILE);
  chrome_app_url = get_window_hint (self, window, _CHROME_APP_URL);
  if (chrome_app_url && strlen (chrome_app_url) > 2)
    {
      /* We have a chrome app, lets find its desktop file */

      /* We need to replace _/ with / as chromium will signal the "base name" of the url
       * by placing a _ after it. So www.google.com/reader becomes www.google.com_/reader to
       * chromium. We are restoring the original URL.
       */
      char *tmp = chrome_app_url;
      GRegex *regex = g_regex_new ("_/", 0, 0, NULL);

      chrome_app_url = g_regex_replace_literal (regex,
                                                chrome_app_url,
                                                -1, 0,
                                                 "/",
                                                 0, NULL);

      g_free (tmp);
      g_regex_unref (regex);

      /* remove trailing slash */
      if (g_str_has_suffix (chrome_app_url, "/"))
        chrome_app_url = g_strndup (chrome_app_url, strlen (chrome_app_url) - 1); /* LEAK */

      /* Do a slow iteration over every .desktop file in our hash table, we are looking
       * for a chromium instance which contains --app and chrome_app_url
       */

      regex = g_regex_new (g_strdup_printf ("chromium-browser (.* )?--app( |=)\"?(http://|https://)?%s/?\"?( |$)",
                                            chrome_app_url), 0, 0, NULL);

      g_hash_table_iter_init (&iter, priv->desktop_file_table);
      while (g_hash_table_iter_next (&iter, &key, &value))
        {
          char *exec_line = (char*) key;
          char *file_path = (char*) value;

          if (g_regex_match (regex, exec_line, 0, NULL))
            {
              /* We have found a match */
              file = g_strdup (file_path);
              g_array_append_val (desktop_files, file);
            }
        }

      g_regex_unref (regex);
    }
  else if (hint && strlen (hint) > 0)
    {
      /* We have a window that has a hint that is NOT a chrome app */
      g_array_append_val (desktop_files, hint);
      /* whew, hard work, didn't even have to make a copy! */
    }
  else
    {
      char *class_name = window_class_name (window);

      if (class_name)
        {
          class_name = g_ascii_strdown (class_name, -1);
          file = g_hash_table_lookup (priv->desktop_id_table, class_name);

          if (file)
            {
              file = g_strdup (file);
              g_array_append_val (desktop_files, file);
            }

         g_free (class_name);
       }

      /* Make a fracking guess */
      exec = bamf_legacy_window_get_exec_string (window);

      if (exec)
        {
          char *tmp = process_exec_string (self, exec);
          g_free (exec);
          exec = tmp;
        }

      if (exec && strlen (exec) > 0)
        {
          file = g_hash_table_lookup (priv->desktop_file_table, exec);
          if (file)
            {
              /* Make sure we make a copy of the string */
              file = g_strdup (file);
              g_array_append_val (desktop_files, file);
            }

          //g_free (exec);
        }
      /* sub-optimal guesswork ends */
    }

  return desktop_files;
}

static void
bamf_matcher_setup_window_state (BamfMatcher *self,
                                 BamfWindow *bamf_window)
{
  GArray *possible_apps;
  BamfLegacyWindow *window;
  GList *views, *a;
  char *desktop_file;
  int i;
  BamfApplication *app = NULL, *best = NULL;
  BamfView *view;

  g_return_if_fail (BAMF_IS_MATCHER (self));
  g_return_if_fail (BAMF_IS_WINDOW (bamf_window));

  window = bamf_window_get_window (bamf_window);
  views = self->priv->views;

  possible_apps = bamf_matcher_possible_applications_for_window (self, bamf_window);

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
      
      if (possible_apps->len > 0)
        {
          /* primary matching */

          for (i = 0; i < possible_apps->len; i++)
            {
              if (g_strcmp0 (desktop_file, g_array_index (possible_apps, char *, i)) == 0)
                {
                  best = app;
                  break;
                }
            }
        }
      else if (desktop_file == NULL) 
        {
          /* secondary matching */
          
          if (bamf_application_contains_similar_to_window (app, bamf_window))
            best = app;
        }
        
      g_free (desktop_file);
    }

  if (!best)
    {
      desktop_file = NULL;

      if (possible_apps->len > 0)
        desktop_file = g_array_index (possible_apps, char *, 0);

      if (desktop_file)
        best = bamf_application_new_from_desktop_file (desktop_file);
      else
        best = bamf_application_new ();

      bamf_matcher_register_view (self, BAMF_VIEW (best));
    }

 for (i = 0; i < possible_apps->len; i++)
  {
    char *str = g_array_index (possible_apps, char *, i);
    g_free (str);
  }


  g_array_free (possible_apps, TRUE);

  bamf_view_add_child (BAMF_VIEW (best), BAMF_VIEW (bamf_window));
}

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

gboolean
bamf_matcher_window_is_match_ready (BamfMatcher * self,
		                    BamfLegacyWindow * window)
{
  char *file;

  if (!is_open_office_window (self, window))
    return TRUE;

  file = get_open_office_window_hint (self, window);

  if (file)
    {
      g_free (file);
      return TRUE;
    }

  return FALSE;
}

static void
handle_window_opened (BamfLegacyScreen * screen, BamfLegacyWindow * window, gpointer data)
{
  BamfWindow *bamfwindow;
  BamfMatcher *self;
  self = (BamfMatcher *) data;

  g_return_if_fail (BAMF_IS_MATCHER (self));
  g_return_if_fail (BAMF_IS_LEGACY_WINDOW (window));

  if (bamf_legacy_window_is_desktop_window (window))
    return;

  gint pid = bamf_legacy_window_get_pid (window);
  if (pid > 1)
    g_array_append_val (self->priv->known_pids, pid);

  ensure_window_hint_set (self, window);

  /* We need to make our objects for bus export, the quickest way to do this, though not
   * always the best is to simply go window by window creating new applications as needed.
   */

  bamfwindow = bamf_window_new (window);
  bamf_matcher_register_view (self, BAMF_VIEW (bamfwindow));

  bamf_matcher_setup_window_state (self, bamfwindow);
}

void
bamf_matcher_load_desktop_file (BamfMatcher * self,
                                char * desktop_file)
{
  g_return_if_fail (BAMF_IS_MATCHER (self));

  load_desktop_file_to_table (self,
                              desktop_file,
                              self->priv->desktop_file_table,
                              self->priv->desktop_id_table);
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
/******** END OLD MATCHER *********/

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
      if (bamf_application_manages_xid (BAMF_APPLICATION (view), xid))
        {
          return desktop_file;
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

          g_print ("%s\n", desktop_file);
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

  create_desktop_file_table (self, &(priv->desktop_file_table), &(priv->desktop_id_table));

  BamfLegacyScreen *screen = bamf_legacy_screen_get_default ();

  g_signal_connect (G_OBJECT (screen), "window-opened",
		    (GCallback) handle_window_opened, self);

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
