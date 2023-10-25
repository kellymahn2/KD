using System.Runtime.CompilerServices;
using System.Collections.Generic;

namespace KaidelCore
{
	internal static partial class Internals
	{
		#region Math
		#region Vector2
		[MethodImpl(MethodImplOptions.InternalCall)]
		internal extern static void Vector2_AddVec(ref Vector2 a, ref Vector2 b, out Vector2 res);
		[MethodImpl(MethodImplOptions.InternalCall)]
		internal extern static void Vector2_SubtractVec(ref Vector2 a, ref Vector2 b, out Vector2 res);
		[MethodImpl(MethodImplOptions.InternalCall)]
		internal extern static void Vector2_AddNum(ref Vector2 a, float b, out Vector2 res);
		[MethodImpl(MethodImplOptions.InternalCall)]
		internal extern static void Vector2_SubtractNum(ref Vector2 a, float b, out Vector2 res);
		[MethodImpl(MethodImplOptions.InternalCall)]
		internal extern static void Vector2_MultNum(ref Vector2 a, float b, out Vector2 res);
		[MethodImpl(MethodImplOptions.InternalCall)]
		internal extern static void Vector2_MultVec(ref Vector2 a, ref Vector2 b, out Vector2 res);
		[MethodImpl(MethodImplOptions.InternalCall)]
		internal extern static void Vector2_DivNum(ref Vector2 a, float b, out Vector2 res);
		[MethodImpl(MethodImplOptions.InternalCall)]
		internal extern static void Vector2_DivVec(ref Vector2 a, ref Vector2 b, out Vector2 res);
		[MethodImpl(MethodImplOptions.InternalCall)]
		internal extern static float Vector2_DotVec(ref Vector2 a, ref Vector2 b);
		[MethodImpl(MethodImplOptions.InternalCall)]
		internal extern static float Vector2_LengthVec(ref Vector2 a);
		#endregion
		#region Vector3
		[MethodImpl(MethodImplOptions.InternalCall)]
		internal extern static void Vector3_AddVec(ref Vector3 a, ref Vector3 b, out Vector3 res);
		[MethodImpl(MethodImplOptions.InternalCall)]
		internal extern static void Vector3_SubtractVec(ref Vector3 a, ref Vector3 b, out Vector3 res);
		[MethodImpl(MethodImplOptions.InternalCall)]
		internal extern static void Vector3_AddNum(ref Vector3 a, float b, out Vector3 res);
		[MethodImpl(MethodImplOptions.InternalCall)]
		internal extern static void Vector3_SubtractNum(ref Vector3 a, float b, out Vector3 res);
		[MethodImpl(MethodImplOptions.InternalCall)]
		internal extern static void Vector3_MultNum(ref Vector3 a, float b, out Vector3 res);
		[MethodImpl(MethodImplOptions.InternalCall)]
		internal extern static void Vector3_MultVec(ref Vector3 a,ref Vector3 b, out Vector3 res);
		[MethodImpl(MethodImplOptions.InternalCall)]
		internal extern static void Vector3_DivNum(ref Vector3 a, float b, out Vector3 res);
		[MethodImpl(MethodImplOptions.InternalCall)]
		internal extern static void Vector3_DivVec(ref Vector3 a, ref Vector3 b, out Vector3 res);
		[MethodImpl(MethodImplOptions.InternalCall)]
		internal extern static float Vector3_DotVec(ref Vector3 a, ref Vector3 b);
		[MethodImpl(MethodImplOptions.InternalCall)]
		internal extern static float Vector3_CrossVec(ref Vector3 a, ref Vector3 b,out Vector3 res);
		[MethodImpl(MethodImplOptions.InternalCall)]
		internal extern static float Vector3_LengthVec(ref Vector3 a);
		[MethodImpl(MethodImplOptions.InternalCall)]
		internal extern static void Vector3_RotateAround(ref Vector3 a, ref Vector3 origin,ref Vector3 angle,out Vector3 res);
		#endregion

		#region Vector4
		[MethodImpl(MethodImplOptions.InternalCall)]
		internal extern static void Vector4_AddVec(ref Vector4 a, ref Vector4 b, out Vector4 res);
		[MethodImpl(MethodImplOptions.InternalCall)]
		internal extern static void Vector4_SubtractVec(ref Vector4 a, ref Vector4 b, out Vector4 res);
		[MethodImpl(MethodImplOptions.InternalCall)]
		internal extern static void Vector4_AddNum(ref Vector4 a, float b, out Vector4 res);
		[MethodImpl(MethodImplOptions.InternalCall)]
		internal extern static void Vector4_SubtractNum(ref Vector4 a, float b, out Vector4 res);
		[MethodImpl(MethodImplOptions.InternalCall)]
		internal extern static void Vector4_MultNum(ref Vector4 a, float b, out Vector4 res);
		[MethodImpl(MethodImplOptions.InternalCall)]
		internal extern static void Vector4_MultVec(ref Vector4 a, ref Vector4 b, out Vector4 res);
		[MethodImpl(MethodImplOptions.InternalCall)]
		internal extern static void Vector4_DivNum(ref Vector4 a, float b, out Vector4 res);
		[MethodImpl(MethodImplOptions.InternalCall)]
		internal extern static void Vector4_DivVec(ref Vector4 a, ref Vector4 b, out Vector4 res);
		[MethodImpl(MethodImplOptions.InternalCall)]
		internal extern static float Vector4_DotVec(ref Vector4 a, ref Vector4 b);
		[MethodImpl(MethodImplOptions.InternalCall)]
		internal extern static float Vector4_LengthVec(ref Vector4 a);
		#endregion

		[MethodImpl(MethodImplOptions.InternalCall)]
		internal extern static ulong Math_Fact(ulong n);
		[MethodImpl(MethodImplOptions.InternalCall)]
		internal extern static void Math_Bezier(List<Vector3> points,float t,out Vector3 res);




		#endregion
	}
}
