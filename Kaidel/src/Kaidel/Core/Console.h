#pragma once
#include "Kaidel/Core/IRCP.h"
#include <vector>
#include <string>
#include <chrono>
namespace Kaidel {

	enum class MessageLevel : uint8_t{
		None,
		Log,
		Info,
		Warn,
		Error,
	};
	struct Message {
		std::string Text;
		std::string TimeText;
		MessageLevel Level;
		std::chrono::system_clock::time_point Time;
		Message(const std::string& text, MessageLevel level);
	};
	class Console : public IRCCounter<false> {
	public:
		void Log(const std::string& message, MessageLevel level = MessageLevel::Log);
		void Log(const Message& message);
		const std::vector<Message>& GetMessages()const { return m_Messages; }
		void Clear();
	private:
		std::vector<Message> m_Messages;
	};
}
