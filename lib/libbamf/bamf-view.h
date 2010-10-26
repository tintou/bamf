/*
 * Copyright 2010 Canonical Ltd.
 *
 * This program is free software: you can redistribute it and/or modify it
 * under the terms of either or both of the following licenses:
 *
 * 1) the GNU Lesser General Public License version 3, as published by the
 * Free Software Foundation; and/or
 * 2) the GNU Lesser General Public License version 2.1, as published by
 * the Free Software Foundation.
 *
 * This program is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranties of
 * MERCHANTABILITY, SATISFACTORY QUALITY or FITNESS FOR A PARTICULAR
 * PURPOSE.  See the applicable version of the GNU Lesser General Public
 * License for more details.
 *
 * You should have received a copy of both the GNU Lesser General Public
 * License version 3 and version 2.1 along with this program.  If not, see
 * <http://www.gnu.org/licenses/>
 *
 * Authored by: Jason Smith <jason.smith@canonical.com>
 *              Neil Jagdish Patel <neil.patel@canonical.com>
 *
 */

#ifndef _BAMF_VIEW_H_
#define _BAMF_VIEW_H_

#include <glib-object.h>

G_BEGIN_DECLS

#define BAMF_TYPE_VIEW (bamf_view_get_type ())

#define BAMF_VIEW(obj) (G_TYPE_CHECK_INSTANCE_CAST ((obj),\
        BAMF_TYPE_VIEW, BamfView))

#define BAMF_VIEW_CLASS(klass) (G_TYPE_CHECK_CLASS_CAST ((klass),\
        BAMF_TYPE_VIEW, BamfViewClass))

#define BAMF_IS_VIEW(obj) (G_TYPE_CHECK_INSTANCE_TYPE ((obj),\
        BAMF_TYPE_VIEW))

#define BAMF_IS_VIEW_CLASS(klass) (G_TYPE_CHECK_CLASS_TYPE ((klass),\
        BAMF_TYPE_VIEW))

#define BAMF_VIEW_GET_CLASS(obj) (G_TYPE_INSTANCE_GET_CLASS ((obj),\
        BAMF_TYPE_VIEW, BamfViewClass))

typedef enum
{
  BAMF_CLICK_BEHAVIOR_OPEN,
  BAMF_CLICK_BEHAVIOR_FOCUS,
  BAMF_CLICK_BEHAVIOR_FOCUS_ALL,
  BAMF_CLICK_BEHAVIOR_MINIMIZE,
  BAMF_CLICK_BEHAVIOR_RESTORE,
  BAMF_CLICK_BEHAVIOR_RESTORE_ALL,
  BAMF_CLICK_BEHAVIOR_PICKER,
} BamfClickBehavior;

typedef struct _BamfView        BamfView;
typedef struct _BamfViewClass   BamfViewClass;
typedef struct _BamfViewPrivate BamfViewPrivate;

struct _BamfView
{
  GObject parent;

  BamfViewPrivate *priv;
};

struct _BamfViewClass
{
  GObjectClass parent_class;
  
  GList            * (*get_children)        (BamfView *view);
  gboolean           (*is_active)           (BamfView *view);
  gboolean           (*is_running)          (BamfView *view);
  gboolean           (*is_urgent)           (BamfView *view);
  gchar            * (*get_name)            (BamfView *view);
  gchar            * (*get_icon)            (BamfView *view);
  const gchar      * (*view_type)           (BamfView *view);
  void               (*set_path)            (BamfView *view, const gchar *path);
  
  /*< signals >*/
  void (*active_changed)              (BamfView *view, gboolean active);
  void (*closed)                      (BamfView *view);
  void (*child_added)                 (BamfView *view, BamfView *child);
  void (*child_removed)               (BamfView *view, BamfView *child);
  void (*running_changed)             (BamfView *view, gboolean running);
  void (*urgent_changed)              (BamfView *view, gboolean urgent);
  void (*user_visible_changed)        (BamfView *view, gboolean user_visible);

  /*< private >*/
  void (*_view_padding1) (void);
  void (*_view_padding2) (void);
  void (*_view_padding3) (void);
  void (*_view_padding4) (void);
  void (*_view_padding5) (void);
  void (*_view_padding6) (void);
};

GType      bamf_view_get_type             (void) G_GNUC_CONST;

/**
 * bamf_view_get_children:
 * @view: a #BamfView
 *
 * Note: Makes sever dbus calls the first time this is called on a view. Dbus messaging is reduced afterwards.
 *
 * Returns: (element-type Bamf.View) (transfer container): Returns a list of #BamfView which must be
 *           freed after usage. Elements of the list are owned by bamf and should not be unreffed.
 */
GList    * bamf_view_get_children  (BamfView *view);

/**
 * bamf_view_is_active:
 * @view: a #BamfView
 *
 * Determines if the view is closed or not.
 */
gboolean   bamf_view_is_closed     (BamfView *view);

/**
 * bamf_view_is_active:
 * @view: a #BamfView
 *
 * Determines if the view is currently active and focused by the user. Useful for an active window indicator. 
 */
gboolean   bamf_view_is_active     (BamfView *view);

/**
 * bamf_view_is_running:
 * @view: a #BamfView
 *
 * Determines if the view is currently running. Useful for a running window indicator. 
 */
gboolean   bamf_view_is_running    (BamfView *view);

/**
 * bamf_view_is_running:
 * @view: a #BamfView
 *
 * Determines if the view is currently requiring attention. Useful for a running window indicator. 
 */
gboolean   bamf_view_is_urgent     (BamfView *view);

/**
 * bamf_view_get_name:
 * @view: a #BamfView
 *
 * Gets the name of a view. This name is a short name best used to represent the view with text. 
 */
gchar    * bamf_view_get_name      (BamfView *view);

/**
 * bamf_view_get_icon:
 * @view: a #BamfView
 *
 * Gets the icon of a view. This icon is used to visually represent the view. 
 */
gchar    * bamf_view_get_icon      (BamfView *view);

/**
 * bamf_view_user_visible:
 * @view: a #BamfView
 *
 * Returns a boolean useful for determining if a particular view is "user visible". User visible
 * is a concept relating to whether or not a window should be shown in a launcher tasklist.
 */
gboolean   bamf_view_user_visible  (BamfView *view);

/**
 * bamf_view_get_view_type:
 * @view: a #BamfView
 *
 * The view type of a window is a short string used to represent all views of the same class. These
 * descriptions should not be used to do casting as they are not considered stable.
 *
 * Returns: (transfer full): A gchar*
 */
const gchar    * bamf_view_get_view_type (BamfView *view);

void bamf_view_set_sticky (BamfView *view, gboolean value);

gboolean bamf_view_is_sticky (BamfView *view);

G_END_DECLS

#endif
