// WindowUtils.cs
// 
// Copyright (C) 2008 GNOME Do, Jason Smith
//
// This program is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with this program.  If not, see <http://www.gnu.org/licenses/>.
//

using System;
using System.Collections.Generic;
using System.Diagnostics;
using System.IO;
using System.Linq;
using System.Text.RegularExpressions;

using Wnck;

namespace WnckSyncDaemon
{
	
	public static class WindowMatcher
	{
		enum OpenOfficeProducts {
			Writer,
			Calc,
			Base,
			Math,
			Impress,
		}
		
		static bool initialized;
		
		static IEnumerable<string> PrefixStrings {
			get {
				yield return "gksu";
				yield return "sudo";
				yield return "java";
				yield return "mono";
				yield return "ruby";
				yield return "padsp";
				yield return "aoss";
				yield return "python(\\d.\\d)?";
				yield return "(ba)?sh";
			}
		}
		
		public static IEnumerable<Regex> BadPrefixes { get; private set; }
		
		static List<Window> window_list;
		static bool window_list_update_needed;
		
		static Dictionary<int, string> exec_lines = new Dictionary<int, string> ();
		static Dictionary<string, string> desktop_file_table;
		static DateTime last_update = new DateTime (0);
		
		public static void Initialize ()
		{
			if (initialized)
				return;
			
			initialized = true;
			
			List<Regex> regex = new List<Regex> ();
			foreach (string s in PrefixStrings) {
				 regex.Add (new Regex (string.Format ("^{0}$", s), RegexOptions.IgnoreCase));
			}
			
			BadPrefixes = regex.AsEnumerable ();
			
			desktop_file_table = CreateDesktopFileTable ();
			
			Wnck.Screen.Default.WindowClosed += delegate {
				window_list_update_needed = true;
			};
			
			Wnck.Screen.Default.WindowOpened += delegate {
				window_list_update_needed = true;
			};
			
			Wnck.Screen.Default.ApplicationOpened += delegate {
				window_list_update_needed = true;
			};
			
			Wnck.Screen.Default.ApplicationClosed += delegate {
				window_list_update_needed = true;
			};
		}
		
		public static IEnumerable<Wnck.Window> WindowListForDesktopFile (string desktopFile)
		{
			Gnome.DesktopItem item = null;
			
			try {
				item = Gnome.DesktopItem.NewFromFile (desktopFile, 0);
			} catch {
				yield break;
			}
			
			if (item == null || !item.AttrExists ("Exec"))
				yield break;
			
			foreach (Wnck.Window window in WindowListForCmd (item.GetString ("Exec")))
				yield return window;
		}
		
		public static string DesktopFileForWindow (Wnck.Window window)
		{
			string file = string.Empty;
			
			do {
				int pid = window.Pid;
				if (pid == 0) continue;
				
				string exec = CmdLineForPid (pid);
				if (exec == null) continue;
				
				exec = ProcessExecString (exec);
				if (desktop_file_table.ContainsKey (exec))
					file = desktop_file_table [exec];
			} while (false);
			
			return file;
		}
		
		static Dictionary<string, string> CreateDesktopFileTable ()
		{
			// fixme -- this is a terrible temporary hack used for testing purposes. Fix before
			// or during porting to C
			
			Dictionary<string, string> table = new Dictionary<string, string> ();
			
			if (!Directory.Exists ("/usr/share/applications"))
				return table;
			
			foreach (string file in Directory.GetFiles ("/usr/share/applications", "*.desktop")) {
				Gnome.DesktopItem item = null;
				try {
					item = Gnome.DesktopItem.NewFromFile (file, 0);
					if (item.AttrExists ("Exec")) {
						string exec = ProcessExecString (item.GetString ("Exec"));
						table [exec] = file;
					}
				} catch (Exception e) {
					Console.WriteLine (e.Message);
					// do nothing
				}
				if (item != null)
					item.Dispose ();
			}
			return table;
		}
		
		static void UpdateExecList ()
		{
			if ((DateTime.UtcNow - last_update).TotalMilliseconds < 200) return;
			
			Dictionary<int, string> old = exec_lines;
			
			exec_lines = new Dictionary<int, string> ();
			
			foreach (string dir in Directory.GetDirectories ("/proc")) {
				int pid;
				try { pid = Convert.ToInt32 (Path.GetFileName (dir)); } 
				catch { continue; }
				
				if (old.ContainsKey (pid)) {
					exec_lines [pid] = old [pid];
					continue;
				}
				
				string exec_line = CmdLineForPid (pid);
				if (string.IsNullOrEmpty (exec_line))
					continue;
				
				if (exec_line.Contains ("java") && exec_line.Contains ("jar")) {
					foreach (Window window in GetWindows ()) {
						if (window == null)
							continue;
						
						if (window.Pid == pid || window.Application.Pid == pid) {
							exec_line = window.ClassGroup.ResClass;
								
							// Vuze is stupid
							if (exec_line == "SWT")
								exec_line = window.Name;
						}
					}
				}	
				
				exec_line = ProcessExecString (exec_line);
					
				exec_lines [pid] = exec_line;
			}
			
			last_update = DateTime.UtcNow;
		}

