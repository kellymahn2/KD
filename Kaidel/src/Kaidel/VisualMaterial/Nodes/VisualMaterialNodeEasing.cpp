#include "KDpch.h"
#include "VisualMaterialNodeEasing.h"

namespace Kaidel {


	std::string VisualMaterialNodeEasing::GenerateCode(const std::vector<std::string>& inputVars) const
	{
		std::string func;
		switch (Ease)
		{
		case Kaidel::EaseType::Linear:		func = "InterpLinear"	; break;
		case Kaidel::EaseType::SineIn:		func = "InterpSineIn"	; break;
		case Kaidel::EaseType::SineOut:		func = "InterpSineOut"	; break;
		case Kaidel::EaseType::Sine:		func = "InterpSine"		; break;
		case Kaidel::EaseType::QuadIn:		func = "InterpQuadIn"	; break;
		case Kaidel::EaseType::QuadOut:		func = "InterpQuadOut"	; break;
		case Kaidel::EaseType::Quad:		func = "InterpQuad"		; break;
		case Kaidel::EaseType::CubicIn:		func = "InterpCubicIn"	; break;
		case Kaidel::EaseType::CubicOut:	func = "InterpCubicOut"	; break;
		case Kaidel::EaseType::Cubic:		func = "InterpCubic"	; break;
		case Kaidel::EaseType::ExpoIn:		func = "InterpExpoIn"	; break;
		case Kaidel::EaseType::ExpoOut:		func = "InterpExpoOut"	; break;
		case Kaidel::EaseType::Expo:		func = "InterpExpo"		; break;
		case Kaidel::EaseType::CircIn:		func = "InterpCircIn"	; break;
		case Kaidel::EaseType::CircOut:		func = "InterpCircOut"	; break;
		case Kaidel::EaseType::Circ:		func = "InterpCirc"		; break;
		case Kaidel::EaseType::BounceIn:	func = "InterpBounceIn"	; break;
		case Kaidel::EaseType::BounceOut:	func = "InterpBounceOut"; break;
		case Kaidel::EaseType::Bounce:		func = "InterpBounce"	; break;
		}

		std::string type;
		switch (Type)
		{
		case Kaidel::VisualMaterialNodeEasing::ValueType::Float: type = "float"; break;
		case Kaidel::VisualMaterialNodeEasing::ValueType::Vec2:  type = "vec2"; break;
		case Kaidel::VisualMaterialNodeEasing::ValueType::Vec3:  type = "vec3"; break;
		case Kaidel::VisualMaterialNodeEasing::ValueType::Vec4:  type = "vec4"; break;
		}

		return fmt::format("{} {} = {}({}, {}, {});\n", type, NODE_PORT_NAME(0), func, inputVars[0], inputVars[1], inputVars[2]);
	}


	std::string_view VisualMaterialNodeEasing::GetCaption() const
	{
		return "Ease function";
	}


	uint32_t VisualMaterialNodeEasing::GetConnectedInputPort(uint32_t port) const
	{
		return InputConnections[port];
	}


	uint32_t VisualMaterialNodeEasing::GetInputPortCount() const
	{
		return 3;
	}


	std::string_view VisualMaterialNodeEasing::GetInputPortName(uint32_t port) const
	{
		static std::string_view Names[] =
		{
			"a",
			"b",
			"t"
		};

		return Names[port];
	}


	Kaidel::VisualMaterialNodePortType VisualMaterialNodeEasing::GetInputPortType(uint32_t port) const
	{
		if (port == 2)
		{
			return VisualMaterialNodePortType::Float;
		}

		switch (Type)
		{
		case Kaidel::VisualMaterialNodeEasing::ValueType::Float: return VisualMaterialNodePortType::Float;
		case Kaidel::VisualMaterialNodeEasing::ValueType::Vec2: return VisualMaterialNodePortType::Vec2;
		case Kaidel::VisualMaterialNodeEasing::ValueType::Vec3: return VisualMaterialNodePortType::Vec3;
		case Kaidel::VisualMaterialNodeEasing::ValueType::Vec4: return VisualMaterialNodePortType::Vec4;
		}

		return VisualMaterialNodePortType::Bool;
	}


	uint32_t VisualMaterialNodeEasing::GetOutputPortCount() const
	{
		return 1;
	}


	std::string_view VisualMaterialNodeEasing::GetOutputPortName(uint32_t port) const
	{
		return "Value";
	}


	Kaidel::VisualMaterialNodePortType VisualMaterialNodeEasing::GetOutputPortType(uint32_t port) const
	{
		switch (Type)
		{
		case Kaidel::VisualMaterialNodeEasing::ValueType::Float: return VisualMaterialNodePortType::Float;
		case Kaidel::VisualMaterialNodeEasing::ValueType::Vec2: return VisualMaterialNodePortType::Vec2;
		case Kaidel::VisualMaterialNodeEasing::ValueType::Vec3: return VisualMaterialNodePortType::Vec3;
		case Kaidel::VisualMaterialNodeEasing::ValueType::Vec4: return VisualMaterialNodePortType::Vec4;
		}
	}


	bool VisualMaterialNodeEasing::IsInputPortConnected(uint32_t port) const
	{
		return InputConnections[port] != -1;
	}


	void VisualMaterialNodeEasing::SetConnectedInputPort(uint32_t port, uint32_t connection)
	{
		InputConnections[port] = connection;
	}

}
