/*
 * Copyright 2010-2011 Canonical Ltd.
 *
 * This program is free software: you can redistribute it and/or modify it
 * under the terms of either or both of the following licenses:
 *
 * 1) the GNU Lesser General Public License version 3, as published by the
 * Free Software Foundation; and/or
 * 2) the GNU Lesser General Public License version 2.1, as published by
 * the Free Software Foundation.
 *
 * This program is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranties of
 * MERCHANTABILITY, SATISFACTORY QUALITY or FITNESS FOR A PARTICULAR
 * PURPOSE.  See the applicable version of the GNU Lesser General Public
 * License for more details.
 *
 * You should have received a copy of both the GNU Lesser General Public
 * License version 3 and version 2.1 along with this program.  If not, see
 * <http://www.gnu.org/licenses/>
 *
 * Authored by: Jason Smith <jason.smith@canonical.com>
 *              Marco Trevisan (Treviño) <3v1n0@ubuntu.com>
 *
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

#define BAMF_TAB_SIGNAL_URI_CHANGED     "uri-changed"
#define BAMF_TAB_SIGNAL_PREVIEW_UPDATED "preview-updated"

typedef struct _BamfTab		BamfTab;
typedef struct _BamfTabClass	BamfTabClass;
typedef struct _BamfTabPrivate	BamfTabPrivate;

struct _BamfTab {
  BamfView parent;
	
  BamfTabPrivate *priv;
};

struct _BamfTabClass {
  BamfViewClass parent_class;

  void (*show) (BamfTab *self);

  /*< signals >*/
  void  (*uri_changed)      (char *old_uri, char *new_uri);
  void  (*preview_updated)  (void);
  
  void (*_tab_padding1) (void);
  void (*_tab_padding2) (void);
  void (*_tab_padding3) (void);
  void (*_tab_padding4) (void);
  void (*_tab_padding5) (void);
  void (*_tab_padding6) (void);
};

GType bamf_tab_get_type (void) G_GNUC_CONST;

gchar   * bamf_tab_get_id      (BamfTab *self);

gchar   * bamf_tab_get_preview (BamfTab *self);

void      bamf_tab_set_preview (BamfTab *self,
                                gchar *uri);

gchar   * bamf_tab_get_uri     (BamfTab *self);

void      bamf_tab_set_uri     (BamfTab *self,
                                gchar *uri);
                                
void      bamf_tab_show        (BamfTab *self);

BamfTab * bamf_tab_new         (gchar *id, gchar *uri);


G_END_DECLS

#endif /* __BAMF_TAB_H__ */
