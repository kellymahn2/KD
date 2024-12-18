#pragma once
#include "Kaidel/Core/Base.h"

namespace Kaidel {

	enum class BufferType {
		None = 0,
		VertexBuffer,
		IndexBuffer,
		UniformBuffer,
		StorageBuffer
	};

	class Buffer : public IRCCounter<false> {
	public:
		virtual ~Buffer() = default;

		virtual BufferType GetBufferType()const = 0;
		virtual RendererID GetBackendID()const = 0;
	};
}
