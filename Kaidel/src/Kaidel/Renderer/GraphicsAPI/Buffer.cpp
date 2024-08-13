#include "KDpch.h"
#include "Kaidel/Renderer/GraphicsAPI/Buffer.h"

#include "Kaidel/Renderer/Renderer.h"

#include "Platform/OpenGL/OpenGLBuffer.h"
#include "Platform/Vulkan/GraphicsAPI/VulkanVertexBuffer.h"
#include "Platform/Vulkan/GraphicsAPI/VulkanIndexBuffer.h"
#include "Platform/Vulkan/GraphicsAPI/VulkanTransferBuffer.h"


namespace Kaidel {
	Ref<VertexBuffer> VertexBuffer::Create(const VertexBufferSpecification& specification)
	{
		switch (Renderer::GetAPI())
		{
		case RendererAPI::API::None:    KD_CORE_ASSERT(false, "RendererAPI::None is currently not supported!"); return nullptr;
		//case RendererAPI::API::OpenGL:  return CreateRef<OpenGLVertexBuffer>(size);
		//case RendererAPI::API::Vulkan: return CreateRef<VulkanVertexBuffer>(specification);
		}

		KD_CORE_ASSERT(false, "Unknown RendererAPI!");
		return nullptr;
	}
	Ref<VertexBuffer> VertexBuffer::Create(uint32_t size)
	{
		switch (Renderer::GetAPI())
		{
			case RendererAPI::API::None:    KD_CORE_ASSERT(false, "RendererAPI::None is currently not supported!"); return nullptr;
			case RendererAPI::API::OpenGL:  return CreateRef<OpenGLVertexBuffer>(size);
			case RendererAPI::API::Vulkan: return CreateRef<VulkanVertexBuffer>(size);
			//case RendererAPI::API::Vulkan: return CreateRef<Vulkan::VulkanVertexBuffer>(size);
		}

		KD_CORE_ASSERT(false, "Unknown RendererAPI!");
		return nullptr;
	}

	Ref<VertexBuffer> VertexBuffer::Create(float* vertices, uint32_t size)
	{
		switch (Renderer::GetAPI())
		{
			case RendererAPI::API::None:    KD_CORE_ASSERT(false, "RendererAPI::None is currently not supported!"); return nullptr;
			case RendererAPI::API::OpenGL:  return CreateRef<OpenGLVertexBuffer>(vertices, size);
			//case RendererAPI::API::Vulkan: return CreateRef<Vulkan::VulkanVertexBuffer>(vertices,size);
		}

		KD_CORE_ASSERT(false, "Unknown RendererAPI!");
		return nullptr;
	}

	Ref<IndexBuffer> IndexBuffer::Create(uint32_t* indices, uint32_t size)
	{
		switch (Renderer::GetAPI())
		{
			case RendererAPI::API::None:    KD_CORE_ASSERT(false, "RendererAPI::None is currently not supported!"); return nullptr;
			case RendererAPI::API::OpenGL:  return CreateRef<OpenGLIndexBuffer>(indices, size);
			case RendererAPI::API::Vulkan: return CreateRef<VulkanIndexBuffer>(indices, size);
		}

		KD_CORE_ASSERT(false, "Unknown RendererAPI!");
		return nullptr;
	}



	Ref<TransferBuffer> TransferBuffer::Create(uint64_t size, const void* initData, uint64_t initDataSize)
	{
		switch (Renderer::GetAPI())
		{
		case RendererAPI::API::Vulkan: return CreateRef<VulkanTransferBuffer>(size,initData,initDataSize);
		}

		KD_CORE_ASSERT(false, "Unknown RendererAPI!");
		return nullptr;
	}

}
