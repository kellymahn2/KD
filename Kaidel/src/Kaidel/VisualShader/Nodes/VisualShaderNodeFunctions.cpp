#include "KDpch.h"
#include "VisualShaderNodeFunctions.h"

#include "Kaidel/Core/BiMap.h"

#define DECL_SER_DESER(className, bimap, valType)\
	void className::Serialize(YAML::Emitter& out) const\
	{\
		out << YAML::Key << "Func" << YAML::Value << bimap.At(Func);\
	\
		out << YAML::Key << "Value" << YAML::Value << Value;\
	}\
	\
	void className::Deserialize(YAML::Node& node)\
	{\
		Func = bimap.At(node["Func"].as<std::string>());\
	\
		Value = node["Value"].as<valType>();\
	}

namespace Kaidel {

	namespace Utils
	{
		static BiMap<IntFunc, std::string> s_IntFuncString =
		{
			{ IntFunc::Abs, "Abs" },
			{ IntFunc::Negate, "Negate" },
			{ IntFunc::Sign, "Sign" },
			{ IntFunc::BitwiseNot, "BitwiseNot" },
		};

		static BiMap<UIntFunc, std::string> s_UIntFuncString =
		{
			{ UIntFunc::BitwiseNot, "BitwiseNot"}
		};

		static BiMap<FloatFunc, std::string> s_FloatFuncString =
		{
			{ FloatFunc::Sin, "Sin" },
			{ FloatFunc::Cos, "Cos" },
			{ FloatFunc::Tan, "Tan" },
			{ FloatFunc::ASin, "ASin" },
			{ FloatFunc::ACos, "ACos" },
			{ FloatFunc::ATan, "ATan" },
			{ FloatFunc::Sinh, "Sinh" },
			{ FloatFunc::Cosh, "Cosh" },
			{ FloatFunc::Tanh, "Tanh" },
			{ FloatFunc::Log, "Log" },
			{ FloatFunc::Exp, "Exp" },
			{ FloatFunc::Sqrt, "Sqrt" },
			{ FloatFunc::Abs, "Abs" },
			{ FloatFunc::Sign, "Sign" },
			{ FloatFunc::Floor, "Floor" },
			{ FloatFunc::Round, "Round" },
			{ FloatFunc::Ceil, "Ceil" },
			{ FloatFunc::Fract, "Fract" },
			{ FloatFunc::Saturate, "Saturate" },
			{ FloatFunc::Negate, "Negate" },
			{ FloatFunc::ASinh, "ASinh" },
			{ FloatFunc::ACosh, "ACosh" },
			{ FloatFunc::ATanh, "ATanh" },
			{ FloatFunc::Degrees, "Degrees" },
			{ FloatFunc::Exp2, "Exp2" },
			{ FloatFunc::InvSqrt, "InvSqrt" },
			{ FloatFunc::Log2, "Log2" },
			{ FloatFunc::Radians, "Radians" },
			{ FloatFunc::Reciprocal, "Reciprocal" },
			{ FloatFunc::RoundEven, "RoundEven" },
			{ FloatFunc::Trunc, "Trunc" },
			{ FloatFunc::OneMinus, "OneMinus" },
		};

		static BiMap<VectorFunc, std::string> s_VecFuncString =
		{
			{ VectorFunc::Normalize, "Normalize" },
			{ VectorFunc::Sin, "Sin" },
			{ VectorFunc::Cos, "Cos" },
			{ VectorFunc::Tan, "Tan" },
			{ VectorFunc::ASin, "ASin" },
			{ VectorFunc::ACos, "ACos" },
			{ VectorFunc::ATan, "ATan" },
			{ VectorFunc::Sinh, "Sinh" },
			{ VectorFunc::Cosh, "Cosh" },
			{ VectorFunc::Tanh, "Tanh" },
			{ VectorFunc::Log, "Log" },
			{ VectorFunc::Exp, "Exp" },
			{ VectorFunc::Sqrt, "Sqrt" },
			{ VectorFunc::Abs, "Abs" },
			{ VectorFunc::Sign, "Sign" },
			{ VectorFunc::Floor, "Floor" },
			{ VectorFunc::Round, "Round" },
			{ VectorFunc::Ceil, "Ceil" },
			{ VectorFunc::Fract, "Fract" },
			{ VectorFunc::Saturate, "Saturate" },
			{ VectorFunc::Negate, "Negate" },
			{ VectorFunc::ASinh, "ASinh" },
			{ VectorFunc::ACosh, "ACosh" },
			{ VectorFunc::ATanh, "ATanh" },
			{ VectorFunc::Degrees, "Degrees" },
			{ VectorFunc::Exp2, "Exp2" },
			{ VectorFunc::InvSqrt, "InvSqrt" },
			{ VectorFunc::Log2, "Log2" },
			{ VectorFunc::Radians, "Radians" },
			{ VectorFunc::Reciprocal, "Reciprocal" },
			{ VectorFunc::RoundEven, "RoundEven" },
			{ VectorFunc::Trunc, "Trunc" },
			{ VectorFunc::OneMinus, "OneMinus" },
		};
	}

