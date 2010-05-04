/*
 * Copyright 2009 Canonical Ltd.
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
 *              Neil Jagdish Patel <neil.patel@canonical.com>
 *
 */

#ifndef _BAMF_VIEW_H_
#define _BAMF_VIEW_H_

#include <glib-object.h>

G_BEGIN_DECLS

#define BAMF_TYPE_VIEW (bamf_view_get_type ())

#define BAMF_VIEW(obj) (G_TYPE_CHECK_INSTANCE_CAST ((obj),\
        BAMF_TYPE_VIEW, BamfView))

#define BAMF_VIEW_CLASS(klass) (G_TYPE_CHECK_CLASS_CAST ((klass),\
        BAMF_TYPE_VIEW, BamfViewClass))

#define BAMF_IS_VIEW(obj) (G_TYPE_CHECK_INSTANCE_TYPE ((obj),\
        BAMF_TYPE_VIEW))

#define BAMF_IS_VIEW_CLASS(klass) (G_TYPE_CHECK_CLASS_TYPE ((klass),\
        BAMF_TYPE_VIEW))

#define BAMF_VIEW_GET_CLASS(obj) (G_TYPE_INSTANCE_GET_CLASS ((obj),\
        BAMF_TYPE_VIEW, BamfViewClass))

typedef struct _BamfView        BamfView;
typedef struct _BamfViewClass   BamfViewClass;
typedef struct _BamfViewPrivate BamfViewPrivate;

struct _BamfView
{
  GObject parent;

  BamfViewPrivate *priv;
};

struct _BamfViewClass
{
  GObjectClass parent_class;

  /*< private >*/
  void (*_view_padding1) (void);
  void (*_view_padding2) (void);
  void (*_view_padding3) (void);
  void (*_view_padding4) (void);
  void (*_view_padding5) (void);
  void (*_view_padding6) (void);
};

GType      bamf_view_get_type             (void) G_GNUC_CONST;

GList    * bamf_view_get_children  (BamfView *view);

gboolean   bamf_view_is_active     (BamfView *view);

gboolean   bamf_view_is_running    (BamfView *view);

gchar    * bamf_view_get_name      (BamfView *view);

BamfView * bamf_view_get_parent    (BamfView *view);

gchar    * bamf_view_get_view_type (BamfView *view);

G_END_DECLS

#endif
