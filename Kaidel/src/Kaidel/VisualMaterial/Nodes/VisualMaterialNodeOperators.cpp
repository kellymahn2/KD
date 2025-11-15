#include "KDpch.h"
#include "VisualMaterialNodeOperators.h"

namespace Kaidel {


	static const char* s_IntOpFormats[] =
	{
		"int {} = {} + {};\n",
		"int {} = {} - {};\n",
		"int {} = {} * {};\n",
		"int {} = {} / {};\n",
		"int {} = {} % {};\n",
		"int {} = max({}, {});\n",
		"int {} = min({}, {});\n",
		"int {} = {} & {};\n",
		"int {} = {} | {};\n",
		"int {} = {} ^ {};\n",
		"int {} = {} << {};\n",
		"int {} = {} >> {};\n"
	};

	static const char* s_FloatOpFormats[] =
	{
		"float {} = {} + {};\n",
		"float {} = {} - {};\n",
		"float {} = {} * {};\n",
		"float {} = {} / {};\n",
		"float {} = mod({}, {});\n",
		"float {} = pow({}, {});\n",
		"float {} = max({}, {});\n",
		"float {} = min({}, {});\n",
		"float {} = atan({}, {});\n",
		"float {} = step{}, {});\n",
	};

	static const char* s_VecOpFormats[] =
	{
		"{} {} = {} + {};\n",
		"{} {} = {} - {};\n",
		"{} {} = {} * {};\n",
		"{} {} = {} / {};\n",
		"{} {} = mod({}, {});\n",
		"{} {} = pow({}, {});\n",
		"{} {} = max({}, {});\n",
		"{} {} = min({}, {});\n",
		"{} {} = cross({}, {});\n",
		"{} {} = dot({}, {});\n",
		"{} {} = atan({}, {});\n",
		"{} {} = reflect({}, {});\n",
		"{} {} = step({}, {});\n",
	};

	uint32_t VisualMaterialNodeOp::GetConnectedInputPort(uint32_t port) const
	{
		return InputConnections[port];
	}


	uint32_t VisualMaterialNodeOp::GetInputPortCount() const
	{
		return 2;
	}


	std::string_view VisualMaterialNodeOp::GetInputPortName(uint32_t port) const
	{
		static std::string_view Names[2] = { "a", "b" };
		return Names[port];
	}


	uint32_t VisualMaterialNodeOp::GetOutputPortCount() const
	{
		return 1;
	}


	std::string_view VisualMaterialNodeOp::GetOutputPortName(uint32_t port) const
	{
		return "Result";
	}


	bool VisualMaterialNodeOp::IsInputPortConnected(uint32_t port) const
	{
		return InputConnections[port] != -1;
	}


	void VisualMaterialNodeOp::SetConnectedInputPort(uint32_t port, uint32_t connection)
	{
		InputConnections[port] = connection;
	}


	std::string_view VisualMaterialNodeIntOp::GetCaption() const
	{
		return "Int operator";
	}


	VisualMaterialNodePortType VisualMaterialNodeIntOp::GetInputPortType(uint32_t port) const
	{
		return VisualMaterialNodePortType::Int;
	}


	VisualMaterialNodePortType VisualMaterialNodeIntOp::GetOutputPortType(uint32_t port) const
	{
		return VisualMaterialNodePortType::Int;
	}

	std::string VisualMaterialNodeIntOp::GenerateCode(const std::vector<std::string>& inputVars) const
	{
		const char* opFormat = s_IntOpFormats[(uint32_t)OpType];
		bool var0 = !IsInputPortConnected(0);
		bool var1 = !IsInputPortConnected(1);
		
		std::string code;
		
		if (var0 ^ var1)
		{
			if (var0)
			{
				code = fmt::format(opFormat, NODE_PORT_NAME(0), ValueA, inputVars[0]);
			}
			else
			{
				code = fmt::format(opFormat, NODE_PORT_NAME(0), inputVars[0], ValueB);
			}
		}
		else
		{
			if (var0)
			{
				code = fmt::format(opFormat, NODE_PORT_NAME(0), ValueA, ValueB);
			}
			else
			{
				code = fmt::format(opFormat, NODE_PORT_NAME(0), inputVars[0], inputVars[1]);
			}
		}
		return code;
	}

	std::string_view VisualMaterialNodeUIntOp::GetCaption() const
	{
		return "UInt operator";
	}


	Kaidel::VisualMaterialNodePortType VisualMaterialNodeUIntOp::GetInputPortType(uint32_t port) const
	{
		return VisualMaterialNodePortType::UInt;
	}


	Kaidel::VisualMaterialNodePortType VisualMaterialNodeUIntOp::GetOutputPortType(uint32_t port) const
	{
		return VisualMaterialNodePortType::UInt;
	}


	std::string VisualMaterialNodeUIntOp::GenerateCode(const std::vector<std::string>& inputVars) const
	{
		const char* opFormat = s_IntOpFormats[(uint32_t)OpType];
		bool var0 = !IsInputPortConnected(0);
		bool var1 = !IsInputPortConnected(1);

		std::string code;

		if (var0 ^ var1)
		{
			if (var0)
			{
				code = fmt::format(opFormat, NODE_PORT_NAME(0), fmt::format("{}U", ValueA), inputVars[0]);
			}
			else
			{
				code = fmt::format(opFormat, NODE_PORT_NAME(0), inputVars[0], fmt::format("{}U", ValueB));
			}
		}
		else
		{
			if (var0)
			{
				code = fmt::format(opFormat, NODE_PORT_NAME(0), fmt::format("{}U", ValueA), fmt::format("{}U", ValueB));
			}
			else
			{
				code = fmt::format(opFormat, NODE_PORT_NAME(0), inputVars[0], inputVars[1]);
			}
		}
		return code;
	}

