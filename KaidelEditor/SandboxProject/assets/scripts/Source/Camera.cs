using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;

using KaidelCore;

namespace Sandbox
{
	public class Camera : Entity
	{

		public float DistanceFromPlayer = 5.0f;

		private Entity m_Player;

		public override void OnCreate()
		{
			m_Player = FindEntityByName("Player");
            Debug.Log("cameraEntity");
		}

		public  override void OnUpdate(float ts)
		{

			if (m_Player != null)
				Position = new Vector3(m_Player.Position.x,m_Player.Position.y, DistanceFromPlayer);
		}

	}
}