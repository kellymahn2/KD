#pragma once
#include "Kaidel/Renderer/GraphicsAPI/VertexArray.h"
#include "Settings.h"

#include "Kaidel/Renderer/GraphicsAPI/Buffer.h"
#include "Kaidel/Renderer/GraphicsAPI/Framebuffer.h"

#include "Kaidel/Renderer/GraphicsAPI/Image.h"
#include "Kaidel/Renderer/GraphicsAPI/RenderPass.h"
#include "Kaidel/Renderer/GraphicsAPI/GraphicsPipeline.h"
#include "Kaidel/Renderer/GraphicsAPI/UniformBuffer.h"
#include <glm/glm.hpp>

namespace Kaidel {
	
	class RendererAPI
	{
	public:
		enum class API
		{
			None = 0, OpenGL = 1,DirectX=2,Vulkan = 3
		};
	public:
		virtual ~RendererAPI() = default;

		virtual void Init() = 0;
		virtual void SetViewport(uint32_t x, uint32_t y, uint32_t width, uint32_t height) = 0;
		virtual void GetViewport(uint32_t& x, uint32_t& y, uint32_t& width, uint32_t& height) = 0;
		virtual void SetClearColor(const glm::vec4& color) = 0;
		virtual void Clear() = 0;
		virtual void Shutdown() = 0;

		//Draw commands
		virtual void Draw(uint32_t vertexCount, uint32_t firstVertexID = 0) = 0;
		virtual void DrawInstanced(uint32_t vertexCount, uint32_t instanceCount, uint32_t firstVertexID = 0) = 0;
		virtual void DrawIndexed(uint32_t indexCount, uint32_t firstIndex = 0, uint32_t vertexOffset = 0) = 0;
		virtual void DrawIndexedInstanced(uint32_t indexCount, uint32_t instanceCount,uint32_t firstIndex = 0, uint32_t vertexOffset = 0) = 0;

		/*virtual void DrawIndexed(const Ref<VertexArray>& vertexArray, uint32_t indexCount = 0) = 0;
		virtual void DrawIndexedInstanced(const Ref<VertexArray>& vertexArray, uint32_t indexCount, uint32_t instanceCount) = 0;
		virtual void DrawLines(const Ref<VertexArray>& vertexArray, uint32_t vertexCount) = 0;
		virtual void DrawPatches(const Ref<VertexArray>& vertexArray, uint32_t vertexCount) = 0;
		virtual void DrawPoints(const Ref<VertexArray>& vertexArray, uint32_t vertexCount) = 0;*/

	/*	virtual void SetLineWidth(float thickness) = 0;
		virtual void SetPointSize(float pixelSize) = 0;
		virtual void SetCullMode(CullMode cullMode) = 0;

		virtual void SetPatchVertexCount(uint32_t count) = 0;*/
		virtual void BindUniformBuffer(Ref<UniformBuffer> uniformBuffer, uint32_t index) = 0;


		virtual void RenderFullScreenQuad(Ref<ShaderModule> shader, uint32_t width, uint32_t height)const = 0;

		virtual void BindVertexBuffers(std::initializer_list<Ref<VertexBuffer>> vertexBuffers) = 0;
		virtual void BindIndexBuffer(Ref<IndexBuffer> indexBuffer) = 0;

		virtual void BindDescriptorSet(Ref<DescriptorSet> descriptorSet,uint32_t setIndex) = 0;

		virtual void BeginRenderPass(Ref<Framebuffer> frameBuffer,Ref<RenderPass> renderPass) = 0;
		virtual void EndRenderPass() = 0;


		virtual void BindGraphicsPipeline(Ref<GraphicsPipeline> pipeline) = 0;

		virtual void BindPushConstants(Ref<GraphicsPipeline> pipeline, ShaderType type, const void* data, uint64_t size) = 0;

		virtual void Submit(std::function<void()>&& func) = 0;

		virtual void Transition(Image& image, ImageLayout newLayout) = 0;

		virtual void CopyBufferToTexture(Ref<TransferBuffer> src, Image& dst, const BufferToTextureCopyRegion& region) = 0;


		virtual void ClearColorImage(Image& image, const AttachmentColorClearValue& clearValue, const TextureSubresourceRegion& region) = 0;

		static RendererSettings& GetSettings() { return s_RendererSettings; }

		static API GetAPI() { return s_API; }
		static Scope<RendererAPI> Create();
	protected:
		static inline RendererSettings s_RendererSettings;
	private:
		static API s_API;
	};
}
