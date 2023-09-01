#include "KDpch.h"
#include "Timer.h"
namespace Kaidel {

	Timer::Timer(const std::string& name)
		:m_Name(name)
	{
		m_Start = std::chrono::high_resolution_clock::now();
	}

	long long Timer::GetMilliseconds()
	{
		auto now = std::chrono::high_resolution_clock::now();
		return std::chrono::duration_cast<std::chrono::milliseconds>(now - m_Start).count();
	}

	Timer::~Timer()
	{
		if (m_Name.empty())
			KD_CORE_INFO("{0}ms", GetMilliseconds());
		else
			KD_CORE_INFO("{0} Took {1}ms", m_Name,GetMilliseconds());
	}

}
