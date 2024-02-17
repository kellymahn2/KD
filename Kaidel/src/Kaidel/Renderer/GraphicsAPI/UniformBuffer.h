#pragma once
#include "Kaidel/Core/Base.h"
namespace Kaidel {
	class UniformBuffer
	{
	public:

		virtual ~UniformBuffer() {}
		virtual void SetData(const void* data, uint32_t size, uint32_t offset = 0) = 0;
		virtual void Bind(uint32_t binding)=0;
		virtual void Bind() = 0;
		virtual void UnBind() = 0;
		static Ref<UniformBuffer> Create(uint32_t size, uint32_t binding);
	protected:
		uint32_t m_Binding = 0;
	};
}

