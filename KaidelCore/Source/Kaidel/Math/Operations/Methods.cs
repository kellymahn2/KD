using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;

namespace KaidelCore
{
	public static class Extensions{
		public static Vector3 RotateAround(this Vector3 v, Vector3 origin, Vector3 rotationAngles)
		{
			Internals.Vector3_RotateAround(ref v, ref origin, ref rotationAngles,out Vector3 res);
			return res;
		}
	}


}
