using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;

namespace KaidelCore
{
	public abstract class Component
	{
		internal Entity Entity { get; set; }

		

	}

	public class TransformComponent : Component
	{
		public Vector3 Position
		{
			get
			{
				return Entity.Position;
			}
			set
			{
				Entity.Position = value;
			}
		}
		public Vector3 Rotation
		{
			get
			{
				return Entity.Rotation;
			}
			set
			{
				Entity.Rotation = value;
			}
		}
		public Vector3 Scale
		{
			get
			{
				return Entity.Scale;
			}
			set
			{
				Entity.Scale = value;
			}
		}
	}

	public class SpriteRendererComponent : Component
	{
		public Vector4 Color
		{
			get
			{
				Internals.SpriteRendererComponent_GetColor(Entity.ID,out Vector4 result);
				return result;
			}
			set
			{
				Internals.SpriteRendererComponent_SetColor(Entity.ID,ref value);
			}
		}
		public override string ToString()
		{
			Vector4 color = Color;
			return $"SpriteRendererComponent: Color{{{color.x},{color.y},{color.z},{color.w}}}";
		}
	}

	public class Rigidbody2DComponent : Component
	{
		public void ApplyLinearImpule(Vector2 impulse,Vector2 center,bool wake = true)
		{
			Internals.Rigidbody2DComponent_ApplyLinearImpulse(Entity.ID,ref impulse,ref center, wake);
		}
		public void ApplyForce(Vector2 force,Vector2 center,bool wake = true)
		{
			Internals.Rigidbody2DComponent_ApplyForce(Entity.ID, ref force, ref center, wake);
		}
	}
}
