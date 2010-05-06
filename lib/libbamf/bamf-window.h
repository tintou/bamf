/*
 * Copyright 2009 Canonical Ltd.
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

#ifndef _BAMF_WINDOW_H_
#define _BAMF_WINDOW_H_

#include <glib-object.h>
#include <libbamf/bamf-view.h>

G_BEGIN_DECLS

#define BAMF_TYPE_WINDOW (bamf_window_get_type ())

#define BAMF_WINDOW(obj) (G_TYPE_CHECK_INSTANCE_CAST ((obj),\
        BAMF_TYPE_WINDOW, BamfWindow))

#define BAMF_WINDOW_CLASS(klass) (G_TYPE_CHECK_CLASS_CAST ((klass),\
        BAMF_TYPE_WINDOW, BamfWindowClass))

#define BAMF_IS_WINDOW(obj) (G_TYPE_CHECK_INSTANCE_TYPE ((obj),\
        BAMF_TYPE_WINDOW))

#define BAMF_IS_WINDOW_CLASS(klass) (G_TYPE_CHECK_CLASS_TYPE ((klass),\
        BAMF_TYPE_WINDOW))

#define BAMF_WINDOW_GET_CLASS(obj) (G_TYPE_INSTANCE_GET_CLASS ((obj),\
        BAMF_TYPE_WINDOW, BamfWindowClass))

typedef struct _BamfWindow        BamfWindow;
typedef struct _BamfWindowClass   BamfWindowClass;
typedef struct _BamfWindowPrivate BamfWindowPrivate;

struct _BamfWindow
{
  BamfView parent;

  BamfWindowPrivate *priv;
};

struct _BamfWindowClass
{
  BamfViewClass parent_class;

  /*< private >*/
  void (*_window_padding1) (void);
  void (*_window_padding2) (void);
  void (*_window_padding3) (void);
  void (*_window_padding4) (void);
  void (*_window_padding5) (void);
  void (*_window_padding6) (void);
};

GType             bamf_window_get_type             (void) G_GNUC_CONST;

/**
 * bamf_window_is_urgent:
 * @window: a #BamfWindow
 *
 * Determines if the view is currently urgent and requires attention from the user. Useful for an 
 * urgent window indicator. 
 */
gboolean          bamf_window_is_urgent            (BamfWindow *self);

gboolean          bamf_window_user_visible         (BamfWindow *self);

G_END_DECLS

#endif
