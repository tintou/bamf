/*
 * Copyright (C) 2010-2011 Canonical Ltd
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 3 as
 * published by the Free Software Foundation.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANAPPLICATIONILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 *
 * Authored by: Jason Smith <jason.smith@canonical.com>
 *              Marco Trevisan (Treviño) <3v1n0@ubuntu.com>
 *
 */

#ifndef __BAMF_UNITY_WEBAPPS_APPLICATION_H__
#define __BAMF_UNITY_WEBAPPS_APPLICATION_H__

#include <unity-webapps-context.h>

#include "bamf-application.h"


#define BAMF_TYPE_UNITY_WEBAPPS_APPLICATION                     (bamf_unity_webapps_application_get_type ())
#define BAMF_UNITY_WEBAPPS_APPLICATION(obj)                     (G_TYPE_CHECK_INSTANCE_CAST ((obj), BAMF_TYPE_UNITY_WEBAPPS_APPLICATION, BamfUnityWebappsApplication))
#define BAMF_IS_UNITY_WEBAPPS_APPLICATION(obj)          (G_TYPE_CHECK_INSTANCE_TYPE ((obj), BAMF_TYPE_UNITY_WEBAPPS_APPLICATION))
#define BAMF_UNITY_WEBAPPS_APPLICATION_CLASS(klass)             (G_TYPE_CHECK_CLASS_CAST ((klass), BAMF_TYPE_UNITY_WEBAPPS_APPLICATION, BamfUnityWebappsApplicationClass))
#define BAMF_IS_UNITY_WEBAPPS_APPLICATION_CLASS(klass)  (G_TYPE_CHECK_CLASS_TYPE ((klass), BAMF_TYPE_UNITY_WEBAPPS_APPLICATION))
#define BAMF_UNITY_WEBAPPS_APPLICATION_GET_CLASS(obj)           (G_TYPE_INSTANCE_GET_CLASS ((obj), BAMF_TYPE_UNITY_WEBAPPS_APPLICATION, BamfUnityWebappsUnityWebappsApplicationClass))

typedef struct _BamfUnityWebappsApplication BamfUnityWebappsApplication;
typedef struct _BamfUnityWebappsApplicationClass BamfUnityWebappsApplicationClass;
typedef struct _BamfUnityWebappsApplicationPrivate BamfUnityWebappsApplicationPrivate;

struct _BamfUnityWebappsApplicationClass
{
  BamfApplicationClass parent;
};

struct _BamfUnityWebappsApplication
{
  BamfApplication parent;

  /* private */
  BamfUnityWebappsApplicationPrivate *priv;
};

GType       bamf_unity_webapps_application_get_type    (void) G_GNUC_CONST;

BamfApplication *bamf_unity_webapps_application_new (UnityWebappsContext *context);
UnityWebappsContext *bamf_unity_webapps_application_get_context (BamfUnityWebappsApplication *application);

void
bamf_unity_webapps_application_add_existing_interests (BamfUnityWebappsApplication *self);



#endif
