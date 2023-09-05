using System;
using System.Runtime.CompilerServices;
using System.Runtime.ConstrainedExecution;
namespace KaidelCore
{

	
	public class Entity
	{
		internal readonly ulong ID;
		internal Entity(ulong id)
		{
			ID = id;
		}
		protected Entity() 
		{ 
			ID = 0;
		}
		public Vector3 Position { 
			get
			{
				Internals.Entity_GetPosition(ID,out Vector3 pos);
				return pos;
			}
			set
			{
				Internals.Entity_SetPosition(ID,ref value);
			}
		}

		public virtual void OnCreate() { }
		public virtual void OnUpdate(float ts) { }

	

	}
}
