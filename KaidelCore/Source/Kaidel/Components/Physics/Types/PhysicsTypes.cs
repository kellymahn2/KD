using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;

namespace KaidelCore
{
	public class Rigidbody2DComponent : Component
	{
		public void ApplyLinearImpule(Vector2 impulse, Vector2 center, bool wake = true)
		{
			Internals.Rigidbody2DComponent_ApplyLinearImpulse(Entity.ID, ref impulse, ref center, wake);
		}
		public void ApplyForce(Vector2 force, Vector2 center, bool wake = true)
		{
			Internals.Rigidbody2DComponent_ApplyForce(Entity.ID, ref force, ref center, wake);
		}
		public enum BodyType
		{
			Static = 0, Dynamic, Kinematic
		}
		public BodyType Type
		{
			get
			{
				return (BodyType)Internals.Rigidbody2DComponent_GetBodyType(Entity.ID);
			}
		}
		public bool FixedRotation
		{
			get
			{
				return Internals.Rigidbody2DComponent_GetFixedRotation(Entity.ID);
			}
			set
			{
				Internals.Rigidbody2DComponent_SetFixedRotation(Entity.ID, value);
			}
		}
	}

	public class BoxCollider2DComponent : Component
	{
		public Vector2 Offset
		{
			get
			{
				Internals.BoxCollider2DComponent_GetOffset(Entity.ID,out Vector2 res);
				return res;
			}
			set
			{
				Internals.BoxCollider2DComponent_SetOffset(Entity.ID, ref value);
			}
		}
		public Vector2 Size
		{
			get
			{
				Internals.BoxCollider2DComponent_GetSize(Entity.ID,out Vector2 res);
				return res;
			}
			set
			{
				Internals.BoxCollider2DComponent_SetSize(Entity.ID, ref value);
			}
		}
		public float Density
		{
			get
			{
				return Internals.BoxCollider2DComponent_GetDensity(Entity.ID);
			}
			set
			{
				Internals.BoxCollider2DComponent_SetDensity(Entity.ID, value);
			}
		}
		public float Friction
		{
			get
			{
				return Internals.BoxCollider2DComponent_GetFriction(Entity.ID);
			}
			set
			{
				Internals.BoxCollider2DComponent_SetFriction(Entity.ID, value);
			}
		}
		public float Restitution
		{
			get
			{
				return Internals.BoxCollider2DComponent_GetRestitution(Entity.ID);
			}
			set
			{
				Internals.BoxCollider2DComponent_SetRestitution(Entity.ID,value);
			}
		}
		public float RestitutionThreshold
		{
			get
			{
				return Internals.BoxCollider2DComponent_GetRestitutionThreshold(Entity.ID);
			}
			set
			{
				Internals.BoxCollider2DComponent_SetRestitutionThreshold(Entity.ID,value);
			}
		}
	}

	public class CircleCollider2DComponent : Component
	{
		public Vector2 Offset
		{
			get
			{
				Internals.CircleCollider2DComponent_GetOffset(Entity.ID, out Vector2 res);
				return res;
			}
			set
			{
				Internals.CircleCollider2DComponent_SetOffset(Entity.ID, ref value);
			}
		}
		public float Radius 
		{
			get
			{
				return Internals.CircleCollider2DComponent_GetRadius(Entity.ID);
			}
			set
			{
				Internals.CircleCollider2DComponent_SetRadius(Entity.ID,value);
			}
		}
		public float Density 
		{
			get
			{
				return Internals.CircleCollider2DComponent_GetDensity(Entity.ID);
			}
			set
			{
				Internals.CircleCollider2DComponent_SetDensity(Entity.ID,value);
			}
		}
		public float Friction 
		{
			get
			{
				return Internals.CircleCollider2DComponent_GetFriction(Entity.ID);
			}
			set
			{
				Internals.CircleCollider2DComponent_SetFriction(Entity.ID, value);
			}
		}
		public float Restitution 
		{
			get
			{
				return Internals.CircleCollider2DComponent_GetRestitution(Entity.ID);
			}
			set
			{
				Internals.CircleCollider2DComponent_SetRestitution(Entity.ID,value);
			}
		}
		public float RestitutionThreshold
		{
			get
			{
				return Internals.CircleCollider2DComponent_GetRestitutionThreshold(Entity.ID);
			}
			set
			{
				Internals.CircleCollider2DComponent_SetRestitutionThreshold(Entity.ID,value);
			}
		}
	}
}
