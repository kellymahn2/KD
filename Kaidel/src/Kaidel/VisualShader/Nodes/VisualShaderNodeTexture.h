#pragma once

#include "Kaidel/Core/Base.h"

#include "VisualShaderNode.h"
#include "kaidel/Renderer//GraphicsAPI/Texture.h"

namespace Kaidel {

	struct VisualShaderNodeSampler : VisualShaderNode {
		SamplerFilter Filter = SamplerFilter::Linear;
		SamplerMipMapMode MipFilter = SamplerMipMapMode::Linear;
		SamplerAddressMode AddressMode = SamplerAddressMode::Repeat;

		virtual std::string_view GetCaption() const override;


		virtual uint32_t GetConnectedInputPort(uint32_t port) const override;


		virtual uint32_t GetInputPortCount() const override;


		virtual std::string_view GetInputPortName(uint32_t port) const override;


		virtual VisualShaderNodePortType GetInputPortType(uint32_t port) const override;


		virtual uint32_t GetOutputPortCount() const override;


		virtual std::string_view GetOutputPortName(uint32_t port) const override;


		virtual VisualShaderNodePortType GetOutputPortType(uint32_t port) const override;


		virtual bool IsInputPortConnected(uint32_t port) const override;


		virtual void SetConnectedInputPort(uint32_t port, uint32_t connection) override;


		virtual std::string GenerateCode(const std::vector<std::string>& inputVars) const override;

		NODE_CLASS_DECL("Sampler");

		void Serialize(YAML::Emitter& out) const override;


		void Deserialize(YAML::Node& node) override;

	};

	struct VisualShaderNodeTexture2D : VisualShaderNode {
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

		Ref<SpriteSheet> Image;

		std::array<uint32_t, PORT_MAX> InputConnections = FillArray<uint32_t, PORT_MAX>(-1);

		VisualShaderNodeTexture2D();

		virtual std::string_view GetCaption() const override;


		virtual uint32_t GetConnectedInputPort(uint32_t port) const override;


		virtual uint32_t GetInputPortCount() const override;


		virtual std::string_view GetInputPortName(uint32_t port) const override;


		virtual VisualShaderNodePortType GetInputPortType(uint32_t port) const override;


		virtual uint32_t GetOutputPortCount() const override;


		virtual std::string_view GetOutputPortName(uint32_t port) const override;


		virtual VisualShaderNodePortType GetOutputPortType(uint32_t port) const override;


		virtual bool IsInputPortConnected(uint32_t port) const override;


		virtual void SetConnectedInputPort(uint32_t port, uint32_t connection) override;


		virtual std::string GenerateCode(const std::vector<std::string>& inputVars) const override;


		virtual Ref<SpriteSheet> GetDescriptorTexture(uint32_t index) const override;

		NODE_CLASS_DECL("Texture2D");

		void Serialize(YAML::Emitter& out) const override;


		void Deserialize(YAML::Node& node) override;

	};
}
