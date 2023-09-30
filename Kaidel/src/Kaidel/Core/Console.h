#pragma once
#include <vector>
#include <string>
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
		MessageLevel Level;
		Message(const std::string& text, MessageLevel level);
	};
	class Console {
	public:
		void Log(const std::string& message, MessageLevel level = MessageLevel::Log);
		void Log(const Message& message);
		const std::vector<Message>& GetMessages()const { return m_Messages; }
		void Clear();
	private:
		std::vector<Message> m_Messages;
	};
}
