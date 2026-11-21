#include "KDpch.h"
#include "VisualShaderNodeConditionals.h"
#include "Kaidel/Core/BiMap.h"

namespace Kaidel {

	namespace Utils
	{
		using CType = VisualShaderNodeCompare::CompareType;
		BiMap<CType, std::string> s_CompareTypeString =
		{
			{CType::Int, "Int"},
			{CType::UInt, "UInt"},
			{CType::Float, "Float"},
			{CType::Vec2, "Vec2"},
			{CType::Vec3, "Vec3"},
			{CType::Vec4, "Vec4"},
		};

		using CFunc = VisualShaderNodeCompare::CompareFunc;
		BiMap<CFunc, std::string> s_CompareFuncString =
		{
			{CFunc::Equal, "Equal"},
			{CFunc::NotEqual, "NotEqual"},
			{CFunc::Greater, "Greater"},
			{CFunc::GreaterEqual, "GreaterEqual"},
			{CFunc::Less, "Less"},
			{CFunc::LessEqual, "LessEqual"},
		};

		using CCond = VisualShaderNodeCompare::CompareCond;
		BiMap<CCond, std::string> s_CompareCondString =
		{
			{CCond::Any, "Any"},
			{CCond::All, "All"},
		};


		using SType = VisualShaderNodeSwitch::SwitchType;
		BiMap<SType, std::string> s_SwitchTypeString =
		{
			{SType::Int, "Int"},
			{SType::UInt, "UInt"},
			{SType::Float, "Float"},
			{SType::Vec2, "Vec2"},
			{SType::Vec3, "Vec3"},
			{SType::Vec4, "Vec4"},
		};

	}

	static const char* s_CompareFormats[] = {
		"({} == {})",
		"({} != {})",
		"({} > {})",
		"({} >= {})",
		"({} < {})",
		"({} <= {})"
	};

	static std::string FormatValue(const VisualShaderNodeCompare::ValueType& value, VisualShaderNodeCompare::CompareType type)
	{
		switch (type)
		{
		case Kaidel::VisualShaderNodeCompare::CompareType::Int: return fmt::format("{}", value.Int);
		case Kaidel::VisualShaderNodeCompare::CompareType::UInt: return fmt::format("{}U", value.UInt);
		case Kaidel::VisualShaderNodeCompare::CompareType::Float: return fmt::format("{:.3f}", value.Float);
		case Kaidel::VisualShaderNodeCompare::CompareType::Vec2: return fmt::format("vec2({:.3f}, {:.3f})", value.Vec2.x, value.Vec2.y);
		case Kaidel::VisualShaderNodeCompare::CompareType::Vec3: return fmt::format("vec3({:.3f}, {:.3f}, {:.3f})", value.Vec3.x, value.Vec3.y, value.Vec3.z);
		case Kaidel::VisualShaderNodeCompare::CompareType::Vec4: return fmt::format("vec4({:.3f}, {:.3f}, {:.3f}, {:.3f})", value.Vec4.x, value.Vec4.y, value.Vec4.z, value.Vec4.w);
		}

		KD_CORE_ASSERT(false);
		return "";
	}

	static std::string FormatValue(const VisualShaderNodeSwitch::ValueType& value, VisualShaderNodeSwitch::SwitchType type)
	{
		switch (type)
		{
		case Kaidel::VisualShaderNodeSwitch::SwitchType::Int: return fmt::format("{}", value.Int);
		case Kaidel::VisualShaderNodeSwitch::SwitchType::UInt: return fmt::format("{}U", value.UInt);
		case Kaidel::VisualShaderNodeSwitch::SwitchType::Float: return fmt::format("{:.3f}", value.Float);
		case Kaidel::VisualShaderNodeSwitch::SwitchType::Vec2: return fmt::format("vec2({:.3f}, {:.3f})", value.Vec2.x, value.Vec2.y);
		case Kaidel::VisualShaderNodeSwitch::SwitchType::Vec3: return fmt::format("vec3({:.3f}, {:.3f}, {:.3f})", value.Vec3.x, value.Vec3.y, value.Vec3.z);
		case Kaidel::VisualShaderNodeSwitch::SwitchType::Vec4: return fmt::format("vec4({:.3f}, {:.3f}, {:.3f}, {:.3f})", value.Vec4.x, value.Vec4.y, value.Vec4.z, value.Vec4.w);
		}

		KD_CORE_ASSERT(false);
		return "";
	}

