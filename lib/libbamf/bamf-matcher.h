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

#ifndef _BAMF_MATCHER_H_
#define _BAMF_MATCHER_H_

#include <glib-object.h>
#include <libbamf/bamf-application.h>

G_BEGIN_DECLS

#define BAMF_TYPE_MATCHER (bamf_matcher_get_type ())

#define BAMF_MATCHER(obj) (G_TYPE_CHECK_INSTANCE_CAST ((obj),\
        BAMF_TYPE_MATCHER, BamfMatcher))

#define BAMF_MATCHER_CLASS(klass) (G_TYPE_CHECK_CLASS_CAST ((klass),\
        BAMF_TYPE_MATCHER, BamfMatcherClass))

#define BAMF_IS_MATCHER(obj) (G_TYPE_CHECK_INSTANCE_TYPE ((obj),\
        BAMF_TYPE_MATCHER))

#define BAMF_IS_MATCHER_CLASS(klass) (G_TYPE_CHECK_CLASS_TYPE ((klass),\
        BAMF_TYPE_MATCHER))

#define BAMF_MATCHER_GET_CLASS(obj) (G_TYPE_INSTANCE_GET_CLASS ((obj),\
        BAMF_TYPE_MATCHER, BamfMatcherClass))

typedef struct _BamfMatcher        BamfMatcher;
typedef struct _BamfMatcherClass   BamfMatcherClass;
typedef struct _BamfMatcherPrivate BamfMatcherPrivate;

struct _BamfMatcher
{
  GObject           parent;

  BamfMatcherPrivate *priv;
};

struct _BamfMatcherClass
{
  GObjectClass parent_class;

  /*< private >*/
  void (*_matcher_padding1) (void);
  void (*_matcher_padding2) (void);
  void (*_matcher_padding3) (void);
  void (*_matcher_padding4) (void);
  void (*_matcher_padding5) (void);
  void (*_matcher_padding6) (void);
};

GType             bamf_matcher_get_type                 (void) G_GNUC_CONST;

/**
 * bamf_matcher_get_default:
 * @matcher: a #BamfMatcher
 *
 * Returns the default matcher. This matcher is owned by bamf and shared between other callers.
 *
 * Returns: (transfer none): A new #BamfMatcher
 */
BamfMatcher *     bamf_matcher_get_default              (void);

/**
 * bamf_matcher_get_application_for_xid:
 * @matcher: a #BamfMatcher
 *
 * Used to fetch the #BamfApplication containing the passed xid.
 *
 * Returns: (transfer container): The #BamfApplication representing the xid passed, or NULL if none exists.
 */
BamfApplication * bamf_matcher_get_application_for_xid  (BamfMatcher *matcher,
                                                         guint32      xid);

gboolean          bamf_matcher_application_is_running   (BamfMatcher *matcher,
                                                         const gchar *application);

/**
 * bamf_matcher_get_applications:
 * @matcher: a #BamfMatcher
 *
 * Used to fetch all #BamfApplication's running or not. Application authors who wish to only 
 * see running applications should use bamf_matcher_get_running_applications instead. The reason
 * this method is needed is bamf will occasionally track applications which are not currently
 * running for nefarious purposes.
 *
 * Returns: (element-type Bamf.Application) (transfer conainer): A list of #BamfApplication's.
 */
GList *           bamf_matcher_get_applications         (BamfMatcher *matcher);

/**
 * bamf_matcher_get_running_applications:
 * @matcher: a #BamfMatcher
 *
 * Used to fetch all #BamfApplication's which are running.
 *
 * Returns: (element-type Bamf.Application) (transfer conainer): A list of #BamfApplication's.
 */
GList *           bamf_matcher_get_running_applications (BamfMatcher *matcher);

/**
 * bamf_matcher_get_tabs:
 * @matcher: a #BamfMatcher
 *
 * Used to fetch all #BamfView's representing tabs. Currently unused.
 *
 * Returns: (element-type Bamf.View) (transfer conainer): A list of #BamfViews's.
 */
GList *           bamf_matcher_get_tabs                 (BamfMatcher *matcher);

/**
 * bamf_matcher_get_applications:
 * @matcher: a #BamfMatcher
 *
 * Used to fetch all xid's associated with an application. Useful for performing window
 * 
 *
 * Returns: (element-type guint32) (transfer conainer): A list of xids.
 */
GArray *          bamf_matcher_get_xids_for_application (BamfMatcher *matcher,
                                                         const gchar *application);

G_END_DECLS

#endif
