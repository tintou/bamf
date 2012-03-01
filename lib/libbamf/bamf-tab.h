/*
 * bamf-tab.h
 * This file is part of BAMF
 *
 * Copyright (C) 2010 - Jason Smith
 *
 * BAMF is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * BAMF is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with BAMF; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin St, Fifth Floor, 
 * Boston, MA  02110-1301  USA
 */



#ifndef __BAMF_TAB_H__
#define __BAMF_TAB_H__

#include <libbamf/bamf-view.h>
#include <glib-object.h>

G_BEGIN_DECLS

#define BAMF_TYPE_TAB			(bamf_tab_get_type ())
#define BAMF_TAB(obj)			(G_TYPE_CHECK_INSTANCE_CAST ((obj), BAMF_TYPE_TAB, BamfTab))
#define BAMF_TAB_CONST(obj)		(G_TYPE_CHECK_INSTANCE_CAST ((obj), BAMF_TYPE_TAB, BamfTab const))
#define BAMF_TAB_CLASS(klass)		(G_TYPE_CHECK_CLASS_CAST ((klass), BAMF_TYPE_TAB, BamfTabClass))
#define BAMF_IS_TAB(obj)		(G_TYPE_CHECK_INSTANCE_TYPE ((obj), BAMF_TYPE_TAB))
#define BAMF_IS_TAB_CLASS(klass)	(G_TYPE_CHECK_CLASS_TYPE ((klass), BAMF_TYPE_TAB))
#define BAMF_TAB_GET_CLASS(obj)		(G_TYPE_INSTANCE_GET_CLASS ((obj), BAMF_TYPE_TAB, BamfTabClass))

typedef struct _BamfTab		BamfTab;
typedef struct _BamfTabClass	BamfTabClass;
typedef struct _BamfTabPrivate	BamfTabPrivate;

struct _BamfTab {
  BamfView parent;
	
  BamfTabPrivate *priv;
};

struct _BamfTabClass {
  BamfViewClass parent_class;
};

typedef void (*BamfTabPreviewReadyCallback) (BamfTab *self, const gchar *preview_data, gpointer user_data);

GType bamf_tab_get_type (void) G_GNUC_CONST;

BamfTab * bamf_tab_new         (const gchar *path);

gboolean bamf_tab_raise        (BamfTab *self);
gboolean bamf_tab_close        (BamfTab *self);
void bamf_tab_request_preview (BamfTab *self, BamfTabPreviewReadyCallback callback, gpointer user_data);

const gchar *bamf_tab_get_desktop_name (BamfTab *self);
const gchar *bamf_tab_get_location (BamfTab *self);
guint64 bamf_tab_get_xid (BamfTab *self);




G_END_DECLS

#endif /* __BAMF_TAB_H__ */