	std::string VisualShaderNodeCompare::GenerateCode(const std::vector<std::string>& inputVars) const
	{
		const char* format = s_CompareFormats[(uint32_t)CFunc];
		std::string var0, var1;
		bool v0 = !IsInputPortConnected(0);
		bool v1 = !IsInputPortConnected(1);

		if (v0 ^ v1)
		{
			if (v0)
			{
				var0 = FormatValue(ValueA, CType);
				var1 = inputVars[0];
			}
			else
			{
				var0 = inputVars[0];
				var1 = FormatValue(ValueB, CType);
			}
		}
		else
		{
			if (v0)
			{
				var0 = FormatValue(ValueA, CType);
				var1 = FormatValue(ValueB, CType);
			}
			else
			{
				var0 = inputVars[0];
				var1 = inputVars[1];
			}
		}

		std::string cond = fmt::format(format, var0, var1);

		if (CType == CompareType::Vec2 || CType == CompareType::Vec3 || CType == CompareType::Vec4)
		{
			switch (CCond)
			{
			case Kaidel::VisualShaderNodeCompare::Any: cond = fmt::format("any({})", cond); break;
			case Kaidel::VisualShaderNodeCompare::All: cond = fmt::format("all({})", cond); break;
			}
		}

		return fmt::format("bool {} = bool({});\n", NODE_PORT_NAME(0), cond);
	}


	std::string_view VisualShaderNodeCompare::GetCaption() const
	{
		return "Compare";
	}


	uint32_t VisualShaderNodeCompare::GetConnectedInputPort(uint32_t port) const
	{
		return InputConnections[port];
	}


	uint32_t VisualShaderNodeCompare::GetInputPortCount() const
	{
		return 2;
	}


	std::string_view VisualShaderNodeCompare::GetInputPortName(uint32_t port) const
	{
		static std::string_view Names[] = { "a", "b" };

		return Names[port];
	}


	Kaidel::VisualShaderNodePortType VisualShaderNodeCompare::GetInputPortType(uint32_t port) const
	{
		switch (CType)
		{
		case Kaidel::VisualShaderNodeCompare::CompareType::Int: return VisualShaderNodePortType::Int;
		case Kaidel::VisualShaderNodeCompare::CompareType::UInt: return VisualShaderNodePortType::UInt;
		case Kaidel::VisualShaderNodeCompare::CompareType::Float: return VisualShaderNodePortType::Float;
		case Kaidel::VisualShaderNodeCompare::CompareType::Vec2: return VisualShaderNodePortType::Vec2;
		case Kaidel::VisualShaderNodeCompare::CompareType::Vec3: return VisualShaderNodePortType::Vec3;
		case Kaidel::VisualShaderNodeCompare::CompareType::Vec4: return VisualShaderNodePortType::Vec4;
		}

		return VisualShaderNodePortType::Bool;
	}


	uint32_t VisualShaderNodeCompare::GetOutputPortCount() const
	{
		return 1;
	}


	std::string_view VisualShaderNodeCompare::GetOutputPortName(uint32_t port) const
	{
		return "Bool";
	}


	Kaidel::VisualShaderNodePortType VisualShaderNodeCompare::GetOutputPortType(uint32_t port) const
	{
		return VisualShaderNodePortType::Bool;
	}


	bool VisualShaderNodeCompare::IsInputPortConnected(uint32_t port) const
	{
		return InputConnections[port] != -1;
	}


	void VisualShaderNodeCompare::SetConnectedInputPort(uint32_t port, uint32_t connection)
	{
		InputConnections[port] = connection;
	}

	void VisualShaderNodeCompare::Serialize(YAML::Emitter& out) const
	{
		out << YAML::Key << "CType" << YAML::Value << Utils::s_CompareTypeString.At(CType);
		
		out << YAML::Key << "CFunc" << YAML::Value << Utils::s_CompareFuncString.At(CFunc);
		
		out << YAML::Key << "CCond" << YAML::Value << Utils::s_CompareCondString.At(CCond);

		out << YAML::Key << "ValueA" << YAML::Value << ValueA.Vec4;

		out << YAML::Key << "ValueB" << YAML::Value << ValueB.Vec4;
	}

	void VisualShaderNodeCompare::Deserialize(YAML::Node& node)
	{
		CType = Utils::s_CompareTypeString.At(node["CType"].as<std::string>());
		
		CFunc = Utils::s_CompareFuncString.At(node["CFunc"].as<std::string>());
		
		CCond = Utils::s_CompareCondString.At(node["CCond"].as<std::string>());

		ValueA.Vec4 = node["ValueA"].as<glm::vec4>();

		ValueB.Vec4 = node["ValueB"].as<glm::vec4>();
	}

