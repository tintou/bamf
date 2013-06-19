/*
 * Copyright (C) 2011-2013 Canonical Ltd
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
 * Authored by: Marco Trevisan (Treviño) <3v1n0@ubuntu.com>
 *
 */

#ifndef __BAMF_PRIVATE_H__
#define __BAMF_PRIVATE_H__

#include <libbamf-private/bamf-gdbus-generated.h>
#include <libbamf-private/bamf-gdbus-view-generated.h>

#define BAMF_DBUS_SERVICE_NAME (g_getenv ("BAMF_TEST_MODE") ? "org.ayatana.bamf.Test" : "org.ayatana.bamf")

#define BAMF_DBUS_BASE_PATH "/org/ayatana/bamf"
#define BAMF_DBUS_CONTROL_PATH BAMF_DBUS_BASE_PATH"/control"
#define BAMF_DBUS_MATCHER_PATH BAMF_DBUS_BASE_PATH"/matcher"

#define BAMF_DBUS_DEFAULT_TIMEOUT 500

/* GLib doesn't provide this by default */
#ifndef G_KEY_FILE_DESKTOP_KEY_FULLNAME
#define G_KEY_FILE_DESKTOP_KEY_FULLNAME "X-GNOME-FullName"
#endif

#define BAMF_APPLICATION_DEFAULT_ICON "application-default-icon"

/* GCC Macros to handle warnings
 * Authored by: Patrick Horgan <patrick@dbp-consulting.com>
 *  http://dbp-consulting.com/tutorials/SuppressingGCCWarnings.html
 */
#if ((__GNUC__ * 100) + __GNUC_MINOR__) >= 402
#define GCC_DIAG_STR(s) #s
#define GCC_DIAG_JOINSTR(x,y) GCC_DIAG_STR(x ## y)
# define GCC_DIAG_DO_PRAGMA(x) _Pragma (#x)
# define GCC_DIAG_PRAGMA(x) GCC_DIAG_DO_PRAGMA(GCC diagnostic x)
# if ((__GNUC__ * 100) + __GNUC_MINOR__) >= 406
#  define GCC_DIAG_OFF(x) GCC_DIAG_PRAGMA(push) \
    GCC_DIAG_PRAGMA(ignored GCC_DIAG_JOINSTR(-W,x))
#  define GCC_DIAG_ON(x) GCC_DIAG_PRAGMA(pop)
# else
#  define GCC_DIAG_OFF(x) GCC_DIAG_PRAGMA(ignored GCC_DIAG_JOINSTR(-W,x))
#  define GCC_DIAG_ON(x)  GCC_DIAG_PRAGMA(warning GCC_DIAG_JOINSTR(-W,x))
# endif
#else
# define GCC_DIAG_OFF(x)
# define GCC_DIAG_ON(x)
#endif

#define GCC_IGNORE_DEPRECATED_BEGIN GCC_DIAG_OFF(deprecated-declarations)
#define GCC_IGNORE_DEPRECATED_END GCC_DIAG_ON(deprecated-declarations)

#endif
