#pragma once

#include <memory>
#include "Kaidel/Renderer/GraphicsAPI/Buffer.h"
#include "Kaidel/Renderer/GraphicsAPI/Shader.h"
namespace Kaidel {

	struct VertexArraySpecification {
		Ref<Shader> UsedShader;
		std::vector<Ref<VertexBuffer>> VertexBuffers;
		Ref<IndexBuffer> IndexBuffer;
	};

	class VertexArray : public IRCCounter<false>
	{
	public:
		virtual ~VertexArray() = default;

		virtual void Bind() const = 0;
		virtual void Unbind() const = 0;

		virtual void SetIndexBuffer(Ref<IndexBuffer> indexBuffer) = 0;
		
		virtual const std::vector<Ref<VertexBuffer>>& GetVertexBuffers() const = 0;
		virtual const Ref<IndexBuffer>& GetIndexBuffer() const = 0;

		static Ref<VertexArray> Create(const VertexArraySpecification& spec);
	};

}