	static const char* s_IntFuncFormats[] =
	{
		"int {} = abs({});\n",
		"int {} = -{};\n",
		"int {} = sign({});\n",
		"int {} = ~{};\n",
		"int {} = {} * {} + {};\n"
	};

	static const char* s_UIntFuncFormats[] =
	{
		"uint {} = ~{};\n"
	};

	static const char* s_FloatFuncFormats[] =
	{
		"float {} = sin({});\n",
		"float {} = cos({});\n",
		"float {} = tan({});\n",
		"float {} = asin({});\n",
		"float {} = acos({});\n",
		"float {} = atan({});\n",
		"float {} = sinh({});\n",
		"float {} = cosh({});\n",
		"float {} = tanh({});\n",
		"float {} = log({});\n",
		"float {} = exp({});\n",
		"float {} = sqrt({});\n",
		"float {} = abs({});\n",
		"float {} = sign({});\n",
		"float {} = floor({});\n",
		"float {} = round({});\n",
		"float {} = ceil({});\n",
		"float {} = fract({});\n",
		
		"float {} = clamp({}, 0.0, 1.0);\n",
		"float {} = -({});\n",
		"float {} = asinh({});\n",
		"float {} = acosh({});\n",
		"float {} = atanh({});\n",
		"float {} = degrees({});\n",
		"float {} = exp2({});\n",
		"float {} = inversesqrt({});\n",
		"float {} = log2({});\n",
		"float {} = radians({});\n",
		"float {} = 1.0 / ({});\n",
		"float {} = roundEven({});\n",
		"float {} = trunc({});\n",
		"float {} = 1.0 - ({});\n",
	};

	static const char* s_VecFuncFormats[]{
		"{} {} = normalize({});\n",
		"{} {} = sin({});\n",
		"{} {} = cos({});\n",
		"{} {} = tan({});\n",
		"{} {} = asin({});\n",
		"{} {} = acos({});\n",
		"{} {} = atan({});\n",
		"{} {} = sinh({});\n",
		"{} {} = cosh({});\n",
		"{} {} = tanh({});\n",
		"{} {} = log({});\n",
		"{} {} = exp({});\n",
		"{} {} = sqrt({});\n",
		"{} {} = abs({});\n",
		"{} {} = sign({});\n",
		"{} {} = floor({});\n",
		"{} {} = round({});\n",
		"{} {} = ceil({});\n",
		"{} {} = fract({});\n",
		"{} {} = clamp({}, 0.0, 1.0);\n",
		"{} {} = -({});\n",
		"{} {} = asinh({});\n",
		"{} {} = acosh({});\n",
		"{} {} = atanh({});\n",
		"{} {} = degrees({});\n",
		"{} {} = exp2({});\n",
		"{} {} = inversesqrt({});\n",
		"{} {} = log2({});\n",
		"{} {} = radians({});\n",
		"{} {} = 1.0 / ({});\n",
		"{} {} = roundEven({});\n",
		"{} {} = trunc({});\n",
		"{} {} = 1.0 - ({});\n",
	};

	uint32_t VisualShaderNodeFunc::GetConnectedInputPort(uint32_t port) const
	{
		return InputConnection;
	}


	uint32_t VisualShaderNodeFunc::GetInputPortCount() const
	{
		return 1;
	}


	std::string_view VisualShaderNodeFunc::GetInputPortName(uint32_t port) const
	{
		static std::string_view Names[2] = { "x" };
		return Names[port];
	}


	uint32_t VisualShaderNodeFunc::GetOutputPortCount() const
	{
		return 1;
	}


	std::string_view VisualShaderNodeFunc::GetOutputPortName(uint32_t port) const
	{
		return "Result";
	}


	bool VisualShaderNodeFunc::IsInputPortConnected(uint32_t port) const
	{
		return InputConnection != -1;
	}


	void VisualShaderNodeFunc::SetConnectedInputPort(uint32_t port, uint32_t connection)
	{
		InputConnection = connection;
	}


	std::string_view VisualShaderNodeIntFunc::GetCaption() const
	{
		return "Int function";
	}


	VisualShaderNodePortType VisualShaderNodeIntFunc::GetInputPortType(uint32_t port) const
	{
		return VisualShaderNodePortType::Int;
	}


	VisualShaderNodePortType VisualShaderNodeIntFunc::GetOutputPortType(uint32_t port) const
	{
		if (Func == IntFunc::Abs)
			return VisualShaderNodePortType::UInt;
		return VisualShaderNodePortType::Int;
	}


	std::string VisualShaderNodeIntFunc::GenerateCode(const std::vector<std::string>& inputVars) const
	{
		const char* funcFormat = s_IntFuncFormats[(uint32_t)Func];
		bool var0 = !IsInputPortConnected(0);

		std::string code;

		if (var0)
		{
			code = fmt::format(funcFormat, NODE_PORT_NAME(0), fmt::format("{}", Value));
		}
		else
		{
			code = fmt::format(funcFormat, NODE_PORT_NAME(0), inputVars[0]);
		}
		return code;
	}

