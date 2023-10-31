#include "KDpch.h"
#include "Timer.h"
#include <iostream>
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
	static long long GetNanoseconds(std::chrono::steady_clock::time_point& start) {
		auto now = std::chrono::high_resolution_clock::now();
		return std::chrono::duration_cast<std::chrono::nanoseconds>(now - start).count();
	}
	Timer::~Timer()
	{
		if (m_Name.empty())
			std::cout<<fmt::format("{0}ns", GetNanoseconds(m_Start))<<std::endl;
		else
			std::cout<<fmt::format("{0} Took {1}ns", m_Name,GetNanoseconds(m_Start))<<std::endl;
	}

}
