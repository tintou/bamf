/*
 * Generated by gdbus-codegen 2.32.1. DO NOT EDIT.
 *
 * The license of this code is the same as for the source it was derived from.
 */

#ifndef __BAMF_GDBUS_BROWSER_GENERATED_H__
#define __BAMF_GDBUS_BROWSER_GENERATED_H__

#include <gio/gio.h>

G_BEGIN_DECLS


/* ------------------------------------------------------------------------ */
/* Declarations for org.ayatana.bamf.browser */

#define BAMF_DBUS_BROWSER_TYPE_ (bamf_dbus_browser__get_type ())
#define BAMF_DBUS_BROWSER_(o) (G_TYPE_CHECK_INSTANCE_CAST ((o), BAMF_DBUS_BROWSER_TYPE_, BamfDBusBrowser))
#define BAMF_DBUS_BROWSER_IS_(o) (G_TYPE_CHECK_INSTANCE_TYPE ((o), BAMF_DBUS_BROWSER_TYPE_))
#define BAMF_DBUS_BROWSER__GET_IFACE(o) (G_TYPE_INSTANCE_GET_INTERFACE ((o), BAMF_DBUS_BROWSER_TYPE_, BamfDBusBrowserIface))

struct _BamfDBusBrowser;
typedef struct _BamfDBusBrowser BamfDBusBrowser;
typedef struct _BamfDBusBrowserIface BamfDBusBrowserIface;

struct _BamfDBusBrowserIface
{
  GTypeInterface parent_iface;


  gboolean (*handle_show_tab) (
    BamfDBusBrowser *object,
    GDBusMethodInvocation *invocation,
    const gchar *arg_id);

  gboolean (*handle_tab_ids) (
    BamfDBusBrowser *object,
    GDBusMethodInvocation *invocation);

  gboolean (*handle_tab_preview) (
    BamfDBusBrowser *object,
    GDBusMethodInvocation *invocation,
    const gchar *arg_id);

  gboolean (*handle_tab_uri) (
    BamfDBusBrowser *object,
    GDBusMethodInvocation *invocation,
    const gchar *arg_id);

  gboolean (*handle_tab_xid) (
    BamfDBusBrowser *object,
    GDBusMethodInvocation *invocation,
    const gchar *arg_id);

  void (*tab_closed) (
    BamfDBusBrowser *object,
    const gchar *arg_id);

  void (*tab_opened) (
    BamfDBusBrowser *object,
    const gchar *arg_id);

  void (*tab_uri_changed) (
    BamfDBusBrowser *object,
    const gchar *arg_id,
    const gchar *arg_old_uri,
    const gchar *arg_new_uri);

};

GType bamf_dbus_browser__get_type (void) G_GNUC_CONST;

GDBusInterfaceInfo *bamf_dbus_browser__interface_info (void);
guint bamf_dbus_browser__override_properties (GObjectClass *klass, guint property_id_begin);


/* D-Bus method call completion functions: */
void bamf_dbus_browser__complete_tab_ids (
    BamfDBusBrowser *object,
    GDBusMethodInvocation *invocation,
    const gchar *const *ids);

void bamf_dbus_browser__complete_show_tab (
    BamfDBusBrowser *object,
    GDBusMethodInvocation *invocation);

void bamf_dbus_browser__complete_tab_uri (
    BamfDBusBrowser *object,
    GDBusMethodInvocation *invocation,
    const gchar *uri);

void bamf_dbus_browser__complete_tab_preview (
    BamfDBusBrowser *object,
    GDBusMethodInvocation *invocation,
    const gchar *preview);

void bamf_dbus_browser__complete_tab_xid (
    BamfDBusBrowser *object,
    GDBusMethodInvocation *invocation,
    guint xid);



/* D-Bus signal emissions functions: */
void bamf_dbus_browser__emit_tab_opened (
    BamfDBusBrowser *object,
    const gchar *arg_id);

void bamf_dbus_browser__emit_tab_closed (
    BamfDBusBrowser *object,
    const gchar *arg_id);

void bamf_dbus_browser__emit_tab_uri_changed (
    BamfDBusBrowser *object,
    const gchar *arg_id,
    const gchar *arg_old_uri,
    const gchar *arg_new_uri);



