//  
//  Copyright (C) 2009 Canonical Ltd.
// 
//  This program is free software: you can redistribute it and/or modify
//  it under the terms of the GNU General Public License as published by
//  the Free Software Foundation, either version 3 of the License, or
//  (at your option) any later version.
// 
//  This program is distributed in the hope that it will be useful,
//  but WITHOUT ANY WARRANTY; without even the implied warranty of
//  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//  GNU General Public License for more details.
// 
//  You should have received a copy of the GNU General Public License
//  along with this program.  If not, see <http://www.gnu.org/licenses/>.
// 

#include "config.h"
#include "bamf-control.h"
#include "bamf-matcher.h"

#include "main.h"

int
main (int argc, char **argv)
{
  BamfControl *control;
  BamfMatcher *matcher;
  
  gtk_init (&argc, &argv);
  glibtop_init ();

  dbus_g_object_type_install_info (BAMF_TYPE_CONTROL,
				   &dbus_glib_bamf_object_info);

  control = bamf_control_get_default ();
  matcher = bamf_matcher_get_default ();
  
  gtk_main ();

  return 0;
}
