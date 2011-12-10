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

#include "bamf-legacy-screen.h"
#include "bamf-legacy-window-test.h"
#include <gio/gio.h>

G_DEFINE_TYPE (BamfLegacyScreen, bamf_legacy_screen, G_TYPE_OBJECT);
#define BAMF_LEGACY_SCREEN_GET_PRIVATE(obj) (G_TYPE_INSTANCE_GET_PRIVATE(obj, \
BAMF_TYPE_LEGACY_SCREEN, BamfLegacyScreenPrivate))

enum
{
  WINDOW_OPENED,
  WINDOW_CLOSED,
  ACTIVE_WINDOW_CHANGED,

  LAST_SIGNAL,
};

static guint legacy_screen_signals[LAST_SIGNAL] = { 0 };

struct _BamfLegacyScreenPrivate
{
  WnckScreen * legacy_screen;
  GList *windows;
  GFile *file;
  GDataInputStream *stream;
};

static void
handle_window_closed (BamfLegacyWindow *window, BamfLegacyScreen *self)
{
  self->priv->windows = g_list_remove (self->priv->windows, window);

  g_signal_emit (self, legacy_screen_signals[WINDOW_CLOSED], 0, window);

  g_object_unref (window);
}

static gboolean
on_state_file_load_timeout (BamfLegacyScreen *self)
{
  BamfLegacyWindow *window;
  GDataInputStream *stream;
  gchar *line, *name, *class, *exec;
  GList *l;
  gchar **parts;
  guint32 xid;

  g_return_val_if_fail (BAMF_IS_LEGACY_SCREEN (self), FALSE);

  stream = self->priv->stream;

  line = g_data_input_stream_read_line (stream, NULL, NULL, NULL);

  if (!line)
    return FALSE;

  // Line format:
  // open	<xid> 	<name>	<wmclass> <exec>
  // close	<xid>
  // attention	<xid>	<true/false>
  // skip	<xid>	<true/false>

  parts = g_strsplit (line, "\t", 0);
  g_free (line);

  xid = (guint32) atol (parts[1]);
  if (g_strcmp0 (parts[0], "open") == 0)
    {
      name  = parts[2];
      class = parts[3];
      exec  = parts[4];

      window = BAMF_LEGACY_WINDOW (bamf_legacy_window_test_new (xid, name, class, exec));
      self->priv->windows = g_list_prepend (self->priv->windows, window);

      g_signal_connect (G_OBJECT (window), "closed",
                        (GCallback) handle_window_closed, self);

      g_signal_emit (self, legacy_screen_signals[WINDOW_OPENED], 0, window);
    }
  else if (g_strcmp0 (parts[0], "close") == 0)
    {
      for (l = self->priv->windows; l; l = l->next)
        {
          window = l->data;
          if (bamf_legacy_window_get_xid (window) == xid)
            {
              bamf_legacy_window_test_close (BAMF_LEGACY_WINDOW_TEST (window));
              break;
            }
        }
    }
  else if (g_strcmp0 (parts[0], "attention") == 0)
    {
      gboolean attention = FALSE;
      if (g_strcmp0 (parts[2], "true") == 0)
        attention = TRUE;
      else if (g_strcmp0 (parts[2], "false") == 0)
        attention = FALSE;
      else
        return TRUE;

      for (l = self->priv->windows; l; l = l->next)
        {
          if (bamf_legacy_window_get_xid (l->data) == xid)
            {
              bamf_legacy_window_test_set_attention (l->data, attention);
              break;
            }
        }
    }
  else if (g_strcmp0 (parts[0], "skip") == 0)
    {
      gboolean skip = FALSE;
      if (g_strcmp0 (parts[2], "true") == 0)
        skip = TRUE;
      else if (g_strcmp0 (parts[2], "false") == 0)
        skip = FALSE;
      else
        return TRUE;

      for (l = self->priv->windows; l; l = l->next)
        {
          if (bamf_legacy_window_get_xid (l->data) == xid)
            {
              bamf_legacy_window_test_set_skip (l->data, skip);
              break;
            }
        }
    }
  else
    {
      g_warning ("Could not parse line\n");
    }

  g_strfreev (parts);
  return TRUE;
}

static void
handle_window_opened (WnckScreen *screen, WnckWindow *window, BamfLegacyScreen *legacy)
{
  BamfLegacyWindow *legacy_window;
  g_return_if_fail (WNCK_IS_WINDOW (window));

  legacy_window = bamf_legacy_window_new (window);

  g_signal_connect (G_OBJECT (legacy_window), "closed",
                    (GCallback) handle_window_closed, legacy);

  legacy->priv->windows = g_list_prepend (legacy->priv->windows, legacy_window);

  g_signal_emit (legacy, legacy_screen_signals[WINDOW_OPENED], 0, legacy_window);
}

void
bamf_legacy_screen_inject_window (BamfLegacyScreen *self, guint xid)
{
  g_return_if_fail (BAMF_IS_LEGACY_SCREEN (self));
  BamfLegacyWindow *window;
  GList *l;

  for (l = self->priv->windows; l; l = l->next)
    {
      window = l->data;

      if (bamf_legacy_window_get_xid (window) == xid)
        {
          return;
        }
    }

  WnckWindow *legacy_window = wnck_window_get(xid);

  if (WNCK_IS_WINDOW (legacy_window))
    {
      handle_window_opened(NULL, legacy_window, self);
    }
}

