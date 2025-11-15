#include "KDpch.h"

#include "VisualMaterialNodeConstant.h"

namespace Kaidel
{


	uint32_t VisualMaterialNodeConstant::GetConnectedInputPort(uint32_t port) const
	{
		KD_ASSERT(false, "Constants don't have inputs");
		return -1;
	}


	uint32_t VisualMaterialNodeConstant::GetInputPortCount() const
	{
		return 0;
	}


	std::string_view VisualMaterialNodeConstant::GetInputPortName(uint32_t port) const
	{
		KD_ASSERT(false, "Constants don't have inputs");
		return {};
	}


	Kaidel::VisualMaterialNodePortType VisualMaterialNodeConstant::GetInputPortType(uint32_t port) const
	{
		KD_ASSERT(false, "Constants don't have inputs");
		return VisualMaterialNodePortType::Bool;
	}


	bool VisualMaterialNodeConstant::IsInputPortConnected(uint32_t port) const
	{
		KD_ASSERT(false, "Constants don't have inputs");
		return false;
	}


	void VisualMaterialNodeConstant::SetConnectedInputPort(uint32_t port, uint32_t connection)
	{
		KD_ASSERT(false, "Constants don't have inputs");
	}


	uint32_t VisualMaterialNodeConstant::GetOutputPortCount() const
	{
		return 1;
	}

	std::string_view VisualMaterialNodeBoolConstant::GetCaption() const
	{
		return "Bool";
	}


	std::string_view VisualMaterialNodeBoolConstant::GetOutputPortName(uint32_t port) const
	{
		return "Bool";
	}


	Kaidel::VisualMaterialNodePortType VisualMaterialNodeBoolConstant::GetOutputPortType(uint32_t port) const
	{
		return VisualMaterialNodePortType::Bool;
	}


	std::string VisualMaterialNodeBoolConstant::GenerateCode(const std::vector<std::string>& inputVars) const
	{
		if (Parameter.Active)
			return fmt::format("bool {} = {};\n", NODE_PORT_NAME(0), Parameter.Name);
		return "bool " + NODE_PORT_NAME(0) + " = " + fmt::format("{}", Value) + ";\n";
	}


	VisualMaterialNodeBoolConstant::VisualMaterialNodeBoolConstant()
	{
		Parameter.Type = VisualMaterialParameterType::Bool;
	}


	std::string_view VisualMaterialNodeIntConstant::GetCaption() const
	{
		return "Int";
	}


	std::string_view VisualMaterialNodeIntConstant::GetOutputPortName(uint32_t port) const
	{
		return "Int";
	}


	Kaidel::VisualMaterialNodePortType VisualMaterialNodeIntConstant::GetOutputPortType(uint32_t port) const
	{
		return VisualMaterialNodePortType::Int;
	}


	std::string VisualMaterialNodeIntConstant::GenerateCode(const std::vector<std::string>& inputVars) const
	{
		if (Parameter.Active)
			return fmt::format("int {} = {};\n", NODE_PORT_NAME(0), Parameter.Name);
		return "int " + NODE_PORT_NAME(0) + " = " + fmt::format("{}", Value) + ";\n";
	}


	VisualMaterialNodeIntConstant::VisualMaterialNodeIntConstant()
	{
		Parameter.Type = VisualMaterialParameterType::Int;
	}


	std::string_view VisualMaterialNodeUIntConstant::GetCaption() const
	{
		return "UInt";
	}


	std::string_view VisualMaterialNodeUIntConstant::GetOutputPortName(uint32_t port) const
	{
		return "UInt";
	}


	Kaidel::VisualMaterialNodePortType VisualMaterialNodeUIntConstant::GetOutputPortType(uint32_t port) const
	{
		return VisualMaterialNodePortType::UInt;

	}


	std::string VisualMaterialNodeUIntConstant::GenerateCode(const std::vector<std::string>& inputVars) const
	{
		if (Parameter.Active)
			return fmt::format("uint {} = {};\n", NODE_PORT_NAME(0), Parameter.Name);
		return "uint " + NODE_PORT_NAME(0) + " = " + fmt::format("{}U", Value) + ";\n";
	}


	VisualMaterialNodeUIntConstant::VisualMaterialNodeUIntConstant()
	{
		Parameter.Type = VisualMaterialParameterType::UInt;
	}


	std::string_view VisualMaterialNodeFloatConstant::GetCaption() const
	{
		return "Float";
	}

	std::string_view VisualMaterialNodeFloatConstant::GetOutputPortName(uint32_t port) const
	{
		return "Float";
	}


	Kaidel::VisualMaterialNodePortType VisualMaterialNodeFloatConstant::GetOutputPortType(uint32_t port) const
	{
		return VisualMaterialNodePortType::Float;
	}


	std::string VisualMaterialNodeFloatConstant::GenerateCode(const std::vector<std::string>& inputVars) const
	{
		if (Parameter.Active)
			return fmt::format("float {} = {};\n", NODE_PORT_NAME(0), Parameter.Name);
		return "float " + NODE_PORT_NAME(0) + " = " + fmt::format("{:.3f}", Value) + ";\n";
	}


