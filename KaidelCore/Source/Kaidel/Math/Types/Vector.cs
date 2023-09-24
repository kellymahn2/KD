using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;

namespace KaidelCore
{
	#region Vector2
	public struct Vector2
	{
		public override int GetHashCode()
		{
			int hash = 17;
			hash = hash * 23 + m_X.GetHashCode();
			hash = hash * 23 + m_Y.GetHashCode();
			return hash;
		}
		public override bool Equals(object obj)
		{
			return obj is Vector2 vector &&
				   x == vector.x &&
				   y == vector.y;
		}
		private float m_X, m_Y;
		private Vector2(ref Vector2 v)
		{
			this = v;
		}
		public Vector2(float x, float y)
		{
			m_X= x;
			m_Y = y;
		}
		public float x
		{
			get
			{
				return m_X;
			}
			set
			{
				m_X = value;
			}
		}
		public float y
		{
			get
			{
				return m_Y;
			}
			set
			{
				m_Y = value;
			}
		}
		public float Dot(Vector2 v)
		{
			return Internals.Vector2_DotVec(ref this, ref v);
		}
		public static float Dot(Vector2 v1, Vector2 v2)
		{
			return Internals.Vector2_DotVec(ref v1, ref v2);
		}

		public static Vector2 operator +(Vector2 a, Vector2 b)
		{
			Internals.Vector2_AddVec(ref a, ref b, out Vector2 res);
			return new Vector2(ref res);
		}
		public static Vector2 operator -(Vector2 a, Vector2 b)
		{
			Internals.Vector2_SubtractVec(ref a, ref b, out Vector2 res);
			return new Vector2(ref res);
		}
		public static Vector2 operator +(Vector2 a, float b)
		{
			Internals.Vector2_AddNum(ref a, b, out Vector2 res);
			return new Vector2(ref res);
		}
		public static Vector2 operator -(float a, Vector2 b)
		{
			Internals.Vector2_SubtractNum(ref b, a, out Vector2 res);
			return new Vector2(ref res);
		}
		public static Vector2 operator *(Vector2 a, float b)
		{
			Internals.Vector2_MultNum(ref a, b, out Vector2 res);
			return new Vector2(ref res);
		}
		public static Vector2 operator *(float a, Vector2 b)
		{
			Internals.Vector2_MultNum(ref b, a, out Vector2 res);
			return new Vector2(ref res);
		}
		public static Vector2 operator *(Vector2 a, Vector2 b)
		{
			Internals.Vector2_MultVec(ref a, ref b, out Vector2 res);
			return new Vector2(ref res);
		}
		public static Vector2 operator /(Vector2 a, float b)
		{
			Internals.Vector2_DivNum(ref a, b, out Vector2 res);
			return new Vector2(ref res);
		}
		public static Vector2 operator /(float a, Vector2 b)
		{
			Internals.Vector2_DivNum(ref b, a, out Vector2 res);
			return new Vector2(ref res);
		}
		public static Vector2 operator /(Vector2 a, Vector2 b)
		{
			Internals.Vector2_DivVec(ref a, ref b, out Vector2 res);
			return new Vector2(ref res);
		}


		public static bool operator ==(Vector2 a, Vector2 b)
		{
			return a.x == b.x && a.y == b.y;
		}
		public static bool operator !=(Vector2 a, Vector2 b)
		{
			return !(a == b);
		}

	}
	#endregion
	#region Vector3
	public struct Vector3
	{
		public override int GetHashCode()
		{
			int hash = 17;
			hash = hash * 23 + m_X.GetHashCode();
			hash = hash * 23 + m_Y.GetHashCode();
			hash = hash * 23 + m_Z.GetHashCode();
			return hash;
		}
		public override bool Equals(object obj)
		{
			return obj is Vector3 vector &&
				   x == vector.x &&
				   y == vector.y &&
				   z == vector.z;
		}
		private float m_X, m_Y, m_Z;
		private Vector3(ref Vector3 v)
		{
			this = v;
		}
		public Vector3(float x, float y, float z)
		{
			m_X = x;
			m_Y = y;
			m_Z = z;
		}
		
	
		public float x
		{
			get
			{
				return m_X;
			}
			set
			{
				m_X = value;
			}
		}
		public float y
		{
			get
			{
				return m_Y;
			}
			set
			{
				m_Y = value;
			}
		}
		public float z
		{
			get
			{
				return m_Z;
			}
			set
			{
				m_Z = value;
			}
		}

		public float Dot(Vector3 v)
		{
			return Internals.Vector3_DotVec(ref this, ref v);
		}
		public static float Dot(Vector3 v1, Vector3 v2)
		{
			return Internals.Vector3_DotVec(ref v1, ref v2);
		}
		public Vector3 Cross(Vector3 v)
		{
			 Internals.Vector3_CrossVec(ref this, ref v, out Vector3 res);
			return new Vector3(ref res);
		}
		public static Vector3 Cross(Vector3 v1, Vector3 v2)
		{
			Internals.Vector3_CrossVec(ref v1, ref v2,out Vector3 res);
			return new Vector3(ref res);
		}







