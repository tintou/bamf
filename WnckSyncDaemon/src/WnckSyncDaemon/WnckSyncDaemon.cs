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

using System;
using System.Collections.Generic;
using System.Linq;

using NDesk.DBus;
using org.freedesktop.DBus;

namespace WnckSyncDaemon
{
	
	
	public class WnckSyncDaemon
	{
		const string BusName = "org.wncksync.WnckSync";
		const string BaseItemPath = "/org/wncksync/WnckSync";
		const string ControlItemPath = BaseItemPath + "/Control";
		
		static Bus Bus { get; set; }
		static IControl Control { get; set; }
		
		public static void Main (string [] args)
		{
			Gtk.Application.Init ();
			BusG.Init ();
			WindowMatcher.Initialize ();
			
			Bus = Bus.Session;
			Control = new Control ();
			
			Bus.RequestName (BusName);
			Bus.Register (new ObjectPath (ControlItemPath), Control);
			
			Gtk.Application.Run ();
		}
	}
}
