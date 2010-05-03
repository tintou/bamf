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
#include "bamf-application-glue.h"
#include "bamf-window.h"
#include "marshal.h"

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
  gboolean urgent;
  GList * windows;
};

char * 
bamf_application_get_application_type (BamfApplication *application)
{
  g_return_val_if_fail (BAMF_IS_APPLICATION (application), NULL);

  return application->priv->app_type;
}

char * 
bamf_application_get_desktop_file (BamfApplication *application)
{
  g_return_val_if_fail (BAMF_IS_APPLICATION (application), NULL);

  return application->priv->desktop_file;
}

void   
bamf_application_set_desktop_file (BamfApplication *application,
                                   char * desktop_file)
{
  g_return_if_fail (BAMF_IS_APPLICATION (application));

  application->priv->desktop_file = g_strdup (desktop_file);
}

gboolean
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
  GList *l;
  GArray *xids;
  BamfView *view;
  guint32 xid;

  g_return_val_if_fail (BAMF_IS_APPLICATION (application), NULL);

  xids = g_array_new (FALSE, TRUE, sizeof (guint32));

  for (l = bamf_view_get_children (BAMF_VIEW (application)); l; l = l->next)
    {
      view = l->data;

      if (!BAMF_IS_WINDOW (view))
        continue;

      xid = wnck_window_get_xid (bamf_window_get_window (BAMF_WINDOW (view)));

      g_array_append_val (xids, xid);
    }

  return xids;
}

static void
bamf_application_dispose (GObject *object)
{
  G_OBJECT_CLASS (bamf_application_parent_class)->dispose (object);
}

static void
bamf_application_init (BamfApplication * self)
{
  BamfApplicationPrivate *priv;
  priv = self->priv = BAMF_APPLICATION_GET_PRIVATE (self);
}

static void
bamf_application_class_init (BamfApplicationClass * klass)
{
  GObjectClass *object_class = G_OBJECT_CLASS (klass);

  object_class->dispose = bamf_application_dispose;

  g_type_class_add_private (klass, sizeof (BamfApplicationPrivate));
  
  dbus_g_object_type_install_info (BAMF_TYPE_APPLICATION,
				   &dbus_glib_bamf_application_object_info);

  application_signals [WINDOWS_CHANGED] = 
  	g_signal_new ("windows-changed",
  	              G_OBJECT_CLASS_TYPE (klass),
  	              0,
  	              0, NULL, NULL,
  	              marshal_VOID__POINTER_POINTER,
  	              G_TYPE_NONE, 2,
  	              G_TYPE_STRV, G_TYPE_STRV);

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

  bamf_application_set_desktop_file (application, desktop_file);

  return application;
}
