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

	static std::unordered_map<std::string, uint64_t> s_Timers;


	AccumulativeTimer::AccumulativeTimer(const std::string& name)
		:m_Name(name),m_AccumulatedTimeInNanoSecs(0)
	{
		if(s_Timers.find(m_Name) == s_Timers.end())
			s_Timers[m_Name] = 0;
	}
	void AccumulativeTimer::Start() {
		m_Start = std::chrono::high_resolution_clock::now();
	}
	void AccumulativeTimer::ResetTimer() {
		s_Timers[m_Name] = 0;
	}

	void AccumulativeTimer::Stop() {
		auto end = std::chrono::high_resolution_clock::now();
		m_AccumulatedTimeInNanoSecs = s_Timers.at(m_Name);
		
		m_AccumulatedTimeInNanoSecs += (uint64_t)std::chrono::duration_cast<std::chrono::nanoseconds>(end - m_Start).count();
		s_Timers.at(m_Name) = m_AccumulatedTimeInNanoSecs;
	}
	const std::unordered_map<std::string, uint64_t>& AccumulativeTimer::GetTimers() {
		return s_Timers;
	}
	void AccumulativeTimer::ResetTimers() {
		for (auto& [name, timer] : s_Timers) {
			timer = 0;
		}
	}

	ScopedAccumulativeTimer::ScopedAccumulativeTimer(const std::string& name) 
		:m_Timer(name)
	{
		m_Timer.Start();
	}

	void ScopedAccumulativeTimer::Reset() {
		m_Timer.ResetTimer();
	}

	ScopedAccumulativeTimer::~ScopedAccumulativeTimer() {
		m_Timer.Stop();
	}

}
