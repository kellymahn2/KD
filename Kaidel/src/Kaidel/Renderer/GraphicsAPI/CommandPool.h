#pragma once

#include "Kaidel/Core/Base.h"

namespace Kaidel {

	enum class CommandPoolOperationType {
		None = 0,
		Graphics,
		Compute,
		Transfer
	};

	enum CommandPoolFlags_ {
		CommandPoolFlags_Transient = BIT(0),
		CommandPoolFlags_CommandBufferReset = BIT(1)
	};

	typedef int CommandPoolFlags;

	class CommandPool : public IRCCounter<false>{
	public:
		virtual ~CommandPool() = default;

		virtual void Reset(int flags) = 0;

		virtual RendererID GetRendererID()const = 0;

		static Ref<CommandPool> Create(CommandPoolOperationType opType, CommandPoolFlags flags);



	private:
	};
}