	VisualMaterialNodeFloatConstant::VisualMaterialNodeFloatConstant()
	{
		Parameter.Type = VisualMaterialParameterType::Float;
	}


	std::string_view VisualMaterialNodeVec2Constant::GetCaption() const
	{
		return "Vec2";
	}


	uint32_t VisualMaterialNodeVec2Constant::GetOutputPortCount() const
	{
		return 3;
	}


	std::string_view VisualMaterialNodeVec2Constant::GetOutputPortName(uint32_t port) const
	{
		switch (port)
		{
		case 0: return "RG";
		case 1: return "R";
		case 2: return "G";
		}
	}


	Kaidel::VisualMaterialNodePortType VisualMaterialNodeVec2Constant::GetOutputPortType(uint32_t port) const
	{
		switch (port)
		{
		case 0: return VisualMaterialNodePortType::Vec2;
		case 1: return VisualMaterialNodePortType::Float;
		case 2: return VisualMaterialNodePortType::Float;
		}
	}


	std::string VisualMaterialNodeVec2Constant::GenerateCode(const std::vector<std::string>& inputVars) const
	{
		if (Parameter.Active)
			return fmt::format("vec2 {} = {};\n", NODE_PORT_NAME(0), Parameter.Name) + EXPANDED_PORTS_DEFINE_VEC2(0);
		return fmt::format("vec2 {} = vec2({:.3f}, {:.3f});\n", NODE_PORT_NAME(0), Value.x, Value.y) + EXPANDED_PORTS_DEFINE_VEC2(0);
	}


	VisualMaterialNodeVec2Constant::VisualMaterialNodeVec2Constant()
	{
		Parameter.Type = VisualMaterialParameterType::Vec2;
	}


	std::string_view VisualMaterialNodeVec3Constant::GetCaption() const
	{
		return "Vec3";
	}


	uint32_t VisualMaterialNodeVec3Constant::GetOutputPortCount() const
	{
		return 4;
	}


	std::string_view VisualMaterialNodeVec3Constant::GetOutputPortName(uint32_t port) const
	{
		switch (port)
		{
		case 0: return "RGB";
		case 1: return "R";
		case 2: return "G";
		case 3: return "B";
		}
	}


	Kaidel::VisualMaterialNodePortType VisualMaterialNodeVec3Constant::GetOutputPortType(uint32_t port) const
	{
		switch (port)
		{
		case 0: return VisualMaterialNodePortType::Vec3;
		case 1: return VisualMaterialNodePortType::Float;
		case 2: return VisualMaterialNodePortType::Float;
		case 3: return VisualMaterialNodePortType::Float;
		}
	}



	std::string VisualMaterialNodeVec3Constant::GenerateCode(const std::vector<std::string>& inputVars) const
	{
		if (Parameter.Active)
			return fmt::format("vec3 {} = {};\n", NODE_PORT_NAME(0), Parameter.Name) + EXPANDED_PORTS_DEFINE_VEC3(0);
		return fmt::format("vec3 {} = vec3({:.3f}, {:.3f}, {:.3f});\n", NODE_PORT_NAME(0), Value.x, Value.y, Value.z) + EXPANDED_PORTS_DEFINE_VEC3(0);
	}


	VisualMaterialNodeVec3Constant::VisualMaterialNodeVec3Constant()
	{
		Parameter.Type = VisualMaterialParameterType::Vec3;
	}


	std::string_view VisualMaterialNodeVec4Constant::GetCaption() const
	{
		return "Vec4";
	}


	uint32_t VisualMaterialNodeVec4Constant::GetOutputPortCount() const
	{
		return 5;
	}


	std::string_view VisualMaterialNodeVec4Constant::GetOutputPortName(uint32_t port) const
	{
		switch (port)
		{
		case 0: return "RGBA";
		case 1: return "R";
		case 2: return "G";
		case 3: return "B";
		case 4: return "A";
		}
	}


	Kaidel::VisualMaterialNodePortType VisualMaterialNodeVec4Constant::GetOutputPortType(uint32_t port) const
	{
		switch (port)
		{
		case 0: return VisualMaterialNodePortType::Vec4;
		case 1: return VisualMaterialNodePortType::Float;
		case 2: return VisualMaterialNodePortType::Float;
		case 3: return VisualMaterialNodePortType::Float;
		case 4: return VisualMaterialNodePortType::Float;
		}
	}


	std::string VisualMaterialNodeVec4Constant::GenerateCode(const std::vector<std::string>& inputVars) const
	{
		if (Parameter.Active)
			return fmt::format("vec4 {} = {};\n", NODE_PORT_NAME(0), Parameter.Name) + EXPANDED_PORTS_DEFINE_VEC4(0);

		return fmt::format("vec4 {} = vec4({:.3f}, {:.3f}, {:.3f}, {:.3f});\n", NODE_PORT_NAME(0), Value.x, Value.y, Value.z, Value.w) + EXPANDED_PORTS_DEFINE_VEC4(0);
	}


	VisualMaterialNodeVec4Constant::VisualMaterialNodeVec4Constant()
	{
		Parameter.Type = VisualMaterialParameterType::Vec4;
	}
}
