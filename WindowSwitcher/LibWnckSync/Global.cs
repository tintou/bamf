//  
//  Copyright (C) 2009 GNOME Do
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
using System.Runtime.InteropServices;
using System.Linq;

namespace LibWnckSync
{
	
	
	public static class Global
	{
		[StructLayout (LayoutKind.Sequential)]
		struct NativeGArray { 
			public IntPtr data; 
			public uint length; 
		}
		
		const string libWnckSync = "libwncksync.so";
		
		[DllImport (libWnckSync, CharSet=CharSet.Ansi)]
		static extern IntPtr wncksync_xids_for_desktop_file (IntPtr desktop_file);
		
		[DllImport (libWnckSync, CharSet=CharSet.Ansi)]
		static extern IntPtr wncksync_desktop_item_for_xid (IntPtr xid);
		
		[DllImport (libWnckSync, CharSet=CharSet.Ansi)]
		static extern void wncksync_init ();
		
		[DllImport (libWnckSync, CharSet=CharSet.Ansi)]
		static extern void wncksync_quit ();
		
		public static void Init ()
		{
			wncksync_init ();
		}
		
		public static void Quit ()
		{
			wncksync_quit ();
		}
		
		public static IEnumerable<Wnck.Window> WindowsForDesktopFile (string desktopFile)
		{
			IntPtr vaPtr = wncksync_xids_for_desktop_file (Marshal.StringToHGlobalAnsi (desktopFile));
			NativeGArray val = (NativeGArray) Marshal.PtrToStructure (vaPtr, typeof (NativeGArray));
			
			for (int i = 0; i < val.length; i++) {
				IntPtr ptr = Marshal.ReadIntPtr (val.data, i * 8);
				yield return Wnck.Window.Get ((ulong) ptr);
			}
			yield break;
		}
		
		public static string DesktopItemForWindow (Wnck.Window window)
		{
			return Marshal.PtrToStringAuto (wncksync_desktop_item_for_xid ((IntPtr) window.Xid));
		}
	}
}
