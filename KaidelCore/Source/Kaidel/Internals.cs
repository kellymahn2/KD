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
		internal extern static void Entity_AddComponent(ulong id, Type componentType);

		[MethodImpl(MethodImplOptions.InternalCall)]
		internal extern static ulong Entity_FindEntityByName(string name);

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
		internal extern static bool Input_IsKeyDown(ref KeyCode keyCode);
		[MethodImpl(MethodImplOptions.InternalCall)]
		internal extern static bool Input_IsMouseDown(ref MouseCode mouseCode);

		[MethodImpl(MethodImplOptions.InternalCall)]
		internal extern static object Instance_GetScriptInstance(ulong id);
	}
	public static class Debug
	{
		public static void Log<T>(T component) 
		{
			Internals.NativeLog(component.ToString());
		}
	}
}