void
bamf_legacy_screen_set_state_file (BamfLegacyScreen *self,
                                   const char *file)
{
  GFile *gfile;
  GDataInputStream *stream;

  g_return_if_fail (BAMF_IS_LEGACY_SCREEN (self));

  // Disconnect our handlers so we can work purely on the file
  g_signal_handlers_disconnect_by_func (self->priv->legacy_screen, handle_window_opened, self);
  g_signal_handlers_disconnect_by_func (self->priv->legacy_screen, handle_window_closed, self);

  gfile = g_file_new_for_path (file);

  if (!file)
    {
      g_error ("Could not open file %s", file);
    }

  stream = g_data_input_stream_new (G_INPUT_STREAM (g_file_read (gfile, NULL, NULL)));

  if (!stream)
    {
      g_error ("Could not open file stream for %s", file);
    }

  self->priv->file = gfile;
  self->priv->stream = stream;

  g_timeout_add (500, (GSourceFunc) on_state_file_load_timeout, self);
}

GList *
bamf_legacy_screen_get_windows (BamfLegacyScreen *screen)
{
  g_return_val_if_fail (BAMF_IS_LEGACY_SCREEN (screen), NULL);

  return screen->priv->windows;
}

BamfLegacyWindow *
bamf_legacy_screen_get_active_window (BamfLegacyScreen *screen)
{
  BamfLegacyWindow *window;
  GList *l;

  g_return_val_if_fail (BAMF_IS_LEGACY_SCREEN (screen), NULL);

  for (l = screen->priv->windows; l; l = l->next)
    {
      window = l->data;

      if (bamf_legacy_window_is_active (window))
        return window;
    }

  return NULL;
}

static void
handle_active_window_changed (WnckScreen *screen, WnckWindow *previous, BamfLegacyScreen *self)
{
  g_return_if_fail (BAMF_IS_LEGACY_SCREEN (self));

  g_signal_emit (self, legacy_screen_signals[ACTIVE_WINDOW_CHANGED], 0);
}

static void
bamf_legacy_screen_dispose (GObject *object)
{
  G_OBJECT_CLASS (bamf_legacy_screen_parent_class)->dispose (object);
}

static void
bamf_legacy_screen_init (BamfLegacyScreen * self)
{
  self->priv = BAMF_LEGACY_SCREEN_GET_PRIVATE (self);
}

static void
bamf_legacy_screen_class_init (BamfLegacyScreenClass * klass)
{
  GObjectClass *object_class = G_OBJECT_CLASS (klass);

  object_class->dispose      = bamf_legacy_screen_dispose;

  g_type_class_add_private (klass, sizeof (BamfLegacyScreenPrivate));

  legacy_screen_signals [WINDOW_OPENED] =
  	g_signal_new ("window-opened",
  	              G_OBJECT_CLASS_TYPE (klass),
  	              G_SIGNAL_RUN_FIRST,
  	              G_STRUCT_OFFSET (BamfLegacyScreenClass, window_opened),
  	              NULL, NULL,
  	              g_cclosure_marshal_VOID__OBJECT,
  	              G_TYPE_NONE, 1,
  	              BAMF_TYPE_LEGACY_WINDOW);

  legacy_screen_signals [WINDOW_CLOSED] =
  	g_signal_new ("window-closed",
  	              G_OBJECT_CLASS_TYPE (klass),
  	              G_SIGNAL_RUN_FIRST,
  	              G_STRUCT_OFFSET (BamfLegacyScreenClass, window_closed),
  	              NULL, NULL,
  	              g_cclosure_marshal_VOID__OBJECT,
  	              G_TYPE_NONE, 1,
  	              BAMF_TYPE_LEGACY_WINDOW);

  legacy_screen_signals [ACTIVE_WINDOW_CHANGED] =
  	g_signal_new ("active-window-changed",
  	              G_OBJECT_CLASS_TYPE (klass),
  	              G_SIGNAL_RUN_FIRST,
  	              G_STRUCT_OFFSET (BamfLegacyScreenClass, active_window_changed),
  	              NULL, NULL,
  	              g_cclosure_marshal_VOID__VOID,
  	              G_TYPE_NONE, 0);
}

static BamfLegacyScreen *self = NULL;

BamfLegacyScreen *
bamf_legacy_screen_get_default ()
{
  if (self)
    return self;

  self = (BamfLegacyScreen *) g_object_new (BAMF_TYPE_LEGACY_SCREEN, NULL);

  self->priv->legacy_screen = wnck_screen_get_default ();

  g_signal_connect (G_OBJECT (self->priv->legacy_screen), "window-opened",
                    (GCallback) handle_window_opened, self);

  g_signal_connect (G_OBJECT (self->priv->legacy_screen), "active-window-changed",
                    (GCallback) handle_active_window_changed, self);

  return self;
}
