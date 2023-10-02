#include "KDpch.h"
#include "Console.h"
#include <datetimeapi.h>
namespace Kaidel {


	void Console::Log(const std::string& message, MessageLevel level /*= MessageLevel::Log*/)
	{
		m_Messages.emplace_back(message, level);
	}


	void Console::Log(const Message& message)
	{
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
