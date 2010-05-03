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

#include "bamf-matcher.h"
#include "bamf-control.h"
#include "bamf-control-glue.h"

G_DEFINE_TYPE (BamfControl, bamf_control, G_TYPE_OBJECT);
#define BAMF_CONTROL_GET_PRIVATE(obj) (G_TYPE_INSTANCE_GET_PRIVATE(obj, \
BAMF_TYPE_CONTROL, BamfControlPrivate))

struct _BamfControlPrivate
{
  char * nothing; /* temporary to fix runtime warning about 0 size private struct */
};

static void
bamf_control_init (BamfControl * self)
{
  BamfControlPrivate *priv;
  priv = self->priv = BAMF_CONTROL_GET_PRIVATE (self);
}

static void
bamf_control_class_init (BamfControlClass * klass)
{
  dbus_g_object_type_install_info (BAMF_TYPE_CONTROL,
				   &dbus_glib_bamf_control_object_info);
  
  g_type_class_add_private (klass, sizeof (BamfControlPrivate));
}

gboolean 
bamf_control_register_application_for_pid (BamfControl *control,
                                           char *application,
                                           gint32 pid,
                                           GError **error)
{
  bamf_matcher_register_desktop_file_for_pid (bamf_matcher_get_default (),
                                              application, pid);

  return TRUE;
}

gboolean 
bamf_control_register_tab_provider (BamfControl *control,
                                    char *path,
                                    GError **error)
{
  return TRUE;
}

BamfControl *
bamf_control_get_default (void)
{
  static BamfControl *control;

  if (!BAMF_IS_CONTROL (control))
    {
      DBusGConnection *bus;
      GError *error = NULL;
      
      control = (BamfControl *) g_object_new (BAMF_TYPE_CONTROL, NULL);

      bus = dbus_g_bus_get (DBUS_BUS_SESSION, &error);

      g_return_val_if_fail (bus, control);
      
      dbus_g_connection_register_g_object (bus, BAMF_CONTROL_PATH,
	    			           G_OBJECT (control));
      
      return control;
    }

  return g_object_ref (G_OBJECT (control));
}
