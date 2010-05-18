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

#include "bamf-tab.h"
#include "bamf-tab-glue.h"
#include "bamf-marshal.h"

G_DEFINE_TYPE (BamfTab, bamf_tab, BAMF_TYPE_VIEW);
#define BAMF_TAB_GET_PRIVATE(obj) (G_TYPE_INSTANCE_GET_PRIVATE(obj, \
BAMF_TYPE_TAB, BamfTabPrivate))

enum
{
  URI_CHANGED,
  
  LAST_SIGNAL,
};

enum
{
  PROP_0,
  
  PROP_ID,
  PROP_SOURCE,
};

static guint tab_signals[LAST_SIGNAL] = { 0 };

struct _BamfTabPrivate
{
  char *id;
  char *uri;
  BamfTabSource *source;
};

char * 
bamf_tab_current_uri (BamfTab *self)
{
  g_return_val_if_fail (BAMF_IS_TAB (self), NULL);
  
  return g_strdup (self->priv->uri);
}

void 
bamf_tab_show (BamfTab *self)
{
  g_return_if_fail (BAMF_IS_TAB (self));

  return;
}

guint32 bamf_tab_parent_xid (BamfTab *tab)
{
  g_return_val_if_fail (BAMF_IS_TAB (tab), 0);

  return 0;
}

GArray *
bamf_tab_get_preview (BamfTab *tab)
{
  return NULL;
}

static char *
bamf_tab_get_view_type (BamfView *view)
{
  return g_strdup ("tab");
}

static void
on_tab_uri_changed (BamfTabSource *source, char *id, char *old_uri, char *new_uri, BamfTab *self)
{
  if (g_strcmp0 (id, self->priv->id) != 0)
    return;
  
  g_free (self->priv->uri);
  self->priv->uri = g_strdup (new_uri);  

  g_signal_emit (self, URI_CHANGED, 0, old_uri, new_uri);
}

static void
bamf_tab_set_property (GObject *object, guint property_id, const GValue *value, GParamSpec *pspec)
{
  BamfTab *self;

  self = BAMF_TAB (object);

  switch (property_id)
    {
      case PROP_ID:
        self->priv->id = g_value_dup_string (value);
        break;
      
      case PROP_SOURCE:
        self->priv->source = g_value_get_object (value);
        break;
        
      default:
        G_OBJECT_WARN_INVALID_PROPERTY_ID(object, property_id, pspec);
    }
}

static void
bamf_tab_get_property (GObject *object, guint property_id, GValue *value, GParamSpec *pspec)
{
  BamfTab *self;

  self = BAMF_TAB (object);

  switch (property_id)
    {
      case PROP_ID:
        g_value_set_string (value, self->priv->id);
        break;
      
      case PROP_SOURCE:
        g_value_set_object (value, self->priv->source);
        break;
      
      default:
        G_OBJECT_WARN_INVALID_PROPERTY_ID(object, property_id, pspec);
    }
}

static void
bamf_tab_constructed (GObject *object)
{
  BamfTab *self;
  
  if (G_OBJECT_CLASS (bamf_tab_parent_class)->constructed)
    G_OBJECT_CLASS (bamf_tab_parent_class)->constructed (object);
  
  self = BAMF_TAB (object);
  
  g_signal_connect (self->priv->source, "remote-tab-uri-changed", (GCallback) on_tab_uri_changed, self);
}

static void
bamf_tab_dispose (GObject *object)
{
  BamfTab *self;

  self = BAMF_TAB (object);

  G_OBJECT_CLASS (bamf_tab_parent_class)->dispose (object);
}

static void
bamf_tab_init (BamfTab * self)
{
  BamfTabPrivate *priv;
  priv = self->priv = BAMF_TAB_GET_PRIVATE (self);
}

static void
bamf_tab_class_init (BamfTabClass * klass)
{
  GParamSpec *pspec;
  GObjectClass *object_class = G_OBJECT_CLASS (klass);
  BamfViewClass *view_class = BAMF_VIEW_CLASS (klass);

  object_class->constructed  = bamf_tab_constructed;
  object_class->get_property = bamf_tab_get_property;
  object_class->set_property = bamf_tab_set_property;
  object_class->dispose      = bamf_tab_dispose;
  view_class->view_type      = bamf_tab_get_view_type;
  
  pspec = g_param_spec_string ("id", "id", "id", NULL, G_PARAM_READWRITE | G_PARAM_CONSTRUCT);
  g_object_class_install_property (object_class, PROP_ID, pspec);
  
  pspec = g_param_spec_object ("source", "source", "source", BAMF_TYPE_TAB_SOURCE, G_PARAM_READWRITE | G_PARAM_CONSTRUCT);
  g_object_class_install_property (object_class, PROP_SOURCE, pspec);
  
  g_type_class_add_private (klass, sizeof (BamfTabPrivate));
  
  dbus_g_object_type_install_info (BAMF_TYPE_TAB,
				   &dbus_glib_bamf_tab_object_info);

  tab_signals [URI_CHANGED] = 
  	g_signal_new ("uri-changed",
  	              G_OBJECT_CLASS_TYPE (klass),
  	              0,
  	              0, NULL, NULL,
  	              bamf_marshal_VOID__STRING_STRING,
  	              G_TYPE_NONE, 2, 
  	              G_TYPE_STRING, G_TYPE_STRING);
}

BamfTab *
bamf_tab_new (BamfTabSource *source, char *id)
{
  BamfTab *self;
  self = (BamfTab *) g_object_new (BAMF_TYPE_TAB, 
                                   "source", source, 
                                   "id", id, 
                                   NULL);

  return self;
}
