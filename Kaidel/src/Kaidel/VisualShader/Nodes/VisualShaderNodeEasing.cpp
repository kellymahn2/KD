#include "KDpch.h"
#include "VisualShaderNodeEasing.h"

#include "Kaidel/Core/BiMap.h"

namespace Kaidel {

	namespace Utils
	{
		static BiMap<EaseType, std::string> s_EaseTypeString =
		{
			{ EaseType::Linear, "Linear"},
			{ EaseType::SineIn, "SineIn"},
			{ EaseType::SineOut, "SineOut"},
			{ EaseType::Sine, "Sine"},
			{ EaseType::QuadIn, "QuadIn"},
			{ EaseType::QuadOut, "QuadOut"},
			{ EaseType::Quad, "Quad"},
			{ EaseType::CubicIn, "CubicIn"},
			{ EaseType::CubicOut, "CubicOut"},
			{ EaseType::Cubic, "Cubic"},
			{ EaseType::ExpoIn, "ExpoIn"},
			{ EaseType::ExpoOut, "ExpoOut"},
			{ EaseType::Expo, "Expo"},
			{ EaseType::CircIn, "CircIn"},
			{ EaseType::CircOut, "CircOut"},
			{ EaseType::Circ, "Circ"},
			{ EaseType::BounceIn, "BounceIn"},
			{ EaseType::BounceOut, "BounceOut"},
			{ EaseType::Bounce, "Bounce"},
		};

		using VType = VisualShaderNodeEasing::ValueType;
		static BiMap<VType, std::string> s_ValueTypeString =
		{
			{ VType::Float, "Float" },
			{ VType::Vec2, "Vec2" },
			{ VType::Vec3, "Vec3" },
			{ VType::Vec4, "Vec4" },
		};
	}

	std::string VisualShaderNodeEasing::GenerateCode(const std::vector<std::string>& inputVars) const
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
		case Kaidel::VisualShaderNodeEasing::ValueType::Float: type = "float"; break;
		case Kaidel::VisualShaderNodeEasing::ValueType::Vec2:  type = "vec2"; break;
		case Kaidel::VisualShaderNodeEasing::ValueType::Vec3:  type = "vec3"; break;
		case Kaidel::VisualShaderNodeEasing::ValueType::Vec4:  type = "vec4"; break;
		}

		return fmt::format("{} {} = {}({}, {}, {});\n", type, NODE_PORT_NAME(0), func, inputVars[0], inputVars[1], inputVars[2]);
	}


	std::string_view VisualShaderNodeEasing::GetCaption() const
	{
		return "Ease function";
	}


	uint32_t VisualShaderNodeEasing::GetConnectedInputPort(uint32_t port) const
	{
		return InputConnections[port];
	}


	uint32_t VisualShaderNodeEasing::GetInputPortCount() const
	{
		return 3;
	}


	std::string_view VisualShaderNodeEasing::GetInputPortName(uint32_t port) const
	{
		static std::string_view Names[] =
		{
			"a",
			"b",
			"t"
		};

		return Names[port];
	}


	Kaidel::VisualShaderNodePortType VisualShaderNodeEasing::GetInputPortType(uint32_t port) const
	{
		if (port == 2)
		{
			return VisualShaderNodePortType::Float;
		}

		switch (Type)
		{
		case Kaidel::VisualShaderNodeEasing::ValueType::Float: return VisualShaderNodePortType::Float;
		case Kaidel::VisualShaderNodeEasing::ValueType::Vec2: return VisualShaderNodePortType::Vec2;
		case Kaidel::VisualShaderNodeEasing::ValueType::Vec3: return VisualShaderNodePortType::Vec3;
		case Kaidel::VisualShaderNodeEasing::ValueType::Vec4: return VisualShaderNodePortType::Vec4;
		}

		return VisualShaderNodePortType::Bool;
	}


	uint32_t VisualShaderNodeEasing::GetOutputPortCount() const
	{
		return 1;
	}


	std::string_view VisualShaderNodeEasing::GetOutputPortName(uint32_t port) const
	{
		return "Value";
	}


	Kaidel::VisualShaderNodePortType VisualShaderNodeEasing::GetOutputPortType(uint32_t port) const
	{
		switch (Type)
		{
		case Kaidel::VisualShaderNodeEasing::ValueType::Float: return VisualShaderNodePortType::Float;
		case Kaidel::VisualShaderNodeEasing::ValueType::Vec2: return VisualShaderNodePortType::Vec2;
		case Kaidel::VisualShaderNodeEasing::ValueType::Vec3: return VisualShaderNodePortType::Vec3;
		case Kaidel::VisualShaderNodeEasing::ValueType::Vec4: return VisualShaderNodePortType::Vec4;
		}

		KD_CORE_ASSERT(false);
		return VisualShaderNodePortType::Bool;
	}


	bool VisualShaderNodeEasing::IsInputPortConnected(uint32_t port) const
	{
		return InputConnections[port] != -1;
	}


	void VisualShaderNodeEasing::SetConnectedInputPort(uint32_t port, uint32_t connection)
	{
		InputConnections[port] = connection;
	}

	void VisualShaderNodeEasing::Serialize(YAML::Emitter& out) const
	{
		out << YAML::Key << "Ease" << YAML::Value << Utils::s_EaseTypeString.At(Ease);

		out << YAML::Key << "VType" << YAML::Value << Utils::s_ValueTypeString.At(Type);
	}

	void VisualShaderNodeEasing::Deserialize(YAML::Node& node)
	{
		Ease = Utils::s_EaseTypeString.At(node["Ease"].as<std::string>());
		
		Type = Utils::s_ValueTypeString.At(node["VType"].as<std::string>());
	}

}
