using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;

namespace KaidelCore
{
	#region Vector2
	internal struct Vector2_Impl
	{
		internal float m_X, m_Y;
		internal Vector2_Impl(float x, float y)
		{
			this.m_X = x;
			this.m_Y = y;
		}
	}
	public struct Vector2
	{
		private void RecalcLength()
		{
			m_Magnitude = Internals.Vector2_LengthVec(ref m_Vector);
		}

		public override bool Equals(object obj)
		{
			return obj is Vector2 vector &&
				   x == vector.x &&
				   y == vector.y;
		}

		private Vector2_Impl m_Vector;
		private Vector2(ref Vector2_Impl v)
		{
			m_Vector = v;
			m_Magnitude = 0.0f;
			RecalcLength();
		}
		public Vector2(float x, float y)
		{
			m_Vector = new Vector2_Impl(x, y);
			m_Magnitude = 0.0f;
			RecalcLength();
		}

		private float m_Magnitude;

		public float Magnitude
		{
			get
			{
				return this.m_Magnitude;
			}

		}
		public float x
		{
			get
			{
				return m_Vector.m_X;
			}
			set
			{
				m_Vector.m_X = value;
				RecalcLength();
			}
		}
		public float y
		{
			get
			{
				return m_Vector.m_Y;
			}
			set
			{
				m_Vector.m_Y = value;
				RecalcLength();
			}
		}
		public float Dot(Vector2 v)
		{
			return Internals.Vector2_DotVec(ref m_Vector, ref v.m_Vector);
		}
		public static float Dot(Vector2 v1, Vector2 v2)
		{
			return Internals.Vector2_DotVec(ref v1.m_Vector, ref v2.m_Vector);
		}

