#define WNCK_I_KNOW_THIS_IS_UNSTABLE

#include <gtk/gtk.h>
#include <gdk/gdk.h>
#include <dbus/dbus.h>
#include <dbus/dbus-glib.h>
#include <dbus/dbus-glib-lowlevel.h>
#include <libwnck/libwnck.h>

#include "libwncksync.h"

DBusGConnection *connection;
GError *error;
DBusGProxy *proxy;

void wncksync_init ()
{
	connection = dbus_g_bus_get (DBUS_BUS_SESSION, &error);
	
	if (connection == NULL)
	{
		g_printerr ("Failed to open bus: %s\n", error->message);
		g_error_free (error);	
	}
	
	proxy = dbus_g_proxy_new_for_name (connection, 
										"org.wncksync.WnckSync", 
										"/org/wncksync/WnckSync/Control", 
										"org.wncksync.WnckSync.Control");
										
	if (!proxy)
	{
		g_printerr ("Failed to get name owner.\n");	
	}
	
}

void wncksync_end ()
{
	//fixme
}

GArray * wncksync_windows_for_desktop_file (gchar *desktop_file)
{
	GArray *arr;
	error = NULL;
	if (!dbus_g_proxy_call (proxy, "XidsForDesktopFile", &error, G_TYPE_STRING, &desktop_file, G_TYPE_INVALID, DBUS_TYPE_G_UINT64_ARRAY, &arr, G_TYPE_INVALID))
	{
		g_printerr ("Failed to fetch desktop file: %s\n", error->message);
		g_error_free (error);
		//Error Handling	
	}
	
	GArray *windows = g_array_new (FALSE, TRUE, sizeof (WnckWindow));
	
	int i;
	for (i = 0; i < arr->len; i++)
	{
		gulong xid = arr->data [i];
		WnckWindow *window = wnck_window_get (xid);
		g_array_append_val (windows, window);
	}
	
	return windows;
}

gchar * wncksync_desktop_item_for_window_xid (gulong xid)
{
	gchar *desktop_file;
	error = NULL;
	if (!dbus_g_proxy_call (proxy, "DesktopFileForXid", &error, G_TYPE_UINT64, xid, G_TYPE_INVALID, G_TYPE_STRING, &desktop_file, G_TYPE_INVALID))
	{
		g_printerr ("Failed to fetch xid array");
		g_error_free (error);	
	}
	
	return desktop_file;
}
