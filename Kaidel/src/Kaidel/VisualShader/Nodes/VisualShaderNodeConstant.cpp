#include "KDpch.h"

#include "VisualShaderNodeConstant.h"

#define DECL_SER_DESER_FOR_CONST(type, valueType, valueName) \
	void type::Serialize(YAML::Emitter& out) const \
	{\
		out << YAML::Key << valueName << YAML::Value << Value;\
	}\
	void type::Deserialize(YAML::Node& node)\
	{\
		Value = node[valueName].as<valueType>();\
	}

namespace Kaidel
{
	uint32_t VisualShaderNodeConstant::GetConnectedInputPort(uint32_t port) const
	{
		KD_ASSERT(false, "Constants don't have inputs");
		return -1;
	}


	uint32_t VisualShaderNodeConstant::GetInputPortCount() const
	{
		return 0;
	}


	std::string_view VisualShaderNodeConstant::GetInputPortName(uint32_t port) const
	{
		KD_ASSERT(false, "Constants don't have inputs");
		return {};
	}


	Kaidel::VisualShaderNodePortType VisualShaderNodeConstant::GetInputPortType(uint32_t port) const
	{
		KD_ASSERT(false, "Constants don't have inputs");
		return VisualShaderNodePortType::Bool;
	}


	bool VisualShaderNodeConstant::IsInputPortConnected(uint32_t port) const
	{
		KD_ASSERT(false, "Constants don't have inputs");
		return false;
	}


	void VisualShaderNodeConstant::SetConnectedInputPort(uint32_t port, uint32_t connection)
	{
		KD_ASSERT(false, "Constants don't have inputs");
	}


	uint32_t VisualShaderNodeConstant::GetOutputPortCount() const
	{
		return 1;
	}

	std::string_view VisualShaderNodeBoolConstant::GetCaption() const
	{
		return "Bool";
	}


	std::string_view VisualShaderNodeBoolConstant::GetOutputPortName(uint32_t port) const
	{
		return "Bool";
	}


	Kaidel::VisualShaderNodePortType VisualShaderNodeBoolConstant::GetOutputPortType(uint32_t port) const
	{
		return VisualShaderNodePortType::Bool;
	}


	std::string VisualShaderNodeBoolConstant::GenerateCode(const std::vector<std::string>& inputVars) const
	{
		if (Parameter.Active)
			return fmt::format("bool {} = {};\n", NODE_PORT_NAME(0), Parameter.Name);
		return "bool " + NODE_PORT_NAME(0) + " = " + fmt::format("{}", Value) + ";\n";
	}


	VisualShaderNodeBoolConstant::VisualShaderNodeBoolConstant()
	{
		Parameter.Type = VisualShaderParameterType::Bool;
	}

	DECL_SER_DESER_FOR_CONST(VisualShaderNodeBoolConstant, bool, "Bool");

	std::string_view VisualShaderNodeIntConstant::GetCaption() const
	{
		return "Int";
	}


	std::string_view VisualShaderNodeIntConstant::GetOutputPortName(uint32_t port) const
	{
		return "Int";
	}


	Kaidel::VisualShaderNodePortType VisualShaderNodeIntConstant::GetOutputPortType(uint32_t port) const
	{
		return VisualShaderNodePortType::Int;
	}


	std::string VisualShaderNodeIntConstant::GenerateCode(const std::vector<std::string>& inputVars) const
	{
		if (Parameter.Active)
			return fmt::format("int {} = {};\n", NODE_PORT_NAME(0), Parameter.Name);
		return "int " + NODE_PORT_NAME(0) + " = " + fmt::format("{}", Value) + ";\n";
	}


	VisualShaderNodeIntConstant::VisualShaderNodeIntConstant()
	{
		Parameter.Type = VisualShaderParameterType::Int;
	}

	DECL_SER_DESER_FOR_CONST(VisualShaderNodeIntConstant, int, "Int");

