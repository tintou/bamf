#ifndef __BAMFDBUS_H__
#define __BAMFDBUS_H__

#include <stdio.h>
#include <stdlib.h>
#include <glib.h>
#include <glib-object.h>
#include <dbus/dbus.h>
#include <dbus/dbus-glib.h>
#include <dbus/dbus-glib-lowlevel.h>


#define BAMF_DBUS_PATH "/org/bamf/Matcher"
#define BAMF_DBUS_SERVICE "org.bamf.Matcher"

#define BAMF_TYPE_DBUS			(bamf_dbus_get_type ())
#define BAMF_DBUS(obj)			(G_TYPE_CHECK_INSTANCE_CAST ((obj), BAMF_TYPE_DBUS, BamfDBus))
#define BAMF_IS_DBUS			(G_TYPE_CHECK_INSTANCE_TYPE ((obj), BAMF_TYPE_DBUS))
#define BAMF_DBUS_CLASS(klass)		(G_TYPE_CHECK_CLASS_CAST ((klass), BAMF_TYPE_DBUS, BamfDBusClass))
#define BAMF_IS_DBUS_CLASS(klass)		(G_TYPE_CHECK_CLASA_TYPE ((klass), BAMF_TYPE_DBUS))
#define BAMF_DBUS_GET_CLASS(obj)		(G_TYPE_INSTANCE_GET_CLASS ((obj), BAMF_TYPE_DbuS, BamfDBusClass))

typedef struct _BamfDBus BamfDBus;
typedef struct _BamfDBusClass BamfDBusClass;

struct _BamfDBus
{
  GObject parent;
};

struct _BamfDBusClass
{
  GObjectClass parent;
};

GType bamf_dbus_get_type (void);

BamfDBus *bamf_dbus_new (void);

gboolean bamf_dbus_window_match_is_ready (BamfDBus * dbus,
					      guint32 xid);

gboolean bamf_dbus_desktop_file_for_xid (BamfDBus * dbus, guint32 xid,
					     gchar ** filename,
					     GError ** error);

gboolean bamf_dbus_xids_for_desktop_file (BamfDBus * dbus,
					      gchar * filename,
					      GArray ** xids,
					      GError ** error);

gboolean bamf_dbus_register_desktop_file_for_pid (BamfDBus * dbus,
						      gchar * filename,
						      gint pid,
						      GError ** error);

#endif
