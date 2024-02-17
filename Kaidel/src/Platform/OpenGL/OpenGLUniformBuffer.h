#pragma once
#include "Kaidel/Renderer/GraphicsAPI/UniformBuffer.h"
namespace Kaidel {

	class OpenGLUniformBuffer:public UniformBuffer
	{
	public:
		OpenGLUniformBuffer(uint32_t size, uint32_t binding);
		virtual ~OpenGLUniformBuffer();
		void Bind()override;
		void Bind(uint32_t binding)override;
		virtual void SetData(const void* data, uint32_t size, uint32_t offset = 0) override;

		void UnBind() override;

	private:
		uint32_t m_RendererID = 0;
	};
}

