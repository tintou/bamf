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

#include "bamf-legacy-window.h"
#include "marshal.h"

G_DEFINE_TYPE (BamfLegacyWindow, bamf_legacy_window, G_TYPE_OBJECT);
#define BAMF_LEGACY_WINDOW_GET_PRIVATE(obj) (G_TYPE_INSTANCE_GET_PRIVATE(obj, \
BAMF_TYPE_LEGACY_WINDOW, BamfLegacyWindowPrivate))

enum
{
  NAME_CHANGED,
  STATE_CHANGED,
  CLOSED,
  
  LAST_SIGNAL,
};

static guint legacy_window_signals[LAST_SIGNAL] = { 0 };

struct _BamfLegacyWindowPrivate
{
  WnckWindow * legacy_window;
  gulong closed_id;
  gulong name_changed_id;
  gulong state_changed_id;
};

gboolean
bamf_legacy_window_is_active (BamfLegacyWindow *self)
{
  WnckWindow *active;

  g_return_val_if_fail (BAMF_IS_LEGACY_WINDOW (self), FALSE);
  
  active = wnck_screen_get_active_window (wnck_screen_get_default ());
  
  return active == self->priv->legacy_window;
}

gboolean
bamf_legacy_window_is_desktop_window (BamfLegacyWindow *self)
{
  g_return_val_if_fail (BAMF_IS_LEGACY_WINDOW (self), FALSE);


  if (BAMF_LEGACY_WINDOW_GET_CLASS (self)->is_desktop)
    return BAMF_LEGACY_WINDOW_GET_CLASS (self)->is_desktop (self);

  if (!self->priv->legacy_window)
    return FALSE;
  return (wnck_window_get_window_type (self->priv->legacy_window) & WNCK_WINDOW_DESKTOP);
}

gboolean
bamf_legacy_window_needs_attention (BamfLegacyWindow *self)
{
  g_return_val_if_fail (BAMF_IS_LEGACY_WINDOW (self), FALSE);


  if (BAMF_LEGACY_WINDOW_GET_CLASS (self)->needs_attention)
    return BAMF_LEGACY_WINDOW_GET_CLASS (self)->needs_attention (self);

  if (!self->priv->legacy_window)
    return FALSE;
  return wnck_window_needs_attention (self->priv->legacy_window);
}

gboolean
bamf_legacy_window_is_skip_tasklist (BamfLegacyWindow *self)
{
  g_return_val_if_fail (BAMF_IS_LEGACY_WINDOW (self), FALSE);


  if (BAMF_LEGACY_WINDOW_GET_CLASS (self)->is_skip_tasklist)
    return BAMF_LEGACY_WINDOW_GET_CLASS (self)->is_skip_tasklist (self);

  if (!self->priv->legacy_window)
    return FALSE;
  return wnck_window_is_skip_tasklist (self->priv->legacy_window);
}

const char *
bamf_legacy_window_get_class_name (BamfLegacyWindow *self)
{
  WnckClassGroup *group;
  WnckWindow *window;

  g_return_val_if_fail (BAMF_IS_LEGACY_WINDOW (self), NULL);
  
  window = self->priv->legacy_window;

  if (BAMF_LEGACY_WINDOW_GET_CLASS (self)->get_name)
    return BAMF_LEGACY_WINDOW_GET_CLASS (self)->get_name (self);

  if (!window)
    return NULL;
  
  group = wnck_window_get_class_group (window);
  
  if (!group)
    return NULL;
    
  return wnck_class_group_get_res_class (group);
}

const char *
bamf_legacy_window_get_name (BamfLegacyWindow *self)
{
  g_return_val_if_fail (BAMF_IS_LEGACY_WINDOW (self), NULL);


  if (BAMF_LEGACY_WINDOW_GET_CLASS (self)->get_name)
    return BAMF_LEGACY_WINDOW_GET_CLASS (self)->get_name (self);

  if (!self->priv->legacy_window)
    return NULL;
  return wnck_window_get_name (self->priv->legacy_window);
}

gint
bamf_legacy_window_get_pid (BamfLegacyWindow *self)
{
  g_return_val_if_fail (BAMF_IS_LEGACY_WINDOW (self), 0);


  if (BAMF_LEGACY_WINDOW_GET_CLASS (self)->get_pid)
    return BAMF_LEGACY_WINDOW_GET_CLASS (self)->get_pid (self);

  if (!self->priv->legacy_window)
    return 0;
  return wnck_window_get_pid (self->priv->legacy_window);
}