		static List<Window> GetWindows ()
		{
			if (window_list == null || window_list_update_needed)
				window_list = new List<Window> (Wnck.Screen.Default.WindowsStacked);
			
			return window_list;
		}
		
		static string CmdLineForPid (int pid)
		{
			string cmdline = null;
			
			try {
				string procPath = new [] { "/proc", pid.ToString (), "cmdline" }.Aggregate (Path.Combine);
				using (StreamReader reader = new StreamReader (procPath)) {
					cmdline = reader.ReadLine ();
					reader.Close ();
				}
			} catch { }
			
			return cmdline;
		}
		
		static List<Window> WindowListForCmd (string exec)
		{
			List<Window> windows = new List<Window> ();
			if (string.IsNullOrEmpty (exec))
				return windows;
			
			// open office hakk
			if (exec.Contains ("ooffice")) {
				return GetOpenOfficeWindows (exec);
			}
			
			exec = ProcessExecString (exec);
			if (string.IsNullOrEmpty (exec))
				return windows;
			
			UpdateExecList ();
			
			foreach (KeyValuePair<int, string> kvp in exec_lines) {
				if (!string.IsNullOrEmpty (kvp.Value) && kvp.Value.Contains (exec)) {
					// we have a matching exec, now we just find every window whose PID matches this exec
					foreach (Window window in GetWindows ()) {
						if (window == null)
							continue;
						
						// this window matches the right PID and exec string, we can match it.
						bool pidMatch = window.Pid == kvp.Key || 
							(window.Application != null && window.Application.Pid == kvp.Key);
						
						if (pidMatch)
							windows.Add (window);
					}
				}
			}
			
			return windows.Distinct ().ToList ();
		}
		
		static List<Window> GetOpenOfficeWindows (string exec)
		{
			if (exec.Contains ("writer")) {
				return GetWindows ().Where ((Wnck.Window w) => w.Name.Contains ("OpenOffice.org Writer")).ToList ();
			} else if (exec.Contains ("math")) {
				return GetWindows ().Where ((Wnck.Window w) => w.Name.Contains ("OpenOffice.org Math")).ToList ();
			} else if (exec.Contains ("calc")) {
				return GetWindows ().Where ((Wnck.Window w) => w.Name.Contains ("OpenOffice.org Calc")).ToList ();
			} else if (exec.Contains ("impress")) {
				return GetWindows ().Where ((Wnck.Window w) => w.Name.Contains ("OpenOffice.org Impress")).ToList ();
			} else if (exec.Contains ("draw")) {
				return GetWindows ().Where ((Wnck.Window w) => w.Name.Contains ("OpenOffice.org Draw")).ToList ();
			} else {
				return new List<Window> (0);
			}
		}
		
		static string ProcessExecString (string exec)
		{
			if (string.IsNullOrEmpty (exec))
				return exec;
			
			// lower it and trim off white space so we can abuse whitespace a bit
			exec = exec.ToLower ().Trim ();
			
			// this is the "split" character or the argument separator.  If the string contains a null
			// it was fetched from /proc/PID/cmdline and will be nicely split up. Otherwise things get a bit
			// nasty, and it likely came from a .desktop file.
			char splitChar = Convert.ToChar (0x0);
			splitChar = exec.Contains (splitChar) ? splitChar : ' ';
			
			// this part is here soley for the remap file so that users may specify to remap based on just the name
			// without the full path.  If no remap file match is found, the net effect of this is nothing.
			if (exec.StartsWith ("/")) {
				string first_part = exec.Split (splitChar) [0];
				int length = first_part.Length;
				first_part = first_part.Split ('/').Last ();
				
				if (length < exec.Length)
					 first_part = first_part + " " + exec.Substring (length + 1);
			}
			
			string [] parts = exec.Split (splitChar);
			for (int i = 0; i < parts.Length; i++) {
				// we're going to use this a lot
				string out_val = parts [i];
				
				// arguments are useless
				if (out_val.StartsWith ("-"))
					continue;
				
				// there is probably a better way to handle this path splitting, we 
				// don't really care right now however.
				// we want the end of paths
				if (out_val.Contains ("/"))
					out_val = out_val.Split ('/').Last ();
				
				// wine apps can do it backwards... who knew?
				if (out_val.Contains ("\\"))
					out_val = out_val.Split ('\\').Last ();
				
				// null out our part if is a bad prefix
				foreach (Regex regex in BadPrefixes) {
					if (regex.IsMatch (out_val)) {
						out_val = null;
						break;
					}
				}
				
				// check if it was a bad prefix...
				if (!string.IsNullOrEmpty (out_val)) {
					// sometimes we hide things with shell scripts.  This is the most common method of doing it.
					if (out_val.EndsWith (".real"))
						out_val = out_val.Substring (0, out_val.Length - ".real".Length);
					
					return out_val;
				}
			}
			return null;
		}
	}
}
