/*
 * Copyright 2010-2011 Canonical Ltd.
 *
 * This program is free software: you can redistribute it and/or modify it
 * under the terms of either or both of the following licenses:
 *
 * 1) the GNU Lesser General Public License version 3, as published by the
 * Free Software Foundation; and/or
 * 2) the GNU Lesser General Public License version 2.1, as published by
 * the Free Software Foundation.
 *
 * This program is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranties of
 * MERCHANTABILITY, SATISFACTORY QUALITY or FITNESS FOR A PARTICULAR
 * PURPOSE.  See the applicable version of the GNU Lesser General Public
 * License for more details.
 *
 * You should have received a copy of both the GNU Lesser General Public
 * License version 3 and version 2.1 along with this program.  If not, see
 * <http://www.gnu.org/licenses/>
 *
 * Authored by: Jason Smith <jason.smith@canonical.com>
 *              Marco Trevisan (Treviño) <3v1n0@ubuntu.com>
 *
 */

#include "bamf-tab.h"
#include "bamf-marshal.h"


#define BAMF_TAB_GET_PRIVATE(object)(G_TYPE_INSTANCE_GET_PRIVATE((object), BAMF_TYPE_TAB, BamfTabPrivate))

enum
{
  PROP_0,
  
  PROP_ID,
  PROP_URI,
  PROP_PREVIEW,
};

enum
{
  URI_CHANGED,
  PREVIEW_UPDATED,

  LAST_SIGNAL,
};

static guint tab_signals[LAST_SIGNAL] = { 0 };

struct _BamfTabPrivate
{
  gchar *uri;
  gchar *preview_uri;
  gchar *id;
};

G_DEFINE_TYPE (BamfTab, bamf_tab, BAMF_TYPE_VIEW)

gchar *
bamf_tab_get_id (BamfTab *tab)
{
  g_return_val_if_fail (BAMF_IS_TAB (tab), NULL);
  
  return tab->priv->id;
}

gchar *
bamf_tab_get_preview (BamfTab *tab)
{
  g_return_val_if_fail (BAMF_IS_TAB (tab), NULL);
  
  return tab->priv->preview_uri;
}

void
bamf_tab_set_preview (BamfTab *tab, gchar *uri)
{
  g_return_if_fail (BAMF_IS_TAB (tab));
  
  tab->priv->preview_uri = uri;
}

gchar *
bamf_tab_get_uri (BamfTab *tab)
{
  g_return_val_if_fail (BAMF_IS_TAB (tab), NULL);
  
  return tab->priv->uri;
}

void
bamf_tab_set_uri (BamfTab *tab, gchar *uri)
{
  gchar *old;

  g_return_if_fail (BAMF_IS_TAB (tab));
  
  old = tab->priv->uri;
  tab->priv->uri = uri;

  g_signal_emit (tab, tab_signals[URI_CHANGED], 0, old, uri);  
}

void bamf_tab_show (BamfTab *self)
{
  if (BAMF_TAB_GET_CLASS (self)->show)
    BAMF_TAB_GET_CLASS (self)->show (self);
  else
    g_warning ("Default tab class implementation cannot perform show!\n");
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
      case PROP_URI:
        bamf_tab_set_uri (self, g_value_dup_string (value));
        break;
      case PROP_PREVIEW:
        bamf_tab_set_preview (self, g_value_dup_string (value));
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
        g_value_set_string (value, bamf_tab_get_id (self));
        break;
      case PROP_URI:
        g_value_set_string (value, bamf_tab_get_uri (self));
        break;
      case PROP_PREVIEW:
        g_value_set_string (value, bamf_tab_get_preview (self));
        break;

      default:
        G_OBJECT_WARN_INVALID_PROPERTY_ID(object, property_id, pspec);
    }
}

static void
bamf_tab_constructed (GObject *object)
{
  G_OBJECT_CLASS (bamf_tab_parent_class)->constructed (object);
}

static void
bamf_tab_dispose (GObject *object)
{
  G_OBJECT_CLASS (bamf_tab_parent_class)->dispose (object);
}

static void
bamf_tab_class_init (BamfTabClass *klass)
{
  GParamSpec *pspec;
  GObjectClass *object_class = G_OBJECT_CLASS (klass);
          
  object_class->constructed = bamf_tab_constructed;
  object_class->dispose = bamf_tab_dispose;
  object_class->set_property = bamf_tab_set_property;
  object_class->get_property = bamf_tab_get_property;

  pspec = g_param_spec_string ("id", "id", "id", NULL, G_PARAM_READWRITE | G_PARAM_CONSTRUCT);
  g_object_class_install_property (object_class, PROP_ID, pspec);

  pspec = g_param_spec_string ("uri", "uri", "uri", NULL, G_PARAM_READWRITE | G_PARAM_CONSTRUCT);
  g_object_class_install_property (object_class, PROP_URI, pspec);

  pspec = g_param_spec_string ("preview", "preview", "preview", NULL, G_PARAM_READWRITE);
  g_object_class_install_property (object_class, PROP_PREVIEW, pspec);

  g_type_class_add_private (object_class, sizeof (BamfTabPrivate));
  
  tab_signals [URI_CHANGED] = 
    g_signal_new (BAMF_TAB_SIGNAL_URI_CHANGED,
                  G_OBJECT_CLASS_TYPE (klass),
                  G_SIGNAL_RUN_FIRST,
                  G_STRUCT_OFFSET (BamfTabClass, uri_changed),
                  NULL, NULL,
                  bamf_marshal_VOID__STRING_STRING,
                  G_TYPE_NONE, 2, 
                  G_TYPE_STRING, G_TYPE_STRING);

  tab_signals [PREVIEW_UPDATED] = 
    g_signal_new (BAMF_TAB_SIGNAL_PREVIEW_UPDATED,
                  G_OBJECT_CLASS_TYPE (klass),
                  G_SIGNAL_RUN_FIRST,
                  G_STRUCT_OFFSET (BamfTabClass, preview_updated),
                  NULL, NULL,
                  g_cclosure_marshal_VOID__VOID,
                  G_TYPE_NONE, 0);
}

static void
bamf_tab_init (BamfTab *self)
{
  self->priv = BAMF_TAB_GET_PRIVATE (self);
}

BamfTab *
bamf_tab_new (gchar *id, gchar *uri)
{
  return g_object_new (BAMF_TYPE_TAB, NULL);
}
