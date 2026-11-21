#pragma once

#include "Kaidel/Core/Base.h"

#include "VisualShaderNode.h"
#include "kaidel/Renderer//GraphicsAPI/Texture.h"
#include "kaidel/Renderer//GraphicsAPI/Sampler.h"

namespace Kaidel {

	enum class InputType {
		FragPos,
		Tangent,
		Normal,
		Bitangent,
		TexCoords,
		CameraPos,
		ScreenSize,
		ScreenUV,
		Time,
		DeltaTime
	};

	struct VisualShaderNodeInput : VisualShaderNode {
		Kaidel::InputType Type = InputType::TexCoords;

		virtual std::string GenerateCode(const std::vector<std::string>& inputVars) const override;


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

		NODE_CLASS_DECL("Input");

		void Serialize(YAML::Emitter& out) const override;


		void Deserialize(YAML::Node& node) override;

	};

}
