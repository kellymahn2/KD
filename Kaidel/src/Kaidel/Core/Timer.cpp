#include "KDpch.h"
#include "Timer.h"
namespace Kaidel {

	Timer::Timer()
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
		//KD_CORE_INFO("{0}ms", GetMilliseconds());
	}

}