/* D-Bus method calls: */
void bamf_dbus_browser__call_tab_ids (
    BamfDBusBrowser *proxy,
    GCancellable *cancellable,
    GAsyncReadyCallback callback,
    gpointer user_data);

gboolean bamf_dbus_browser__call_tab_ids_finish (
    BamfDBusBrowser *proxy,
    gchar ***out_ids,
    GAsyncResult *res,
    GError **error);

gboolean bamf_dbus_browser__call_tab_ids_sync (
    BamfDBusBrowser *proxy,
    gchar ***out_ids,
    GCancellable *cancellable,
    GError **error);

void bamf_dbus_browser__call_show_tab (
    BamfDBusBrowser *proxy,
    const gchar *arg_id,
    GCancellable *cancellable,
    GAsyncReadyCallback callback,
    gpointer user_data);

gboolean bamf_dbus_browser__call_show_tab_finish (
    BamfDBusBrowser *proxy,
    GAsyncResult *res,
    GError **error);

gboolean bamf_dbus_browser__call_show_tab_sync (
    BamfDBusBrowser *proxy,
    const gchar *arg_id,
    GCancellable *cancellable,
    GError **error);

void bamf_dbus_browser__call_tab_uri (
    BamfDBusBrowser *proxy,
    const gchar *arg_id,
    GCancellable *cancellable,
    GAsyncReadyCallback callback,
    gpointer user_data);

gboolean bamf_dbus_browser__call_tab_uri_finish (
    BamfDBusBrowser *proxy,
    gchar **out_uri,
    GAsyncResult *res,
    GError **error);

gboolean bamf_dbus_browser__call_tab_uri_sync (
    BamfDBusBrowser *proxy,
    const gchar *arg_id,
    gchar **out_uri,
    GCancellable *cancellable,
    GError **error);

void bamf_dbus_browser__call_tab_preview (
    BamfDBusBrowser *proxy,
    const gchar *arg_id,
    GCancellable *cancellable,
    GAsyncReadyCallback callback,
    gpointer user_data);

gboolean bamf_dbus_browser__call_tab_preview_finish (
    BamfDBusBrowser *proxy,
    gchar **out_preview,
    GAsyncResult *res,
    GError **error);

gboolean bamf_dbus_browser__call_tab_preview_sync (
    BamfDBusBrowser *proxy,
    const gchar *arg_id,
    gchar **out_preview,
    GCancellable *cancellable,
    GError **error);

void bamf_dbus_browser__call_tab_xid (
    BamfDBusBrowser *proxy,
    const gchar *arg_id,
    GCancellable *cancellable,
    GAsyncReadyCallback callback,
    gpointer user_data);

gboolean bamf_dbus_browser__call_tab_xid_finish (
    BamfDBusBrowser *proxy,
    guint *out_xid,
    GAsyncResult *res,
    GError **error);

gboolean bamf_dbus_browser__call_tab_xid_sync (
    BamfDBusBrowser *proxy,
    const gchar *arg_id,
    guint *out_xid,
    GCancellable *cancellable,
    GError **error);



/* ---- */

#define BAMF_DBUS_BROWSER_TYPE__PROXY (bamf_dbus_browser__proxy_get_type ())
#define BAMF_DBUS_BROWSER__PROXY(o) (G_TYPE_CHECK_INSTANCE_CAST ((o), BAMF_DBUS_BROWSER_TYPE__PROXY, BamfDBusBrowserProxy))
#define BAMF_DBUS_BROWSER__PROXY_CLASS(k) (G_TYPE_CHECK_CLASS_CAST ((k), BAMF_DBUS_BROWSER_TYPE__PROXY, BamfDBusBrowserProxyClass))
#define BAMF_DBUS_BROWSER__PROXY_GET_CLASS(o) (G_TYPE_INSTANCE_GET_CLASS ((o), BAMF_DBUS_BROWSER_TYPE__PROXY, BamfDBusBrowserProxyClass))
#define BAMF_DBUS_BROWSER_IS__PROXY(o) (G_TYPE_CHECK_INSTANCE_TYPE ((o), BAMF_DBUS_BROWSER_TYPE__PROXY))
#define BAMF_DBUS_BROWSER_IS__PROXY_CLASS(k) (G_TYPE_CHECK_CLASS_TYPE ((k), BAMF_DBUS_BROWSER_TYPE__PROXY))

