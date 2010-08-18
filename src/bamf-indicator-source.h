/*
 * bamf-notification-approver.h
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

#ifndef __BAMF_INDICATOR_SOURCE_H__
#define __BAMF_INDICATOR_SOURCE_H__

#include <glib-object.h>
#include "bamf-indicator.h"

G_BEGIN_DECLS

#define BAMF_TYPE_INDICATOR_SOURCE			(bamf_indicator_source_get_type ())
#define BAMF_INDICATOR_SOURCE(obj)			(G_TYPE_CHECK_INSTANCE_CAST ((obj), BAMF_TYPE_INDICATOR_SOURCE, BamfIndicatorSource))
#define BAMF_INDICATOR_SOURCE_CONST(obj)		(G_TYPE_CHECK_INSTANCE_CAST ((obj), BAMF_TYPE_INDICATOR_SOURCE, BamfIndicatorSource const))
#define BAMF_INDICATOR_SOURCE_CLASS(klass)		(G_TYPE_CHECK_CLASS_CAST ((klass), BAMF_TYPE_INDICATOR_SOURCE, BamfIndicatorSourceClass))
#define BAMF_IS_INDICATOR_SOURCE(obj)		(G_TYPE_CHECK_INSTANCE_TYPE ((obj), BAMF_TYPE_INDICATOR_SOURCE))
#define BAMF_IS_INDICATOR_SOURCE_CLASS(klass)	(G_TYPE_CHECK_CLASS_TYPE ((klass), BAMF_TYPE_INDICATOR_SOURCE))
#define BAMF_INDICATOR_SOURCE_GET_CLASS(obj)	(G_TYPE_INSTANCE_GET_CLASS ((obj), BAMF_TYPE_INDICATOR_SOURCE, BamfIndicatorSourceClass))

typedef struct _BamfIndicatorSource	BamfIndicatorSource;
typedef struct _BamfIndicatorSourceClass	BamfIndicatorSourceClass;
typedef struct _BamfIndicatorSourcePrivate	BamfIndicatorSourcePrivate;

enum
{
  BAMF_INDICATOR_SOURCE_APPROVE_NONE,
  BAMF_INDICATOR_SOURCE_APPROVE_MATCHED,
  BAMF_INDICATOR_SOURCE_APPROVE_ALL
};

struct _BamfIndicatorSource {
  GObject parent;
	
  BamfIndicatorSourcePrivate *priv;
};

struct _BamfIndicatorSourceClass {
  GObjectClass parent_class;
  
  /*< signals >*/
  void (*indicator_opened) (BamfIndicatorSource *approver, BamfIndicator *indicator);
  void (*indicator_closed) (BamfIndicatorSource *approver, BamfIndicator *indicator);
  void (*revise_judgement) (BamfIndicatorSource *approver, gboolean approve, char *addr, char *path);
  
  void (*padding1) (void);
  void (*padding2) (void);
  void (*padding3) (void);
  void (*padding4) (void);
  void (*padding5) (void);
};

GType bamf_indicator_source_get_type (void) G_GNUC_CONST;

GList * bamf_indicator_source_get_indicators (BamfIndicatorSource *self);

gboolean bamf_indicator_source_approve_item (BamfIndicatorSource *self,
                                                  const char *id,
                                                  const char *category,
                                                  guint32 pid,
                                                  const char *address,
                                                  const char *proxy,
                                                  gboolean *approve,
                                                  GError **error);
                                                  
void bamf_indicator_source_set_behavior (BamfIndicatorSource *self,
                                         gint32 behavior);

BamfIndicatorSource *bamf_indicator_source_get_default (void);

G_END_DECLS

#endif /* __BAMF_INDICATOR_SOURCE_H__ */
