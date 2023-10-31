using System;
using System.Collections.Generic;
using System.Linq;
using System.Runtime.CompilerServices;
using System.Text;
using System.Threading.Tasks;

namespace KaidelCore
{
	internal static partial class Internals
	{
		[MethodImpl(MethodImplOptions.InternalCall)]
		internal extern static bool Entity_HasParent(ulong ID);
		[MethodImpl(MethodImplOptions.InternalCall)]
		internal extern static bool Entity_HasChildren(ulong ID);
		[MethodImpl(MethodImplOptions.InternalCall)]
		internal extern static ulong Entity_GetParentID(ulong ID);
		[MethodImpl(MethodImplOptions.InternalCall)]
		internal extern static ulong Entity_GetChildEntityIDWithIndex(ulong ID,uint index);
		[MethodImpl(MethodImplOptions.InternalCall)]
		internal extern static ulong Entity_GetChildEntityIDWithName(ulong ID, string name);
	}
}
