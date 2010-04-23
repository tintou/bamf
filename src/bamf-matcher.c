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

#include "bamf-matcher.h"
#include "bamfdbus-glue.h"

G_DEFINE_TYPE (BamfMatcher, bamf_matcher, G_TYPE_OBJECT);
#define BAMF_MATCHER_GET_PRIVATE(obj) (G_TYPE_INSTANCE_GET_PRIVATE(obj, \
BAMF_TYPE_MATCHER, BamfMatcherPrivate))

enum
{
  VIEW_OPENED,
  VIEW_CLOSED,
  
  LAST_SIGNAL,
};

static guint matcher_signals[LAST_SIGNAL] = { 0 };

struct _BamfMatcherPrivate
{
};

static void
bamf_matcher_init (BamfMatcher * self)
{
  BamfMatcherPrivate *priv;
  priv = self->priv = BAMF_MATCHER_GET_PRIVATE (appman);
}

static void
bamf_matcher_class_init (BamfMatcherClass * klass)
{
  GObjectClass *object_class = G_OBJECT_CLASS (klass);

  matcher_signals [VIEW_OPENED] = 
  	g_signal_new ("ViewOpened",
  	              G_OBJECT_CLASS_TYPE (klass),
  	              0,
  	              0, NULL, NULL,
  	              marshal_VOID__STRING_STRING,
  	              G_TYPE_NONE, 2, 
  	              G_TYPE_STRING, G_TYPE_STRING);

  matcher_signals [VIEW_OPENED] = 
  	g_signal_new ("ViewClosed",
  	              G_OBJECT_CLASS_TYPE (klass),
  	              0,
  	              0, NULL, NULL,
  	              marshal_VOID__STRING_STRING,
  	              G_TYPE_NONE, 2, 
  	              G_TYPE_STRING, G_TYPE_STRING);
}

char * bamf_matcher_application_for_xid (BamfMatcher *matcher,
                                         guint32 xid)
{
  return NULL;
}

gboolean bamf_matcher_application_is_running (BamfMatcher *matcher,
                                              char *application)
{
  return FALSE;
}

GArray * bamf_matcher_application_dbus_paths (BamfMatcher *matcher)
{
  return NULL;
}

char * bamf_matcher_dbus_path_for_application (BamfMatcher *matcher,
                                               char *application)
{
  return NULL;
}

GArray * bamf_matcher_running_application_paths (BamfMatcher *matcher)
{
  return NULL;
}

GArray * bamf_matcher_tab_dbus_paths (BamfMatcher *matcher)
{
  return NULL;
}

GArray * bamf_matcher_xids_for_application (BamfMatcher *matcher,
                                            char *application)
{
  return NULL;
}

BamfMatcher *
bamf_matcher_get_default (void)
{
  static BamfMatcher *matcher;

  if (BAMF_IS_MATCHER (matcher))
    {
      matcher = (BamfMatcher *) g_object_new (BAMF_TYPE_MATCHER, NULL);
      return matcher;
    }

  return g_object_ref (matcher);
}