	std::string_view VisualShaderNodeUIntConstant::GetCaption() const
	{
		return "UInt";
	}

	std::string_view VisualShaderNodeUIntConstant::GetOutputPortName(uint32_t port) const
	{
		return "UInt";
	}


	Kaidel::VisualShaderNodePortType VisualShaderNodeUIntConstant::GetOutputPortType(uint32_t port) const
	{
		return VisualShaderNodePortType::UInt;

	}


	std::string VisualShaderNodeUIntConstant::GenerateCode(const std::vector<std::string>& inputVars) const
	{
		if (Parameter.Active)
			return fmt::format("uint {} = {};\n", NODE_PORT_NAME(0), Parameter.Name);
		return "uint " + NODE_PORT_NAME(0) + " = " + fmt::format("{}U", Value) + ";\n";
	}


	VisualShaderNodeUIntConstant::VisualShaderNodeUIntConstant()
	{
		Parameter.Type = VisualShaderParameterType::UInt;
	}

	DECL_SER_DESER_FOR_CONST(VisualShaderNodeUIntConstant, uint32_t, "UInt");

	std::string_view VisualShaderNodeFloatConstant::GetCaption() const
	{
		return "Float";
	}

	std::string_view VisualShaderNodeFloatConstant::GetOutputPortName(uint32_t port) const
	{
		return "Float";
	}


	Kaidel::VisualShaderNodePortType VisualShaderNodeFloatConstant::GetOutputPortType(uint32_t port) const
	{
		return VisualShaderNodePortType::Float;
	}


	std::string VisualShaderNodeFloatConstant::GenerateCode(const std::vector<std::string>& inputVars) const
	{
		if (Parameter.Active)
			return fmt::format("float {} = {};\n", NODE_PORT_NAME(0), Parameter.Name);
		return "float " + NODE_PORT_NAME(0) + " = " + fmt::format("{:.3f}", Value) + ";\n";
	}


	VisualShaderNodeFloatConstant::VisualShaderNodeFloatConstant()
	{
		Parameter.Type = VisualShaderParameterType::Float;
	}

	DECL_SER_DESER_FOR_CONST(VisualShaderNodeFloatConstant, float, "Float");

	std::string_view VisualShaderNodeVec2Constant::GetCaption() const
	{
		return "Vec2";
	}


	uint32_t VisualShaderNodeVec2Constant::GetOutputPortCount() const
	{
		return 3;
	}


	std::string_view VisualShaderNodeVec2Constant::GetOutputPortName(uint32_t port) const
	{
		switch (port)
		{
		case 0: return "RG";
		case 1: return "R";
		case 2: return "G";
		}

		KD_CORE_ASSERT(false);
		return "";
	}


	Kaidel::VisualShaderNodePortType VisualShaderNodeVec2Constant::GetOutputPortType(uint32_t port) const
	{
		switch (port)
		{
		case 0: return VisualShaderNodePortType::Vec2;
		case 1: return VisualShaderNodePortType::Float;
		case 2: return VisualShaderNodePortType::Float;
		}

		KD_CORE_ASSERT(false);
		return VisualShaderNodePortType::Bool;
	}


	std::string VisualShaderNodeVec2Constant::GenerateCode(const std::vector<std::string>& inputVars) const
	{
		if (Parameter.Active)
			return fmt::format("vec2 {} = {};\n", NODE_PORT_NAME(0), Parameter.Name) + EXPANDED_PORTS_DEFINE_VEC2(0);
		return fmt::format("vec2 {} = vec2({:.3f}, {:.3f});\n", NODE_PORT_NAME(0), Value.x, Value.y) + EXPANDED_PORTS_DEFINE_VEC2(0);
	}


	VisualShaderNodeVec2Constant::VisualShaderNodeVec2Constant()
	{
		Parameter.Type = VisualShaderParameterType::Vec2;
	}

	DECL_SER_DESER_FOR_CONST(VisualShaderNodeVec2Constant, glm::vec2, "Vec2");

