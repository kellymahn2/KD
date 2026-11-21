#include "KDpch.h"
#include "VisualShaderNodeOperators.h"

#include "Kaidel/Core/BiMap.h"

#define DECL_SER_DESER(className, bimap, valType)\
	void className::Serialize(YAML::Emitter& out) const\
	{\
		out << YAML::Key << "OpType" << YAML::Value << bimap.At(OpType);\
		\
		out << YAML::Key << "ValueA" << YAML::Value << ValueA;\
		\
		out << YAML::Key << "ValueB" << YAML::Value << ValueB;\
	}\
	\
	void className::Deserialize(YAML::Node& node)\
	{\
		OpType = bimap.At(node["OpType"].as<std::string>());\
		\
		ValueA = node["ValueA"].as<valType>();\
		\
		ValueB = node["ValueB"].as<valType>();\
	}

namespace Kaidel {

	namespace Utils
	{
		static BiMap<IntOp, std::string> s_IntOpString =
		{
			{ IntOp::Add, "Add" },
			{ IntOp::Subtract, "Subtract" },
			{ IntOp::Multiply, "Multiply" },
			{ IntOp::Divide, "Divide" },
			{ IntOp::Remainder, "Remainder" },
			{ IntOp::Max, "Max" },
			{ IntOp::Min, "Min" },
			{ IntOp::BitwiseAnd, "BitwiseAnd" },
			{ IntOp::BitwiseOr, "BitwiseOr" },
			{ IntOp::BitsiseXor, "BitsiseXor" },
			{ IntOp::LeftShift, "LeftShift" },
			{ IntOp::RightShift, "RightShift" },
		};

		static BiMap<FloatOp, std::string> s_FloatOpString =
		{
			{ FloatOp::Add, "Add" },
			{ FloatOp::Subtract, "Subtract" },
			{ FloatOp::Multiply, "Multiply" },
			{ FloatOp::Divide, "Divide" },
			{ FloatOp::Remainder, "Remainder" },
			{ FloatOp::Power, "Power" },
			{ FloatOp::Max, "Max" },
			{ FloatOp::Min, "Min" },
			{ FloatOp::Atan2, "Atan2" },
			{ FloatOp::Step, "Step" },
		};

		static BiMap<VectorOp, std::string> s_VectorOpString =
		{
			{ VectorOp::Add, "Add" },
			{ VectorOp::Subtract, "Subtract" },
			{ VectorOp::Multiply, "Multiply" },
			{ VectorOp::Divide, "Divide" },
			{ VectorOp::Remainder, "Remainder" },
			{ VectorOp::Power, "Power" },
			{ VectorOp::Max, "Max" },
			{ VectorOp::Min, "Min" },
			{ VectorOp::Cross, "Cross" },
			{ VectorOp::Dot, "Dot" },
			{ VectorOp::Atan2, "Atan2" },
			{ VectorOp::Reflect, "Reflect" },
			{ VectorOp::Step, "Step" },
			{ VectorOp::Lerp, "Lerp" },
		};
	}

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

	uint32_t VisualShaderNodeOp::GetConnectedInputPort(uint32_t port) const
	{
		return InputConnections[port];
	}


	uint32_t VisualShaderNodeOp::GetInputPortCount() const
	{
		return 2;
	}


	std::string_view VisualShaderNodeOp::GetInputPortName(uint32_t port) const
	{
		static std::string_view Names[2] = { "a", "b" };
		return Names[port];
	}


	uint32_t VisualShaderNodeOp::GetOutputPortCount() const
	{
		return 1;
	}


	std::string_view VisualShaderNodeOp::GetOutputPortName(uint32_t port) const
	{
		return "Result";
	}


	bool VisualShaderNodeOp::IsInputPortConnected(uint32_t port) const
	{
		return InputConnections[port] != -1;
	}


	void VisualShaderNodeOp::SetConnectedInputPort(uint32_t port, uint32_t connection)
	{
		InputConnections[port] = connection;
	}


	std::string_view VisualShaderNodeIntOp::GetCaption() const
	{
		return "Int operator";
	}


	VisualShaderNodePortType VisualShaderNodeIntOp::GetInputPortType(uint32_t port) const
	{
		return VisualShaderNodePortType::Int;
	}


	VisualShaderNodePortType VisualShaderNodeIntOp::GetOutputPortType(uint32_t port) const
	{
		return VisualShaderNodePortType::Int;
	}

