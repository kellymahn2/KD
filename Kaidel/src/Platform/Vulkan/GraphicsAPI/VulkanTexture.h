#pragma once
#include "Kaidel/Renderer/GraphicsAPI/Texture.h"
#include "Backend.h"

namespace Kaidel {
	class VulkanTexture2D : public Texture2D {
	public:
		VulkanTexture2D() = default;
		VulkanTexture2D(const Texture2DSpecification& specs);
		~VulkanTexture2D();
		virtual RendererID GetBackendInfo()const override { return (RendererID)&m_Info; }
		virtual const TextureSpecification& GetTextureSpecification()const override { 
			return static_cast<const TextureSpecification&>(m_Specification); 
		}
		virtual void SetImageLayout(ImageLayout layout) override { m_Specification.Layout = layout; }

		void Init(const Texture2DSpecification& specs);

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

	class VulkanTextureCube : public TextureCube {
	public:
		VulkanTextureCube(const TextureCubeSpecification& specs);
		~VulkanTextureCube();

		virtual RendererID GetBackendInfo()const override { return (RendererID)&m_Info; }
		virtual const TextureSpecification& GetTextureSpecification()const override {
			return static_cast<const TextureSpecification&>(m_Specification);
		}
		virtual void SetImageLayout(ImageLayout layout) override { m_Specification.Layout = layout; }
	private:
		TextureCubeSpecification m_Specification;
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

	class VulkanTextureReference : public TextureReference {
	public:
		VulkanTextureReference(const TextureReferenceSpecification& specs);
		~VulkanTextureReference();

		RendererID GetBackendInfo() const override { return (RendererID)&m_Info; }
		const TextureSpecification& GetTextureSpecification() const override { return m_Specification.Reference->GetTextureSpecification(); }
		void SetImageLayout(ImageLayout layout) override { m_Specification.Reference->SetImageLayout(layout); }
		Ref<Texture> GetReferencedTexture() const override { return m_Specification.Reference; }
		const TextureReferenceSpecification& GetReferenceSpecification() const override { return m_Specification; }
	private:
		TextureReferenceSpecification m_Specification;
		VulkanBackend::TextureInfo m_Info;
	};

	class VulkanTextureAtlas : public TextureAtlas {
	public:

		VulkanTextureAtlas(const TextureAtlasSpecification& specification);
		~VulkanTextureAtlas();

		virtual const AtlasSamplingCoords& GetSamplingCoords(uint32_t i) const override;

		virtual RendererID GetBackendInfo()const override { return (RendererID)&m_Info; }
		virtual const TextureSpecification& GetTextureSpecification()const override {
			return static_cast<const TextureSpecification&>(m_Specification);
		}
		virtual void SetImageLayout(ImageLayout layout) override { m_Specification.Layout = layout; }

		virtual uint32_t AddToAtlas(const void* data, uint32_t width, uint32_t height) override;

		virtual void SetData(const TextureData& data, uint32_t i) override;
	private:
		TextureAtlasSpecification m_Specification;
		AtlasManager m_AtlasManager;
		VulkanBackend::TextureInfo m_Info;
	};

}
