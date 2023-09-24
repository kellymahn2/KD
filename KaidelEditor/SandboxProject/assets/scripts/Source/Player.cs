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
		public float Speed;
		

		public override void OnCreate()
		{
            Debug.Log("playerEntity");
			
		}

		public override void OnUpdate(float ts)
		{
			
			float speed = Speed;
			Vector3 velocity = new Vector3(0,0,0);
			if (Input.IsKeyDown(KeyCode.W))
				velocity.y = 1.0f;
			else if (Input.IsKeyDown(KeyCode.S))
				velocity.y = -1.0f;

			if (Input.IsKeyDown(KeyCode.A))
				velocity.x = -1.0f;
			else if (Input.IsKeyDown(KeyCode.D))
				velocity.x = 1.0f;

			Entity cameraEntity = FindEntityByName("Camera");

			if (cameraEntity != null)
			{
				Debug.Log("Not Null");
				Camera camera = cameraEntity.As<Camera>();

				if (Input.IsKeyDown(KeyCode.Q))
					camera.DistanceFromPlayer += speed * 2.0f * ts;
				else if (Input.IsKeyDown(KeyCode.E))
					camera.DistanceFromPlayer -= speed * 2.0f * ts;
			}
			else 
				Debug.Log("Null");

			velocity *= speed * ts;

			// m_Rigidbody.ApplyLinearImpulse(velocity.XY, true);

			Position += velocity;
		}

	}
}