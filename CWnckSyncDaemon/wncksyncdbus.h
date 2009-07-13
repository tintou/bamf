#ifndef __WNCKSYNCDBUS_H__
#define __WNCKSYNCDBUS_H__

#include <stdio.h>
#include <stdlib.h>
#include <glib.h>
#include <dbus/dbus.h>
#include <dbus/dbus-glib.h>
#include <dbus/dbus-glib-lowlevel.h>


#define WNCKSYNC_DBUS_PATH "/org/wncksync/matcher/Controller"
#define WNCKSYNC_DBUS_SERVICE "org.wncksync.matcher.Controller"

#define WNCKSYNC_TYPE_DBUS				(wncksync_dbus_get_type ())
#define WNCKSYNC_DBUS(obj)				(G_TYPE_CHECK_INSTANCE_CAST ((obj), WNCKSYNC_TYPE_DBUS, WnckSyncDBus))
#define WNCKSYNC_IS_DBUS				(G_TYPE_CHECK_INSTANCE_TYPE ((obj), WNCKSYNC_TYPE_DBUS))
#define WNCKSYNC_DBUS_CLASS(klass)		(G_TYPE_CHECK_CLASS_CAST ((klass), WNCKSYNC_TYPE_DBUS, WnckSyncDBusClass))
#define WNCKSYNC_IS_DBUS_CLASS(klass)	(G_TYPE_CHECK_CLASA_TYPE ((klass), WNCKSYNC_TYPE_DBUS))
#define WNCKSYNC_DBUS_GET_CLASS(obj)	(G_TYPE_INSTANCE_GET_CLASS ((obj), WNCKSYNC_TYPE_DbuS, WnckSyncDBusClass))

typedef struct _WnckSyncDBus		WnckSyncDBus;
typedef struct _WnckSyncDBusClass	WnckSyncDBusClass;

typedef struct _WnckSyncDBus {
        GObject parent;
};

typedef struct _WnckSyncDBusClass {
        GObjectClass parent;
};

GType wncksync_dbus_get_type (void);

WnckSyncDBus * wncksync_dbus_new (void);

gboolean wncksync_dbus_desktop_file_for_xid (WnckSyncDBus *dbus, gulong xid, gchar **filename, GError **error);

gboolean wncksync_dbus_xids_for_desktop_file (WnckSyncDBus *dbus, gchar *filename, GArray **xids, GError **error);

#endif