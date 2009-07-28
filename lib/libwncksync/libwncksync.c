#define WNCK_I_KNOW_THIS_IS_UNSTABLE

#include "config.h"

#include <gtk/gtk.h>
#include <gdk/gdk.h>
#include <dbus/dbus.h>
#include <dbus/dbus-glib.h>
#include <dbus/dbus-glib-lowlevel.h>

#include "libwncksync.h"

DBusGConnection *connection;
GError *wncksync_error;
DBusGProxy *wncksync_proxy;
gboolean wncksync_initialized = FALSE;

void wncksync_init ()
{
	if (wncksync_initialized)
		return;

	wncksync_initialized = TRUE;
	
	connection = dbus_g_bus_get (DBUS_BUS_SESSION, &wncksync_error);
	
	if (connection == NULL) {
		g_printerr ("Failed to open bus: %s\n", wncksync_error->message);
		g_error_free (wncksync_error);	
	}
	
	wncksync_proxy = dbus_g_proxy_new_for_name (connection, 
					   "org.wncksync.Matcher", 
					   "/org/wncksync/Matcher", 
					   "org.wncksync.Matcher");
										
	if (!wncksync_proxy) {
		g_printerr ("Failed to get name owner.\n");	
	}
}

void wncksync_quit ()
{
	//fixme
}

void wncksync_register_desktop_file_for_pid (gchar *desktop_file, gint pid)
{
	if (!wncksync_initialized) wncksync_init ();

	g_return_if_fail (desktop_file);
	dbus_g_proxy_call_no_reply (wncksync_proxy, 
	                        "RegisterDesktopFileForPid", 
	                         G_TYPE_STRING, desktop_file, 
	                         G_TYPE_INT, pid, 
	                         G_TYPE_INVALID);
}

GArray * wncksync_xids_for_desktop_file (gchar *desktop_file)
{
	if (!wncksync_initialized) wncksync_init ();

	GArray *arr;
	wncksync_error = NULL;
	if (!dbus_g_proxy_call (wncksync_proxy, 
	                        "XidsForDesktopFile", 
	                        &wncksync_error, 
	                        G_TYPE_STRING, desktop_file, 
	                        G_TYPE_INVALID, 
	                        DBUS_TYPE_G_UINT64_ARRAY, &arr, 
	                        G_TYPE_INVALID)) {
		g_printerr ("Failed to fetch xid array: %s\n", wncksync_error->message);
		g_error_free (wncksync_error);
		//Error Handling
		arr = g_array_new (FALSE, TRUE, sizeof (gulong));
	}
	
	return arr;
}

gchar * wncksync_desktop_item_for_window (WnckWindow *window)
{
	if (!wncksync_initialized) wncksync_init ();

	if (window == NULL)
		return "";
		
	gulong xid = wnck_window_get_xid (window);

	gchar *desktop_file;
	wncksync_error = NULL;
	if (!dbus_g_proxy_call (wncksync_proxy, 
	                        "DesktopFileForXid", 
	                        &wncksync_error, 
	                        G_TYPE_UINT64, xid, 
	                        G_TYPE_INVALID, 
	                        G_TYPE_STRING, &desktop_file, 
	                        G_TYPE_INVALID)) {
		g_printerr ("Failed to fetch desktop file: %s\n", wncksync_error->message);
		g_error_free (wncksync_error);
		
		return "";
	}
	
	gchar *result = g_strdup (desktop_file);
	g_free (desktop_file);
	
	return result;
}
