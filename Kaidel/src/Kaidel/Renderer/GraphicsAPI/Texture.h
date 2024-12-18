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
		uint32_t Mips = 1;
		const ImageType Type = ImageType::_2D;
		TextureSamples Samples = TextureSamples::x1;
		TextureSwizzle Swizzles[4] = { TextureSwizzle::Red,TextureSwizzle::Green,TextureSwizzle::Blue,TextureSwizzle::Alpha };
		bool IsCube = false;
		bool IsCpuReadable = false;

		TextureSpecification(ImageType type)
			:Type(type) 
		{}

		TextureSpecification() = delete;
	};

	class Texture : public IRCCounter<false> {
	public:
		virtual ~Texture() = default;
		virtual RendererID GetBackendInfo()const = 0;
		virtual const TextureSpecification& GetTextureSpecification()const = 0;
		virtual void SetImageLayout(ImageLayout layout) = 0;
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

	class FramebufferTexture : public Texture {
	public:
		virtual ~FramebufferTexture() = default;
	};
}
