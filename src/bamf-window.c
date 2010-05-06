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

#include "bamf-window.h"
#include "bamf-window-glue.h"
#include "marshal.h"

G_DEFINE_TYPE (BamfWindow, bamf_window, BAMF_TYPE_VIEW);
#define BAMF_WINDOW_GET_PRIVATE(obj) (G_TYPE_INSTANCE_GET_PRIVATE(obj, \
BAMF_TYPE_WINDOW, BamfWindowPrivate))

enum
{
  URGENT_CHANGED,
  
  LAST_SIGNAL,
};

enum
{
  PROP_0,

  PROP_WINDOW,
};

static guint window_signals[LAST_SIGNAL] = { 0 };

struct _BamfWindowPrivate
{
  WnckWindow * window;
  gulong closed_id;
  gulong name_changed_id;
  gulong state_changed_id;
};

gboolean
bamf_window_is_urgent (BamfWindow *self)
{
  g_return_val_if_fail (BAMF_IS_WINDOW (self), FALSE);

  if (BAMF_WINDOW_GET_CLASS (self)->is_urgent)
    return BAMF_WINDOW_GET_CLASS (self)->is_urgent (self);

  return wnck_window_needs_attention (bamf_window_get_window (self));
}

WnckWindow *
bamf_window_get_window (BamfWindow *self)
{
  g_return_val_if_fail (BAMF_IS_WINDOW (self), NULL);

  if (BAMF_WINDOW_GET_CLASS (self)->get_window)
    return BAMF_WINDOW_GET_CLASS (self)->get_window (self);

  return self->priv->window;
}

guint32
bamf_window_get_xid (BamfWindow *window)
{
  g_return_val_if_fail (BAMF_IS_WINDOW (window), 0);


  if (BAMF_WINDOW_GET_CLASS (window)->get_xid)
    return BAMF_WINDOW_GET_CLASS (window)->get_xid (window);

  return (guint32) wnck_window_get_xid (window->priv->window);
}

static void
handle_window_closed (WnckScreen * screen, WnckWindow * window, gpointer data)
{
  BamfWindow *self;
  self = (BamfWindow *) data;
  
  g_return_if_fail (BAMF_IS_WINDOW (self));
  g_return_if_fail (WNCK_IS_WINDOW (window));

  if (window == self->priv->window)
    {
      g_object_unref (G_OBJECT (self));
    }
}

static void
handle_name_changed (WnckWindow *window, BamfWindow *self)
{
  g_return_if_fail (WNCK_IS_WINDOW (window));

  bamf_view_set_name (BAMF_VIEW (self), wnck_window_get_name (window));
}

static void
handle_state_changed (WnckWindow *window, 
                      WnckWindowState change_mask, 
                      WnckWindowState new_state, 
                      BamfWindow *self)
{
  if (!(change_mask & WNCK_WINDOW_STATE_URGENT))
    return;

  g_signal_emit (self, window_signals[URGENT_CHANGED], 0, wnck_window_needs_attention (window));
}

static char *
bamf_window_get_view_type (BamfView *view)
{
  return "window";
}

static void
bamf_window_set_property (GObject *object, guint property_id, const GValue *value, GParamSpec *pspec)
{
  BamfWindow *self;

  self = BAMF_WINDOW (object);

  switch (property_id)
    {
      case PROP_WINDOW:
        self->priv->window = WNCK_WINDOW (g_value_get_object (value));

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
  WnckWindow *window;
  
  if (G_OBJECT_CLASS (bamf_window_parent_class)->constructed)
    G_OBJECT_CLASS (bamf_window_parent_class)->constructed (object);
  
  g_object_get (object, "window", &window, NULL);
  
  self = BAMF_WINDOW (object);
  
  bamf_view_set_name (BAMF_VIEW (self), wnck_window_get_name (window));

  self->priv->name_changed_id = g_signal_connect (G_OBJECT (window), "name-changed",
    		                       (GCallback) handle_name_changed, self);

  self->priv->state_changed_id = g_signal_connect (G_OBJECT (window), "state-changed",
                                       (GCallback) handle_state_changed, self);
}

static void
bamf_window_dispose (GObject *object)
{
  BamfWindow *self;

  self = BAMF_WINDOW (object);

  g_signal_handler_disconnect (wnck_screen_get_default (),
                               self->priv->closed_id);

  if (self->priv->window)
    {
      g_signal_handler_disconnect (self->priv->window,
                                   self->priv->name_changed_id);

      g_signal_handler_disconnect (self->priv->window,
                                   self->priv->state_changed_id);
    }
  G_OBJECT_CLASS (bamf_window_parent_class)->dispose (object);
}

static void
bamf_window_init (BamfWindow * self)
{
  WnckScreen *screen;

  BamfWindowPrivate *priv;
  priv = self->priv = BAMF_WINDOW_GET_PRIVATE (self);

  screen = wnck_screen_get_default ();

  priv->closed_id = g_signal_connect (G_OBJECT (screen), "window-closed",
    		                       (GCallback) handle_window_closed, self);
}

static void
bamf_window_class_init (BamfWindowClass * klass)
{
  GParamSpec *pspec;
  GObjectClass *object_class = G_OBJECT_CLASS (klass);
  BamfViewClass *view_class = BAMF_VIEW_CLASS (klass);

  object_class->dispose      = bamf_window_dispose;
  object_class->get_property = bamf_window_get_property;
  object_class->set_property = bamf_window_set_property;
  object_class->constructed  = bamf_window_constructed;
  view_class->view_type      = bamf_window_get_view_type;
  
  pspec = g_param_spec_object ("window", "window", "window", WNCK_TYPE_WINDOW, G_PARAM_READWRITE | G_PARAM_CONSTRUCT);
  g_object_class_install_property (object_class, PROP_WINDOW, pspec);

  g_type_class_add_private (klass, sizeof (BamfWindowPrivate));
  
  dbus_g_object_type_install_info (BAMF_TYPE_WINDOW,
				   &dbus_glib_bamf_window_object_info);

  window_signals [URGENT_CHANGED] = 
  	g_signal_new ("urgent-changed",
  	              G_OBJECT_CLASS_TYPE (klass),
  	              0,
  	              0, NULL, NULL,
  	              g_cclosure_marshal_VOID__BOOLEAN,
  	              G_TYPE_NONE, 1, 
  	              G_TYPE_BOOLEAN);
}

BamfWindow *
bamf_window_new (WnckWindow *window)
{
  BamfWindow *self;
  self = (BamfWindow *) g_object_new (BAMF_TYPE_WINDOW, "window", window, NULL);

  return self;
}
