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

#include "bamf-legacy-screen.h"
#include "marshal.h"

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
};

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
handle_window_closed (BamfLegacyWindow *window, BamfLegacyScreen *legacy)
{
  legacy->priv->windows = g_list_remove (legacy->priv->windows, window);

  g_signal_emit (legacy, legacy_screen_signals[WINDOW_CLOSED], 0, window);
  
  g_object_unref (window);
}

static void
handle_window_opened (WnckScreen *screen, WnckWindow *window, BamfLegacyScreen *legacy)
{
  BamfLegacyWindow *legacy_window;
  
  legacy_window = bamf_legacy_window_new (window);
  
  g_signal_connect (G_OBJECT (legacy_window), "closed",
                    (GCallback) handle_window_closed, legacy);
  
  legacy->priv->windows = g_list_prepend (legacy->priv->windows, legacy_window);
  
  g_signal_emit (legacy, legacy_screen_signals[WINDOW_OPENED], 0, legacy_window);
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
  BamfLegacyScreen *self;

  self = BAMF_LEGACY_SCREEN (object);

  G_OBJECT_CLASS (bamf_legacy_screen_parent_class)->dispose (object);
}

static void
bamf_legacy_screen_init (BamfLegacyScreen * self)
{
  BamfLegacyScreenPrivate *priv;
  priv = self->priv = BAMF_LEGACY_SCREEN_GET_PRIVATE (self);
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
  	              0,
  	              G_STRUCT_OFFSET (BamfLegacyScreenClass, window_opened), 
  	              NULL, NULL,
  	              g_cclosure_marshal_VOID__POINTER,
  	              G_TYPE_NONE, 1,
  	              BAMF_TYPE_LEGACY_WINDOW);
  
  legacy_screen_signals [WINDOW_CLOSED] = 
  	g_signal_new ("window-closed",
  	              G_OBJECT_CLASS_TYPE (klass),
  	              0,
  	              G_STRUCT_OFFSET (BamfLegacyScreenClass, window_closed), 
  	              NULL, NULL,
  	              g_cclosure_marshal_VOID__POINTER,
  	              G_TYPE_NONE, 1,
  	              BAMF_TYPE_LEGACY_WINDOW);
  	              
  legacy_screen_signals [ACTIVE_WINDOW_CHANGED] = 
  	g_signal_new ("active-window-changed",
  	              G_OBJECT_CLASS_TYPE (klass),
  	              0,
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