	DECL_SER_DESER(VisualShaderNodeIntFunc, Utils::s_IntFuncString, int);

	std::string_view VisualShaderNodeUIntFunc::GetCaption() const
	{
		return "UInt function";
	}


	VisualShaderNodePortType VisualShaderNodeUIntFunc::GetInputPortType(uint32_t port) const
	{
		return VisualShaderNodePortType::UInt;
	}


	VisualShaderNodePortType VisualShaderNodeUIntFunc::GetOutputPortType(uint32_t port) const
	{
		return VisualShaderNodePortType::UInt;
	}


	std::string VisualShaderNodeUIntFunc::GenerateCode(const std::vector<std::string>& inputVars) const
	{
		const char* funcFormat = s_UIntFuncFormats[(uint32_t)Func];
		bool var0 = !IsInputPortConnected(0);

		std::string code;

		if (var0)
		{
			code = fmt::format(funcFormat, NODE_PORT_NAME(0), fmt::format("{}U", Value));
		}
		else
		{
			code = fmt::format(funcFormat, NODE_PORT_NAME(0), inputVars[0]);
		}
		return code;
	}

	DECL_SER_DESER(VisualShaderNodeUIntFunc, Utils::s_UIntFuncString, uint32_t);

	std::string_view VisualShaderNodeFloatFunc::GetCaption() const
	{
		return "Float function";
	}


	VisualShaderNodePortType VisualShaderNodeFloatFunc::GetInputPortType(uint32_t port) const
	{
		return VisualShaderNodePortType::Float;
	}


	VisualShaderNodePortType VisualShaderNodeFloatFunc::GetOutputPortType(uint32_t port) const
	{
		return VisualShaderNodePortType::Float;
	}


	std::string VisualShaderNodeFloatFunc::GenerateCode(const std::vector<std::string>& inputVars) const
	{
		const char* funcFormat = s_FloatFuncFormats[(uint32_t)Func];
		bool var0 = !IsInputPortConnected(0);

		std::string code;

		if (var0)
		{
			code = fmt::format(funcFormat, NODE_PORT_NAME(0), fmt::format("{:.3f}", Value));
		}
		else
		{
			code = fmt::format(funcFormat, NODE_PORT_NAME(0), inputVars[0]);
		}
		return code;
	}

	DECL_SER_DESER(VisualShaderNodeFloatFunc, Utils::s_FloatFuncString, float);

	uint32_t VisualShaderNodeVectorFunc::GetOutputPortCount() const
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


	std::string_view VisualShaderNodeVectorFunc::GetOutputPortName(uint32_t port) const
	{
		static std::string_view Names[] = { "R", "G", "B", "A" };

		if (port == 0)
			return "Result";

		return Names[port - 1];
	}


	std::string_view VisualShaderNodeVectorFunc::GetCaption() const
	{
		return "Vector function";
	}


	VisualShaderNodePortType VisualShaderNodeVectorFunc::GetInputPortType(uint32_t port) const
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


	VisualShaderNodePortType VisualShaderNodeVectorFunc::GetOutputPortType(uint32_t port) const
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

	std::string VisualShaderNodeVectorFunc::GenerateCode(const std::vector<std::string>& inputVars) const
	{
		const char* funcFormat = s_VecFuncFormats[(uint32_t)Func];
		bool var0 = !IsInputPortConnected(0);

		std::string code;

		const char* type;

		switch (Type)
		{
		case Kaidel::VectorType::Vec2: type = "vec2"; break;
		case Kaidel::VectorType::Vec3: type = "vec3"; break;
		case Kaidel::VectorType::Vec4: type = "vec4"; break;
		}

		if (var0)
		{
			code = fmt::format(funcFormat, type, NODE_PORT_NAME(0), ToString(Value, Type));
		}
		else
		{
			code = fmt::format(funcFormat, type, NODE_PORT_NAME(0), inputVars[0]);
		}

		switch (Type)
		{
		case Kaidel::VectorType::Vec2: code += EXPANDED_PORTS_DEFINE_VEC2(0); break;
		case Kaidel::VectorType::Vec3: code += EXPANDED_PORTS_DEFINE_VEC3(0); break;
		case Kaidel::VectorType::Vec4: code += EXPANDED_PORTS_DEFINE_VEC4(0); break;
		}
		return code;
	}

	void VisualShaderNodeVectorFunc::Serialize(YAML::Emitter& out) const 
	{
		VisualShaderNodeVector::Serialize(out, Type);

		out << YAML::Key << "Func" << YAML::Value << Utils::s_VecFuncString.At(Func); 

		out << YAML::Key << "Value" << YAML::Value << Value;

	} 
	
	void VisualShaderNodeVectorFunc::Deserialize(YAML::Node& node) 
	{
		Type = VisualShaderNodeVector::Deserialize(node);

		Func = Utils::s_VecFuncString.At(node["Func"].as<std::string>()); 
		
		Value = node["Value"].as<glm::vec4>();
	}

}
