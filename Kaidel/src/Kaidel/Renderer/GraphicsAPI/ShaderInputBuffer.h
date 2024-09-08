#pragma once
#include "Kaidel/Core/Base.h"


namespace Kaidel {
	class ShaderInputBuffer : public IRCCounter<false>{
	public:
		virtual ~ShaderInputBuffer() = default;
		virtual RendererID GetRendererID()const = 0;
	};
}
