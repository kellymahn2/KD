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
		internal extern static void Rigidbody2DComponent_ApplyLinearImpulse(ulong id, ref Vector2 impulse, ref Vector2 point, bool wake);
		[MethodImpl(MethodImplOptions.InternalCall)]
		internal extern static void Rigidbody2DComponent_ApplyForce(ulong id, ref Vector2 force, ref Vector2 point, bool wake);
		[MethodImpl(MethodImplOptions.InternalCall)]
		internal extern static int Rigidbody2DComponent_GetBodyType(ulong id);
		[MethodImpl(MethodImplOptions.InternalCall)]
		internal extern static bool Rigidbody2DComponent_GetFixedRotation(ulong id);
		[MethodImpl(MethodImplOptions.InternalCall)]
		internal extern static void Rigidbody2DComponent_SetFixedRotation(ulong id,bool value);
		[MethodImpl(MethodImplOptions.InternalCall)]
		internal extern static void BoxCollider2DComponent_GetOffset(ulong id,out Vector2 res);
		[MethodImpl(MethodImplOptions.InternalCall)]
		internal extern static void BoxCollider2DComponent_SetOffset(ulong id, ref Vector2 value);
		[MethodImpl(MethodImplOptions.InternalCall)]
		internal extern static void BoxCollider2DComponent_GetSize(ulong id, out Vector2 res);
		[MethodImpl(MethodImplOptions.InternalCall)]
		internal extern static void BoxCollider2DComponent_SetSize(ulong id, ref Vector2 value);
		[MethodImpl(MethodImplOptions.InternalCall)]
		internal extern static float BoxCollider2DComponent_GetDensity(ulong id);
		[MethodImpl(MethodImplOptions.InternalCall)]
		internal extern static void BoxCollider2DComponent_SetDensity(ulong id,float value);
		[MethodImpl(MethodImplOptions.InternalCall)]
		internal extern static float BoxCollider2DComponent_GetFriction(ulong id);
		[MethodImpl(MethodImplOptions.InternalCall)]
		internal extern static void BoxCollider2DComponent_SetFriction(ulong id, float value);
		[MethodImpl(MethodImplOptions.InternalCall)]
		internal extern static float BoxCollider2DComponent_GetRestitution(ulong id);
		[MethodImpl(MethodImplOptions.InternalCall)]
		internal extern static void BoxCollider2DComponent_SetRestitution(ulong id,float value);
		[MethodImpl(MethodImplOptions.InternalCall)]
		internal extern static float BoxCollider2DComponent_GetRestitutionThreshold(ulong id);
		[MethodImpl(MethodImplOptions.InternalCall)]
		internal extern static void BoxCollider2DComponent_SetRestitutionThreshold(ulong id, float value);
		[MethodImpl(MethodImplOptions.InternalCall)]
		internal extern static void CircleCollider2DComponent_GetOffset(ulong id, out Vector2 res);
		[MethodImpl(MethodImplOptions.InternalCall)]
		internal extern static void CircleCollider2DComponent_SetOffset(ulong id,ref Vector2 value);
		[MethodImpl(MethodImplOptions.InternalCall)]
		internal extern static float CircleCollider2DComponent_GetRadius(ulong id);
		[MethodImpl(MethodImplOptions.InternalCall)]
		internal extern static void CircleCollider2DComponent_SetRadius(ulong id, float value);
		[MethodImpl(MethodImplOptions.InternalCall)]
		internal extern static float CircleCollider2DComponent_GetDensity(ulong id);
		[MethodImpl(MethodImplOptions.InternalCall)]
		internal extern static void CircleCollider2DComponent_SetDensity(ulong id, float value);
		[MethodImpl(MethodImplOptions.InternalCall)]
		internal extern static float CircleCollider2DComponent_GetFriction(ulong id);
		[MethodImpl(MethodImplOptions.InternalCall)]
		internal extern static void CircleCollider2DComponent_SetFriction(ulong id,float value);
		[MethodImpl(MethodImplOptions.InternalCall)]
		internal extern static float CircleCollider2DComponent_GetRestitution(ulong id);
		[MethodImpl(MethodImplOptions.InternalCall)]
		internal extern static void CircleCollider2DComponent_SetRestitution(ulong id,float value);
		[MethodImpl(MethodImplOptions.InternalCall)]
		internal extern static float CircleCollider2DComponent_GetRestitutionThreshold(ulong id);
		[MethodImpl(MethodImplOptions.InternalCall)]
		internal extern static void CircleCollider2DComponent_SetRestitutionThreshold(ulong id, float value);
	}
}
