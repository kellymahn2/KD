using System;
using System.Diagnostics;
using System.Runtime.CompilerServices;

namespace KaidelCore
{
	

	internal static partial class Internals
	{
		[MethodImpl(MethodImplOptions.InternalCall)]
		internal extern static void NativeLog(string message);

		[MethodImpl(MethodImplOptions.InternalCall)]
		internal extern static string Entity_GetName(ulong id);
		[MethodImpl(MethodImplOptions.InternalCall)]
		internal extern static bool Entity_HasComponent(ulong id,Type componentType);

		[MethodImpl(MethodImplOptions.InternalCall)]
		internal extern static void TransformComponent_GetPosition(ulong id, out Vector3 position);
		[MethodImpl(MethodImplOptions.InternalCall)]
		internal extern static void TransformComponent_SetPosition(ulong id, ref Vector3 position);

		[MethodImpl(MethodImplOptions.InternalCall)]
		internal extern static void TransformComponent_GetRotation(ulong id, out Vector3 rotation);
		[MethodImpl(MethodImplOptions.InternalCall)]
		internal extern static void TransformComponent_SetRotation(ulong id, ref Vector3 rotation);


		[MethodImpl(MethodImplOptions.InternalCall)]
		internal extern static void TransformComponent_GetScale(ulong id, out Vector3 scale);
		[MethodImpl(MethodImplOptions.InternalCall)]
		internal extern static void TransformComponent_SetScale(ulong id, ref Vector3 scale);


		[MethodImpl(MethodImplOptions.InternalCall)]
		internal extern static void SpriteRendererComponent_GetColor(ulong id, out Vector4 color);
		[MethodImpl(MethodImplOptions.InternalCall)]
		internal extern static void SpriteRendererComponent_SetColor(ulong id, ref Vector4 color);




		[MethodImpl(MethodImplOptions.InternalCall)]
		internal extern static void Rigidbody2DComponent_ApplyLinearImpulse(ulong id,ref Vector2 impulse, ref Vector2 point,bool wake);
		[MethodImpl(MethodImplOptions.InternalCall)]
		internal extern static void Rigidbody2DComponent_ApplyForce(ulong id, ref Vector2 force, ref Vector2 point, bool wake);

		[MethodImpl(MethodImplOptions.InternalCall)]
		internal extern static bool Input_IsKeyDown(ref KeyCode keyCode);
		[MethodImpl(MethodImplOptions.InternalCall)]
		internal extern static bool Input_IsMouseDown(ref MouseCode mouseCode);
	}
	public static class Debug
	{
		public static void Log<T>(T component) 
		{
			Internals.NativeLog(component.ToString());
		}
	}
}
