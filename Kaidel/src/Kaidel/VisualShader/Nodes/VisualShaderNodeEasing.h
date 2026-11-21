#pragma once

#include "VisualShaderNode.h"

namespace Kaidel
{
	enum class EaseType {
		Linear,

		SineIn,
		SineOut,
		Sine,

		QuadIn,
		QuadOut,
		Quad,

		CubicIn,
		CubicOut,
		Cubic,

		ExpoIn,
		ExpoOut,
		Expo,
		
		CircIn,
		CircOut,
		Circ,

		BounceIn,
		BounceOut,
		Bounce
	};

	struct VisualShaderNodeEasing : VisualShaderNode
	{
		enum class ValueType {
			Float,
			Vec2,
			Vec3,
			Vec4
		};

		uint32_t InputConnections[3] = { UINT32_MAX, UINT32_MAX, UINT32_MAX };

		EaseType Ease = EaseType::Linear;
		ValueType Type = ValueType::Vec4;

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

		NODE_CLASS_DECL("Easing");

		void Serialize(YAML::Emitter& out) const override;


		void Deserialize(YAML::Node& node) override;

	};
}
