using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;

namespace KaidelCore
{
	public class Input
	{
		public static bool IsKeyDown(KeyCode keyCode)
		{
			return Internals.Input_IsKeyDown(ref keyCode);
		}
		public static bool IsMouseDown(MouseCode mouseCode) {
			return Internals.Input_IsMouseDown(ref mouseCode);
		}
	}
}
