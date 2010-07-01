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

#ifndef __BAMF_NOTIFICATION_APPROVER_H__
#define __BAMF_NOTIFICATION_APPROVER_H__

#include <glib-object.h>

G_BEGIN_DECLS

#define BAMF_TYPE_NOTIFICATION_APPROVER			(bamf_notification_approver_get_type ())
#define BAMF_NOTIFICATION_APPROVER(obj)			(G_TYPE_CHECK_INSTANCE_CAST ((obj), BAMF_TYPE_NOTIFICATION_APPROVER, BamfNotificationApprover))
#define BAMF_NOTIFICATION_APPROVER_CONST(obj)		(G_TYPE_CHECK_INSTANCE_CAST ((obj), BAMF_TYPE_NOTIFICATION_APPROVER, BamfNotificationApprover const))
#define BAMF_NOTIFICATION_APPROVER_CLASS(klass)		(G_TYPE_CHECK_CLASS_CAST ((klass), BAMF_TYPE_NOTIFICATION_APPROVER, BamfNotificationApproverClass))
#define BAMF_IS_NOTIFICATION_APPROVER(obj)		(G_TYPE_CHECK_INSTANCE_TYPE ((obj), BAMF_TYPE_NOTIFICATION_APPROVER))
#define BAMF_IS_NOTIFICATION_APPROVER_CLASS(klass)	(G_TYPE_CHECK_CLASS_TYPE ((klass), BAMF_TYPE_NOTIFICATION_APPROVER))
#define BAMF_NOTIFICATION_APPROVER_GET_CLASS(obj)	(G_TYPE_INSTANCE_GET_CLASS ((obj), BAMF_TYPE_NOTIFICATION_APPROVER, BamfNotificationApproverClass))

typedef struct _BamfNotificationApprover	BamfNotificationApprover;
typedef struct _BamfNotificationApproverClass	BamfNotificationApproverClass;
typedef struct _BamfNotificationApproverPrivate	BamfNotificationApproverPrivate;

struct _BamfNotificationApprover {
  GObject parent;
	
  BamfNotificationApproverPrivate *priv;
};

struct _BamfNotificationApproverClass {
  GObjectClass parent_class;
};

GType bamf_notification_approver_get_type (void) G_GNUC_CONST;

gboolean bamf_notification_approver_approve_item (BamfNotificationApprover *self,
                                                  const char *id,
                                                  const char *category,
                                                  const guint32 pid,
                                                  const char *address,
                                                  const char *path,
                                                  gboolean *approve,
                                                  GError **error);

BamfNotificationApprover *bamf_notification_approver_get_default (void);

G_END_DECLS

#endif /* __BAMF_NOTIFICATION_APPROVER_H__ */