		public static Vector2 operator +(Vector2 a, Vector2 b)
		{
			Internals.Vector2_AddVec(ref a.m_Vector, ref b.m_Vector, out Vector2_Impl res);
			return new Vector2(ref res);
		}
		public static Vector2 operator -(Vector2 a, Vector2 b)
		{
			Internals.Vector2_SubtractVec(ref a.m_Vector, ref b.m_Vector, out Vector2_Impl res);
			return new Vector2(ref res);
		}
		public static Vector2 operator +(Vector2 a, float b)
		{
			Internals.Vector2_AddNum(ref a.m_Vector, b, out Vector2_Impl res);
			return new Vector2(ref res);
		}
		public static Vector2 operator -(float a, Vector2 b)
		{
			Internals.Vector2_SubtractNum(ref b.m_Vector, a, out Vector2_Impl res);
			return new Vector2(ref res);
		}
		public static Vector2 operator *(Vector2 a, float b)
		{
			Internals.Vector2_MultNum(ref a.m_Vector, b, out Vector2_Impl res);
			return new Vector2(ref res);
		}
		public static Vector2 operator *(float a, Vector2 b)
		{
			Internals.Vector2_MultNum(ref b.m_Vector, a, out Vector2_Impl res);
			return new Vector2(ref res);
		}
		public static Vector2 operator *(Vector2 a, Vector2 b)
		{
			Internals.Vector2_MultVec(ref a.m_Vector, ref b.m_Vector, out Vector2_Impl res);
			return new Vector2(ref res);
		}
		public static Vector2 operator /(Vector2 a, float b)
		{
			Internals.Vector2_DivNum(ref a.m_Vector, b, out Vector2_Impl res);
			return new Vector2(ref res);
		}
		public static Vector2 operator /(float a, Vector2 b)
		{
			Internals.Vector2_DivNum(ref b.m_Vector, a, out Vector2_Impl res);
			return new Vector2(ref res);
		}
		public static Vector2 operator /(Vector2 a, Vector2 b)
		{
			Internals.Vector2_DivVec(ref a.m_Vector, ref b.m_Vector, out Vector2_Impl res);
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
	internal struct Vector3_Impl
	{
		internal float m_X, m_Y, m_Z;
		internal Vector3_Impl(float x, float y, float z)
		{
			this.m_X = x;
			this.m_Y = y;
			this.m_Z = z;
		}
	}
	public struct Vector3
	{
		private void RecalcLength()
		{
			m_Magnitude = Internals.Vector3_LengthVec(ref m_Vector);
		}

		public override bool Equals(object obj)
		{
			return obj is Vector3 vector &&
				   x == vector.x &&
				   y == vector.y &&
				   z == vector.z;
		}

		private Vector3_Impl m_Vector;
		private Vector3(ref Vector3_Impl v)
		{
			m_Vector = v;
			m_Magnitude = 0.0f;
			RecalcLength();
		}
		public Vector3(float x, float y, float z)
		{
			m_Vector = new Vector3_Impl(x, y, z);
			m_Magnitude = 0.0f;
			RecalcLength();
		}
		
		private float m_Magnitude;

		public float Magnitude
		{
			get
			{
				return this.m_Magnitude;
			}

		}
		public float x
		{
			get
			{
				return m_Vector.m_X;
			}
			set
			{
				m_Vector.m_X = value;
				RecalcLength();
			}
		}
		public float y
		{
			get
			{
				return m_Vector.m_Y;
			}
			set
			{
				m_Vector.m_Y = value;
				RecalcLength();
			}
		}
		public float z
		{
			get
			{
				return m_Vector.m_Z;
			}
			set
			{
				m_Vector.m_Z = value;
				RecalcLength();
			}
		}

		public float Dot(Vector3 v)
		{
			return Internals.Vector3_DotVec(ref m_Vector, ref v.m_Vector);
		}
		public static float Dot(Vector3 v1, Vector3 v2)
		{
			return Internals.Vector3_DotVec(ref v1.m_Vector, ref v2.m_Vector);
		}
		public Vector3 Cross(Vector3 v)
		{
			 Internals.Vector3_CrossVec(ref m_Vector, ref v.m_Vector, out Vector3_Impl res);
			return new Vector3(ref res);
		}
		public static Vector3 Cross(Vector3 v1, Vector3 v2)
		{
			Internals.Vector3_CrossVec(ref v1.m_Vector, ref v2.m_Vector,out Vector3_Impl res);
			return new Vector3(ref res);
		}







		public static Vector3 operator +(Vector3 a, Vector3 b)
		{
			Internals.Vector3_AddVec(ref a.m_Vector, ref b.m_Vector, out Vector3_Impl res);
			return new Vector3(ref res);
		}
		public static Vector3 operator -(Vector3 a, Vector3 b)
		{
			Internals.Vector3_SubtractVec(ref a.m_Vector, ref b.m_Vector, out Vector3_Impl res);
			return new Vector3(ref res);
		}
		public static Vector3 operator +(Vector3 a, float b)
		{
			Internals.Vector3_AddNum(ref a.m_Vector, b, out Vector3_Impl res);
			return new Vector3(ref res);
		}
		public static Vector3 operator -(float a, Vector3 b)
		{
			Internals.Vector3_SubtractNum(ref b.m_Vector, a, out Vector3_Impl res);
			return new Vector3(ref res);
		}
		public static Vector3 operator *(Vector3 a, float b)
		{
			Internals.Vector3_MultNum(ref a.m_Vector, b, out Vector3_Impl res);
			return new Vector3(ref res);
		}
		public static Vector3 operator *(float a, Vector3 b)
		{
			Internals.Vector3_MultNum(ref b.m_Vector, a, out Vector3_Impl res);
			return new Vector3(ref res);
		}
		public static Vector3 operator *(Vector3 a, Vector3 b)
		{
			Internals.Vector3_MultVec(ref a.m_Vector, ref b.m_Vector, out Vector3_Impl res);
			return new Vector3(ref res);
		}
		public static Vector3 operator /(Vector3 a, float b)
		{
			Internals.Vector3_DivNum(ref a.m_Vector, b, out Vector3_Impl res);
			return new Vector3(ref res);
		}
		public static Vector3 operator /(float a, Vector3 b)
		{
			Internals.Vector3_DivNum(ref b.m_Vector, a, out Vector3_Impl res);
			return new Vector3(ref res);
		}
		public static Vector3 operator /(Vector3 a, Vector3 b)
		{
			Internals.Vector3_DivVec(ref a.m_Vector, ref b.m_Vector, out Vector3_Impl res);
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
	internal struct Vector4_Impl
	{
		internal float m_X, m_Y, m_Z,m_W;
		internal Vector4_Impl(float x, float y, float z,float w)
		{
			this.m_X = x;
			this.m_Y = y;
			this.m_Z = z;
			this.m_W = w;
		}
	}
	public struct Vector4
	{
		private void RecalcLength()
		{
			m_Magnitude = Internals.Vector4_LengthVec(ref m_Vector);
		}

		public override bool Equals(object obj)
		{
			return obj is Vector4 vector &&
				   x == vector.x &&
				   y == vector.y &&
				   z == vector.z&&
				   w == vector.w;
		}

		private Vector4_Impl m_Vector;
		private Vector4(ref Vector4_Impl v)
		{
			m_Vector = v;
			m_Magnitude = 0.0f;
			RecalcLength();
		}
		public Vector4(float x, float y, float z,float w)
		{
			m_Vector = new Vector4_Impl(x, y, z,w);
			m_Magnitude = 0.0f;
			RecalcLength();
		}

		private float m_Magnitude;

		public float Magnitude
		{
			get
			{
				return this.m_Magnitude;
			}

		}
		public float x
		{
			get
			{
				return m_Vector.m_X;
			}
			set
			{
				m_Vector.m_X = value;
				RecalcLength();
			}
		}
		public float y
		{
			get
			{
				return m_Vector.m_Y;
			}
			set
			{
				m_Vector.m_Y = value;
				RecalcLength();
			}
		}
		public float z
		{
			get
			{
				return m_Vector.m_Z;
			}
			set
			{
				m_Vector.m_Z = value;
				RecalcLength();
			}
		}
		public float w
		{
			get
			{
				return m_Vector.m_W;
			}
			set
			{
				m_Vector.m_W = value;
				RecalcLength();
			}
		}

		public float Dot(Vector4 v)
		{
			return Internals.Vector4_DotVec(ref m_Vector, ref v.m_Vector);
		}
		public static float Dot(Vector4 v1, Vector4 v2)
		{
			return Internals.Vector4_DotVec(ref v1.m_Vector, ref v2.m_Vector);
		}







		public static Vector4 operator +(Vector4 a, Vector4 b)
		{
			Internals.Vector4_AddVec(ref a.m_Vector, ref b.m_Vector, out Vector4_Impl res);
			return new Vector4(ref res);
		}
		public static Vector4 operator -(Vector4 a, Vector4 b)
		{
			Internals.Vector4_SubtractVec(ref a.m_Vector, ref b.m_Vector, out Vector4_Impl res);
			return new Vector4(ref res);
		}
		public static Vector4 operator +(Vector4 a, float b)
		{
			Internals.Vector4_AddNum(ref a.m_Vector, b, out Vector4_Impl res);
			return new Vector4(ref res);
		}
		public static Vector4 operator -(float a, Vector4 b)
		{
			Internals.Vector4_SubtractNum(ref b.m_Vector, a, out Vector4_Impl res);
			return new Vector4(ref res);
		}
		public static Vector4 operator *(Vector4 a, float b)
		{
			Internals.Vector4_MultNum(ref a.m_Vector, b, out Vector4_Impl res);
			return new Vector4(ref res);
		}
		public static Vector4 operator *(float a, Vector4 b)
		{
			Internals.Vector4_MultNum(ref b.m_Vector, a, out Vector4_Impl res);
			return new Vector4(ref res);
		}
		public static Vector4 operator *(Vector4 a, Vector4 b)
		{
			Internals.Vector4_MultVec(ref a.m_Vector, ref b.m_Vector, out Vector4_Impl res);
			return new Vector4(ref res);
		}
		public static Vector4 operator /(Vector4 a, float b)
		{
			Internals.Vector4_DivNum(ref a.m_Vector, b, out Vector4_Impl res);
			return new Vector4(ref res);
		}
		public static Vector4 operator /(float a, Vector4 b)
		{
			Internals.Vector4_DivNum(ref b.m_Vector, a, out Vector4_Impl res);
			return new Vector4(ref res);
		}
		public static Vector4 operator /(Vector4 a, Vector4 b)
		{
			Internals.Vector4_DivVec(ref a.m_Vector, ref b.m_Vector, out Vector4_Impl res);
			return new Vector4(ref res);
		}


		public static bool operator ==(Vector4 a, Vector4 b)
		{
			return a.x == b.x && a.y == b.y && a.z == b.z;
		}
		public static bool operator !=(Vector4 a, Vector4 b)
		{
			return !(a == b);
		}

	}
	#endregion
}
