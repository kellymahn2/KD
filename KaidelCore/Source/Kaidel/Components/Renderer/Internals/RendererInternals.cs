using System.Runtime.CompilerServices;


namespace KaidelCore
{
	internal static partial class Internals
	{

		[MethodImpl(MethodImplOptions.InternalCall)]
		internal extern static void SpriteRendererComponent_GetColor(ulong id, out Vector4 color);
		[MethodImpl(MethodImplOptions.InternalCall)]
		internal extern static void SpriteRendererComponent_SetColor(ulong id, ref Vector4 color);

		[MethodImpl(MethodImplOptions.InternalCall)]
		internal extern static void CircleRendererComponent_GetColor(ulong id, out Vector4 res);
		[MethodImpl(MethodImplOptions.InternalCall)]
		internal extern static void CircleRendererComponent_SetColor(ulong id, ref Vector4 color);
		[MethodImpl(MethodImplOptions.InternalCall)]
		internal extern static float CircleRendererComponent_GetThickness(ulong id);
		[MethodImpl(MethodImplOptions.InternalCall)]
		internal extern static void CircleRendererComponent_SetThickness(ulong id,float value);
		[MethodImpl(MethodImplOptions.InternalCall)]
		internal extern static float CircleRendererComponent_GetFade(ulong id);
		[MethodImpl(MethodImplOptions.InternalCall)]
		internal extern static void CircleRendererComponent_SetFade(ulong id, float value);

		[MethodImpl(MethodImplOptions.InternalCall)]
		internal extern static void LineRendererComponent_GetColor(ulong id,out Vector4 color);
		[MethodImpl(MethodImplOptions.InternalCall)]
		internal extern static void LineRendererComponent_SetColor(ulong id,ref Vector4 color);
		[MethodImpl(MethodImplOptions.InternalCall)]
		internal extern static ulong LineRendererComponent_GetTesselation(ulong id);
		[MethodImpl(MethodImplOptions.InternalCall)]
		internal extern static void LineRendererComponent_SetTesselation(ulong id, ulong tesselation);
		[MethodImpl(MethodImplOptions.InternalCall)]
		internal extern static void LineRendererComponent_GetPoint(ulong id,ulong index,out LineRendererComponent.Point point);
		[MethodImpl(MethodImplOptions.InternalCall)]
		internal extern static void LineRendererComponent_SetPoint(ulong id, ulong index,ref LineRendererComponent.Point point);
		[MethodImpl(MethodImplOptions.InternalCall)]
		internal extern static ulong LineRendererComponent_GetPointCount(ulong id);


	}
}
