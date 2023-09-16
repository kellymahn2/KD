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


}
