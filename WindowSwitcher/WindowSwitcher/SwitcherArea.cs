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

using Cairo;
using Gdk;
using Gtk;
using Gnome;
using Wnck;

namespace WindowSwitcher
{
	
	
	public class SwitcherArea : Gtk.DrawingArea
	{
		class SwitcherItem
		{
			public DesktopItem DesktopItem { get; private set; }
			
			public string DesktopFile { get; private set; }

			IEnumerable<Wnck.Window> windows;
			public IEnumerable<Wnck.Window> Windows {
				get {
					return windows;
				}
			}
			
			public IEnumerable<Wnck.Window> VisibleWindows {
				get { 
					return windows.Where (w => !w.IsSkipTasklist);
				}
			}
			
			public SwitcherItem (string desktopFile)
			{
				DesktopItem = Gnome.DesktopItem.NewFromFile (desktopFile, 0);
				DesktopFile = desktopFile;
				Build ();
			}
			
			public SwitcherItem (DesktopItem item)
			{
				DesktopItem = item;
				DesktopFile = item.Location;
				if (DesktopFile.StartsWith ("file://"))
					DesktopFile = DesktopFile.Substring ("file://".Length);
				Build ();
			}
			
			void Build ()
			{
				// to array for performance reasons
				windows = LibWnckSync.Global.WindowsForDesktopFile (DesktopFile)
					
					.OrderBy (w => w.Xid)
					.ToArray ();
			}
		}
		
		const int IconSize = 64;
		const int BufferSize = 8;
		const int WindowItemWidth = 300;
		const int WindowItemHeight = 20;
		
		Cairo.Color BackgroundColor = new Cairo.Color (0.7, 0.7, 0.7, .55);
		Cairo.Color HighlightColorTop = new Cairo.Color (0.15, 0.15, 0.15, .85);
		Cairo.Color HighlightColorBottom = new Cairo.Color (0, 0, 0, .85);
		Cairo.Color BorderColor = new Cairo.Color (.3, .3, .3, .7);
		
		IEnumerable<SwitcherItem> SwitcherItems { get; set; }
		
		Wnck.Window current_window;
		
		Wnck.Window CurrentWindow { 
			get { return current_window; }
		}
		
		public SwitcherArea ()
		{
			AddEvents ((int) Gdk.EventMask.AllEventsMask);
			
			SwitcherItems = null;
			
			Wnck.Global.ClientType = ClientType.Pager;
			
			Realized += delegate {
				GdkWindow.SetBackPixmap (null, false);
			};
			
			StyleSet += delegate {
				if (IsRealized)
					GdkWindow.SetBackPixmap (null, false);
			};
			
			Wnck.Screen.Default.WindowOpened += HandleWindowOpened;
			Wnck.Screen.Default.WindowClosed += HandleWindowClosed; 
		}
		
		public void Next ()
		{
			bool found = false;
			foreach (SwitcherItem si in SwitcherItems) {
				foreach (Wnck.Window window in si.VisibleWindows) {
					if (found) {
						SetWindow (window);
						return;
					} else if (window == CurrentWindow) {
						found = true;
					}
				}
			}
			
			Wnck.Window w = SwitcherItems.First ().VisibleWindows.First ();
			SetWindow (w);
		}
		
		public void Prev ()
		{
			if (SwitcherItems.First ().VisibleWindows.First () == CurrentWindow) {
				SetWindow (SwitcherItems.Last ().VisibleWindows.Last ());
				return;
			}
			
			Wnck.Window prev = null;
			foreach (SwitcherItem si in SwitcherItems) {
				foreach (Wnck.Window window in si.VisibleWindows) {
					if (window == CurrentWindow) {
						if (prev != null)
							SetWindow (prev);
						return;
					}
					prev = window;
				}
			}
		}
		
		public void NextApp ()
		{
			bool found = false;
			foreach (SwitcherItem si in SwitcherItems) {
				if (found) {
					SetWindow (si.VisibleWindows.First ());
					return;
				} else if (si.VisibleWindows.Contains (CurrentWindow)) {
					found = true;
				}
			}
			
			Wnck.Window w = SwitcherItems.First ().VisibleWindows.First ();
			SetWindow (w);
		}
		
