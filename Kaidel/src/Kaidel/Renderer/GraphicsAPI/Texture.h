#pragma once
#include "Kaidel/Core/Base.h"
#include "Kaidel/Renderer/RendererDefinitions.h"
#include "Kaidel/Asset/Asset.h"
#include "AtlasPacker.h"

#include <glm/glm.hpp>

namespace Kaidel {
	struct TextureData {
		uint32_t Layer = 0;
		uint32_t OffsetX = 0, OffsetY = 0, OffsetZ = 0;
		uint32_t Width = -1, Height = -1, Depth = -1;
		const void* Data = nullptr;
	};

	struct TextureSpecification {
		virtual ~TextureSpecification() = default;
		
		ImageLayout Layout = ImageLayout::None;
		Format Format = Format::RGBA8UN;
		uint32_t Width = 1;
		uint32_t Height = 1;
		uint32_t Depth = 1;
		uint32_t Layers = 1;
		uint32_t Mips = 1;
		//Set by constructor
		TextureSamples Samples = TextureSamples::x1;
		TextureSwizzle Swizzles[4] = { TextureSwizzle::Red,TextureSwizzle::Green,TextureSwizzle::Blue,TextureSwizzle::Alpha };
		
		bool Mipped = true;

		//Set by constructor
		bool IsCpuReadable = false;

		TextureSpecification(ImageType type, bool isCube = false)
			:m_Type(type), m_IsCube(isCube)
		{}

		bool IsCube() const { return m_IsCube; }
		ImageType Type() const { return m_Type; }

		TextureSpecification() = default;

	private:
		ImageType m_Type = ImageType::_2D;
		bool m_IsCube = false;
	};

	class Texture : public AssetBase {
	public:
		Texture() {

		}
		virtual ~Texture() = default;
		virtual RendererID GetBackendInfo()const = 0;
		virtual const TextureSpecification& GetTextureSpecification()const = 0;
		virtual void SetImageLayout(ImageLayout layout) = 0;

		virtual bool IsReference()const { return false; }
	};

	struct Texture2DSpecification : TextureSpecification {
		virtual ~Texture2DSpecification() = default;
		Texture2DSpecification()
			: TextureSpecification(ImageType::_2D)
		{}

		std::vector<TextureData> InitialDatas;
	};

	class Texture2D : public Texture {
	public:
		virtual ~Texture2D() = default;
		Texture2D() {

		}

		static Ref<Texture2D> CreateUninitialized();
		static Ref<Texture2D> Create(const Texture2DSpecification& specs);
		static Ref<Texture2D> CreateAt(Ref<Texture2D> texture, const Texture2DSpecification& specs);

		DECL_ASSET_TYPE(Texture2D);
	};

	struct TextureLayeredSpecification : TextureSpecification {
		TextureLayeredSpecification(ImageType type = ImageType::_2D_Array)
			: TextureSpecification(type)
		{}

		std::vector<TextureData> InitialDatas;
	};

	class TextureLayered : public Texture {
	public:
		virtual ~TextureLayered() = default;
		
		static Ref<TextureLayered> Create(const TextureLayeredSpecification& specs);
	};

	struct TextureCubeSpecification : TextureSpecification {
		TextureCubeSpecification()
			: TextureSpecification(ImageType::_2D, true)
		{
		}

		std::vector<TextureData> InitialDatas;
	};

	class TextureCube : public Texture
	{
	public:
		virtual ~TextureCube() = default;

		static Ref<TextureCube> Create(const TextureCubeSpecification& specs);
	};


	class FramebufferTexture : public Texture {
	public:
		virtual ~FramebufferTexture() = default;
	};

	struct TextureAtlasSpecification : TextureSpecification 
	{
		TextureAtlasSpecification()
			: TextureSpecification(ImageType::_2D, false)
		{
		}
	};


	class TextureAtlas : public Texture
	{
	public:

		virtual ~TextureAtlas() = default;

		virtual const AtlasSamplingCoords& GetSamplingCoords(uint32_t i) const = 0;

		virtual uint32_t AddToAtlas(const void* data, uint32_t width, uint32_t height) = 0;

		virtual void SetData(const TextureData& data, uint32_t i) = 0;

		static Ref<TextureAtlas> Create(const TextureAtlasSpecification& specs);
	};

	struct TextureReferenceSpecification {
		Ref<Texture> Reference;
		uint32_t StartMip = 0;
		uint32_t MipCount = 1;
		uint32_t StartLayer = 0;
		uint32_t LayerCount = 1;
		TextureSwizzle Swizzles[4] = { TextureSwizzle::Red,TextureSwizzle::Green,TextureSwizzle::Blue,TextureSwizzle::Alpha };
	};

	class TextureReference : public Texture {
	public:

		virtual Ref<Texture> GetReferencedTexture()const = 0;
		virtual const TextureReferenceSpecification& GetReferenceSpecification()const = 0;
		virtual ~TextureReference() = default;
		virtual bool IsReference()const override final { return true; }

		static Ref<TextureReference> Create(const TextureReferenceSpecification& specs);
	};
}
