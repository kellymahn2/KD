#include "KDpch.h"
#include "RenderPass.h"
#include "RenderCommand.h"

namespace Kaidel {
	Ref<RenderPass> RenderPass::Create(const RenderPassSpecification& spec, Ref<Shader> shader, Ref<Framebuffer> outputBuffer)
	{
		return CreateRef<RenderPassShaderedColored>(spec, shader, outputBuffer);
	}
	void RenderPass::SetInput(Ref<UAVInput> unorderedAccessView)
	{
		m_UnorderedAccessViews.push_back({ unorderedAccessView,BindingType::Input });
	}

	void RenderPass::SetInput(Ref<UniformBuffer> uniformBuffer)
	{
		m_ConstantBuffers.push_back({ uniformBuffer,BindingType::Input });
	}

	void RenderPass::SetInput(Ref<Texture2D> texture)
	{
		m_Textures.push_back({ texture,BindingType::Input });
	}

	void RenderPass::SetInput(Ref<Texture2DArray> array)
	{
		m_TextureArrays.push_back({ array,BindingType::Input });
	}

	void RenderPass::BindObjects()
	{
		BindTextures();
		BindUnorderedAccessViews();
		BindConstantBuffers();
		BindTextureArrays();
	}

	void RenderPass::BindTextures()
	{
		uint32_t inputSlot = 0;
		uint32_t outputSlot = 0;
		for (uint32_t i = 0; i < m_Textures.size(); ++i) {
			const auto& textureObject = m_Textures.at(i);

			if (textureObject.Type == BindingType::Input)
				textureObject.Object->Bind(inputSlot++);
			//else
			//TODO: implement when we have binding of textures to image slots

		}
	}

	void RenderPass::BindUnorderedAccessViews()
	{
		for (uint32_t i = 0; i < m_UnorderedAccessViews.size(); ++i) {
			const auto& unorderedAccessViewObject = m_UnorderedAccessViews.at(i);
			unorderedAccessViewObject.Object->Bind(i);
		}
	}

	void RenderPass::BindConstantBuffers()
	{
		for (uint32_t i = 0; i < m_ConstantBuffers.size(); ++i) {
			const auto& constantBufferObject = m_ConstantBuffers.at(i);
			constantBufferObject.Object->Bind(i);
		}
	}

	void RenderPass::BindTextureArrays()
	{
		uint32_t inputSlot = 0;
		for (uint32_t i = 0; i < m_TextureArrays.size(); ++i) {
			const auto& textureArrayObject = m_TextureArrays.at(i);
			textureArrayObject.Object->Bind(i);
		}
	}

	void RenderPass::UnbindObjects()
	{
		UnbindTextureArrays();
		UnbindConstantBuffers();
		UnbindTextureArrays();
		UnbindTextures();
	}

	void RenderPass::UnbindTextures()
	{
		for (uint32_t i = 0; i < m_Textures.size(); ++i) {
			const auto& textureObject = m_Textures.at(i);
			//TODO: implement unbinding of textures
		}
	}

	void RenderPass::UnbindUnorderedAccessViews()
	{
		for (uint32_t i = 0; i < m_UnorderedAccessViews.size(); ++i) {
			const auto& unorderedAccessViewObject = m_UnorderedAccessViews.at(i);
			unorderedAccessViewObject.Object->Unbind();
		}
	}

	void RenderPass::UnbindConstantBuffers()
	{
		for (uint32_t i = 0; i < m_ConstantBuffers.size(); ++i) {
			const auto& constantBufferObject = m_ConstantBuffers.at(i);
			constantBufferObject.Object->UnBind();
		}
	}

	void RenderPass::UnbindTextureArrays()
	{
		//TODO: implement unbinding
	}

	RenderPassShaderedColored::RenderPassShaderedColored(const RenderPassSpecification& spec, Ref<Shader> shader, Ref<Framebuffer> outputBuffer)
		:m_Specification(spec), m_PassShader(shader),m_OutputBuffer(outputBuffer),m_State(RenderPassState::Idle)
	{
		KD_CORE_ASSERT(shader);
		VertexArraySpecification vertexArraySpec;
		vertexArraySpec.VertexBuffers = m_Specification.Buffers;
		vertexArraySpec.UsedShader = shader;
		m_VertexArray = VertexArray::Create(vertexArraySpec);
	}

	void RenderPassShaderedColored::SetInputBuffers(std::initializer_list<Ref<VertexBuffer>> inputBuffers)
	{
		if (inputBuffers.size() <= m_VertexBuffers.size()) {
			for (uint32_t i = 0; i < inputBuffers.size(); ++i) {
				m_VertexBuffers[i] = *(inputBuffers.begin() + i);
			}
		}

		m_VertexArray->SetVertexBuffers(inputBuffers);
	}

	void RenderPassShaderedColored::SetPassShader(Ref<Shader> shader)
	{
		m_PassShader = shader;
	}

	void RenderPassShaderedColored::SetPassShader(Ref<ComputeShader> computeShader)
	{
		KD_CORE_ASSERT(false,"Expected type _, Found type RenderPassShaderedColored");
	}

	void RenderPassShaderedColored::BeginPass()
	{
		m_State = RenderPassState::InPass;
		BindObjects();
	}

	void RenderPassShaderedColored::Flush(std::initializer_list<RenderPassVertexBufferContent> contents)
	{
		m_PassShader->Bind();
		m_OutputBuffer->Bind();
		uint32_t count = std::min(contents.size(), m_VertexBuffers.size());
		for (uint32_t i = 0; i < count; ++i) {
			m_VertexBuffers[i]->SetData((contents.begin() + i)->Data, (contents.begin() + i)->Size);
		}
		m_Specification.FlushFunction(m_VertexArray);
		m_PassShader->Unbind();
		m_OutputBuffer->Unbind();
	}

	void RenderPassShaderedColored::EndPass(std::initializer_list<RenderPassVertexBufferContent> contents)
	{
		Flush(contents);
		KD_CORE_ASSERT(m_State == RenderPassState::InPass, "You forgot to call BeginPass()");
		UnbindObjects();
		m_State = RenderPassState::Idle;
	}

	void RenderPassShaderedColored::SetOutput(Ref<UAVInput> unorderedAccessView)
	{
		m_UnorderedAccessViews.push_back({ unorderedAccessView,BindingType::Output });
	}

	void RenderPassShaderedColored::SetOutput(Ref<UniformBuffer> uniformBuffer)
	{
		m_ConstantBuffers.push_back({ uniformBuffer,BindingType::Output });
	}

	void RenderPassShaderedColored::SetOutput(Ref<Texture2D> texture)
	{
		m_Textures.push_back({ texture,BindingType::Output });
	}

	void RenderPassShaderedColored::SetOutput(Ref<Texture2DArray> array)
	{
		m_TextureArrays.push_back({ array,BindingType::Output });
	}


	void RenderPassShaderedColored::SetOutput(Ref<Framebuffer> framebuffer)
	{
		m_OutputBuffer = framebuffer;
	}

	
	void RenderPassShaderedColored::SetIndexBuffer(Ref<IndexBuffer> indexBuffer)
	{
		m_IndexBuffer = indexBuffer;
		m_VertexArray->SetIndexBuffer(indexBuffer);
	}
	void RenderPassShaderedColored::SetObjects(Ref<VertexArray> vertexArray, std::initializer_list<Ref<VertexBuffer>> inputBuffers)
	{

	}
}
