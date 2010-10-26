/*
 * bamf-view-private.h
 * This file is part of BAMF
 *
 * Copyright (C) 2010 - Jason Smith
 *
 * BAMF is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * BAMF is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with BAMF; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin St, Fifth Floor, 
 * Boston, MA  02110-1301  USA
 */

#ifndef _BAMF_VIEW_PRIVATE_H_
#define _BAMF_VIEW_PRIVATE_H_

#include <libbamf/bamf-view.h>

void bamf_view_set_path (BamfView *view, const char *dbus_path);

gboolean bamf_view_remote_ready (BamfView *view);

void bamf_view_set_name (BamfView *view, const char *name);

void bamf_view_set_icon (BamfView *view, const char *icon);
#endif
