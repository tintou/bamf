#ifndef __WINDOWMATCHER_H__
#define __WINDOWMATCHER_H__

#define WNCK_I_KNOW_THIS_IS_UNSTABLE

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <gdk/gdk.h>
#include <glib.h>
#include <glib-object.h>
#include <gio/gio.h>
#include <gio/gdesktopappinfo.h>
#include <libwnck/libwnck.h>
#include <libgtop-2.0/glibtop.h>
#include <glibtop/procargs.h>
#include <glibtop/procuid.h>
#include <X11/Xatom.h>
#include <X11/Xlib.h>

#define WINDOW_MATCHER_TYPE			(window_matcher_get_type ())
#define WINDOW_MATCHER(obj)			(G_TYPE_CHECK_INSTANCE_CAST ((obj), WINDOW_MATCHER_TYPE, WindowMatcher))
#define IS_WINDOW_MATCHER			(G_TYPE_CHECK_INSTANCE_TYPE ((obj), WINDOW_MATCHER_TYPE))
#define WINDOW_MATCHER_CLASS(klass)		(G_TYPE_CHECK_CLASS_CAST ((klass), WINDOW_MATCHER_TYPE, WindowMatcherClass))
#define IS_WINDOW_MATCHER_CLASS(klass)		(G_TYPE_CHECK_CLASA_TYPE ((klass), WINDOW_MATCHER_TYPE))
#define WINDOW_MATCHER_GET_CLASS(obj)		(G_TYPE_INSTANCE_GET_CLASS ((obj), WINDOW_MATCHER_TYPE, WindowMatcherClass))

#define _NET_WM_DESKTOP_FILE			"_NET_WM_DESKTOP_FILE"

typedef struct _WindowMatcher			WindowMatcher;
typedef struct _WindowMatcherClass		WindowMatcherClass;
typedef struct _WindowMatcherPrivate		WindowMatcherPrivate;

struct _WindowMatcher {
	GObject parent;
	
	/*< private >*/
	WindowMatcherPrivate *priv;
};

struct _WindowMatcherClass {
	GObjectClass parent;
};

GType window_matcher_get_type (void);

WindowMatcher * window_matcher_new (void);

gboolean window_matcher_window_is_match_ready (WindowMatcher *self, WnckWindow *window);

void window_matcher_register_desktop_file_for_pid (WindowMatcher *self, GString *desktopFile, gint pid);

GString * window_matcher_desktop_file_for_window (WindowMatcher *self, WnckWindow *window);

GArray * window_matcher_window_list_for_desktop_file (WindowMatcher *self, GString *desktopFile);


#endif
