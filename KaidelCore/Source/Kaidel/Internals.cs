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
		internal extern static void TransformComponent_GetLocalPosition(ulong id, out Vector3 localPosition);
		[MethodImpl(MethodImplOptions.InternalCall)]	  
		internal extern static void TransformComponent_SetLocalPosition(ulong id, ref Vector3 localPosition);
		[MethodImpl(MethodImplOptions.InternalCall)]
		internal extern static void TransformComponent_GetLocalRotation(ulong id, out Vector3 localRotation);
		[MethodImpl(MethodImplOptions.InternalCall)]
		internal extern static void TransformComponent_SetLocalRotation(ulong id, ref Vector3 localRotation);
		[MethodImpl(MethodImplOptions.InternalCall)]
		internal extern static void TransformComponent_RotateAround(ulong id, ulong parentID, ref Vector3 rotation);


		[MethodImpl(MethodImplOptions.InternalCall)]
		internal extern static bool Input_IsKeyDown(ref KeyCode keyCode);
		[MethodImpl(MethodImplOptions.InternalCall)]
		internal extern static bool Input_IsMouseDown(ref MouseCode mouseCode);

		[MethodImpl(MethodImplOptions.InternalCall)]
		internal extern static object Instance_GetScriptInstance(ulong id);
		[MethodImpl(MethodImplOptions.InternalCall)]
		internal extern static ulong Console_InitNewConsole();
		[MethodImpl(MethodImplOptions.InternalCall)]
		internal extern static void Console_DestroyConsole(ulong id);
		[MethodImpl(MethodImplOptions.InternalCall)]
		internal extern static void Console_MessageLog(ulong id, string s);
		[MethodImpl(MethodImplOptions.InternalCall)]
		internal extern static void Console_MessageStaticLog( string s);
		[MethodImpl(MethodImplOptions.InternalCall)]
		internal extern static void Console_MessageInfo(ulong id,  string s);
		[MethodImpl(MethodImplOptions.InternalCall)]
		internal extern static void Console_MessageStaticInfo( string s);
		[MethodImpl(MethodImplOptions.InternalCall)]
		internal extern static void Console_MessageWarn(ulong id,  string s);
		[MethodImpl(MethodImplOptions.InternalCall)]
		internal extern static void Console_MessageStaticWarn( string s);
		[MethodImpl(MethodImplOptions.InternalCall)]
		internal extern static void Console_MessageError(ulong id,  string s);
		[MethodImpl(MethodImplOptions.InternalCall)]
		internal extern static void Console_MessageStaticError( string s);
	}
	public static class Debug
	{
		public static void Log(string fmt,params object[] objects)
		{
			fmt = String.Format(fmt, objects);
			Internals.Console_MessageStaticLog( fmt);
		}
		public static void Info(string fmt, params object[] objects)
		{
			fmt = String.Format(fmt, objects);
			Internals.Console_MessageStaticInfo( fmt);
		}
		public static void Warn(string fmt, params object[] objects)
		{
			fmt = String.Format(fmt, objects);
			Internals.Console_MessageStaticWarn( fmt);
		}
		public static void Error(string fmt, params object[] objects)
		{
			fmt = String.Format(fmt, objects);
			Internals.Console_MessageStaticError( fmt);
		}
	}
}
