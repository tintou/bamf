/* -*- Mode: C; indent-tabs-mode: t; c-basic-offset: 4; tab-width: 4 -*- */
/*
 * bamf-unity-webapps-observer.h
 * Copyright (C) Canonical LTD 2011
 *
 * Author: Robert Carr <racarr@canonical.com>
 *
 * This program is free software: you can redistribute it and/or modify it
 * under the terms of the GNU Lesser General Public License as published
 * by the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * unity-webapps is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
 * See the GNU Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.";
 */


#ifndef __BAMF_UNITY_WEBAPPS_OBSERVER_H
#define __BAMF_UNITY_WEBAPPS_OBSERVER_H


#define BAMF_TYPE_UNITY_WEBAPPS_OBSERVER              (bamf_unity_webapps_observer_get_type())
#define BAMF_UNITY_WEBAPPS_OBSERVER(obj)              (G_TYPE_CHECK_INSTANCE_CAST((obj), BAMF_TYPE_UNITY_WEBAPPS_OBSERVER, BamfUnityWebappsObserver))
#define BAMF_UNITY_WEBAPPS_OBSERVER_CLASS(klass)      (G_TYPE_CHECK_CLASS_CAST((klass), BAMF_TYPE_UNITY_WEBAPPS_OBSERVER, BamfUnityWebappsObserverClass))
#define BAMF_IS_UNITY_WEBAPPS_OBSERVER(obj)           (G_TYPE_CHECK_INSTANCE_TYPE((obj), BAMF_TYPE_UNITY_WEBAPPS_OBSERVER))
#define BAMF_IS_UNITY_WEBAPPS_OBSERVER_CLASS(klass)   (G_TYPE_CHECK_CLASS_TYPE ((klass), BAMF_TYPE_UNITY_WEBAPPS_OBSERVER))
#define BAMF_UNITY_WEBAPPS_OBSERVER_GET_CLASS(obj)    (G_TYPE_INSTANCE_GET_CLASS((obj), BAMF_TYPE_UNITY_WEBAPPS_OBSERVER, BamfUnityWebappsObserverClass))

typedef struct _BamfUnityWebappsObserverPrivate BamfUnityWebappsObserverPrivate;
typedef struct _BamfUnityWebappsObserverClass BamfUnityWebappsObserverClass;
typedef struct _BamfUnityWebappsObserver BamfUnityWebappsObserver;


struct _BamfUnityWebappsObserver {
        GObject object;

        BamfUnityWebappsObserverPrivate *priv;
};


struct _BamfUnityWebappsObserverClass {
        GObjectClass parent_class;
};

GType bamf_unity_webapps_observer_get_type (void) G_GNUC_CONST;

BamfUnityWebappsObserver *bamf_unity_webapps_observer_new ();

#endif
