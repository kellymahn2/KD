#include "KDpch.h"
#include "Timer.h"
#include <iostream>
namespace Kaidel {

	static uint64_t s_IndentCount = 0;
	static std::unordered_map<std::string, std::string> s_TimerData;

	const std::unordered_map<std::string, std::string>& Timer::GetTimerData() { return s_TimerData; }

	Timer::Timer(const std::string& name)
		:m_Name(name)
	{
		m_Start = std::chrono::high_resolution_clock::now();
	}

	static long long GetNanoseconds(std::chrono::steady_clock::time_point& start) {
		auto now = std::chrono::high_resolution_clock::now();
		return std::chrono::duration_cast<std::chrono::nanoseconds>(now - start).count();
	}

	double Timer::GetMilliseconds()
	{
		return GetNanoseconds(m_Start)*1e-6;
	}
	
	Timer::~Timer()
	{
		s_TimerData[m_Name] = fmt::format("{0}{1} Took {2}ns ({3}ms)", std::string(s_IndentCount,' '), m_Name, GetNanoseconds(m_Start), GetMilliseconds());
	}

}