typedef struct _BamfDBusBrowserProxy BamfDBusBrowserProxy;
typedef struct _BamfDBusBrowserProxyClass BamfDBusBrowserProxyClass;
typedef struct _BamfDBusBrowserProxyPrivate BamfDBusBrowserProxyPrivate;

struct _BamfDBusBrowserProxy
{
  /*< private >*/
  GDBusProxy parent_instance;
  BamfDBusBrowserProxyPrivate *priv;
};

struct _BamfDBusBrowserProxyClass
{
  GDBusProxyClass parent_class;
};

GType bamf_dbus_browser__proxy_get_type (void) G_GNUC_CONST;

void bamf_dbus_browser__proxy_new (
    GDBusConnection     *connection,
    GDBusProxyFlags      flags,
    const gchar         *name,
    const gchar         *object_path,
    GCancellable        *cancellable,
    GAsyncReadyCallback  callback,
    gpointer             user_data);
BamfDBusBrowser *bamf_dbus_browser__proxy_new_finish (
    GAsyncResult        *res,
    GError             **error);
BamfDBusBrowser *bamf_dbus_browser__proxy_new_sync (
    GDBusConnection     *connection,
    GDBusProxyFlags      flags,
    const gchar         *name,
    const gchar         *object_path,
    GCancellable        *cancellable,
    GError             **error);

void bamf_dbus_browser__proxy_new_for_bus (
    GBusType             bus_type,
    GDBusProxyFlags      flags,
    const gchar         *name,
    const gchar         *object_path,
    GCancellable        *cancellable,
    GAsyncReadyCallback  callback,
    gpointer             user_data);
BamfDBusBrowser *bamf_dbus_browser__proxy_new_for_bus_finish (
    GAsyncResult        *res,
    GError             **error);
BamfDBusBrowser *bamf_dbus_browser__proxy_new_for_bus_sync (
    GBusType             bus_type,
    GDBusProxyFlags      flags,
    const gchar         *name,
    const gchar         *object_path,
    GCancellable        *cancellable,
    GError             **error);


/* ---- */

#define BAMF_DBUS_BROWSER_TYPE__SKELETON (bamf_dbus_browser__skeleton_get_type ())
#define BAMF_DBUS_BROWSER__SKELETON(o) (G_TYPE_CHECK_INSTANCE_CAST ((o), BAMF_DBUS_BROWSER_TYPE__SKELETON, BamfDBusBrowserSkeleton))
#define BAMF_DBUS_BROWSER__SKELETON_CLASS(k) (G_TYPE_CHECK_CLASS_CAST ((k), BAMF_DBUS_BROWSER_TYPE__SKELETON, BamfDBusBrowserSkeletonClass))
#define BAMF_DBUS_BROWSER__SKELETON_GET_CLASS(o) (G_TYPE_INSTANCE_GET_CLASS ((o), BAMF_DBUS_BROWSER_TYPE__SKELETON, BamfDBusBrowserSkeletonClass))
#define BAMF_DBUS_BROWSER_IS__SKELETON(o) (G_TYPE_CHECK_INSTANCE_TYPE ((o), BAMF_DBUS_BROWSER_TYPE__SKELETON))
#define BAMF_DBUS_BROWSER_IS__SKELETON_CLASS(k) (G_TYPE_CHECK_CLASS_TYPE ((k), BAMF_DBUS_BROWSER_TYPE__SKELETON))

typedef struct _BamfDBusBrowserSkeleton BamfDBusBrowserSkeleton;
typedef struct _BamfDBusBrowserSkeletonClass BamfDBusBrowserSkeletonClass;
typedef struct _BamfDBusBrowserSkeletonPrivate BamfDBusBrowserSkeletonPrivate;

struct _BamfDBusBrowserSkeleton
{
  /*< private >*/
  GDBusInterfaceSkeleton parent_instance;
  BamfDBusBrowserSkeletonPrivate *priv;
};

struct _BamfDBusBrowserSkeletonClass
{
  GDBusInterfaceSkeletonClass parent_class;
};

GType bamf_dbus_browser__skeleton_get_type (void) G_GNUC_CONST;

BamfDBusBrowser *bamf_dbus_browser__skeleton_new (void);


G_END_DECLS

#endif /* __BAMF_GDBUS_BROWSER_GENERATED_H__ */
