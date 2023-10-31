using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;

using KaidelCore;

namespace Sandbox
{
	public class Player2 : Entity
	{
		public float Speed;
		public float X;
		public override void OnCreate()
		{
		}

		public override void OnUpdate(float ts)
		{
            if(Input.IsKeyDown(KeyCode.W)){
                Debug.Log("W Pressed! Speed was {0}",Speed);
            }
        }

	}
}