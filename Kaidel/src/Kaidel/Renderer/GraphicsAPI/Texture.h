#pragma once
#include "Kaidel/Core/Base.h"
#include "Kaidel/Renderer/RendererDefinitions.h"

namespace Kaidel {
	struct TextureData {
		uint32_t Layer = 0;
		uint32_t OffsetX = 0, OffsetY = 0, OffsetZ = 0;
		const void* Data = nullptr;
	};

	struct TextureSpecification {
		virtual ~TextureSpecification() = default;
		std::vector<TextureData> InitialDatas;
		ImageLayout Layout = ImageLayout::None;
		Format Format = Format::RGBA8UN;
		uint32_t Width = 1;
		uint32_t Height = 1;
		uint32_t Depth = 1;
		uint32_t Layers = 1;
		bool Mipped = true;
		uint32_t Mips = 1;
		const ImageType Type = ImageType::_2D;
		TextureSamples Samples = TextureSamples::x1;
		TextureSwizzle Swizzles[4] = { TextureSwizzle::Red,TextureSwizzle::Green,TextureSwizzle::Blue,TextureSwizzle::Alpha };
		const bool IsCube = false;
		bool IsCpuReadable = false;

		TextureSpecification(ImageType type, bool isCube = false)
			:Type(type), IsCube(isCube)
		{}

		TextureSpecification() = delete;
	};

	class Texture : public IRCCounter<false> {
	public:
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
	};

	class Texture2D : public Texture {
	public:
		virtual ~Texture2D() = default;

		static Ref<Texture2D> Create(const Texture2DSpecification& specs);
	};

	struct TextureLayeredSpecification : TextureSpecification {
		TextureLayeredSpecification(ImageType type = ImageType::_2D_Array)
			: TextureSpecification(type)
		{}
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
