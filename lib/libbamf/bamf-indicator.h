/*
 * Copyright 2010 Canonical Ltd.
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

#ifndef _BAMF_INDICATOR_H_
#define _BAMF_INDICATOR_H_

#include <time.h>
#include <glib-object.h>
#include <libbamf/bamf-view.h>

G_BEGIN_DECLS

#define BAMF_TYPE_INDICATOR (bamf_indicator_get_type ())

#define BAMF_INDICATOR(obj) (G_TYPE_CHECK_INSTANCE_CAST ((obj),\
        BAMF_TYPE_INDICATOR, BamfIndicator))

#define BAMF_INDICATOR_CLASS(klass) (G_TYPE_CHECK_CLASS_CAST ((klass),\
        BAMF_TYPE_INDICATOR, BamfIndicatorClass))

#define BAMF_IS_INDICATOR(obj) (G_TYPE_CHECK_INSTANCE_TYPE ((obj),\
        BAMF_TYPE_INDICATOR))

#define BAMF_IS_INDICATOR_CLASS(klass) (G_TYPE_CHECK_CLASS_TYPE ((klass),\
        BAMF_TYPE_INDICATOR))

#define BAMF_INDICATOR_GET_CLASS(obj) (G_TYPE_INSTANCE_GET_CLASS ((obj),\
        BAMF_TYPE_INDICATOR, BamfIndicatorClass))

typedef struct _BamfIndicator        BamfIndicator;
typedef struct _BamfIndicatorClass   BamfIndicatorClass;
typedef struct _BamfIndicatorPrivate BamfIndicatorPrivate;

struct _BamfIndicator
{
  BamfView parent;

  BamfIndicatorPrivate *priv;
};

struct _BamfIndicatorClass
{
  BamfViewClass parent_class;

  /*< private >*/
  void (*_indicator_padding1) (void);
  void (*_indicator_padding2) (void);
  void (*_indicator_padding3) (void);
  void (*_indicator_padding4) (void);
  void (*_indicator_padding5) (void);
  void (*_indicator_padding6) (void);
};

GType         bamf_indicator_get_type           (void) G_GNUC_CONST;

const gchar * bamf_indicator_get_remote_path    (BamfIndicator *self);

const gchar * bamf_indicator_get_remote_address (BamfIndicator *self);

G_END_DECLS

#endif
