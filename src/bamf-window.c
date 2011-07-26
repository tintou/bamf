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

#include "bamf-application.h"
#include "bamf-window.h"
#include "bamf-window-glue.h"
#include "bamf-legacy-screen.h"

G_DEFINE_TYPE (BamfWindow, bamf_window, BAMF_TYPE_VIEW);
#define BAMF_WINDOW_GET_PRIVATE(obj) (G_TYPE_INSTANCE_GET_PRIVATE(obj, \
BAMF_TYPE_WINDOW, BamfWindowPrivate))

static GList *bamf_windows;

enum
{
  PROP_0,

  PROP_WINDOW,
};

struct _BamfWindowPrivate
{
  BamfLegacyWindow * window;
  gulong closed_id;
  gulong name_changed_id;
  gulong state_changed_id;
  time_t last_active;
  time_t opened;
};

BamfLegacyWindow *
bamf_window_get_window (BamfWindow *self)
{
  g_return_val_if_fail (BAMF_IS_WINDOW (self), NULL);

  if (BAMF_WINDOW_GET_CLASS (self)->get_window)
    return BAMF_WINDOW_GET_CLASS (self)->get_window (self);

  return self->priv->window;
}

BamfWindow *
bamf_window_get_transient (BamfWindow *self)
{
  BamfLegacyWindow *legacy, *transient;
  BamfWindow *other;
  GList *l;
  
  g_return_val_if_fail (BAMF_IS_WINDOW (self), NULL);
  
  legacy = bamf_window_get_window (self);
  transient = bamf_legacy_window_get_transient (legacy);
  
  if (transient)
    {
      for (l = bamf_windows; l; l = l->next)
        {
          other = l->data;
      
          if (!BAMF_IS_WINDOW (other))
            continue;
      
          if (transient == bamf_window_get_window (other))
            return other;
        }
    }
  return NULL;
}

char *
bamf_window_get_transient_path (BamfWindow *self)
{
  BamfWindow *transient;
  
  g_return_val_if_fail (BAMF_IS_WINDOW (self), NULL);
  
  transient = bamf_window_get_transient (self);
  
  if (transient == NULL)
    return NULL;
  
  return g_strdup (bamf_view_get_path (BAMF_VIEW (transient)));
} 

guint32
bamf_window_get_window_type (BamfWindow *window)
{
  g_return_val_if_fail (BAMF_IS_WINDOW (window), 0);
  
  return (guint32) bamf_legacy_window_get_window_type (window->priv->window);
}

guint32
bamf_window_get_xid (BamfWindow *window)
{
  g_return_val_if_fail (BAMF_IS_WINDOW (window), 0);


  if (BAMF_WINDOW_GET_CLASS (window)->get_xid)
    return BAMF_WINDOW_GET_CLASS (window)->get_xid (window);

  return (guint32) bamf_legacy_window_get_xid (window->priv->window);
}

time_t
bamf_window_last_active (BamfWindow *self)
{
  g_return_val_if_fail (BAMF_IS_WINDOW (self), (time_t) 0);
  
  if (bamf_view_is_active (BAMF_VIEW (self)))
    return time (NULL);
  return self->priv->last_active;
}

time_t
bamf_window_opened (BamfWindow *self)
{
  g_return_val_if_fail (BAMF_IS_WINDOW (self), (time_t) 0);
  
  return self->priv->opened;
}

static void
handle_window_closed (BamfLegacyWindow * window, gpointer data)
{
  BamfWindow *self;
  self = (BamfWindow *) data;

  g_return_if_fail (BAMF_IS_WINDOW (self));
  g_return_if_fail (BAMF_IS_LEGACY_WINDOW (window));

  if (window == self->priv->window)
    {
      bamf_view_close (BAMF_VIEW (self));
    }
}

static void
handle_name_changed (BamfLegacyWindow *window, BamfWindow *self)
{
  g_return_if_fail (BAMF_IS_LEGACY_WINDOW (window));

  bamf_view_set_name (BAMF_VIEW (self), bamf_legacy_window_get_name (window));
}

static void
bamf_window_ensure_flags (BamfWindow *self)
{
  g_return_if_fail (BAMF_IS_WINDOW (self));

  /* if we are going innactive, set our last active time */
  if (bamf_view_is_active (BAMF_VIEW (self)) && !bamf_legacy_window_is_active (self->priv->window))
    self->priv->last_active = time (NULL);
  
  bamf_view_set_active       (BAMF_VIEW (self), bamf_legacy_window_is_active (self->priv->window));
  bamf_view_set_urgent       (BAMF_VIEW (self), bamf_legacy_window_needs_attention (self->priv->window));
  
  if (g_strcmp0 (bamf_legacy_window_get_class_name (self->priv->window), "Nautilus") == 0 &&
      g_strcmp0 (bamf_legacy_window_get_name (self->priv->window), "File Operations") == 0)
    bamf_view_set_user_visible (BAMF_VIEW (self), FALSE);
  else    
    bamf_view_set_user_visible (BAMF_VIEW (self), !bamf_legacy_window_is_skip_tasklist (self->priv->window));
}

