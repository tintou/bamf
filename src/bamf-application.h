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

#ifndef __BAMFAPPLICATION_H__
#define __BAMFAPPLICATION_H__

#include "bamf.h"
#include "bamf-view.h"
#include <glib.h>
#include <glib-object.h>

#define BAMF_TYPE_APPLICATION			(bamf_application_get_type ())
#define BAMF_APPLICATION(obj)			(G_TYPE_CHECK_INSTANCE_CAST ((obj), BAMF_TYPE_APPLICATION, BamfApplication))
#define BAMF_IS_APPLICATION(obj)		(G_TYPE_CHECK_INSTANCE_TYPE ((obj), BAMF_TYPE_APPLICATION))
#define BAMF_APPLICATION_CLASS(klass)		(G_TYPE_CHECK_CLASS_CAST ((klass), BAMF_TYPE_APPLICATION, BamfApplicationClass))
#define BAMF_IS_APPLICATION_CLASS(klass)	(G_TYPE_CHECK_CLASS_TYPE ((klass), BAMF_TYPE_APPLICATION))
#define BAMF_APPLICATION_GET_CLASS(obj)		(G_TYPE_INSTANCE_GET_CLASS ((obj), BAMF_TYPE_APPLICATION, BamfApplicationClass))

typedef struct _BamfApplication BamfApplication;
typedef struct _BamfApplicationClass BamfApplicationClass;
typedef struct _BamfApplicationPrivate BamfApplicationPrivate;

struct _BamfApplicationClass
{
  BamfViewClass parent;
};

struct _BamfApplication
{
  BamfView parent;

  /* private */
  BamfApplicationPrivate *priv;
};

GType             bamf_application_get_type              (void) G_GNUC_CONST;

char            * bamf_application_get_application_type  (BamfApplication *application);

char            * bamf_application_get_desktop_file      (BamfApplication *application);
void              bamf_application_set_desktop_file      (BamfApplication *application,
                                                          char * desktop_file);
 
GArray          * bamf_application_get_xids              (BamfApplication *application);

gboolean          bamf_application_manages_xid           (BamfApplication *application,
                                                          guint32 xid);

BamfApplication * bamf_application_new                   (void);

BamfApplication * bamf_application_new_from_desktop_file (char * desktop_file);

#endif
