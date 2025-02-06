#include "KDpch.h"
#include "Console.h"
#include <datetimeapi.h>
namespace Kaidel {


	void Console::Log(const std::string& message, MessageLevel level /*= MessageLevel::Log*/)
	{
		if (m_Messages.size() >= 300)
			Clear();
		m_Messages.emplace_back(message, level);
	}


	void Console::Log(const Message& message)
	{
		if (m_Messages.size() >= 300)
			Clear();
		m_Messages.push_back(message);
	}


	void Console::Clear()
	{
		m_Messages.clear();
	}


	Message::Message(const std::string& text, MessageLevel level)
		:Text(text), Level(level)
	{
		std::time_t time = std::chrono::system_clock::to_time_t(std::chrono::system_clock::now());
		std::tm tm = *std::localtime(&time);
		char buf[80] = { 0 };
		std::strftime(buf, sizeof(buf), "%d/%m/%Y-%H:%M:%S", &tm);
		TimeText = buf;
	}

}
