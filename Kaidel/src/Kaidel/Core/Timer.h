#pragma once
#include "Log.h"

#include <chrono>
#include <unordered_map>

namespace Kaidel {
	class Timer
	{
	public:
		Timer(const std::string& name);
		void Start();
		void ResetTimer();
		void Stop();

		void Print(const std::string& padding = "");

		double GetMS()const;
		double GetS()const;
		double GetNS()const;

	private:
		std::chrono::steady_clock::time_point m_Start;
		uint64_t m_AccumulatedTimeInNanoSecs;
		std::string m_Name;
	};

	class ScopedTimer {
	public:
		ScopedTimer(const std::string& name);
		~ScopedTimer();
		void Reset();
	private:
		Timer m_Timer;
		static inline std::string s_Padding = "";
	};

	class AccumulativeTimer {
	public:
		AccumulativeTimer(const std::string& name);

		void Start();

		void ResetTimer();

		void Stop();

		static const std::unordered_map<std::string, uint64_t>& GetTimers();

		static void ResetTimers();

	private:
		std::chrono::steady_clock::time_point m_Start;
		uint64_t m_AccumulatedTimeInNanoSecs;
		std::string m_Name;
	};

	class ScopedAccumulativeTimer {
	public:
		ScopedAccumulativeTimer(const std::string& name);
		~ScopedAccumulativeTimer();
		void Reset();
	private:
		AccumulativeTimer m_Timer;
	};


}

#define EXPAND(arg) arg
#define MAKE(arg1,arg2) EXPAND(arg1) ## EXPAND(arg2)


#define SCOPED_TIMER(name) ::Kaidel::ScopedAccumulativeTimer timer = ::Kaidel::ScopedAccumulativeTimer{#name};
#define NEW_SCOPED_TIMER(name) SCOPED_TIMER(name); MAKE(_Timer,__LINE__).Reset();

#define SCOPED_TIMER(name) ::Kaidel::ScopedTimer timer(name);
