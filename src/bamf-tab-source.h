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

#ifndef __BAMFTABSOURCE_H__
#define __BAMFTABSOURCE_H__

#include "bamf.h"
#include <glib.h>
#include <glib-object.h>

#define BAMF_TYPE_TAB_SOURCE			(bamf_tab_source_get_type ())
#define BAMF_TAB_SOURCE(obj)			(G_TYPE_CHECK_INSTANCE_CAST ((obj), BAMF_TYPE_TAB_SOURCE, BamfTabSource))
#define BAMF_IS_TAB_SOURCE(obj)			(G_TYPE_CHECK_INSTANCE_TYPE ((obj), BAMF_TYPE_TAB_SOURCE))
#define BAMF_TAB_SOURCE_CLASS(klass)		(G_TYPE_CHECK_CLASS_CAST ((klass), BAMF_TYPE_TAB_SOURCE, BamfTabSourceClass))
#define BAMF_IS_TAB_SOURCE_CLASS(klass)		(G_TYPE_CHECK_CLASS_TYPE ((klass), BAMF_TYPE_TAB_SOURCE))
#define BAMF_TAB_SOURCE_GET_CLASS(obj)		(G_TYPE_INSTANCE_GET_CLASS ((obj), BAMF_TYPE_TAB_SOURCE, BamfTabSourceClass))

typedef struct _BamfTabSource BamfTabSource;
typedef struct _BamfTabSourceClass BamfTabSourceClass;
typedef struct _BamfTabSourcePrivate BamfTabSourcePrivate;

struct _BamfTabSourceClass
{
  GObjectClass parent;
  
  /*< signals >*/
  void    (*tab_uri_changed)   (char *id, char *old_uri, char *new_uri);
  void    (*tab_opened)        (char *id);
  void    (*tab_closed)        (char *id);
};

struct _BamfTabSource
{
  GObject parent;

  /* private */
  BamfTabSourcePrivate *priv;
};

GType           bamf_tab_source_get_type         (void) G_GNUC_CONST;

char         ** bamf_tab_source_tab_ids          (BamfTabSource *self);

void            bamf_tab_source_show_tab         (BamfTabSource *self, char *id);

GArray        * bamf_tab_source_get_tab_preview  (BamfTabSource *tab_source, char *id);

char          * bamf_tab_source_get_tab_uri      (BamfTabSource *tab_source, char *id);

guint32         bamf_tab_source_get_xid          (BamfTabSource *tab_source, char *id);

BamfTabSource * bamf_tab_source_new              (char *bus, char *path);

#endif
