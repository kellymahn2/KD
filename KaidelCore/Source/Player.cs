using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;
using KaidelCore;
namespace Sandbox
{
	public class Player : Entity
	{
		public override void OnCreate()
		{
		}
		public override void OnUpdate(float ts)
		{
			Vector3 velocity=new Vector3();
			if (Input.IsKeyDown(KeyCode.A))
				velocity.x = -2.0f;
			if (Input.IsKeyDown(KeyCode.D))
				velocity.x = 2.0f;
			if (Input.IsKeyDown(KeyCode.S))
				velocity.y = -2.0f;
			if (Input.IsKeyDown(KeyCode.W))
				velocity.y = 2.0f;
			Vector3 translation = Position;
			translation = translation + velocity*ts;
			Position = translation;
		}
	}
}