		public void PrevApp ()
		{
			if (SwitcherItems.First ().VisibleWindows.Contains (CurrentWindow)) {
				SetWindow (SwitcherItems.Last ().VisibleWindows.First ());
				return;
			}
			
			SwitcherItem prev = null;
			foreach (SwitcherItem si in SwitcherItems) {
				if (si.VisibleWindows.Contains (CurrentWindow)) {
					if (prev != null)
						SetWindow (prev.VisibleWindows.First ());
					return;
				}
				prev = si;
			}
		}
		
		void SetWindow (Wnck.Window w)
		{
			current_window = w;
			QueueDraw ();			
		}
		
		void ActivateWindow (Wnck.Window w)
		{
			if (!w.IsOnWorkspace (Wnck.Screen.Default.ActiveWorkspace))
				w.Workspace.Activate (Gtk.Global.CurrentEventTime);
			w.Activate (Gtk.Global.CurrentEventTime);
		}

		void HandleWindowOpened (object o, WindowOpenedArgs args)
		{
			
		}

		void HandleWindowClosed (object o, WindowClosedArgs args)
		{
			
		}
		
		protected override bool OnExposeEvent (Gdk.EventExpose evnt)
		{
			using (Cairo.Context cr = CairoHelper.Create (GdkWindow)) {
				cr.Operator = Operator.Source;
				cr.Color = new Cairo.Color (0, 0, 0, 0);
				cr.Paint ();
				cr.Operator = Operator.Over;
				
				DrawAndClipBackground (cr, evnt.Area);
				DrawIcons (cr, evnt.Area);
			}
			
			return true;
		}
		
		void DrawAndClipBackground (Cairo.Context cr, Gdk.Rectangle region)
		{
			cr.RoundedRectangle (region.X + 2, region.Y + 2, region.Width - 4, region.Height - 4, 8);
			cr.Color = BackgroundColor;
			cr.FillPreserve ();
			
			cr.Color = BorderColor;
			cr.Stroke ();
			
			cr.RoundedRectangle (region.X + 3, region.Y + 3, region.Width - 6, region.Height - 6, 7);
			cr.Clip ();
		}
		
		void DrawIcons (Cairo.Context cr, Gdk.Rectangle region)
		{
			int offset = 0;
			foreach (SwitcherItem item in SwitcherItems) {
				DrawIcon (cr, region, item, offset);
				offset++;
			}
		}
		
		void DrawIcon (Cairo.Context cr, Gdk.Rectangle region, SwitcherItem item, int offset)
		{
			Gnome.DesktopItem di = item.DesktopItem;
			Gdk.Pixbuf icon;
			
			if (di.AttrExists ("Icon") && Gtk.IconTheme.Default.HasIcon (di.GetString ("Icon"))) {
				icon = Gtk.IconTheme.Default.LoadIcon (di.GetString ("Icon"), IconSize, 0);
			} else if (item.VisibleWindows.First ().Icon != null) {
				icon = item.VisibleWindows.First ().Icon;
			} else {
				icon = Gtk.IconTheme.Default.LoadIcon ("emblem-noread", IconSize, 0);
			}
			
			if (icon.Height != IconSize) {
				double delta = IconSize / (double) icon.Height;
				Gdk.Pixbuf tmp = icon.ScaleSimple ((int) (icon.Height * delta), (int) (icon.Width * delta), InterpType.Bilinear);
				icon.Dispose ();
				icon = tmp;
			}
				
			bool isCurrent = item.VisibleWindows.Contains (CurrentWindow);
			Gdk.Rectangle iconRegion = new Gdk.Rectangle (offset * (2 * BufferSize + IconSize), region.Y, IconSize + BufferSize * 2, IconSize + BufferSize * 2);
			
			// draw highlight
			if (isCurrent) {
				int yOffset = iconRegion.Y + iconRegion.Height + BufferSize;
				cr.MoveTo (iconRegion.X, 0);
				cr.LineTo (iconRegion.X, yOffset);
				cr.LineTo (0, yOffset);
				cr.LineTo (0, HeightRequest);
				cr.LineTo (WidthRequest, HeightRequest);
				cr.LineTo (WidthRequest, yOffset);
				cr.LineTo (iconRegion.X + iconRegion.Width, yOffset);
				cr.LineTo (iconRegion.X + iconRegion.Width, 0);
				
				
				LinearGradient lg = new LinearGradient (0, region.Y, 0, region.Height);
				lg.AddColorStop (0, HighlightColorTop);
				lg.AddColorStop (1, HighlightColorBottom);
				
				cr.Pattern = lg;
				
				lg.Destroy ();
				cr.FillPreserve ();
				
				cr.Color = BorderColor;
				cr.Stroke ();
			}
				
			CairoHelper.SetSourcePixbuf (cr, icon, iconRegion.X + BufferSize, iconRegion.Y + BufferSize);
			cr.Paint ();
			
			icon.Dispose ();
			
			if (isCurrent) {
				int i = 0;
				int columns = WidthRequest / WindowItemWidth;
				int columnBuffer = (WidthRequest % WindowItemWidth) / (columns + 1);
				
				foreach (Wnck.Window w in item.VisibleWindows) {
					int column = i % columns;
					int row = i / columns;
					DrawWindowItem (cr, 
					                new Gdk.Rectangle (columnBuffer + (WindowItemWidth + columnBuffer) * column, 
					                                   (region.Y + 4 * BufferSize + IconSize) + (row * (WindowItemHeight + BufferSize)), 
					                                   WindowItemWidth, 
					                                   WindowItemHeight),
					                w);
					
					i++;
				}
			}
		}
		
