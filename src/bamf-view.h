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

#ifndef __BAMFVIEW_H__
#define __BAMFVIEW_H__

#include "bamf.h"
#include <glib.h>
#include <glib-object.h>
#include <dbus/dbus.h>
#include <dbus/dbus-glib.h>
#include <dbus/dbus-glib-lowlevel.h>

#define BAMF_TYPE_VIEW			(bamf_view_get_type ())
#define BAMF_VIEW(obj)			(G_TYPE_CHECK_INSTANCE_CAST ((obj), BAMF_TYPE_VIEW, BamfView))
#define BAMF_IS_VIEW(obj)				(G_TYPE_CHECK_INSTANCE_TYPE ((obj), BAMF_TYPE_VIEW))
#define BAMF_VIEW_CLASS(klass)		(G_TYPE_CHECK_CLASS_CAST ((klass), BAMF_TYPE_VIEW, BamfViewClass))
#define BAMF_IS_VIEW_CLASS(klass)		(G_TYPE_CHECK_CLASA_TYPE ((klass), BAMF_TYPE_VIEW))
#define BAMF_VIEW_GET_CLASS(obj)		(G_TYPE_INSTANCE_GET_CLASS ((obj), BAMF_TYPE_VIEW, BamfViewClass))

typedef struct _BamfView BamfView;
typedef struct _BamfViewClass BamfViewClass;
typedef struct _BamfViewPrivate BamfViewPrivate;

struct _BamfViewClass
{
  GObjectClass parent;

  char * (*view_type) (BamfView *view);
};

struct _BamfView
{
  GObject parent;

  /* private */
  BamfViewPrivate *priv;
};

GType bamf_view_get_type (void) G_GNUC_CONST;

GArray * bamf_view_get_children_paths (BamfView *view);

GList * bamf_view_get_children (BamfView *view);

void bamf_view_add_child (BamfView *view,
                          BamfView *child);

void bamf_view_remove_child (BamfView *view,
                             BamfView *child);

gboolean bamf_view_is_active  (BamfView *view);
void     bamf_view_set_active (BamfView *view,
                               gboolean active);

gboolean bamf_view_is_running  (BamfView *view);
void     bamf_view_set_running (BamfView *view,
                                gboolean running);

char * bamf_view_get_name (BamfView *view);
void   bamf_view_set_name (BamfView *view,
                           char * name);

char * bamf_view_get_parent_path (BamfView *view);

BamfView * bamf_view_get_parent (BamfView *view);
void       bamf_view_set_parent (BamfView *view,
                                 BamfView *parent);

char * bamf_view_get_view_type (BamfView *view);

char * bamf_view_export_on_bus (BamfView *view,
                                DBusGConnection *bus);

BamfView * bamf_view_new (void);

#endif
