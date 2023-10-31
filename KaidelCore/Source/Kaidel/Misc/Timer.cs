using System;

namespace KaidelCore
{
	public class Timer
	{
		public Timer()
		{
			m_Now = DateTime.Now;

		}

		public TimeSpan Curr()
		{
			return DateTime.Now - m_Now;
		}

		public Timer Update()
		{
			m_Now = DateTime.Now;
			return this;
		}
		

		~Timer()
		{
			Debug.Log("It took {}",DateTime.Now - m_Now);
		}

		private DateTime m_Now;

	}
}
