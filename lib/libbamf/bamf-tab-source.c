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

#include "bamf-tab-source.h"
#include "bamf-marshal.h"


G_DEFINE_TYPE (BamfTabSource, bamf_tab_source, G_TYPE_OBJECT)
#define BAMF_TAB_SOURCE_GET_PRIVATE(object)(G_TYPE_INSTANCE_GET_PRIVATE((object), BAMF_TYPE_TAB_SOURCE, BamfTabSourcePrivate))

enum
{
  TAB_URI_CHANGED,
  TAB_OPENED,
  TAB_CLOSED,

  LAST_SIGNAL,
};

enum
{
  PROP_0,
  
  PROP_ID,
};

static guint bamf_tab_source_signals[LAST_SIGNAL] = { 0 };

struct _BamfTabSourcePrivate
{
  char *id;
};

void           
bamf_tab_source_show_tab        (BamfTabSource *source, 
                                 char *tab_id)
{
  g_return_if_fail (BAMF_IS_TAB_SOURCE (source));
  
  if (BAMF_TAB_SOURCE_GET_CLASS (source)->show_tab)
    BAMF_TAB_SOURCE_GET_CLASS (source)->show_tab (source);
}

char ** 
bamf_tab_source_get_tab_ids     (BamfTabSource *source)
{
  g_return_val_if_fail (BAMF_IS_TAB_SOURCE (source), NULL);
  
  if (BAMF_TAB_SOURCE_GET_CLASS (source)->tab_ids)
    return BAMF_TAB_SOURCE_GET_CLASS (source)->tab_ids (source);
  
  return NULL;
}

GArray * 
bamf_tab_source_get_tab_preview (BamfTabSource *source,
                                 char *tab_id)
{
  g_return_val_if_fail (BAMF_IS_TAB_SOURCE (source), NULL);
  
  if (BAMF_TAB_SOURCE_GET_CLASS (source)->tab_preview)
    return BAMF_TAB_SOURCE_GET_CLASS (source)->tab_preview (source);
  
  return NULL;
}

char * 
bamf_tab_source_get_tab_uri     (BamfTabSource *source,
                                 char *tab_id)
{
  g_return_val_if_fail (BAMF_IS_TAB_SOURCE (source), NULL);
  
  if (BAMF_TAB_SOURCE_GET_CLASS (source)->tab_uri)
    return BAMF_TAB_SOURCE_GET_CLASS (source)->tab_uri (source);
  
  return NULL;
}

guint32 
bamf_tab_source_get_tab_xid     (BamfTabSource *source,
                                 char *tab_id)
{
  g_return_val_if_fail (BAMF_IS_TAB_SOURCE (source), 0);
  
  if (BAMF_TAB_SOURCE_GET_CLASS (source)->tab_xid)
    return BAMF_TAB_SOURCE_GET_CLASS (source)->tab_xid (source);
  
  return 0;
}

static void
bamf_tab_source_set_property (GObject *object, guint property_id, const GValue *value, GParamSpec *pspec)
{
  BamfTabSource *self;

  self = BAMF_TAB_SOURCE (object);

  switch (property_id)
    {
      case PROP_ID:
        self->priv->id = g_strdup (g_value_get_string (value));
        break;
      default:
        G_OBJECT_WARN_INVALID_PROPERTY_ID(object, property_id, pspec);
    }

}

static void
bamf_tab_source_get_property (GObject *object, guint property_id, GValue *value, GParamSpec *pspec)
{
  BamfTabSource *self;

  self = BAMF_TAB_SOURCE (object);

  switch (property_id)
    {
      case PROP_ID:
        g_value_set_string (value, self->priv->id);
        break;
      default:
        G_OBJECT_WARN_INVALID_PROPERTY_ID(object, property_id, pspec);
    }

}

static void
bamf_tab_source_dispose (GObject *object)
{
  G_OBJECT_CLASS (bamf_tab_source_parent_class)->dispose (object);
}

static void
bamf_tab_source_constructed (GObject *object)
{
  if (G_OBJECT_CLASS (bamf_tab_source_parent_class)->constructed)
    G_OBJECT_CLASS (bamf_tab_source_parent_class)->constructed (object);
}

static void
bamf_tab_source_class_init (BamfTabSourceClass *klass)
{
  GParamSpec *pspec;
  GObjectClass *object_class = G_OBJECT_CLASS (klass);
	
  object_class->constructed  = bamf_tab_source_constructed;
  object_class->dispose      = bamf_tab_source_dispose;
  object_class->get_property = bamf_tab_source_get_property;
  object_class->set_property = bamf_tab_source_set_property;
	
  pspec = g_param_spec_string ("id", "id", "id", NULL, G_PARAM_READWRITE | G_PARAM_CONSTRUCT);
  g_object_class_install_property (object_class, PROP_ID, pspec);

  g_type_class_add_private (object_class, sizeof (BamfTabSourcePrivate));
  
  bamf_tab_source_signals [TAB_URI_CHANGED] = 
  	g_signal_new ("tab-uri-changed",
  	              G_OBJECT_CLASS_TYPE (klass),
  	              G_SIGNAL_RUN_FIRST,
  	              0, 
  	              NULL, NULL,
  	              bamf_marshal_VOID__STRING_STRING_STRING,
  	              G_TYPE_NONE, 3,
  	              G_TYPE_STRING, G_TYPE_STRING, G_TYPE_STRING);
  
  bamf_tab_source_signals [TAB_OPENED] = 
  	g_signal_new ("tab-opened",
  	              G_OBJECT_CLASS_TYPE (klass),
  	              G_SIGNAL_RUN_FIRST,
  	              0, 
  	              NULL, NULL,
  	              g_cclosure_marshal_VOID__STRING,
  	              G_TYPE_NONE, 1,
  	              G_TYPE_STRING);
  
  bamf_tab_source_signals [TAB_CLOSED] = 
  	g_signal_new ("tab-closed",
  	              G_OBJECT_CLASS_TYPE (klass),
  	              G_SIGNAL_RUN_FIRST,
  	              0, 
  	              NULL, NULL,
  	              g_cclosure_marshal_VOID__STRING,
  	              G_TYPE_NONE, 1,
  	              G_TYPE_STRING);
}

static void
bamf_tab_source_init (BamfTabSource *self)
{
  self->priv = BAMF_TAB_SOURCE_GET_PRIVATE (self);
}
