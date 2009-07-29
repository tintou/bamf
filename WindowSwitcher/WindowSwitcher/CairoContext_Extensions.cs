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

using Cairo;

namespace WindowSwitcher
{
	
	
	public static class CairoContext_Extensions
	{
		public static void RoundedRectangle (this Cairo.Context self, double x, double y, double width, double height, double radius)
		{
			self.MoveTo (x+radius, y);
			self.Arc (x+width-radius, y+radius, radius, Math.PI*1.5, Math.PI*2);
			self.Arc (x+width-radius, y+height-radius, radius, 0, Math.PI*.5);
			self.Arc (x+radius, y+height-radius, radius, Math.PI*.5, Math.PI);
			self.Arc (x+radius, y+radius, radius, Math.PI, Math.PI*1.5);
		}
		
	}
}
