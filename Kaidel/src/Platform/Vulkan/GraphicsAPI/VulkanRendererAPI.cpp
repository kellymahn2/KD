#include "KDpch.h"
#include "VulkanRendererAPI.h"


namespace Kaidel {
	namespace Vulkan {
		void VulkanRendererAPI::Init()
		{

		}
		void VulkanRendererAPI::SetViewport(uint32_t x, uint32_t y, uint32_t width, uint32_t height)
		{
		}
		void VulkanRendererAPI::GetViewport(uint32_t& x, uint32_t& y, uint32_t& width, uint32_t& height)
		{
		}
		void VulkanRendererAPI::SetClearColor(const glm::vec4& color)
		{
		}
		void VulkanRendererAPI::Clear()
		{

		}
		void VulkanRendererAPI::Shutdown()
		{
		}
		void VulkanRendererAPI::DrawIndexed(const Ref<VertexArray>& vertexArray, uint32_t indexCount)
		{
		}
		void VulkanRendererAPI::DrawIndexedInstanced(const Ref<VertexArray>& vertexArray, uint32_t indexCount, uint32_t instanceCount)
		{
		}
		void VulkanRendererAPI::DrawLines(const Ref<VertexArray>& vertexArray, uint32_t vertexCount)
		{
		}
		void VulkanRendererAPI::DrawPatches(const Ref<VertexArray>& vertexArray, uint32_t vertexCount)
		{
		}
		void VulkanRendererAPI::DrawPoints(const Ref<VertexArray>& vertexArray, uint32_t vertexCount)
		{
		}
		void VulkanRendererAPI::SetLineWidth(float thickness)
		{
		}
		void VulkanRendererAPI::SetPointSize(float pixelSize)
		{
		}
		void VulkanRendererAPI::SetCullMode(CullMode cullMode)
		{
		}
		int VulkanRendererAPI::QueryMaxTextureSlots()
		{
			return 0;
		}
		float VulkanRendererAPI::QueryMaxTessellationLevel()
		{
			return 0.0f;
		}
		void VulkanRendererAPI::SetPatchVertexCount(uint32_t count)
		{
		}
		void VulkanRendererAPI::SetDefaultTessellationLevels(const glm::vec4& outer, const glm::vec2& inner)
		{
		}
		void VulkanRendererAPI::RenderFullScreenQuad(Ref<Shader> shader, uint32_t width, uint32_t height) const
		{
		}
	}
}
