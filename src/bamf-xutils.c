/*
 * Copyright (C) 2012 Canonical Ltd
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 3 as
 * published by the Free Software Foundation.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 *
 * Authored by: Jason Smith <jason.smith@canonical.com>
 *              Marco Trevisan (Treviño) <3v1n0@ubuntu.com>
 *
 */

#include "bamf-xutils.h"
#include <string.h>

static Display *
get_xdisplay(gboolean *opened)
{
  Display *xdisplay;
  xdisplay = gdk_x11_get_default_xdisplay ();

  if (opened)
    *opened = FALSE;

  if (!xdisplay)
    {
      xdisplay = XOpenDisplay (NULL);

      if (xdisplay)
        {
          if (opened)
            *opened = TRUE;
        }
    }

  return xdisplay;
}

char *
bamf_xutils_get_window_hint (Window xid, const char *atom_name, Atom type)
{
  Display *XDisplay;
  Atom atom;
  char *hint = NULL;
  Atom result_type;
  gint format;
  gulong numItems;
  gulong bytesAfter;
  unsigned char *buffer;
  gboolean close_display = FALSE;

  g_return_val_if_fail ((xid != 0), NULL);
  g_return_val_if_fail (atom_name, NULL);

  XDisplay = get_xdisplay (&close_display);

  if (!XDisplay)
  {
    g_warning ("%s: Unable to get a valid XDisplay", G_STRFUNC);
    return NULL;
  }

  atom = XInternAtom (XDisplay, atom_name, FALSE);

  int result = XGetWindowProperty (XDisplay,
                                   xid,
                                   atom,
                                   0,
                                   G_MAXINT,
                                   FALSE,
                                   type,
                                   &result_type,
                                   &format,
                                   &numItems,
                                   &bytesAfter,
                                   &buffer);

  if (close_display)
    XCloseDisplay (XDisplay);

  if (result == Success && numItems > 0)
    {
      hint = g_strdup ((char*) buffer);
      XFree (buffer);
    }

  return hint;
}

void
bamf_xutils_set_window_hint (Window xid, const char *atom_name, Atom type, const char *data)
{
  Display *XDisplay;
  gboolean close_display = FALSE;

  g_return_if_fail (xid != 0);
  g_return_if_fail (atom_name);
  g_return_if_fail (data);
  
  XDisplay = get_xdisplay (&close_display);

  if (!XDisplay)
  {
    g_warning ("%s: Unable to get a valid XDisplay", G_STRFUNC);
    return;
  }

  XChangeProperty (XDisplay,
                   xid,
                   XInternAtom (XDisplay,
                   atom_name,
                   FALSE),
                   type,
                   8,
                   PropModeReplace,
                   (unsigned char *) data,
                   strlen (data));

  if (close_display)
    XCloseDisplay (XDisplay);
}

void
bamf_xutils_get_window_class_hints (Window xid, char **class_instance_name, char **class_name)
{
  Display *xdisplay;
  gboolean close_display = FALSE;

  xdisplay = get_xdisplay (&close_display);

  if (!xdisplay)
  {
    g_warning ("%s: Unable to get a valid XDisplay", G_STRFUNC);
    return;
  }

  XClassHint class_hint;
  class_hint.res_name = NULL;
  class_hint.res_class = NULL;

  XGetClassHint(xdisplay, xid, &class_hint);

  if (class_name && class_hint.res_class)
    *class_name = g_convert (class_hint.res_class, -1, "utf-8", "iso-8859-1",
                             NULL, NULL, NULL);

  if (class_instance_name && class_hint.res_name)
    *class_instance_name = g_convert (class_hint.res_name, -1, "utf-8", "iso-8859-1",
                                      NULL, NULL, NULL);

  XFree (class_hint.res_class);
  XFree (class_hint.res_name);
}
