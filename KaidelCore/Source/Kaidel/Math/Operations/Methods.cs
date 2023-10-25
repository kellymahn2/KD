using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;

namespace KaidelCore
{
	public static class Extensions 
	{
		public static Vector3 RotateAround(this Vector3 v, Vector3 origin, Vector3 rotationAngles)
		{
			Internals.Vector3_RotateAround(ref v, ref origin, ref rotationAngles, out Vector3 res);
			return res;
		}
	}
	public static class Math
	{
		static Vector3 Bezier(List<Vector3> points,float t)
		{
			Internals.Math_Bezier(points,t,out Vector3 res);
			return res;
		}
		static T Max<T>(T a, T b)
		{
			if (typeof(IComparable<T>).IsAssignableFrom(typeof(T)))
			{
				if (((IComparable<T>)a).CompareTo(b) > 0)
					return a;
				return b;
			}
			if(Comparer<T>.Default.Compare(a, b) > 0){
				return a;
			}
			return b;
		}
		static T Min<T>(T a, T b)
		{
			if (typeof(IComparable<T>).IsAssignableFrom(typeof(T)))
			{
				if (((IComparable<T>)a).CompareTo(b) < 0)
					return a;
				return b;
			}
			if (Comparer<T>.Default.Compare(a, b) < 0)
			{
				return a;
			}
			return b;
		}
		static T Clamp<T>(T value, T min,T max)
		{
			value = Max<T>(value, min);
			value = Min<T>(value, max);
			return value;
		}
		static ulong Fact(ulong n)
		{
			return Internals.Math_Fact(n);
		}



	}

}
