#include "KDpch.h"
#include "Timer.h"
#include <iostream>
namespace Kaidel {

	static uint64_t s_IndentCount = 0;



	Timer::Timer(const std::string& name)
		:m_Name(name),m_AccumulatedTimeInNanoSecs(0)
	{}

	void Timer::Start()
	{
		m_Start = std::chrono::high_resolution_clock::now();
	}

	void Timer::ResetTimer()
	{
		m_AccumulatedTimeInNanoSecs = 0;
	}

	void Timer::Stop()
	{
		auto end = std::chrono::high_resolution_clock::now();

		m_AccumulatedTimeInNanoSecs += (uint64_t)std::chrono::duration_cast<std::chrono::nanoseconds>(end - m_Start).count();
	}

	void Timer::Print()
	{
		KD_CORE_INFO("{} Took:({}ns, {}ms, {}s)",m_Name,GetNS(),GetMS(),GetS());
	}

	double Timer::GetNS() const
	{
		return static_cast<double>(m_AccumulatedTimeInNanoSecs);
	}

	double Timer::GetMS() const
	{
		return GetNS() * 1e-6;
	}
	double Timer::GetS() const
	{
		return GetNS() * 1e-9;
	}

	

	/*static long long GetNanoseconds(std::chrono::steady_clock::time_point& start) {
		auto now = std::chrono::high_resolution_clock::now();
		return std::chrono::duration_cast<std::chrono::nanoseconds>(now - start).count();
	}*/

	/*double Timer::GetMilliseconds()
	{
		return GetNanoseconds(m_Start)*1e-6;
	}*/
	
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

	ScopedTimer::ScopedTimer(const std::string& name)
		:m_Timer(name)
	{
		m_Timer.Start();
	}

	ScopedTimer::~ScopedTimer()
	{
		m_Timer.Stop();
		m_Timer.Print();
	}

	void ScopedTimer::Reset()
	{
		m_Timer.ResetTimer();
	}

}
