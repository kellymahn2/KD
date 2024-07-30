#pragma once

#include "Kaidel/Core/Base.h"
#include "Kaidel/Renderer/RendererDefinitions.h"


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

	class SamplerState : public IRCCounter<false> {
	public:
		virtual ~SamplerState() = default;

		virtual const SamplerParameters& GetParameters()const = 0;

		virtual RendererID GetRendererID()const = 0;
		
		
		static Ref<SamplerState> Create(const SamplerParameters& params);
	};

}
