#pragma once
#include "Kaidel/Core/Base.h"
#include "Kaidel/Renderer/RendererDefinitions.h"


namespace Kaidel {

	struct SamplerState {
		SamplerFilter MagFilter = SamplerFilter::Nearest;
		SamplerFilter MinFilter = SamplerFilter::Nearest;
		SamplerMipMapMode MipFilter = SamplerMipMapMode::Nearest;
		SamplerAddressMode AddressModeU = SamplerAddressMode::Repeat;
		SamplerAddressMode AddressModeV = SamplerAddressMode::Repeat;
		SamplerAddressMode AddressModeW = SamplerAddressMode::Repeat;
		float LodBias = 0.0f;
		bool Aniso = false;
		float AnisoMax = 1.0f;
		bool Compare = false;
		CompareOp CompareOp = CompareOp::Always;
		float MinLod = 0.0f;
		float MaxLod = 1e20;
		SamplerBorderColor BorderColor = SamplerBorderColor::FloatOpaqueBlack;
		bool Unnormalized = false;
	};

	class Sampler : public IRCCounter<false> {
	public:
		
		virtual ~Sampler() = default;
		virtual const SamplerState& GetSamplerState()const = 0;

		static Ref<Sampler> Create(const SamplerState& state);
	};
}
