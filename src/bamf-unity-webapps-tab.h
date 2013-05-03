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

#ifndef __BAMF_UNITY_WEBAPPS_TAB_H__
#define __BAMF_UNITY_WEBAPPS_TAB_H__

#include <unity-webapps-context.h>

#include "bamf-legacy-window.h"
#include "bamf-tab.h"


#define BAMF_TYPE_UNITY_WEBAPPS_TAB                     (bamf_unity_webapps_tab_get_type ())
#define BAMF_UNITY_WEBAPPS_TAB(obj)                     (G_TYPE_CHECK_INSTANCE_CAST ((obj), BAMF_TYPE_UNITY_WEBAPPS_TAB, BamfUnityWebappsTab))
#define BAMF_IS_UNITY_WEBAPPS_TAB(obj)          (G_TYPE_CHECK_INSTANCE_TYPE ((obj), BAMF_TYPE_UNITY_WEBAPPS_TAB))
#define BAMF_UNITY_WEBAPPS_TAB_CLASS(klass)             (G_TYPE_CHECK_CLASS_CAST ((klass), BAMF_TYPE_UNITY_WEBAPPS_TAB, BamfUnityWebappsTabClass))
#define BAMF_IS_UNITY_WEBAPPS_TAB_CLASS(klass)  (G_TYPE_CHECK_CLASS_TYPE ((klass), BAMF_TYPE_UNITY_WEBAPPS_TAB))
#define BAMF_UNITY_WEBAPPS_TAB_GET_CLASS(obj)           (G_TYPE_INSTANCE_GET_CLASS ((obj), BAMF_TYPE_UNITY_WEBAPPS_TAB, BamfUnityWebappsUnityWebappsTabClass))

typedef struct _BamfUnityWebappsTab BamfUnityWebappsTab;
typedef struct _BamfUnityWebappsTabClass BamfUnityWebappsTabClass;
typedef struct _BamfUnityWebappsTabPrivate BamfUnityWebappsTabPrivate;

struct _BamfUnityWebappsTabClass
{
  BamfTabClass parent;
};

struct _BamfUnityWebappsTab
{
  BamfTab parent;

  /* private */
  BamfUnityWebappsTabPrivate *priv;
};

GType       bamf_unity_webapps_tab_get_type    (void) G_GNUC_CONST;

BamfUnityWebappsTab *bamf_unity_webapps_tab_new (UnityWebappsContext *context, gint interest_id);

gint bamf_unity_webapps_tab_get_interest_id (BamfUnityWebappsTab *tab);
BamfLegacyWindow* bamf_unity_webapps_tab_get_legacy_window_for(BamfUnityWebappsTab *tab);


#endif
