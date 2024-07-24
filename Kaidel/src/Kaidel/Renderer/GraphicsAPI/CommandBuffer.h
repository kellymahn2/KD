#pragma once

#include "Kaidel/Core/Base.h"

#include "CommandPool.h"

namespace Kaidel {

	enum class CommandBufferType {
		None,
		Primary,
		Secondary
	};


	class CommandBuffer : public IRCCounter<false>{
	public:
		
		virtual ~CommandBuffer() = default;

		virtual RendererID GetCommandBufferID()const = 0;

		static Ref<CommandBuffer> Create(Ref<CommandPool> commandPool,CommandBufferType type, uint32_t flags);



	};
}
