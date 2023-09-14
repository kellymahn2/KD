using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;
using KaidelCore;
namespace Sandbox
{
	public class Player1 :Entity{

	}
	public class Player2 :Entity{

	}
	public class Player3 :Entity{

	}
	public class Player4 :Entity{

	}
	public class Player5 :Entity{

	}
	public class Player6 :Entity{

	}
	public class Playe71 :Entity{

	}
	public class Player8 :Entity{

	}
	public class Player9 :Entity{

	}
	public class Player10:Entity{

	}
	public class Player11:Entity{

	}
	public class Player12 :Entity{

	}
	public class Player13 :Entity{

	}
	public class Player : Entity
	{

		public float Speed = 0.5f;
		public float Speed2 = 0.5f;
		public int X = 0;
		public int Y = 5;
		public string Hey = "Hello, How Are You?";

		private SpriteRendererComponent m_SpriteRenderer;
		public override void OnCreate()
		{
			m_SpriteRenderer = GetComponent<SpriteRendererComponent>();
		}
		public override void OnUpdate(float ts)
		{


			Vector3 velocity = new Vector3();
			if (Input.IsKeyDown(KeyCode.A))
				velocity.x = -4.0f;
			if (Input.IsKeyDown(KeyCode.D))
				velocity.x = 4.0f;
			if (Input.IsKeyDown(KeyCode.S))
				velocity.y = -4.0f;
			if (Input.IsKeyDown(KeyCode.W))
			{
				velocity.y = 4.0f;
				//Debug.Log("W Pressed");
			}
			if (Input.IsKeyDown(KeyCode.R))
			{
				m_SpriteRenderer.Color = new Vector4(1.0f,0.0f,0.0f,1.0f);
			}
			else if (Input.IsKeyDown(KeyCode.G))
			{
				m_SpriteRenderer.Color = new Vector4(0.0f, 1.0f, 0.0f, 1.0f);
			}
			else if (Input.IsKeyDown(KeyCode.B))
			{
				m_SpriteRenderer.Color = new Vector4(0.0f, 0.0f, 1.0f, 1.0f);
			}
			Vector3 translation = Position;
			translation = translation + velocity*Speed* ts;
			Position = translation;
		}
	}
}