	std::string_view VisualMaterialNodeFloatOp::GetCaption() const
	{
		return "Float operator";
	}


	Kaidel::VisualMaterialNodePortType VisualMaterialNodeFloatOp::GetInputPortType(uint32_t port) const
	{
		return VisualMaterialNodePortType::Float;
	}


	Kaidel::VisualMaterialNodePortType VisualMaterialNodeFloatOp::GetOutputPortType(uint32_t port) const
	{
		return VisualMaterialNodePortType::Float;
	}


	std::string VisualMaterialNodeFloatOp::GenerateCode(const std::vector<std::string>& inputVars) const
	{
		const char* opFormat = s_FloatOpFormats[(uint32_t)OpType];
		bool var0 = !IsInputPortConnected(0);
		bool var1 = !IsInputPortConnected(1);

		std::string code;

		if (var0 ^ var1)
		{
			if (var0)
			{
				code = fmt::format(opFormat, NODE_PORT_NAME(0), fmt::format("{:.3f}", ValueA), inputVars[0]);
			}
			else
			{
				code = fmt::format(opFormat, NODE_PORT_NAME(0), inputVars[0], fmt::format("{:.3f}", ValueB));
			}
		}
		else
		{
			if (var0)
			{
				code = fmt::format(opFormat, NODE_PORT_NAME(0), fmt::format("{:.3f}", ValueA), fmt::format("{:.3f}", ValueB));
			}
			else
			{
				code = fmt::format(opFormat, NODE_PORT_NAME(0), inputVars[0], inputVars[1]);
			}
		}
		return code;
	}

	uint32_t VisualMaterialNodeVectorOp::GetOutputPortCount() const
	{
		switch (Type)
		{
		case Kaidel::VectorType::Vec2: return 3;
		case Kaidel::VectorType::Vec3: return 4;
		case Kaidel::VectorType::Vec4: return 5;
		}
	}


	std::string_view VisualMaterialNodeVectorOp::GetOutputPortName(uint32_t port) const
	{
		static std::string_view Names[] = { "R", "G", "B", "A" };

		if (port == 0)
			return "Result";

		return Names[port - 1];
	}


	std::string_view VisualMaterialNodeVectorOp::GetCaption() const
	{
		return "Vector operator";
	}


	Kaidel::VisualMaterialNodePortType VisualMaterialNodeVectorOp::GetInputPortType(uint32_t port) const
	{
		switch (Type)
		{
		case Kaidel::VectorType::Vec2: return VisualMaterialNodePortType::Vec2;
		case Kaidel::VectorType::Vec3: return VisualMaterialNodePortType::Vec3;
		case Kaidel::VectorType::Vec4: return VisualMaterialNodePortType::Vec4;
		}
	}


	Kaidel::VisualMaterialNodePortType VisualMaterialNodeVectorOp::GetOutputPortType(uint32_t port) const
	{
		switch (Type)
		{
		case Kaidel::VectorType::Vec2: return VisualMaterialNodePortType::Vec2;
		case Kaidel::VectorType::Vec3: return VisualMaterialNodePortType::Vec3;
		case Kaidel::VectorType::Vec4: return VisualMaterialNodePortType::Vec4;
		}
	}


	static std::string ToString(const glm::vec4& val, VectorType type)
	{
		switch (type)
		{
		case Kaidel::VectorType::Vec2: return fmt::format("vec2({:.3f}, {:.3f})", val.x, val.y);
		case Kaidel::VectorType::Vec3: return fmt::format("vec3({:.3f}, {:.3f}, {:.3f})", val.x, val.y, val.z);
		case Kaidel::VectorType::Vec4: return fmt::format("vec4({:.3f}, {:.3f}, {:.3f}, {:.3f})", val.x, val.y, val.z, val.w);
		}

		return "";
	}

	std::string VisualMaterialNodeVectorOp::GenerateCode(const std::vector<std::string>& inputVars) const
	{
		const char* opFormat = s_VecOpFormats[(uint32_t)OpType];
		bool var0 = !IsInputPortConnected(0);
		bool var1 = !IsInputPortConnected(1);

		std::string code;

		const char* type;

		switch (Type)
		{
		case Kaidel::VectorType::Vec2: type = "vec2"; break;
		case Kaidel::VectorType::Vec3: type = "vec3"; break;
		case Kaidel::VectorType::Vec4: type = "vec4"; break;
		}

		if (var0 ^ var1)
		{
			if (var0)
			{
				code = fmt::format(opFormat, type, NODE_PORT_NAME(0), ToString(ValueA, Type), inputVars[0]);
			}
			else
			{
				code = fmt::format(opFormat, type, NODE_PORT_NAME(0), inputVars[0], ToString(ValueB, Type));
			}
		}
		else
		{
			if (var0)
			{
				code = fmt::format(opFormat, type, NODE_PORT_NAME(0), ToString(ValueA, Type), ToString(ValueB, Type));
			}
			else
			{
				code = fmt::format(opFormat, type, NODE_PORT_NAME(0), inputVars[0], inputVars[1]);
			}
		}

		switch (Type)
		{
		case Kaidel::VectorType::Vec2: code += EXPANDED_PORTS_DEFINE_VEC2(0); break;
		case Kaidel::VectorType::Vec3: code += EXPANDED_PORTS_DEFINE_VEC3(0); break;
		case Kaidel::VectorType::Vec4: code += EXPANDED_PORTS_DEFINE_VEC4(0); break;
		}
		return code;
	}

}