static void
handle_state_changed (BamfLegacyWindow *window,
                      BamfWindow *self)
{
  bamf_window_ensure_flags (self);
}

static char *
bamf_window_get_view_type (BamfView *view)
{
  return g_strdup ("window");
}

static char *
bamf_window_get_stable_bus_name (BamfView *view)
{
  BamfWindow *self;

  g_return_val_if_fail (BAMF_IS_WINDOW (view), NULL);  
  self = BAMF_WINDOW (view);
  
  return g_strdup_printf ("window%i", bamf_legacy_window_get_xid (self->priv->window));
}

static void
active_window_changed (BamfLegacyScreen *screen, BamfWindow *window)
{
  bamf_window_ensure_flags (window);
}

static void
bamf_window_set_property (GObject *object, guint property_id, const GValue *value, GParamSpec *pspec)
{
  BamfWindow *self;

  self = BAMF_WINDOW (object);

  switch (property_id)
    {
      case PROP_WINDOW:
        self->priv->window = BAMF_LEGACY_WINDOW (g_value_get_object (value));
        break;

      default:
        G_OBJECT_WARN_INVALID_PROPERTY_ID(object, property_id, pspec);
    }
}

static void
bamf_window_get_property (GObject *object, guint property_id, GValue *value, GParamSpec *pspec)
{
  BamfWindow *self;

  self = BAMF_WINDOW (object);

  switch (property_id)
    {
      case PROP_WINDOW:
        g_value_set_object (value, self->priv->window);

        break;

      default:
        G_OBJECT_WARN_INVALID_PROPERTY_ID(object, property_id, pspec);
    }
}

static void
bamf_window_constructed (GObject *object)
{
  BamfWindow *self;
  BamfLegacyWindow *window;

  if (G_OBJECT_CLASS (bamf_window_parent_class)->constructed)
    G_OBJECT_CLASS (bamf_window_parent_class)->constructed (object);

  g_object_get (object, "window", &window, NULL);

  self = BAMF_WINDOW (object);
  bamf_windows = g_list_prepend (bamf_windows, self);

  self->priv->opened = time (NULL);

  bamf_view_set_name (BAMF_VIEW (self), bamf_legacy_window_get_name (window));

  self->priv->name_changed_id = g_signal_connect (G_OBJECT (window), "name-changed",
    		                       (GCallback) handle_name_changed, self);

  self->priv->state_changed_id = g_signal_connect (G_OBJECT (window), "state-changed",
                                       (GCallback) handle_state_changed, self);

  self->priv->closed_id = g_signal_connect (G_OBJECT (window), "closed",
                                            (GCallback) handle_window_closed, self);

  bamf_window_ensure_flags (self);
}

static void
bamf_window_dispose (GObject *object)
{
  BamfWindow *self;

  self = BAMF_WINDOW (object);
  bamf_windows = g_list_remove (bamf_windows, self);

  g_signal_handlers_disconnect_by_func (G_OBJECT (bamf_legacy_screen_get_default ()), active_window_changed, object);

  if (self->priv->window)
    {
      g_signal_handler_disconnect (self->priv->window,
                                   self->priv->name_changed_id);

      g_signal_handler_disconnect (self->priv->window,
                                   self->priv->state_changed_id);

      g_signal_handler_disconnect (self->priv->window,
                                   self->priv->closed_id);

      g_object_unref (self->priv->window);
      self->priv->window = NULL;
    }
  G_OBJECT_CLASS (bamf_window_parent_class)->dispose (object);
}

static void
bamf_window_init (BamfWindow * self)
{
  g_signal_connect (G_OBJECT (bamf_legacy_screen_get_default ()), "active-window-changed",
		    (GCallback) active_window_changed, self);
}

static void
bamf_window_class_init (BamfWindowClass * klass)
{
  GParamSpec *pspec;
  GObjectClass *object_class = G_OBJECT_CLASS (klass);
  BamfViewClass *view_class = BAMF_VIEW_CLASS (klass);

  object_class->dispose       = bamf_window_dispose;
  object_class->get_property  = bamf_window_get_property;
  object_class->set_property  = bamf_window_set_property;
  object_class->constructed   = bamf_window_constructed;
  view_class->view_type       = bamf_window_get_view_type;
  view_class->stable_bus_name = bamf_window_get_stable_bus_name;

  pspec = g_param_spec_object ("window", "window", "window", BAMF_TYPE_LEGACY_WINDOW, G_PARAM_READWRITE | G_PARAM_CONSTRUCT);
  g_object_class_install_property (object_class, PROP_WINDOW, pspec);

  g_type_class_add_private (klass, sizeof (BamfWindowPrivate));

  dbus_g_object_type_install_info (BAMF_TYPE_WINDOW,
				   &dbus_glib_bamf_window_object_info);
}

BamfWindow *
bamf_window_new (BamfLegacyWindow *window)
{
  BamfWindow *self;
  self = (BamfWindow *) g_object_new (BAMF_TYPE_WINDOW, "window", window, NULL);
  
  return self;
}
