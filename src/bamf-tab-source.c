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
//  MERCHANTAB_SOURCEILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//  GNU General Public License for more details.
// 
//  You should have received a copy of the GNU General Public License
//  along with this program.  If not, see <http://www.gnu.org/licenses/>.
// 

#include "bamf-tab-source.h"
#include "bamf-marshal.h"
#include <dbus/dbus-glib.h>

G_DEFINE_TYPE (BamfTabSource, bamf_tab_source, G_TYPE_OBJECT);
#define BAMF_TAB_SOURCE_GET_PRIVATE(obj) (G_TYPE_INSTANCE_GET_PRIVATE(obj, \
BAMF_TYPE_TAB_SOURCE, BamfTabSourcePrivate))

enum
{
  PROP_0,

  PROP_BUS,
  PROP_PATH,
};

enum
{
  TAB_OPENED,
  TAB_CLOSED,
  TAB_URI_CHANGED,
  
  LAST_SIGNAL,
};

static guint tab_source_signals[LAST_SIGNAL] = { 0 };

struct _BamfTabSourcePrivate
{
  char *bus;
  char *path;
  DBusGProxy *proxy;
};

static void
bamf_tab_source_set_property (GObject *object, guint property_id, const GValue *value, GParamSpec *pspec)
{
  BamfTabSource *self;

  self = BAMF_TAB_SOURCE (object);

  switch (property_id)
    {
      case PROP_PATH:
        self->priv->path = g_strdup (g_value_get_string (value));
        break;
      case PROP_BUS:
        self->priv->bus = g_strdup (g_value_get_string (value));
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
      case PROP_PATH:
        g_value_set_string (value, self->priv->path);
        break;
      case PROP_BUS:
        g_value_set_string (value, self->priv->path);
        break;        
      default:
        G_OBJECT_WARN_INVALID_PROPERTY_ID(object, property_id, pspec);
    }
}

static void
bamf_tab_source_constructed (GObject *object)
{
  BamfTabSource *source;
  BamfTabSourcePrivate *priv;
  DBusGConnection *connection;
  GError *error = NULL;
 
  if (G_OBJECT_CLASS (bamf_tab_source_parent_class)->constructed)
    G_OBJECT_CLASS (bamf_tab_source_parent_class)->constructed (object);
  
  source = BAMF_TAB_SOURCE (object);
  priv = source->priv;
  
  connection = dbus_g_bus_get (DBUS_BUS_SESSION, &error);
  if (connection == NULL)
    {
      g_critical ("Failed to open connection to bus: %s",
               error != NULL ? error->message : "Unknown");
      if (error)
        g_error_free (error);
      return;
    }
  
  priv->proxy = dbus_g_proxy_new_for_name (connection,
                                           priv->bus,
                                           priv->path,
                                           "org.ayatana.bamf.browser");
  
  if (priv->proxy == NULL)
    {
      g_critical ("Unable to get org.ayatana.bamf.browser object from bus");
    }
}

static void
bamf_tab_source_dispose (GObject *object)
{
  BamfTabSource *self;

  self = BAMF_TAB_SOURCE (object);

  G_OBJECT_CLASS (bamf_tab_source_parent_class)->dispose (object);
}

static void
bamf_tab_source_init (BamfTabSource * self)
{
  BamfTabSourcePrivate *priv;
  priv = self->priv = BAMF_TAB_SOURCE_GET_PRIVATE (self);
}

static void
bamf_tab_source_class_init (BamfTabSourceClass * klass)
{
  GParamSpec *pspec;
  GObjectClass *object_class = G_OBJECT_CLASS (klass);

  object_class->dispose      = bamf_tab_source_dispose;
  object_class->set_property = bamf_tab_source_set_property;
  object_class->get_property = bamf_tab_source_get_property;
  object_class->constructed  = bamf_tab_source_constructed;
  
  g_type_class_add_private (klass, sizeof (BamfTabSourcePrivate));
  
  pspec = g_param_spec_string ("path", "path", "path", NULL, G_PARAM_READWRITE | G_PARAM_CONSTRUCT);
  g_object_class_install_property (object_class, PROP_PATH, pspec);
  
  pspec = g_param_spec_string ("bus", "bus", "bus", NULL, G_PARAM_READWRITE | G_PARAM_CONSTRUCT);
  g_object_class_install_property (object_class, PROP_BUS, pspec);
  
  tab_source_signals [TAB_URI_CHANGED] = 
  	g_signal_new ("tab-uri-changed",
  	              G_OBJECT_CLASS_TYPE (klass),
  	              0,
  	              0, NULL, NULL,
  	              bamf_marshal_VOID__STRING_STRING_STRING,
  	              G_TYPE_NONE, 3, 
  	              G_TYPE_STRING, G_TYPE_STRING, G_TYPE_STRING);
  
  tab_source_signals [TAB_OPENED] = 
  	g_signal_new ("tab-opened",
  	              G_OBJECT_CLASS_TYPE (klass),
  	              0,
  	              0, NULL, NULL,
  	              g_cclosure_marshal_VOID__STRING,
  	              G_TYPE_NONE, 1, 
  	              G_TYPE_STRING);
  
  tab_source_signals [TAB_CLOSED] = 
  	g_signal_new ("tab-closed",
  	              G_OBJECT_CLASS_TYPE (klass),
  	              0,
  	              0, NULL, NULL,
  	              g_cclosure_marshal_VOID__STRING,
  	              G_TYPE_NONE, 1, 
  	              G_TYPE_STRING);
}

BamfTabSource *
bamf_tab_source_new (char *bus, char *path)
{
  BamfTabSource *self;
  self = (BamfTabSource *) g_object_new (BAMF_TYPE_TAB_SOURCE, 
                                         "path", path, 
                                         "bus", bus,
                                         NULL);

  return self;
}
