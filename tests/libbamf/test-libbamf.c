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

#include <stdio.h>
#include <glib.h>

#include <sys/types.h>
#include <unistd.h>


void test_matcher_create_suite (void);

gint
main (gint argc, gchar *argv[])
{
#if !GLIB_CHECK_VERSION(2, 35, 0)
  g_type_init ();
#endif
  g_test_init (&argc, &argv, NULL);

  test_matcher_create_suite ();

  return g_test_run ();
}
