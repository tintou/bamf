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

#ifndef __BAMFWINDOW_H__
#define __BAMFWINDOW_H__

#include "bamf.h"
#include "bamf-view.h"
#include "bamf-legacy-window.h"
#include <glib.h>
#include <glib-object.h>

#define BAMF_TYPE_WINDOW			(bamf_window_get_type ())
#define BAMF_WINDOW(obj)			(G_TYPE_CHECK_INSTANCE_CAST ((obj), BAMF_TYPE_WINDOW, BamfWindow))
#define BAMF_IS_WINDOW(obj)			(G_TYPE_CHECK_INSTANCE_TYPE ((obj), BAMF_TYPE_WINDOW))
#define BAMF_WINDOW_CLASS(klass)		(G_TYPE_CHECK_CLASS_CAST ((klass), BAMF_TYPE_WINDOW, BamfWindowClass))
#define BAMF_IS_WINDOW_CLASS(klass)		(G_TYPE_CHECK_CLASS_TYPE ((klass), BAMF_TYPE_WINDOW))
#define BAMF_WINDOW_GET_CLASS(obj)		(G_TYPE_INSTANCE_GET_CLASS ((obj), BAMF_TYPE_WINDOW, BamfWindowClass))

typedef struct _BamfWindow BamfWindow;
typedef struct _BamfWindowClass BamfWindowClass;
typedef struct _BamfWindowPrivate BamfWindowPrivate;

struct _BamfWindowClass
{
  BamfViewClass parent;

  gboolean     (*user_visible) (BamfWindow *window);
  gboolean     (*is_urgent)    (BamfWindow *window);
  guint32      (*get_xid)      (BamfWindow *window);
  BamfLegacyWindow * (*get_window)   (BamfWindow *window);
};

struct _BamfWindow
{
  BamfView parent;

  /* private */
  BamfWindowPrivate *priv;
};

GType bamf_window_get_type (void) G_GNUC_CONST;

BamfLegacyWindow * bamf_window_get_window (BamfWindow *self);

gboolean bamf_window_is_urgent (BamfWindow *self);
void bamf_window_set_is_urgent (BamfWindow *self, gboolean urgent);

gboolean bamf_window_user_visible (BamfWindow *self);
void bamf_window_set_user_visible (BamfWindow *self, gboolean visible);

guint32 bamf_window_get_xid (BamfWindow *window);

BamfWindow * bamf_window_new (BamfLegacyWindow *window);

#endif