	std::string VisualShaderNodeSwitch::GenerateCode(const std::vector<std::string>& inputVars) const
	{
		std::string var0, var1;
		bool v0 = !IsInputPortConnected(0);
		bool v1 = !IsInputPortConnected(1);

		if (v0 ^ v1)
		{
			if (v0)
			{
				var0 = FormatValue(ValueA, SType);
				var1 = inputVars[0];
			}
			else
			{
				var0 = inputVars[0];
				var1 = FormatValue(ValueB, SType);
			}
		}
		else
		{
			if (v0)
			{
				var0 = FormatValue(ValueA, SType);
				var1 = FormatValue(ValueB, SType);
			}
			else
			{
				var0 = inputVars[0];
				var1 = inputVars[1];
			}
		}

		std::string cond;
		if (IsInputPortConnected(2))
		{
			cond = inputVars.back();
		}
		else
		{
			cond = fmt::format("{}", Condition ? "true" : "false");
		}

		std::string type;
		switch (SType)
		{
		case Kaidel::VisualShaderNodeSwitch::SwitchType::Int: type = "int"; break;
		case Kaidel::VisualShaderNodeSwitch::SwitchType::UInt: type = "uint"; break;
		case Kaidel::VisualShaderNodeSwitch::SwitchType::Float: type = "float"; break;
		case Kaidel::VisualShaderNodeSwitch::SwitchType::Vec2: type = "vec2"; break;
		case Kaidel::VisualShaderNodeSwitch::SwitchType::Vec3: type = "vec3"; break;
		case Kaidel::VisualShaderNodeSwitch::SwitchType::Vec4: type = "vec4"; break;
		}

		return fmt::format("{} {} = ({}) ? {} : {};\n", type, NODE_PORT_NAME(0), cond, var0, var1);
	}


	std::string_view VisualShaderNodeSwitch::GetCaption() const
	{
		return "Switch";
	}


	uint32_t VisualShaderNodeSwitch::GetConnectedInputPort(uint32_t port) const
	{
		return InputConnections[port];
	}


	uint32_t VisualShaderNodeSwitch::GetInputPortCount() const
	{
		return 3;
	}


	std::string_view VisualShaderNodeSwitch::GetInputPortName(uint32_t port) const
	{
		static std::string_view Names[] = 
		{
			"a",
			"b",
			"condition"
		};

		return Names[port];
	}


	Kaidel::VisualShaderNodePortType VisualShaderNodeSwitch::GetInputPortType(uint32_t port) const
	{
		if (port == 2)
			return VisualShaderNodePortType::Bool;

		switch (SType)
		{
		case Kaidel::VisualShaderNodeSwitch::SwitchType::Int: return VisualShaderNodePortType::Int;
		case Kaidel::VisualShaderNodeSwitch::SwitchType::UInt: return VisualShaderNodePortType::UInt;
		case Kaidel::VisualShaderNodeSwitch::SwitchType::Float: return VisualShaderNodePortType::Float;
		case Kaidel::VisualShaderNodeSwitch::SwitchType::Vec2: return VisualShaderNodePortType::Vec2;
		case Kaidel::VisualShaderNodeSwitch::SwitchType::Vec3: return VisualShaderNodePortType::Vec3;
		case Kaidel::VisualShaderNodeSwitch::SwitchType::Vec4: return VisualShaderNodePortType::Vec4;
		}

		return VisualShaderNodePortType::Bool;
	}


	uint32_t VisualShaderNodeSwitch::GetOutputPortCount() const
	{
		return 1;
	}


	std::string_view VisualShaderNodeSwitch::GetOutputPortName(uint32_t port) const
	{
		return "Value";
	}


	Kaidel::VisualShaderNodePortType VisualShaderNodeSwitch::GetOutputPortType(uint32_t port) const
	{
		switch (SType)
		{
		case Kaidel::VisualShaderNodeSwitch::SwitchType::Int: return VisualShaderNodePortType::Int;
		case Kaidel::VisualShaderNodeSwitch::SwitchType::UInt: return VisualShaderNodePortType::UInt;
		case Kaidel::VisualShaderNodeSwitch::SwitchType::Float: return VisualShaderNodePortType::Float;
		case Kaidel::VisualShaderNodeSwitch::SwitchType::Vec2: return VisualShaderNodePortType::Vec2;
		case Kaidel::VisualShaderNodeSwitch::SwitchType::Vec3: return VisualShaderNodePortType::Vec3;
		case Kaidel::VisualShaderNodeSwitch::SwitchType::Vec4: return VisualShaderNodePortType::Vec4;
		}

		return VisualShaderNodePortType::Bool;
	}


	bool VisualShaderNodeSwitch::IsInputPortConnected(uint32_t port) const
	{
		return InputConnections[port] != -1;
	}


	void VisualShaderNodeSwitch::SetConnectedInputPort(uint32_t port, uint32_t connection)
	{
		InputConnections[port] = connection;
	}

	void VisualShaderNodeSwitch::Serialize(YAML::Emitter& out) const
	{
		out << YAML::Key << "SType" << YAML::Value << Utils::s_SwitchTypeString.At(SType);

		out << YAML::Key << "Condition" << YAML::Value << Condition;

		out << YAML::Key << "ValueA" << YAML::Value << ValueA.Vec4;
		
		out << YAML::Key << "ValueB" << YAML::Value << ValueB.Vec4;
	}

	void VisualShaderNodeSwitch::Deserialize(YAML::Node& node)
	{
		SType = Utils::s_SwitchTypeString.At(node["SType"].as<std::string>());

		Condition = node["Condition"].as<bool>();
		
		ValueA.Vec4 = node["ValueA"].as<glm::vec4>();
		
		ValueB.Vec4 = node["ValueB"].as<glm::vec4>();
	}

}
