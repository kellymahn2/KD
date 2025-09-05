#pragma once
#include "Application.h"

namespace Kaidel
{
	class DirtyCounter {
	public:
		DirtyCounter(bool initDirty = false)
		{
			if(!initDirty)
				m_LastUpdateFrame = GetCurrFrameNumber();
		}
		bool IsDirty() const { return m_LastUpdateFrame != GetCurrFrameNumber(); }
		
		void Update() { m_LastUpdateFrame = GetCurrFrameNumber(); }
	private:
		uint64_t GetCurrFrameNumber() const { Application::Get().GetCurrentFrameNumber(); }
	private:
		uint64_t m_LastUpdateFrame = -1;
	};
}
