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

char *
bamf_xutils_get_window_hint (Window xid, const char *atom_name)
{
  Display *XDisplay;
  Atom atom;
  char *hint = NULL;
  Atom type;
  gint format;
  gulong numItems;
  gulong bytesAfter;
  unsigned char *buffer;
  gboolean close_display = TRUE;

  g_return_val_if_fail ((xid != 0), NULL);
  g_return_val_if_fail (atom_name, NULL);

  XDisplay = XOpenDisplay (NULL);
  if (!XDisplay)
  {
    XDisplay = gdk_x11_get_default_xdisplay ();
    if (!XDisplay)
    {
      g_warning ("%s: Unable to get a valid XDisplay", G_STRFUNC);
      return hint;
    }
    
    close_display = FALSE;
  }

  atom = XInternAtom (XDisplay, atom_name, FALSE);

  int result = XGetWindowProperty (XDisplay,
                                   xid,
                                   atom,
                                   0,
                                   G_MAXINT,
                                   FALSE,
                                   XA_STRING,
                                   &type,
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
bamf_xutils_set_window_hint (Window xid, const char *atom_name, const char *data)
{
  Display *XDisplay;
  gboolean close_display = TRUE;

  g_return_if_fail (xid != 0);
  g_return_if_fail (atom_name);
  g_return_if_fail (data);
  
  XDisplay = XOpenDisplay (NULL);
  if (!XDisplay)
  {
    XDisplay = gdk_x11_get_default_xdisplay ();
    if (!XDisplay)
    {
      g_warning ("%s: Unable to get a valid XDisplay", G_STRFUNC);
      return;
    }
    close_display = FALSE;
  }

  XChangeProperty (XDisplay,
                   xid,
                   XInternAtom (XDisplay,
                   atom_name,
                   FALSE),
                   XA_STRING,
                   8,
                   PropModeReplace,
                   (unsigned char *) data,
                   strlen (data));

  if (close_display)
    XCloseDisplay (XDisplay);
}
