#pragma once

#include "Constants.h"
#include "Kaidel/Core/Base.h"
#include "Image.h"
#include <string>
#include <unordered_map>
namespace Kaidel {

	struct SamplerParameters {
		SamplerFilter MinificationFilter;
		SamplerFilter MagnificationFilter;
		SamplerMipMapMode MipmapMode;
		SamplerAddressMode AddressModeU;//x Coords
		SamplerAddressMode AddressModeV;//y Coords
		SamplerAddressMode AddressModeW;//z Coords
		float LODBias = 0.0f;
		float MinLOD = 0.0f;
		float MaxLOD = 0.0f;
		SamplerBorderColor BorderColor;
	};

	struct Texture2DSpecification {
		SamplerParameters Sampler;

		DeviceMemoryType TextureMemoryType;
		Format TextureFormat;

		uint32_t Width;
		uint32_t Height;
		uint32_t Layers;
		uint32_t Levels;
		void* Data;
	};

	class Texture2D : public IRCCounter<false> {
	public:
		virtual ~Texture2D() = default;
		
		virtual Image& GetImage() = 0;
		
		virtual void AddLayer(void* data, uint64_t size) = 0;

		virtual const Texture2DSpecification& GetSpecification()const = 0;

	};

	class ImmutableTexture2D : public Texture2D {
	public:
		static Ref<ImmutableTexture2D> Create(const Texture2DSpecification& spec);
	};


	class MutableTexture2D : public Texture2D {
	public:
		virtual void SetData(void* data, uint32_t layerIndex) = 0;
	};


}