guint32
bamf_legacy_window_get_xid (BamfLegacyWindow *self)
{
  g_return_val_if_fail (BAMF_IS_LEGACY_WINDOW (self), 0);


  if (BAMF_LEGACY_WINDOW_GET_CLASS (self)->get_xid)
    return BAMF_LEGACY_WINDOW_GET_CLASS (self)->get_xid (self);

  if (!self->priv->legacy_window)
    return 0;
   
  return (guint32) wnck_window_get_xid (self->priv->legacy_window);
}

static void
handle_name_changed (WnckWindow *window, BamfLegacyWindow *self)
{
  g_return_if_fail (BAMF_IS_LEGACY_WINDOW (self));

  g_signal_emit (self, legacy_window_signals[NAME_CHANGED], 0);
}

static void
handle_state_changed (WnckWindow *window, 
                      WnckWindowState change_mask, 
                      WnckWindowState new_state, 
                      BamfLegacyWindow *self)
{
  g_return_if_fail (BAMF_IS_LEGACY_WINDOW (self));

  g_signal_emit (self, legacy_window_signals[STATE_CHANGED], 0);
}

static void
handle_window_closed (WnckScreen *screen,
                      WnckWindow *window,
                      BamfLegacyWindow *self)
{
  g_return_if_fail (BAMF_IS_LEGACY_WINDOW (self));
  g_return_if_fail (WNCK_IS_WINDOW (window));
  
  if (window == self->priv->legacy_window)
    {
      g_signal_emit (self, legacy_window_signals[CLOSED], 0);
    }
}

static void
bamf_legacy_window_dispose (GObject *object)
{
  BamfLegacyWindow *self;

  self = BAMF_LEGACY_WINDOW (object);

  g_signal_handler_disconnect (wnck_screen_get_default (),
                               self->priv->closed_id);

  if (self->priv->legacy_window)
    {
      g_signal_handler_disconnect (self->priv->legacy_window,
                                   self->priv->name_changed_id);

      g_signal_handler_disconnect (self->priv->legacy_window,
                                   self->priv->state_changed_id);
    }
    
  G_OBJECT_CLASS (bamf_legacy_window_parent_class)->dispose (object);
}

static void
bamf_legacy_window_init (BamfLegacyWindow * self)
{
  WnckScreen *screen;

  BamfLegacyWindowPrivate *priv;
  priv = self->priv = BAMF_LEGACY_WINDOW_GET_PRIVATE (self);

  screen = wnck_screen_get_default ();

  priv->closed_id = g_signal_connect (G_OBJECT (screen), "window-closed",
    		                       (GCallback) handle_window_closed, self);
}

static void
bamf_legacy_window_class_init (BamfLegacyWindowClass * klass)
{
  GObjectClass *object_class = G_OBJECT_CLASS (klass);

  object_class->dispose      = bamf_legacy_window_dispose;
  
  g_type_class_add_private (klass, sizeof (BamfLegacyWindowPrivate));
  
  legacy_window_signals [NAME_CHANGED] = 
  	g_signal_new ("name-changed",
  	              G_OBJECT_CLASS_TYPE (klass),
  	              G_SIGNAL_RUN_FIRST,
  	              G_STRUCT_OFFSET (BamfLegacyWindowClass, name_changed), 
  	              NULL, NULL,
  	              g_cclosure_marshal_VOID__VOID,
  	              G_TYPE_NONE, 0);
  
  legacy_window_signals [STATE_CHANGED] = 
  	g_signal_new ("state-changed",
  	              G_OBJECT_CLASS_TYPE (klass),
  	              G_SIGNAL_RUN_FIRST,
  	              G_STRUCT_OFFSET (BamfLegacyWindowClass, state_changed), 
  	              NULL, NULL,
  	              g_cclosure_marshal_VOID__VOID,
  	              G_TYPE_NONE, 0);
  
  legacy_window_signals [CLOSED] = 
  	g_signal_new ("closed",
  	              G_OBJECT_CLASS_TYPE (klass),
  	              G_SIGNAL_RUN_FIRST,
  	              G_STRUCT_OFFSET (BamfLegacyWindowClass, closed), 
  	              NULL, NULL,
  	              g_cclosure_marshal_VOID__VOID,
  	              G_TYPE_NONE, 0);
  
  
}

BamfLegacyWindow *
bamf_legacy_window_new (WnckWindow *legacy_window)
{
  BamfLegacyWindow *self;
  self = (BamfLegacyWindow *) g_object_new (BAMF_TYPE_LEGACY_WINDOW, NULL);
  
  self->priv->legacy_window = legacy_window;

  self->priv->name_changed_id = g_signal_connect (G_OBJECT (legacy_window), "name-changed",
    		                       (GCallback) handle_name_changed, self);

  self->priv->state_changed_id = g_signal_connect (G_OBJECT (legacy_window), "state-changed",
                                       (GCallback) handle_state_changed, self);

  return self;
}
