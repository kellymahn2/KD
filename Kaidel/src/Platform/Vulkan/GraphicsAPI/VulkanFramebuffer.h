#pragma once
#include "Kaidel/Renderer/GraphicsAPI/Framebuffer.h"
#include "Backend.h"
#include "VulkanTexture.h"

namespace Kaidel {

	struct VulkanFramebufferResources {
		VkFramebuffer Framebuffer;
		std::vector<Ref<VulkanFramebufferTexture>> Textures;
		std::vector<Ref<VulkanFramebufferTexture>> Resolves;
	};


	class VulkanFramebuffer : public Framebuffer {
	public:

		VulkanFramebuffer(const FramebufferSpecification& specs);
		~VulkanFramebuffer();

		virtual const FramebufferSpecification& GetSpecification()const override { return m_Specification; }
		virtual void Recreate(const FramebufferSpecification& newSpecs) override;
		virtual void Resize(uint32_t width, uint32_t height) override;
		
		
		virtual Ref<Texture2D> GetColorAttachment(uint32_t index)const override{ return m_Framebuffer.Textures[index]; }
		virtual Ref<Texture2D> GetDepthAttachment()const override { return m_HasDepth ? m_Framebuffer.Textures.back() : nullptr; }
		virtual Ref<Texture2D> GetResolveAttachment(uint32_t index)const override { return m_Framebuffer.Resolves[index]; }

		VkFramebuffer GetFramebuffer()const { return m_Framebuffer.Framebuffer; }
		auto& GetResources() { return m_Framebuffer; }
	private:
		void Invalidate();
		void ClearData();
		void RegisterAttachments();
		void Destroy();
	private:
		VulkanFramebufferResources m_Framebuffer;
		
		std::vector<Texture2DSpecification> m_ColorInfos;
		Texture2DSpecification* m_DepthInfo = nullptr;

		bool m_HasDepth = false;
		
		FramebufferSpecification m_Specification;
	};
}
