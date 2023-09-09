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
		static bool x = true;
		private SpriteRendererComponent m_SpriteRenderer;
		private Rigidbody2DComponent m_RigidBody;
		public override void OnCreate()
		{
			m_SpriteRenderer = GetComponent<SpriteRendererComponent>();
			m_RigidBody = GetComponent<Rigidbody2DComponent>();
			if (m_RigidBody == null)
			{
				Debug.Log("Was Null");
			}
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
				velocity.y = 9.80f;
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
			if (Input.IsKeyDown(KeyCode.Space))
			{
				m_RigidBody.ApplyForce(new Vector2(0.0f, 80.0f),new  Vector2(0.0f, 0.0f),true);
			}
			Vector3 translation = Position;
			translation = translation + velocity * ts;
			Position = translation;
		}
	}
}
