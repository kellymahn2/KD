#pragma once

#include "Constants.h"
#include "Kaidel/Core/Base.h"
#include "Image.h"
#include <string>
#include <unordered_map>
namespace Kaidel {

	struct Texture2DSpecification {
		Format TextureFormat;
		uint32_t Width = 1;
		uint32_t Height = 1;
		uint32_t MipMaps = 1;
		TextureSamples Samples = TextureSamples::x1;
		TextureUsage Usage = 0;

		TextureSwizzle SwizzleR = TextureSwizzle::Red;
		TextureSwizzle SwizzleG = TextureSwizzle::Green;
		TextureSwizzle SwizzleB = TextureSwizzle::Blue;
		TextureSwizzle SwizzleA = TextureSwizzle::Alpha;

		void* InitialData = nullptr;
	};


	class Texture2D : public IRCCounter<false> {
	public:

		virtual ~Texture2D() = default;
		virtual const Texture2DSpecification& GetSpecification()const = 0;
		virtual const Image& GetImage()const = 0;
		virtual Image& GetImage() = 0;

		virtual void* Map(uint32_t mipMap)const = 0;
		virtual void Unmap()const = 0;

		static Ref<Texture2D> Create(const Texture2DSpecification& spec);

	};

	struct TextureLayered2DSpecification {
		Format TextureFormat;
		uint32_t InitialWidth = 1;
		uint32_t InitialHeight = 1;

		TextureSamples Samples = TextureSamples::x1;
		TextureUsage Usage = 0;

		TextureSwizzle SwizzleR = TextureSwizzle::Red;
		TextureSwizzle SwizzleG = TextureSwizzle::Green;
		TextureSwizzle SwizzleB = TextureSwizzle::Blue;
		TextureSwizzle SwizzleA = TextureSwizzle::Alpha;
	};

	struct TextureLayered2DLayerSpecification {
		uint32_t Width = 1;
		uint32_t Height = 1;

		void* InitialData;
	};

	class TextureLayered2D : public IRCCounter<false> {
	public:
		virtual ~TextureLayered2D() = default;
		
		virtual const TextureLayered2DSpecification& GetSpecification()const = 0;
		virtual const TextureLayered2DLayerSpecification& GetLayerSpecification(uint32_t layer)const = 0;

		virtual const Image& GetImage()const = 0;
		virtual Image& GetImage() = 0;

		virtual void* Map(uint32_t mipMap,uint32_t layer)const = 0;
		virtual void Unmap()const = 0;

		virtual uint32_t Push(const TextureLayered2DLayerSpecification& layerSpec) = 0;

		
		static Ref<TextureLayered2D> Create(const TextureLayered2DSpecification& spec);
	};


}
