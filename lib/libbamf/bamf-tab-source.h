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
 *              Neil Jagdish Patel <neil.patel@canonical.com>
 *              Marco Trevisan (Treviño) <3v1n0@ubuntu.com>
 *
 */

#ifndef __BAMF_TAB_SOURCE_H__
#define __BAMF_TAB_SOURCE_H__

#include <glib-object.h>
#include "bamf-tab.h"

G_BEGIN_DECLS

#define BAMF_TYPE_TAB_SOURCE            (bamf_tab_source_get_type ())
#define BAMF_TAB_SOURCE(obj)            (G_TYPE_CHECK_INSTANCE_CAST ((obj), BAMF_TYPE_TAB_SOURCE, BamfTabSource))
#define BAMF_TAB_SOURCE_CONST(obj)      (G_TYPE_CHECK_INSTANCE_CAST ((obj), BAMF_TYPE_TAB_SOURCE, BamfTabSource const))
#define BAMF_TAB_SOURCE_CLASS(klass)    (G_TYPE_CHECK_CLASS_CAST ((klass), BAMF_TYPE_TAB_SOURCE, BamfTabSourceClass))
#define BAMF_IS_TAB_SOURCE(obj)         (G_TYPE_CHECK_INSTANCE_TYPE ((obj), BAMF_TYPE_TAB_SOURCE))
#define BAMF_IS_TAB_SOURCE_CLASS(klass) (G_TYPE_CHECK_CLASS_TYPE ((klass), BAMF_TYPE_TAB_SOURCE))
#define BAMF_TAB_SOURCE_GET_CLASS(obj)  (G_TYPE_INSTANCE_GET_CLASS ((obj), BAMF_TYPE_TAB_SOURCE, BamfTabSourceClass))

#define BAMF_TAB_SOURCE_SIGNAL_TAB_OPENED      "tab-opened"
#define BAMF_TAB_SOURCE_SIGNAL_TAB_CLOSED      "tab-closed"
#define BAMF_TAB_SOURCE_SIGNAL_TAB_URI_CHANGED "tab-uri-changed"

typedef struct _BamfTabSource           BamfTabSource;
typedef struct _BamfTabSourceClass      BamfTabSourceClass;
typedef struct _BamfTabSourcePrivate    BamfTabSourcePrivate;

struct _BamfTabSource {
        GObject parent;

        BamfTabSourcePrivate *priv;
};

struct _BamfTabSourceClass {
        GObjectClass parent_class;

        /*< methods >*/
        void      (*show_tab)      (BamfTabSource *source, char *tab_id);
        char   ** (*tab_ids)       (BamfTabSource *source);
        GArray  * (*tab_preview)   (BamfTabSource *source, char *tab_id);
        char    * (*tab_uri)       (BamfTabSource *source, char *tab_id);
        guint32   (*tab_xid)       (BamfTabSource *source, char *tab_id);
};

GType bamf_tab_source_get_type (void) G_GNUC_CONST;

gboolean       bamf_tab_source_show_tab        (BamfTabSource *source,
                                                char *tab_id,
                                                GError *error);

char        ** bamf_tab_source_get_tab_ids     (BamfTabSource *source);

GArray       * bamf_tab_source_get_tab_preview (BamfTabSource *source,
                                                char *tab_id);

char         * bamf_tab_source_get_tab_uri     (BamfTabSource *source,
                                                char *tab_id);

guint32        bamf_tab_source_get_tab_xid     (BamfTabSource *source,
                                                char *tab_id);

G_END_DECLS

#endif /* __BAMF_TAB_SOURCE_H__ */