	std::string VisualShaderNodeIntOp::GenerateCode(const std::vector<std::string>& inputVars) const
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

	DECL_SER_DESER(VisualShaderNodeIntOp, Utils::s_IntOpString, int);

	std::string_view VisualShaderNodeUIntOp::GetCaption() const
	{
		return "UInt operator";
	}

	Kaidel::VisualShaderNodePortType VisualShaderNodeUIntOp::GetInputPortType(uint32_t port) const
	{
		return VisualShaderNodePortType::UInt;
	}


	Kaidel::VisualShaderNodePortType VisualShaderNodeUIntOp::GetOutputPortType(uint32_t port) const
	{
		return VisualShaderNodePortType::UInt;
	}


	std::string VisualShaderNodeUIntOp::GenerateCode(const std::vector<std::string>& inputVars) const
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

	DECL_SER_DESER(VisualShaderNodeUIntOp, Utils::s_IntOpString, uint32_t);

	std::string_view VisualShaderNodeFloatOp::GetCaption() const
	{
		return "Float operator";
	}


	Kaidel::VisualShaderNodePortType VisualShaderNodeFloatOp::GetInputPortType(uint32_t port) const
	{
		return VisualShaderNodePortType::Float;
	}


	Kaidel::VisualShaderNodePortType VisualShaderNodeFloatOp::GetOutputPortType(uint32_t port) const
	{
		return VisualShaderNodePortType::Float;
	}


	std::string VisualShaderNodeFloatOp::GenerateCode(const std::vector<std::string>& inputVars) const
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

	DECL_SER_DESER(VisualShaderNodeFloatOp, Utils::s_FloatOpString, float);

	uint32_t VisualShaderNodeVectorOp::GetOutputPortCount() const
	{
		switch (Type)
		{
		case Kaidel::VectorType::Vec2: return 3;
		case Kaidel::VectorType::Vec3: return 4;
		case Kaidel::VectorType::Vec4: return 5;
		}

		KD_CORE_ASSERT(false);
		return 0;
	}


	std::string_view VisualShaderNodeVectorOp::GetOutputPortName(uint32_t port) const
	{
		static std::string_view Names[] = { "R", "G", "B", "A" };

		if (port == 0)
			return "Result";

		return Names[port - 1];
	}


	std::string_view VisualShaderNodeVectorOp::GetCaption() const
	{
		return "Vector operator";
	}


	Kaidel::VisualShaderNodePortType VisualShaderNodeVectorOp::GetInputPortType(uint32_t port) const
	{
		switch (Type)
		{
		case Kaidel::VectorType::Vec2: return VisualShaderNodePortType::Vec2;
		case Kaidel::VectorType::Vec3: return VisualShaderNodePortType::Vec3;
		case Kaidel::VectorType::Vec4: return VisualShaderNodePortType::Vec4;
		}

		KD_CORE_ASSERT(false);
		return VisualShaderNodePortType::Bool;
	}


	Kaidel::VisualShaderNodePortType VisualShaderNodeVectorOp::GetOutputPortType(uint32_t port) const
	{
		switch (Type)
		{
		case Kaidel::VectorType::Vec2: return VisualShaderNodePortType::Vec2;
		case Kaidel::VectorType::Vec3: return VisualShaderNodePortType::Vec3;
		case Kaidel::VectorType::Vec4: return VisualShaderNodePortType::Vec4;
		}

		KD_CORE_ASSERT(false);
		return VisualShaderNodePortType::Bool;
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

	std::string VisualShaderNodeVectorOp::GenerateCode(const std::vector<std::string>& inputVars) const
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

	void VisualShaderNodeVectorOp::Serialize(YAML::Emitter& out) const 
	{
		VisualShaderNodeVector::Serialize(out, Type);

		out << YAML::Key << "OpType" << YAML::Value << Utils::s_VectorOpString.At(OpType); 
		
		out << YAML::Key << "ValueA" << YAML::Value << ValueA; 
		
		out << YAML::Key << "ValueB" << YAML::Value << ValueB;
	} 
	
	void VisualShaderNodeVectorOp::Deserialize(YAML::Node& node) 
	{
		Type = VisualShaderNodeVector::Deserialize(node);

		OpType = Utils::s_VectorOpString.At(node["OpType"].as<std::string>()); 
		
		ValueA = node["ValueA"].as<glm::vec4>();
		
		ValueB = node["ValueB"].as<glm::vec4>();
	};

}
