#pragma once
#include "Log.h"

#include <chrono>
#include <unordered_map>

namespace Kaidel {
	class Timer
	{
	public:
		Timer(const std::string& name);
		double GetMilliseconds();
		~Timer();

		static const std::unordered_map<std::string, std::string>& GetTimerData();
	private:
		std::chrono::steady_clock::time_point m_Start;
		std::string m_Name;
	};

}

