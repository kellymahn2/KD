#pragma once
#include <chrono>
#include "Log.h"
namespace Kaidel {
	class Timer
	{
	public:
		Timer(const std::string& name= "");
		long long GetMilliseconds();
		~Timer();
	private:
		std::chrono::steady_clock::time_point m_Start;
		std::string m_Name;
	};

}

