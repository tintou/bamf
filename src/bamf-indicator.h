/*
 * Copyright (C) 2010-2011 Canonical Ltd
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 3 as
 * published by the Free Software Foundation.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 *
 * Authored by: Jason Smith <jason.smith@canonical.com>
 *              Marco Trevisan (Treviño) <3v1n0@ubuntu.com>
 *
 */

#ifndef __BAMF_INDICATOR_H__
#define __BAMF_INDICATOR_H__

#include <glib-object.h>
#include "bamf-view.h"

G_BEGIN_DECLS

#define BAMF_TYPE_INDICATOR		(bamf_indicator_get_type ())
#define BAMF_INDICATOR(obj)		(G_TYPE_CHECK_INSTANCE_CAST ((obj), BAMF_TYPE_INDICATOR, BamfIndicator))
#define BAMF_INDICATOR_CONST(obj)	(G_TYPE_CHECK_INSTANCE_CAST ((obj), BAMF_TYPE_INDICATOR, BamfIndicator const))
#define BAMF_INDICATOR_CLASS(klass)	(G_TYPE_CHECK_CLASS_CAST ((klass), BAMF_TYPE_INDICATOR, BamfIndicatorClass))
#define BAMF_IS_INDICATOR(obj)		(G_TYPE_CHECK_INSTANCE_TYPE ((obj), BAMF_TYPE_INDICATOR))
#define BAMF_IS_INDICATOR_CLASS(klass)	(G_TYPE_CHECK_CLASS_TYPE ((klass), BAMF_TYPE_INDICATOR))
#define BAMF_INDICATOR_GET_CLASS(obj)	(G_TYPE_INSTANCE_GET_CLASS ((obj), BAMF_TYPE_INDICATOR, BamfIndicatorClass))

typedef struct _BamfIndicator		BamfIndicator;
typedef struct _BamfIndicatorClass	BamfIndicatorClass;
typedef struct _BamfIndicatorPrivate	BamfIndicatorPrivate;

struct _BamfIndicator {
  BamfView parent;

  BamfIndicatorPrivate *priv;
};

struct _BamfIndicatorClass {
  BamfViewClass parent_class;
};

GType bamf_indicator_get_type (void) G_GNUC_CONST;

const char    * bamf_indicator_get_id            (BamfIndicator *self);

const char    * bamf_indicator_get_path          (BamfIndicator *self);

const char    * bamf_indicator_get_address       (BamfIndicator *self);

guint32         bamf_indicator_get_pid           (BamfIndicator *self);

gboolean        bamf_indicator_matches_signature (BamfIndicator *self, 
                                                  gint pid, 
                                                  const char *address, 
                                                  const char *path);

BamfIndicator * bamf_indicator_new               (const char *id,
                                                  const char *proxy, 
                                                  const char *address, 
                                                  guint32 pid);

G_END_DECLS

#endif /* __BAMF_INDICATOR_H__ */