		void DrawWindowItem (Cairo.Context cr, Gdk.Rectangle area, Wnck.Window window)
		{
			bool isCurrent = window == CurrentWindow;
			Gdk.Pixbuf icon = window.Icon;
			
			if (icon.Height != area.Height) {
				double delta = area.Height / (double) icon.Height;
				Gdk.Pixbuf tmp = icon.ScaleSimple ((int) (icon.Height * delta), (int) (icon.Width * delta), InterpType.Bilinear);
				icon.Dispose ();
				icon = tmp;
			}
			
			CairoHelper.SetSourcePixbuf (cr, icon, area.X, area.Y);
			cr.PaintWithAlpha ((isCurrent) ? 1 : .5);
			
			Pango.Layout layout = Pango.CairoHelper.CreateLayout (cr);
			
			layout.FontDescription = Style.FontDescription;
			layout.FontDescription.AbsoluteSize = Pango.Units.FromPixels (12);
			layout.FontDescription.Weight = Pango.Weight.Bold;
			
			layout.SetText (window.Name);
			layout.Width = Pango.Units.FromPixels (WindowItemWidth - area.Height - BufferSize);
			layout.Ellipsize = Pango.EllipsizeMode.End;
			
			Pango.Rectangle inkRect, logicalRect;
			layout.GetExtents (out inkRect, out logicalRect);
			
			int y = area.Y + (int) ((area.Height - Pango.Units.ToPixels (logicalRect.Height)) / 2.0);
			
			cr.MoveTo (area.X + area.Height + BufferSize, y);
			Pango.CairoHelper.LayoutPath (cr, layout);
			
			if (isCurrent)
				cr.Color = new Cairo.Color (1, 1, 1);
			else
				cr.Color = new Cairo.Color (1, 1, 1, .5);
			cr.Fill ();
			
			icon.Dispose ();
		}
		
		protected override void OnShown ()
		{
			current_window = PreviousWindow ();
			
			// order things to get consistent feel, to array for performance
			SwitcherItems = GetSwitcherItems ().OrderBy (si => si.DesktopItem.GetString ("Name")).ToArray ();
			
			int width = (IconSize + BufferSize * 2) * SwitcherItems.Count ();
			
			SetSizeRequest (width, 175);
			
			base.OnShown ();
		}
		
		Wnck.Window PreviousWindow ()
		{
			return Wnck.Screen.Default.WindowsStacked
				.Reverse ()
				.Where (w => !w.IsSkipTasklist)
				.Skip (1)
				.First ();
		}
		
		protected override void OnHidden ()
		{
			if (current_window != null)
				ActivateWindow (current_window);
			base.OnHidden ();
		}


		IEnumerable<SwitcherItem> GetSwitcherItems ()
		{
			foreach (string file in RunningDesktopFiles ()) {
				SwitcherItem si = new SwitcherItem (file);
				if (si.VisibleWindows.Any ())
					yield return si;
			}
		}
			         
		IEnumerable<string> RunningDesktopFiles ()
		{
			return Wnck.Screen.Default.Windows
				.Select (w => LibWnckSync.Global.DesktopItemForWindow (w))
				.Where (s => !string.IsNullOrEmpty (s))
				.Distinct ();
		}
	}
}
