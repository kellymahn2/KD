#pragma once
#include "Kaidel/Renderer/GraphicsAPI/Texture.h"
#include "Backend.h"

namespace Kaidel {
	class VulkanTexture2D : public Texture2D {
	public:
		VulkanTexture2D(const Texture2DSpecification& specs);
		~VulkanTexture2D();
		virtual RendererID GetBackendInfo()const override { return (RendererID)&m_Info; }
		virtual const TextureSpecification& GetTextureSpecification()const override { 
			return static_cast<const TextureSpecification&>(m_Specification); 
		}
		virtual void SetImageLayout(ImageLayout layout) override { m_Specification.Layout = layout; }
	private:
		Texture2DSpecification m_Specification;
		VulkanBackend::TextureInfo m_Info;
	};

	class VulkanTextureLayered : public TextureLayered {
	public:
		VulkanTextureLayered(const TextureLayeredSpecification& specs);
		~VulkanTextureLayered();
		virtual RendererID GetBackendInfo()const override { return (RendererID)&m_Info; }
		virtual const TextureSpecification& GetTextureSpecification()const override {
			return static_cast<const TextureSpecification&>(m_Specification);
		}
		virtual void SetImageLayout(ImageLayout layout) override { m_Specification.Layout = layout; }
	private:
		TextureLayeredSpecification m_Specification;
		VulkanBackend::TextureInfo m_Info;
	};

	class VulkanFramebufferTexture : public FramebufferTexture {
	public:
		VulkanFramebufferTexture(uint32_t width, uint32_t height, uint32_t layers, Format format, TextureSamples samples, bool isDepth);
		~VulkanFramebufferTexture();

		virtual RendererID GetBackendInfo()const override { return (RendererID)&m_Info; }
		virtual const TextureSpecification& GetTextureSpecification()const override {
			return static_cast<const TextureSpecification&>(m_Specification);
		}
		virtual void SetImageLayout(ImageLayout layout) override { m_Specification.Layout = layout; }
	private:
		TextureSpecification m_Specification;
		VulkanBackend::TextureInfo m_Info;
	};

}
