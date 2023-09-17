using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;

namespace KaidelCore
{
	public class ChildNode{
		internal ulong m_ID;

		public Entity this[uint index]
		{
			get
			{
				ulong childEntityID = Internals.Entity_GetChildEntityIDWithIndex(m_ID,index);
				return new Entity(childEntityID);
			}
		}
		public Entity this[string name]
		{
			get
			{
				ulong childEntityID = Internals.Entity_GetChildEntityIDWithName(m_ID, name);
				return new Entity(childEntityID);
			}
		}
	}
}
