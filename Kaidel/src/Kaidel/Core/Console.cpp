#include "KDpch.h"
#include "Console.h"
#include <datetimeapi.h>
namespace Kaidel {


	void Console::Log(const std::string& message, MessageLevel level /*= MessageLevel::Log*/)
	{
		if (m_Messages.size() >= 1500)
			Clear();
		m_Messages.emplace_back(message, level);
	}


	void Console::Log(const Message& message)
	{
		if (m_Messages.size() >= 1500)
			Clear();
		m_Messages.push_back(message);
	}


	void Console::Clear()
	{
		m_Messages.clear();
	}


	Message::Message(const std::string& text, MessageLevel level)
		:Text(text),Level(level)
	{
		Time = std::chrono::system_clock::now();
			
		
	}

}
