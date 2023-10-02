using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;

namespace KaidelCore
{
	public class KaidelConsole
	{
		private ulong m_ID;

		public KaidelConsole()
		{
			m_ID = Internals.Console_InitNewConsole();
		}
		public void Log(string fmt, params object[] objects)
		{
			fmt = String.Format(fmt, objects);
			Internals.Console_MessageLog(m_ID, fmt);
		}
		public  void Info(string fmt, params object[] objects)
		{
			fmt = String.Format(fmt, objects);
			Internals.Console_MessageInfo(m_ID,  fmt);
		}
		public  void Warn(string fmt, params object[] objects)
		{
			fmt = String.Format(fmt, objects);
			Internals.Console_MessageWarn(m_ID,  fmt);
		}
		public  void Error(string fmt, params object[] objects)
		{
			fmt = String.Format(fmt, objects);
			Internals.Console_MessageError(m_ID,  fmt);
		}
		~KaidelConsole()
		{
			Internals.Console_DestroyConsole(m_ID);
		}

	}
}
