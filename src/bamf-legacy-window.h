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

#ifndef __BAMFLEGACY_WINDOW_H__
#define __BAMFLEGACY_WINDOW_H__

#include "bamf.h"
#include "bamf-view.h"
#include <glib.h>
#include <glib-object.h>
#include <libwnck/libwnck.h>

#define BAMF_TYPE_LEGACY_WINDOW			(bamf_legacy_window_get_type ())
#define BAMF_LEGACY_WINDOW(obj)			(G_TYPE_CHECK_INSTANCE_CAST ((obj), BAMF_TYPE_LEGACY_WINDOW, BamfLegacyWindow))
#define BAMF_IS_LEGACY_WINDOW(obj)		(G_TYPE_CHECK_INSTANCE_TYPE ((obj), BAMF_TYPE_LEGACY_WINDOW))
#define BAMF_LEGACY_WINDOW_CLASS(klass)		(G_TYPE_CHECK_CLASS_CAST ((klass), BAMF_TYPE_LEGACY_WINDOW, BamfLegacyWindowClass))
#define BAMF_IS_LEGACY_WINDOW_CLASS(klass)	(G_TYPE_CHECK_CLASS_TYPE ((klass), BAMF_TYPE_LEGACY_WINDOW))
#define BAMF_LEGACY_WINDOW_GET_CLASS(obj)	(G_TYPE_INSTANCE_GET_CLASS ((obj), BAMF_TYPE_LEGACY_WINDOW, BamfLegacyWindowClass))

typedef struct _BamfLegacyWindow BamfLegacyWindow;
typedef struct _BamfLegacyWindowClass BamfLegacyWindowClass;
typedef struct _BamfLegacyWindowPrivate BamfLegacyWindowPrivate;

struct _BamfLegacyWindowClass
{
  GObjectClass parent;
  
  char   * (*get_name)           (BamfLegacyWindow *legacy_window);
  gint     (*get_pid)            (BamfLegacyWindow *legacy_window);
  guint32  (*get_xid)            (BamfLegacyWindow *legacy_window);
  gboolean (*needs_attention)    (BamfLegacyWindow *legacy_window);
  gboolean (*is_skip_tasklist)   (BamfLegacyWindow *legacy_window);
  gboolean (*is_desktop)         (BamfLegacyWindow *legacy_window);

  /*< signals >*/
  void     (*name_changed)    (BamfLegacyWindow *legacy_window);
  void     (*state_changed)   (BamfLegacyWindow *legacy_window);
  void     (*closed)          (BamfLegacyWindow *legacy_window);
};

struct _BamfLegacyWindow
{
  GObject parent;

  /* private */
  BamfLegacyWindowPrivate *priv;
};

GType              bamf_legacy_window_get_type          (void) G_GNUC_CONST;

guint32            bamf_legacy_window_get_xid           (BamfLegacyWindow *self);

gint               bamf_legacy_window_get_pid           (BamfLegacyWindow *self);

gboolean           bamf_legacy_window_is_active         (BamfLegacyWindow *self);

gboolean           bamf_legacy_window_is_skip_tasklist  (BamfLegacyWindow *self);

gboolean           bamf_legacy_window_needs_attention   (BamfLegacyWindow *self);

gboolean           bamf_legacy_window_is_desktop_window (BamfLegacyWindow *self);

const char       * bamf_legacy_window_get_class_name    (BamfLegacyWindow *self);

const char       * bamf_legacy_window_get_name          (BamfLegacyWindow *self);

BamfLegacyWindow * bamf_legacy_window_new               (WnckWindow *legacy_window);

#endif
