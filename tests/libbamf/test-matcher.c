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

#include <glib.h>
#include <stdlib.h>
#include "libbamf.h"

static void
test_allocation (void)
{
  BamfMatcher *matcher;

  matcher = bamf_matcher_get_default ();
  g_assert (BAMF_IS_MATCHER (matcher));

  g_object_unref (matcher);
}

static void
test_singleton (void)
{
  BamfMatcher *matcher;

  matcher = bamf_matcher_get_default ();
  g_assert (BAMF_IS_MATCHER (matcher));
  g_assert (matcher == bamf_matcher_get_default ());

  g_object_unref (matcher);
}

void
test_matcher_create_suite (void)
{
#define DOMAIN "/Matcher"

  g_test_add_func (DOMAIN"/Allocation", test_allocation);
  g_test_add_func (DOMAIN"/Singleton", test_singleton);
}