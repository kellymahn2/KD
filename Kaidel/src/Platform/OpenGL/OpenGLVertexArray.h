#pragma once

#include "Kaidel/Renderer/GraphicsAPI/VertexArray.h"

namespace Kaidel {

	class OpenGLVertexArray : public VertexArray
	{
	public:
		OpenGLVertexArray(const VertexArraySpecification& spec);
		virtual ~OpenGLVertexArray();

		virtual void Bind() const override;
		virtual void Unbind() const override;

		virtual void SetIndexBuffer(Ref<IndexBuffer> indexBuffer) override;
		virtual const std::vector<Ref<VertexBuffer>>& GetVertexBuffers() const { return m_Specification.VertexBuffers; }
		virtual const Ref<IndexBuffer>& GetIndexBuffer() const { return m_Specification.IndexBuffer; }
	private:
		uint32_t m_RendererID = 0;
		VertexArraySpecification m_Specification;
	};

}
