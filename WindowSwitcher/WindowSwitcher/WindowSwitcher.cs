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
using System.Threading;
using System.Collections.Generic;
using System.Linq;

using Cairo;
using Gdk;
using Gtk;
using Wnck;

namespace WindowSwitcher
{
	public class WindowSwitcher : Gtk.Window
	{
		internal static void Main ()
		{
			Gtk.Application.Init ();
			LibWnckSync.Global.Init ();
			
			new WindowSwitcher ();
			
			Gtk.Application.Run ();
		}

		SwitcherArea area;
		XKeybinder binder;
		
		public WindowSwitcher () : base (Gtk.WindowType.Toplevel)
		{
			
			SkipPagerHint = true;
			SkipTaskbarHint = true;
			AcceptFocus = false;
			Decorated = false;
			AppPaintable = true;
			KeepAbove = true;
			
			Stick ();
			this.SetCompositeColormap ();
			
			area = new SwitcherArea ();
			Add (area);
			
			AddEvents ((int) (Gdk.EventMask.KeyPressMask | Gdk.EventMask.KeyReleaseMask));
			
			ExposeEvent += HandleExposeEvent; 
			SizeAllocated += HandleSizeAllocated;
			
			binder = new XKeybinder ();
			binder.Bind ("<super>ISO_Left_Tab", OnSummoned);
			binder.Bind ("<super>quoteleft", OnSummoned);
		}

		protected override bool OnKeyPressEvent (Gdk.EventKey evnt)
		{
			switch (evnt.Key) {
			case Gdk.Key.asciitilde:
				area.PrevApp ();
				break;
			case Gdk.Key.quoteleft:
				area.NextApp ();
				break;
			case Gdk.Key.Tab:
				area.Next ();
				break;
			case Gdk.Key.ISO_Left_Tab:
				area.Prev ();
				break;
			}
			return base.OnKeyPressEvent (evnt);
		}
		
		protected override bool OnKeyReleaseEvent (Gdk.EventKey evnt)
		{
			if (evnt.Key == Gdk.Key.Super_L || evnt.Key == Gdk.Key.Return)
				HideAll ();
			
			return base.OnKeyReleaseEvent (evnt);
		}
		
		protected override void OnShown ()
		{
			Resize (1, 1);
			
			base.OnShown ();
		}
		
		void OnSummoned (object sender, System.EventArgs args)
		{
			if (Visible)
				return;
			
			ShowAll ();
			Stick ();
			
			for (int i = 0; i < 100; i++) {
				if (TryGrab ()) {
					break;
				}
				Thread.Sleep (100);
			}
		}
		
		bool TryGrab ()
		{
			uint time;
			time = Gtk.Global.CurrentEventTime;

			if (Keyboard.Grab (GdkWindow, true, time) == GrabStatus.Success) {
				Gtk.Grab.Add (this);
				return true;
			}
			return false;
		}

		void HandleSizeAllocated(object o, SizeAllocatedArgs args)
		{
			int x = (Screen.Width - args.Allocation.Width) / 2;
			int y = (Screen.Height - args.Allocation.Height) / 2;
			
			Move (x, y);
		}

		void HandleExposeEvent (object o, ExposeEventArgs args)
		{
			using (Cairo.Context cr = CairoHelper.Create (args.Event.Window)) {
				cr.Operator = Operator.Source;
				
				cr.Color = new Cairo.Color (0, 0, .3, .3);
				cr.Paint ();
				
				cr.Operator = Operator.Over;
			}
		}
	}
}
