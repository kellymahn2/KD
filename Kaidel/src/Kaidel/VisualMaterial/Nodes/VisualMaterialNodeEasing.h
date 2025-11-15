#pragma once

#include "VisualMaterialNode.h"

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

	struct VisualMaterialNodeEasing : VisualMaterialNode
	{
		enum class ValueType {
			Float,
			Vec2,
			Vec3,
			Vec4
		};

		uint32_t InputConnections[3] = { -1U, -1U, -1U };

		EaseType Ease = EaseType::Linear;
		ValueType Type = ValueType::Vec4;

		virtual std::string GenerateCode(const std::vector<std::string>& inputVars) const override;


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

	};
}
