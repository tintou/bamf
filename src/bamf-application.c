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
  g_return_val_if_fail (BAMF_IS_APPLICATION (applicaton), NULL);

  return application->priv->app->type;
}

char * 
bamf_application_get_desktop_file (BamfApplication *application)
{
  g_return_val_if_fail (BAMF_IS_APPLICATION (applicaton), NULL);

  return application->priv->desktop_file;
}

void   
bamf_application_set_desktop_file (BamfApplication *application,
                                   char * desktop_file)
{
  g_return_if_fail (BAMF_IS_APPLICATION (application));

  application->priv->desktop_file = desktop_file;
}

gboolean * 
bamf_application_is_urgent  (BamfApplication *application)
{
  g_return_val_if_fail (BAMF_IS_APPLICATION (application), FALSE);

  return application->priv->urgent;
}

void       
bamf_application_set_urgent (BamfApplication *application,
                             gboolean urgent)
{
  g_return_if_fail (BAMF_IS_APPLICATION (application));

  if (application->priv->urgent == urgent)
    return;

  application->priv->urgent = urgent;
  g_signal_emit (application, application_signals[URGENT_CHANGED], 0, urgent);
}

GArray * 
bamf_application_get_xids (BamfApplication *application)
{
  GArray *xids;
  GList *windows, *w;
  guint32 xid;
  
  g_return_val_if_fail (BAMF_IS_APPLICATION (applicaton), NULL);

  xids = g_array_new (FALSE, TRUE, sizeof (guint32));
  windows = application->priv->windows;

  for (l = windows; l != null; l = l->next)
    {
      xid = wnck_window_get_xid (l->data);
      g_array_append_val (xids, xid);
    }

  return xids;
}

GList * 
bamf_application_get_windows (BamfApplication *application)
{
  g_return_val_if_fail (BAMF_IS_APPLICATION (applicaton), NULL);

  return application->priv->windows;
}

void 
bamf_application_add_window    (BamfApplication *application,
                                WnckWindow *window)
{
  g_return_if_fail (BAMF_IS_APPLICATION (application));

  application->priv->windows = g_list_prepend (application->priv->windows, window);
}

void 
bamf_application_remove_window (BamfApplication *application,
                                WnckWindow *window)
{
  g_return_if_fail (BAMF_IS_APPLICATION (application));

  application->priv->windows = g_list_remove (application->priv->windows, window);
}

void
bamf_application_update_windows (BamfApplication *application)
{
  g_return_if_fail (BAMF_IS_APPLICATION (application));

  
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
  	g_signal_new ("windows-changed",
  	              G_OBJECT_CLASS_TYPE (klass),
  	              0,
  	              0, NULL, NULL,
  	              marshal_VOID__POINTER_POINTER,
  	              G_TYPE_NONE, 2,
  	              G_TYPE_ARRAY, G_TYPE_ARRAY);

  application_signals [URGENT_CHANGED] = 
  	g_signal_new ("urgent-changed",
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

BamfApplication *
bamf_application_new_from_desktop_file (char * desktop_file)
{
  BamfApplication *application;

  application = bamf_application_new ();

  bamf_application_set_desktop_file (desktop_file);

  return application;
}
