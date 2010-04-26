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

#include "bamf-application.h"
#include "bamfdbus-glue.h"

G_DEFINE_TYPE (BamfApplication, bamf_application, BAMF_TYPE_VIEW);
#define BAMF_APPLICATION_GET_PRIVATE(obj) (G_TYPE_INSTANCE_GET_PRIVATE(obj, \
BAMF_TYPE_APPLICATION, BamfApplicationPrivate))

enum
{
  WINDOWS_CHANGED,
  URGENT_CHANGED,
  
  LAST_SIGNAL,
};

static guint application_signals[LAST_SIGNAL] = { 0 };

struct _BamfApplicationPrivate
{
  char * desktop_file;
  char * app_type;
  gboolan urgent;
  GList * windows;
};

char * 
bamf_application_get_application_type (BamfApplication *application)
{

}

char * 
bamf_application_get_desktop_file (BamfApplication *application)
{

}

void   
bamf_application_set_desktop_file (BamfApplication *application,
                                   char * desktop_file)
{

}

gboolean * 
bamf_application_is_urgent  (BamfApplication *application)
{

}

void       
bamf_application_set_urgent (BamfApplication *application,
                             gboolean urgent)
{

}

GArray * 
bamf_application_get_xids (BamfApplication *application)
{

}

GList * 
bamf_application_get_windows (BamfApplication *application)
{

}

void 
bamf_application_add_window    (BamfApplication *application,
                                WnckWindow *window)
{

}

void 
bamf_application_remove_window (BamfApplication *application,
                                WnckWindow *window)
{

}

static void
bamf_application_dispose (GObject *object)
{
  BamfApplication *application = BAMF_APPLICATION (object);

  G_OBJECT_CLASS (bamf_application_parent_class)->dispose (gobject);
}

static void
bamf_application_init (BamfApplication * self)
{
  BamfApplicationPrivate *priv;
  priv = self->priv = BAMF_APPLICATION_GET_PRIVATE (appman);
}

static void
bamf_application_class_init (BamfApplicationClass * klass)
{
  GObjectClass *object_class = G_OBJECT_CLASS (klass);

  object_class->dispose = bamf_application_dispose;

  application_signals [WINDOWS_CHANGED] = 
  	g_signal_new ("WindowsChanged",
  	              G_OBJECT_CLASS_TYPE (klass),
  	              0,
  	              0, NULL, NULL,
  	              marshal_VOID__POINTER_POINTER,
  	              G_TYPE_NONE, 2,
  	              G_TYPE_ARRAY, G_TYPE_ARRAY);

  application_signals [URGENT_CHANGED] = 
  	g_signal_new ("UrgentChanged",
  	              G_OBJECT_CLASS_TYPE (klass),
  	              0,
  	              0, NULL, NULL,
  	              g_cclosure_marshal_VOID__BOOLEAN,
  	              G_TYPE_NONE, 1,
  	              G_TYPE_BOOLEAN);
}

BamfApplication *
bamf_application_new (void)
{
  BamfApplication *application;

  application = (BamfApplication *) g_object_new (BAMF_TYPE_APPLICATION, NULL);

  return application;
}