		public static Vector3 operator +(Vector3 a, Vector3 b)
		{
			Internals.Vector3_AddVec(ref a, ref b, out Vector3 res);
			return new Vector3(ref res);
		}
		public static Vector3 operator -(Vector3 a, Vector3 b)
		{
			Internals.Vector3_SubtractVec(ref a, ref b, out Vector3 res);
			return new Vector3(ref res);
		}
		public static Vector3 operator +(Vector3 a, float b)
		{
			Internals.Vector3_AddNum(ref a, b, out Vector3 res);
			return new Vector3(ref res);
		}
		public static Vector3 operator -(float a, Vector3 b)
		{
			Internals.Vector3_SubtractNum(ref b, a, out Vector3 res);
			return new Vector3(ref res);
		}
		public static Vector3 operator *(Vector3 a, float b)
		{
			Internals.Vector3_MultNum(ref a, b, out Vector3 res);
			return new Vector3(ref res);
		}
		public static Vector3 operator *(float a, Vector3 b)
		{
			Internals.Vector3_MultNum(ref b, a, out Vector3 res);
			return new Vector3(ref res);
		}
		public static Vector3 operator *(Vector3 a, Vector3 b)
		{
			Internals.Vector3_MultVec(ref a, ref b, out Vector3 res);
			return new Vector3(ref res);
		}
		public static Vector3 operator /(Vector3 a, float b)
		{
			Internals.Vector3_DivNum(ref a, b, out Vector3 res);
			return new Vector3(ref res);
		}
		public static Vector3 operator /(float a, Vector3 b)
		{
			Internals.Vector3_DivNum(ref b, a, out Vector3 res);
			return new Vector3(ref res);
		}
		public static Vector3 operator /(Vector3 a, Vector3 b)
		{
			Internals.Vector3_DivVec(ref a, ref b, out Vector3 res);
			return new Vector3(ref res);
		}


		public static bool operator==(Vector3 a,  Vector3 b)
		{
			return a.x == b.x && a.y == b.y && a.z == b.z;
		}
		public static bool operator!=(Vector3 a, Vector3 b)
		{
			return !(a == b);
		}

	}
	#endregion
	#region Vector4
	public struct Vector4
	{
		public override int GetHashCode()
		{
			int hash = 17;
			hash = hash * 23 + m_X.GetHashCode();
			hash = hash * 23 + m_Y.GetHashCode();
			hash = hash * 23 + m_Z.GetHashCode();
			hash = hash * 23 + m_W.GetHashCode();
			return hash;
		}
		public override bool Equals(object obj)
		{
			return obj is Vector4 vector &&
				   x == vector.x &&
				   y == vector.y &&
				   z == vector.z&&
				   w == vector.w;
		}
		private float m_X, m_Y, m_Z, m_W;
		private Vector4(ref Vector4 v)
		{
			this = v;
		}
		public Vector4(float x, float y, float z,float w)
		{
			m_X = x;
			m_Y = y;
			m_Z = z;
			m_W = w;
		}
		public float x
		{
			get
			{
				return m_X;
			}
			set
			{
				m_X = value;
			}
		}
		public float y
		{
			get
			{
				return m_Y;
			}
			set
			{
				m_Y = value;
			}
		}
		public float z
		{
			get
			{
				return m_Z;
			}
			set
			{
				m_Z = value;
			}
		}
		public float w
		{
			get
			{
				return m_W;
			}
			set
			{
				m_W = value;
			}
		}

		public float Dot(Vector4 v)
		{
			return Internals.Vector4_DotVec(ref this, ref v);
		}
		public static float Dot(Vector4 v1, Vector4 v2)
		{
			return Internals.Vector4_DotVec(ref v1, ref v2);
		}







		public static Vector4 operator +(Vector4 a, Vector4 b)
		{
			Internals.Vector4_AddVec(ref a, ref b, out Vector4 res);
			return new Vector4(ref res);
		}
		public static Vector4 operator -(Vector4 a, Vector4 b)
		{
			Internals.Vector4_SubtractVec(ref a, ref b, out Vector4 res);
			return new Vector4(ref res);
		}
		public static Vector4 operator +(Vector4 a, float b)
		{
			Internals.Vector4_AddNum(ref a, b, out Vector4 res);
			return new Vector4(ref res);
		}
		public static Vector4 operator -(float a, Vector4 b)
		{
			Internals.Vector4_SubtractNum(ref b, a, out Vector4 res);
			return new Vector4(ref res);
		}
		public static Vector4 operator *(Vector4 a, float b)
		{
			Internals.Vector4_MultNum(ref a, b, out Vector4 res);
			return new Vector4(ref res);
		}
		public static Vector4 operator *(float a, Vector4 b)
		{
			Internals.Vector4_MultNum(ref b, a, out Vector4 res);
			return new Vector4(ref res);
		}
		public static Vector4 operator *(Vector4 a, Vector4 b)
		{
			Internals.Vector4_MultVec(ref a, ref b, out Vector4 res);
			return new Vector4(ref res);
		}
		public static Vector4 operator /(Vector4 a, float b)
		{
			Internals.Vector4_DivNum(ref a, b, out Vector4 res);
			return new Vector4(ref res);
		}
		public static Vector4 operator /(float a, Vector4 b)
		{
			Internals.Vector4_DivNum(ref b, a, out Vector4 res);
			return new Vector4(ref res);
		}
		public static Vector4 operator /(Vector4 a, Vector4 b)
		{
			Internals.Vector4_DivVec(ref a, ref b, out Vector4 res);
			return new Vector4(ref res);
		}


		public static bool operator ==(Vector4 a, Vector4 b)
		{
			return a.x == b.x && a.y == b.y && a.z == b.z&&a.w==b.w;
		}
		public static bool operator !=(Vector4 a, Vector4 b)
		{
			return !(a == b);
		}

	}
	#endregion
}
