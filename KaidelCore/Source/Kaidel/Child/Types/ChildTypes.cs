using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;

namespace KaidelCore
{
	public class ChildNode{
		internal ulong m_ID;
		public Entity Get()
		{

			return new Entity(m_ID);
		}
		//TODO: Add bool so that we know if entity doesnt have the child.
		public ChildNode this[uint index]
		{
			get
			{
				ulong childEntityID = Internals.Entity_GetChildEntityIDWithIndex(m_ID, index);
				return new ChildNode(){m_ID = childEntityID};
			}
		}
		public ChildNode this[string name]
		{
			get
			{
				ulong childEntityID = Internals.Entity_GetChildEntityIDWithName(m_ID, name);
				return new ChildNode() { m_ID = childEntityID };
			}
		}
	}
}
