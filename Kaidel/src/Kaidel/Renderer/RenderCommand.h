#pragma once

#include "Kaidel/Renderer/RendererAPI.h"
#include "Kaidel/Renderer/GraphicsAPI/Shader.h"
#include "Kaidel/Renderer/GraphicsAPI/UniformBuffer.h"
#include "Kaidel/Renderer/GraphicsAPI/Image.h"
#include "Kaidel/Renderer/GraphicsAPI/RenderPass.h"
#include "Kaidel/Renderer/GraphicsAPI/DescriptorSet.h"

namespace Kaidel {

	class RenderCommand
	{
	public:
		static void Init()
		{
			s_RendererAPI->Init();
		}

		static void SetViewport(uint32_t x, uint32_t y, uint32_t width, uint32_t height)
		{
			s_RendererAPI->SetViewport(x, y, width, height);
		}
		
		static void GetViewport(uint32_t& x, uint32_t& y, uint32_t& width, uint32_t& height) {
			s_RendererAPI->GetViewport(x, y, width, height);
		}

		static void SetClearColor(const glm::vec4& color)
		{
			s_RendererAPI->SetClearColor(color);
		}

		static void Clear()
		{
			s_RendererAPI->Clear();
		}

		static void DrawIndexed(const Ref<VertexArray>& vertexArray, uint32_t indexCount = 0)
		{
			s_RendererAPI->DrawIndexed(vertexArray, indexCount);
		}
		static void DrawIndexedInstanced(const Ref<VertexArray>& vertexArray, uint32_t indexCount, uint32_t instanceCount) {
			s_RendererAPI->DrawIndexedInstanced(vertexArray, indexCount, instanceCount);
		}


		/*static void DrawLines(const Ref<VertexArray>& vertexArray, uint32_t vertexCount = 0)
		{
			s_RendererAPI->DrawLines(vertexArray, vertexCount);
		}

		static void DrawPatches(const Ref<VertexArray>& vertexArray, uint32_t vertexCount) {
			s_RendererAPI->DrawPatches(vertexArray, vertexCount);
		}

		static void DrawPoints(const Ref<VertexArray>& vertexArray, uint32_t vertexCount) {
			s_RendererAPI->DrawPoints(vertexArray, vertexCount);
		}

		static void SetCullMode(CullMode cullMode) {
			s_RendererAPI->SetCullMode(cullMode);
		}
		static void SetLineWidth(float width) {
			s_RendererAPI->SetLineWidth(width);
		}


		static void SetPointSize(float pixelSize) {
			s_RendererAPI->SetPointSize(pixelSize);
		}


		static void SetPatchVertexCount(uint32_t count) {
			s_RendererAPI->SetPatchVertexCount(count);
		}*/


		static void RenderFullScreenQuad(Ref<ShaderModule> shader,uint32_t width,uint32_t height) {
			s_RendererAPI->RenderFullScreenQuad(shader,width,height);
		}

		static Scope<RendererAPI>& GetRendererAPI() { return s_RendererAPI; }

		
		static void BindVertexBuffers(std::initializer_list<Ref<VertexBuffer>> vertexBuffers) {
			s_RendererAPI->BindVertexBuffers(vertexBuffers);
		}

		static void BindDescriptorSet(Ref<DescriptorSet> descriptorSet, uint32_t setIndex) {
			s_RendererAPI->BindDescriptorSet(descriptorSet, setIndex);
		}

		static void BindIndexBuffer(Ref<IndexBuffer> indexBuffer) {
			s_RendererAPI->BindIndexBuffer(indexBuffer);
		}

		static void BeginRenderPass(Ref<Framebuffer> frameBuffer, Ref<RenderPass> renderPass) {
			s_RendererAPI->BeginRenderPass(frameBuffer, renderPass);
		}
		static void EndRenderPass() {
			s_RendererAPI->EndRenderPass();
		}

		static void BindUniformBuffer(Ref<UniformBuffer> uniformBuffer, uint32_t index) {
			s_RendererAPI->BindUniformBuffer(uniformBuffer, index);
		}

		static void BindGraphicsPipeline(Ref<GraphicsPipeline> pipeline) {
			s_RendererAPI->BindGraphicsPipeline(pipeline);
		}

		static void Draw(uint32_t vertexCount, uint32_t firstVertexID = 0) {
			s_RendererAPI->Draw(vertexCount, firstVertexID);
		}
		static void DrawInstanced(uint32_t vertexCount, uint32_t instanceCount, uint32_t firstVertexID = 0) {
			s_RendererAPI->DrawInstanced(vertexCount, instanceCount, firstVertexID);
		}
		static void DrawIndexed(uint32_t indexCount, uint32_t firstIndex = 0, uint32_t vertexOffset = 0) {
			s_RendererAPI->DrawIndexed(indexCount, firstIndex, vertexOffset);
		}
		static void DrawIndexedInstanced(uint32_t indexCount, uint32_t instanceCount, uint32_t firstIndex = 0, uint32_t vertexOffset = 0) {
			s_RendererAPI->DrawIndexedInstanced(indexCount, instanceCount, firstIndex, vertexOffset);
		}

		static void Submit(std::function<void()>&& func) {
			s_RendererAPI->Submit(std::move(func));
		}

		static void Transition(Image& image, ImageLayout newLayout) {
			s_RendererAPI->Transition(image, newLayout);
		}


		static Scope<RendererAPI>& Get() { return s_RendererAPI; }

	private:
		static Scope<RendererAPI> s_RendererAPI;
	};

}
