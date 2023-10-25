using System;
using System.Collections.Generic;
using System.Linq;
using System.Runtime.InteropServices.WindowsRuntime;
using System.Text;
using System.Threading.Tasks;

namespace KaidelCore
{
	public class SpriteRendererComponent : Component
	{
		public Vector4 Color
		{
			get
			{
				Internals.SpriteRendererComponent_GetColor(Entity.ID, out Vector4 result);
				return result;
			}
			set
			{
				Internals.SpriteRendererComponent_SetColor(Entity.ID, ref value);
			}
		}
		public override string ToString()
		{
			Vector4 color = Color;
			return $"SpriteRendererComponent: Color{{{color.x},{color.y},{color.z},{color.w}}}";
		}
	}

	public class CircleRendererComponent : Component
	{
		public Vector4 Color
		{
			get
			{
				Internals.CircleRendererComponent_GetColor(Entity.ID, out Vector4 res);
				return res;
			}
			set
			{
				Internals.CircleRendererComponent_SetColor(Entity.ID, ref value);
			}
		}
		public float Thickness
		{
			get
			{
				return Internals.CircleRendererComponent_GetThickness(Entity.ID);
			}
			set
			{
				Internals.CircleRendererComponent_SetThickness(Entity.ID, value);
			}
		}
		public float Fade
		{
			get
			{
				return Internals.CircleRendererComponent_GetFade(Entity.ID);
			}
			set
			{
				Internals.CircleRendererComponent_SetFade(Entity.ID, value);
			}
		}

	}
	public class LineRendererComponent : Component
	{

		public struct Point
		{
			Vector3 Position;
		}
		public Vector4 Color
		{
			get
			{
				Internals.LineRendererComponent_GetColor(Entity.ID,out Vector4 res);
				return res;
			}
			set
			{
				Internals.LineRendererComponent_SetColor(Entity.ID, ref value);
			}
		}
		public Point this[ulong index]
		{
			get
			{
				Internals.LineRendererComponent_GetPoint(Entity.ID,index,out  Point res);
				return res;
			}
			set
			{
				Internals.LineRendererComponent_SetPoint(Entity.ID, index,ref value);
			}
		}
		public ulong Tesselation
		{
			get
			{
				return Internals.LineRendererComponent_GetTesselation(Entity.ID);
			}
			set
			{
				Internals.LineRendererComponent_SetTesselation(Entity.ID,value);
			}
		}
		public ulong Count()
		{
			return Internals.LineRendererComponent_GetPointCount(Entity.ID);
		}





	}



}
