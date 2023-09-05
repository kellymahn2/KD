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
		#region Math
		#region Vector2
		[MethodImpl(MethodImplOptions.InternalCall)]
		internal extern static void Vector2_AddVec(ref Vector2_Impl a, ref Vector2_Impl b, out Vector2_Impl res);
		[MethodImpl(MethodImplOptions.InternalCall)]
		internal extern static void Vector2_SubtractVec(ref Vector2_Impl a, ref Vector2_Impl b, out Vector2_Impl res);
		[MethodImpl(MethodImplOptions.InternalCall)]
		internal extern static void Vector2_AddNum(ref Vector2_Impl a, float b, out Vector2_Impl res);
		[MethodImpl(MethodImplOptions.InternalCall)]
		internal extern static void Vector2_SubtractNum(ref Vector2_Impl a, float b, out Vector2_Impl res);
		[MethodImpl(MethodImplOptions.InternalCall)]
		internal extern static void Vector2_MultNum(ref Vector2_Impl a, float b, out Vector2_Impl res);
		[MethodImpl(MethodImplOptions.InternalCall)]
		internal extern static void Vector2_MultVec(ref Vector2_Impl a, ref Vector2_Impl b, out Vector2_Impl res);
		[MethodImpl(MethodImplOptions.InternalCall)]
		internal extern static void Vector2_DivNum(ref Vector2_Impl a, float b, out Vector2_Impl res);
		[MethodImpl(MethodImplOptions.InternalCall)]
		internal extern static void Vector2_DivVec(ref Vector2_Impl a, ref Vector2_Impl b, out Vector2_Impl res);
		[MethodImpl(MethodImplOptions.InternalCall)]
		internal extern static float Vector2_DotVec(ref Vector2_Impl a, ref Vector2_Impl b);
		[MethodImpl(MethodImplOptions.InternalCall)]
		internal extern static float Vector2_LengthVec(ref Vector2_Impl a);
		#endregion
		#region Vector3
		[MethodImpl(MethodImplOptions.InternalCall)]
		internal extern static void Vector3_AddVec(ref Vector3_Impl a, ref Vector3_Impl b, out Vector3_Impl res);
		[MethodImpl(MethodImplOptions.InternalCall)]
		internal extern static void Vector3_SubtractVec(ref Vector3_Impl a, ref Vector3_Impl b, out Vector3_Impl res);
		[MethodImpl(MethodImplOptions.InternalCall)]
		internal extern static void Vector3_AddNum(ref Vector3_Impl a, float b, out Vector3_Impl res);
		[MethodImpl(MethodImplOptions.InternalCall)]
		internal extern static void Vector3_SubtractNum(ref Vector3_Impl a, float b, out Vector3_Impl res);
		[MethodImpl(MethodImplOptions.InternalCall)]
		internal extern static void Vector3_MultNum(ref Vector3_Impl a, float b, out Vector3_Impl res);
		[MethodImpl(MethodImplOptions.InternalCall)]
		internal extern static void Vector3_MultVec(ref Vector3_Impl a,ref Vector3_Impl b, out Vector3_Impl res);
		[MethodImpl(MethodImplOptions.InternalCall)]
		internal extern static void Vector3_DivNum(ref Vector3_Impl a, float b, out Vector3_Impl res);
		[MethodImpl(MethodImplOptions.InternalCall)]
		internal extern static void Vector3_DivVec(ref Vector3_Impl a, ref Vector3_Impl b, out Vector3_Impl res);
		[MethodImpl(MethodImplOptions.InternalCall)]
		internal extern static float Vector3_DotVec(ref Vector3_Impl a, ref Vector3_Impl b);
		[MethodImpl(MethodImplOptions.InternalCall)]
		internal extern static float Vector3_CrossVec(ref Vector3_Impl a, ref Vector3_Impl b,out Vector3_Impl res);
		[MethodImpl(MethodImplOptions.InternalCall)]
		internal extern static float Vector3_LengthVec(ref Vector3_Impl a);
		#endregion

		#region Vector4
		[MethodImpl(MethodImplOptions.InternalCall)]
		internal extern static void Vector4_AddVec(ref Vector4_Impl a, ref Vector4_Impl b, out Vector4_Impl res);
		[MethodImpl(MethodImplOptions.InternalCall)]
		internal extern static void Vector4_SubtractVec(ref Vector4_Impl a, ref Vector4_Impl b, out Vector4_Impl res);
		[MethodImpl(MethodImplOptions.InternalCall)]
		internal extern static void Vector4_AddNum(ref Vector4_Impl a, float b, out Vector4_Impl res);
		[MethodImpl(MethodImplOptions.InternalCall)]
		internal extern static void Vector4_SubtractNum(ref Vector4_Impl a, float b, out Vector4_Impl res);
		[MethodImpl(MethodImplOptions.InternalCall)]
		internal extern static void Vector4_MultNum(ref Vector4_Impl a, float b, out Vector4_Impl res);
		[MethodImpl(MethodImplOptions.InternalCall)]
		internal extern static void Vector4_MultVec(ref Vector4_Impl a, ref Vector4_Impl b, out Vector4_Impl res);
		[MethodImpl(MethodImplOptions.InternalCall)]
		internal extern static void Vector4_DivNum(ref Vector4_Impl a, float b, out Vector4_Impl res);
		[MethodImpl(MethodImplOptions.InternalCall)]
		internal extern static void Vector4_DivVec(ref Vector4_Impl a, ref Vector4_Impl b, out Vector4_Impl res);
		[MethodImpl(MethodImplOptions.InternalCall)]
		internal extern static float Vector4_DotVec(ref Vector4_Impl a, ref Vector4_Impl b);
		[MethodImpl(MethodImplOptions.InternalCall)]
		internal extern static float Vector4_LengthVec(ref Vector4_Impl a);
		#endregion


		#endregion
	}
}