	std::string_view VisualShaderNodeVec3Constant::GetCaption() const
	{
		return "Vec3";
	}


	uint32_t VisualShaderNodeVec3Constant::GetOutputPortCount() const
	{
		return 4;
	}


	std::string_view VisualShaderNodeVec3Constant::GetOutputPortName(uint32_t port) const
	{
		switch (port)
		{
		case 0: return "RGB";
		case 1: return "R";
		case 2: return "G";
		case 3: return "B";
		}

		KD_CORE_ASSERT("Incorrect Parameter");

		return "";
	}


	Kaidel::VisualShaderNodePortType VisualShaderNodeVec3Constant::GetOutputPortType(uint32_t port) const
	{
		switch (port)
		{
		case 0: return VisualShaderNodePortType::Vec3;
		case 1: return VisualShaderNodePortType::Float;
		case 2: return VisualShaderNodePortType::Float;
		case 3: return VisualShaderNodePortType::Float;
		}

		KD_CORE_ASSERT(false);
		return VisualShaderNodePortType::Bool;
	}

	std::string VisualShaderNodeVec3Constant::GenerateCode(const std::vector<std::string>& inputVars) const
	{
		if (Parameter.Active)
			return fmt::format("vec3 {} = {};\n", NODE_PORT_NAME(0), Parameter.Name) + EXPANDED_PORTS_DEFINE_VEC3(0);
		return fmt::format("vec3 {} = vec3({:.3f}, {:.3f}, {:.3f});\n", NODE_PORT_NAME(0), Value.x, Value.y, Value.z) + EXPANDED_PORTS_DEFINE_VEC3(0);
	}


	VisualShaderNodeVec3Constant::VisualShaderNodeVec3Constant()
	{
		Parameter.Type = VisualShaderParameterType::Vec3;
	}

	DECL_SER_DESER_FOR_CONST(VisualShaderNodeVec3Constant, glm::vec3, "Vec3");

	std::string_view VisualShaderNodeVec4Constant::GetCaption() const
	{
		return "Vec4";
	}


	uint32_t VisualShaderNodeVec4Constant::GetOutputPortCount() const
	{
		return 5;
	}


	std::string_view VisualShaderNodeVec4Constant::GetOutputPortName(uint32_t port) const
	{
		switch (port)
		{
		case 0: return "RGBA";
		case 1: return "R";
		case 2: return "G";
		case 3: return "B";
		case 4: return "A";
		}

		KD_CORE_ASSERT(false);
		return "";
	}


	Kaidel::VisualShaderNodePortType VisualShaderNodeVec4Constant::GetOutputPortType(uint32_t port) const
	{
		switch (port)
		{
		case 0: return VisualShaderNodePortType::Vec4;
		case 1: return VisualShaderNodePortType::Float;
		case 2: return VisualShaderNodePortType::Float;
		case 3: return VisualShaderNodePortType::Float;
		case 4: return VisualShaderNodePortType::Float;
		}

		KD_CORE_ASSERT(false);
		return VisualShaderNodePortType::Bool;
	}


	std::string VisualShaderNodeVec4Constant::GenerateCode(const std::vector<std::string>& inputVars) const
	{
		if (Parameter.Active)
			return fmt::format("vec4 {} = {};\n", NODE_PORT_NAME(0), Parameter.Name) + EXPANDED_PORTS_DEFINE_VEC4(0);

		return fmt::format("vec4 {} = vec4({:.3f}, {:.3f}, {:.3f}, {:.3f});\n", NODE_PORT_NAME(0), Value.x, Value.y, Value.z, Value.w) + EXPANDED_PORTS_DEFINE_VEC4(0);
	}


	VisualShaderNodeVec4Constant::VisualShaderNodeVec4Constant()
	{
		Parameter.Type = VisualShaderParameterType::Vec4;
	}

	DECL_SER_DESER_FOR_CONST(VisualShaderNodeVec4Constant, glm::vec4, "Vec4");
	
}
