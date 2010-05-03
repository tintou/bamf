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
#include <string.h>

G_DEFINE_TYPE (BamfApplication, bamf_application, BAMF_TYPE_VIEW);
#define BAMF_APPLICATION_GET_PRIVATE(obj) (G_TYPE_INSTANCE_GET_PRIVATE(obj, \
BAMF_TYPE_APPLICATION, BamfApplicationPrivate))

enum
{
  WINDOW_ADDED,
  WINDOW_REMOVED,
  URGENT_CHANGED,
  
  LAST_SIGNAL,
};

static guint application_signals[LAST_SIGNAL] = { 0 };

struct _BamfApplicationPrivate
{
  char * desktop_file;
  char * app_type;
  gboolean urgent;
};

char * 
bamf_application_get_application_type (BamfApplication *application)
{
  g_return_val_if_fail (BAMF_IS_APPLICATION (application), NULL);

  return g_strdup (application->priv->app_type);
}

char * 
bamf_application_get_desktop_file (BamfApplication *application)
{
  g_return_val_if_fail (BAMF_IS_APPLICATION (application), NULL);

  return g_strdup (application->priv->desktop_file);
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

gboolean 
bamf_application_manages_xid (BamfApplication *application,
                              guint32 xid)
{
  GArray *xids;
  int i;
  gboolean result = FALSE;

  g_return_val_if_fail (BAMF_IS_APPLICATION (application), FALSE);

  xids = bamf_application_get_xids (application);

  for (i = 0; i < xids->len; i++)
    {
      if (g_array_index (xids, guint32, i) == xid)
        {
          result = TRUE;
          break;
        }
    }

  g_array_free (xids, TRUE);

  return result;
}

static char *
bamf_application_get_view_type (BamfView *view)
{
  return "application";
}

static void
bamf_application_ensure_state (BamfApplication *application)
{
  /* THIS METHOD SUCKS, FIXME */

  WnckWindow *active_window, *window;
  GArray *xids;
  guint32 xid;
  int i;
  gboolean active = FALSE;

  g_return_if_fail (BAMF_IS_APPLICATION (application));

  xids = bamf_application_get_xids (application);

  if (xids->len > 0)
    bamf_view_set_running (BAMF_VIEW (application), TRUE);
  else
    bamf_view_set_running (BAMF_VIEW (application), FALSE);

  active_window = wnck_screen_get_active_window (wnck_screen_get_default ());

  if (WNCK_IS_WINDOW (active_window))
    {
      for (i = 0; i < xids->len; i++)
        {
          xid = g_array_index (xids, guint32, i);
          window = wnck_window_get (xid);

          if (window == active_window)
            active = TRUE;
        }
    }
    
  bamf_view_set_active (BAMF_VIEW (application), active);
  g_array_free (xids, TRUE);
}

static void
bamf_application_child_added (BamfView *view, BamfView *child)
{
  BamfApplication *application;

  application = BAMF_APPLICATION (view);

  if (BAMF_IS_WINDOW (child))
    {
      g_signal_emit (BAMF_APPLICATION (view), application_signals[WINDOW_ADDED],0, bamf_view_get_path (view));
    }
  bamf_application_ensure_state (BAMF_APPLICATION (view));  
}

static void
bamf_application_child_removed (BamfView *view, BamfView *child)
{
  BamfApplication *application;

  application = BAMF_APPLICATION (view);

  if (BAMF_IS_WINDOW (child))
    {
      g_signal_emit (BAMF_APPLICATION (view), application_signals[WINDOW_REMOVED],0, bamf_view_get_path (view));
    }
  bamf_application_ensure_state (BAMF_APPLICATION (view));  
}

static void
active_window_changed (WnckScreen *screen, WnckWindow *previous, BamfApplication *app)
{
  bamf_application_ensure_state (app);
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
  BamfViewClass *view_class = BAMF_VIEW_CLASS (klass);

  object_class->dispose = bamf_application_dispose;

  view_class->view_type = bamf_application_get_view_type;
  view_class->child_added = bamf_application_child_added;
  view_class->child_removed = bamf_application_child_removed;

  g_type_class_add_private (klass, sizeof (BamfApplicationPrivate));
  
  dbus_g_object_type_install_info (BAMF_TYPE_APPLICATION,
				   &dbus_glib_bamf_application_object_info);

  application_signals [WINDOW_ADDED] = 
  	g_signal_new ("window-added",
  	              G_OBJECT_CLASS_TYPE (klass),
  	              0,
  	              0, NULL, NULL,
  	              g_cclosure_marshal_VOID__STRING,
  	              G_TYPE_NONE, 1,
  	              G_TYPE_STRING);

  application_signals [WINDOW_REMOVED] = 
  	g_signal_new ("window-removed",
  	              G_OBJECT_CLASS_TYPE (klass),
  	              0,
  	              0, NULL, NULL,
  	              g_cclosure_marshal_VOID__STRING,
  	              G_TYPE_NONE, 1,
  	              G_TYPE_STRING);

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


  g_signal_connect (G_OBJECT (wnck_screen_get_default ()), "active-window-changed",
		    (GCallback) active_window_changed, application);

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
