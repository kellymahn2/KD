#pragma once

#include "Kaidel/Core/Base.h"

#include "VisualMaterialNode.h"
#include "kaidel/Renderer//GraphicsAPI/Texture.h"
#include "kaidel/Renderer//GraphicsAPI/Sampler.h"

namespace Kaidel {

	struct VisualMaterialNodeSampler : VisualMaterialNode {
		SamplerFilter Filter = SamplerFilter::Linear;
		SamplerMipMapMode MipFilter = SamplerMipMapMode::Linear;
		SamplerAddressMode AddressMode = SamplerAddressMode::Repeat;

		virtual std::string_view GetCaption() const override;


		virtual uint32_t GetConnectedInputPort(uint32_t port) const override;


		virtual uint32_t GetInputPortCount() const override;


		virtual std::string_view GetInputPortName(uint32_t port) const override;


		virtual VisualMaterialNodePortType GetInputPortType(uint32_t port) const override;


		virtual uint32_t GetOutputPortCount() const override;


		virtual std::string_view GetOutputPortName(uint32_t port) const override;


		virtual VisualMaterialNodePortType GetOutputPortType(uint32_t port) const override;


		virtual bool IsInputPortConnected(uint32_t port) const override;


		virtual void SetConnectedInputPort(uint32_t port, uint32_t connection) override;


		virtual std::string GenerateCode(const std::vector<std::string>& inputVars) const override;

	};

	struct VisualMaterialNodeTexture2D : VisualMaterialNode {
		enum PORT {
			PORT_UV,
			PORT_SAMPLER,
			PORT_MAX
		};

		enum class SampleType {
			Data, 
			Color, 
			Normal
		};

		SampleType SamplingType = SampleType::Data;

		Ref<Texture2D> Image;

		std::array<uint32_t, PORT_MAX> InputConnections = FillArray<uint32_t, PORT_MAX>(-1U);

		VisualMaterialNodeTexture2D();

		virtual std::string_view GetCaption() const override;


		virtual uint32_t GetConnectedInputPort(uint32_t port) const override;


		virtual uint32_t GetInputPortCount() const override;


		virtual std::string_view GetInputPortName(uint32_t port) const override;


		virtual VisualMaterialNodePortType GetInputPortType(uint32_t port) const override;


		virtual uint32_t GetOutputPortCount() const override;


		virtual std::string_view GetOutputPortName(uint32_t port) const override;


		virtual VisualMaterialNodePortType GetOutputPortType(uint32_t port) const override;


		virtual bool IsInputPortConnected(uint32_t port) const override;


		virtual void SetConnectedInputPort(uint32_t port, uint32_t connection) override;


		virtual std::string GenerateCode(const std::vector<std::string>& inputVars) const override;


		virtual Ref<Texture> GetDescriptorTexture(uint32_t index) const override;

	};
}
