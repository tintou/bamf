/*
 * Copyright 2010 Canonical Ltd.
 *
 * This program is free software: you can redistribute it and/or modify it
 * under the terms of the the GNU General Public License version 3, as
 * published by the Free Software Foundation.
 *
 * This program is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranties of
 * MERCHANTABILITY, SATISFACTORY QUALITY or FITNESS FOR A PARTICULAR
 * PURPOSE.  See the applicable version of the GNU Lesser General Public
 * License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * version 3 along with this program.  If not, see
 * <http://www.gnu.org/licenses/>
 *
 * Authored by: Jason Smith <jason.smith@canonical.com>
 *
 */

#ifndef __BAMFTAB_H__
#define __BAMFTAB_H__

#include "bamf.h"
#include "bamf-view.h"
#include "bamf-tab-source.h"
#include <glib.h>
#include <glib-object.h>

#define BAMF_TYPE_TAB			(bamf_tab_get_type ())
#define BAMF_TAB(obj)			(G_TYPE_CHECK_INSTANCE_CAST ((obj), BAMF_TYPE_TAB, BamfTab))
#define BAMF_IS_TAB(obj)		(G_TYPE_CHECK_INSTANCE_TYPE ((obj), BAMF_TYPE_TAB))
#define BAMF_TAB_CLASS(klass)		(G_TYPE_CHECK_CLASS_CAST ((klass), BAMF_TYPE_TAB, BamfTabClass))
#define BAMF_IS_TAB_CLASS(klass)	(G_TYPE_CHECK_CLASS_TYPE ((klass), BAMF_TYPE_TAB))
#define BAMF_TAB_GET_CLASS(obj)		(G_TYPE_INSTANCE_GET_CLASS ((obj), BAMF_TYPE_TAB, BamfTabClass))

typedef struct _BamfTab BamfTab;
typedef struct _BamfTabClass BamfTabClass;
typedef struct _BamfTabPrivate BamfTabPrivate;

struct _BamfTabClass
{
  BamfViewClass parent;
};

struct _BamfTab
{
  BamfView parent;

  /* private */
  BamfTabPrivate *priv;
};

GType       bamf_tab_get_type    (void) G_GNUC_CONST;

char      * bamf_tab_current_uri (BamfTab *self);

void        bamf_tab_show        (BamfTab *self);

guint32     bamf_tab_parent_xid  (BamfTab *tab);

GArray    * bamf_tab_get_preview (BamfTab *tab);

BamfTab   * bamf_tab_new         (BamfTabSource *source, char *tab_id);

#endif
