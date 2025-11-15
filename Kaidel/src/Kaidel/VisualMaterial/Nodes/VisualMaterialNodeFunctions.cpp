#include "KDpch.h"
#include "VisualMaterialNodeFunctions.h"


namespace Kaidel {

	static const char* s_IntFuncFormats[] =
	{
		"int {} = abs({});\n",
		"int {} = -{};\n",
		"int {} = sign({});\n",
		"int {} = ~{};\n"
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
		"float {} = 1.0 - ({});\n"
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
		"{} {} = 1.0 - ({});\n"
	};

	uint32_t VisualMaterialNodeFunc::GetConnectedInputPort(uint32_t port) const
	{
		return InputConnection;
	}


	uint32_t VisualMaterialNodeFunc::GetInputPortCount() const
	{
		return 1;
	}


	std::string_view VisualMaterialNodeFunc::GetInputPortName(uint32_t port) const
	{
		static std::string_view Names[2] = { "x" };
		return Names[port];
	}


	uint32_t VisualMaterialNodeFunc::GetOutputPortCount() const
	{
		return 1;
	}


	std::string_view VisualMaterialNodeFunc::GetOutputPortName(uint32_t port) const
	{
		return "Result";
	}


	bool VisualMaterialNodeFunc::IsInputPortConnected(uint32_t port) const
	{
		return InputConnection != -1;
	}


	void VisualMaterialNodeFunc::SetConnectedInputPort(uint32_t port, uint32_t connection)
	{
		InputConnection = connection;
	}


	std::string_view VisualMaterialNodeIntFunc::GetCaption() const
	{
		return "Int function";
	}


	VisualMaterialNodePortType VisualMaterialNodeIntFunc::GetInputPortType(uint32_t port) const
	{
		return VisualMaterialNodePortType::Int;
	}


	VisualMaterialNodePortType VisualMaterialNodeIntFunc::GetOutputPortType(uint32_t port) const
	{
		if (Func == IntFunc::Abs)
			return VisualMaterialNodePortType::UInt;
		return VisualMaterialNodePortType::Int;
	}


	std::string VisualMaterialNodeIntFunc::GenerateCode(const std::vector<std::string>& inputVars) const
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

	std::string_view VisualMaterialNodeUIntFunc::GetCaption() const
	{
		return "UInt function";
	}


	VisualMaterialNodePortType VisualMaterialNodeUIntFunc::GetInputPortType(uint32_t port) const
	{
		return VisualMaterialNodePortType::UInt;
	}


	VisualMaterialNodePortType VisualMaterialNodeUIntFunc::GetOutputPortType(uint32_t port) const
	{
		return VisualMaterialNodePortType::UInt;
	}


	std::string VisualMaterialNodeUIntFunc::GenerateCode(const std::vector<std::string>& inputVars) const
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

	std::string_view VisualMaterialNodeFloatFunc::GetCaption() const
	{
		return "Float function";
	}


	VisualMaterialNodePortType VisualMaterialNodeFloatFunc::GetInputPortType(uint32_t port) const
	{
		return VisualMaterialNodePortType::Float;
	}


	VisualMaterialNodePortType VisualMaterialNodeFloatFunc::GetOutputPortType(uint32_t port) const
	{
		return VisualMaterialNodePortType::Float;
	}


	std::string VisualMaterialNodeFloatFunc::GenerateCode(const std::vector<std::string>& inputVars) const
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

	uint32_t VisualMaterialNodeVectorFunc::GetOutputPortCount() const
	{
		switch (Type)
		{
		case Kaidel::VectorType::Vec2: return 3;
		case Kaidel::VectorType::Vec3: return 4;
		case Kaidel::VectorType::Vec4: return 5;
		}
	}


	std::string_view VisualMaterialNodeVectorFunc::GetOutputPortName(uint32_t port) const
	{
		static std::string_view Names[] = { "R", "G", "B", "A" };

		if (port == 0)
			return "Result";

		return Names[port - 1];
	}


	std::string_view VisualMaterialNodeVectorFunc::GetCaption() const
	{
		return "Vector function";
	}


	VisualMaterialNodePortType VisualMaterialNodeVectorFunc::GetInputPortType(uint32_t port) const
	{
		switch (Type)
		{
		case Kaidel::VectorType::Vec2: return VisualMaterialNodePortType::Vec2;
		case Kaidel::VectorType::Vec3: return VisualMaterialNodePortType::Vec3;
		case Kaidel::VectorType::Vec4: return VisualMaterialNodePortType::Vec4;
		}
	}


	VisualMaterialNodePortType VisualMaterialNodeVectorFunc::GetOutputPortType(uint32_t port) const
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

	std::string VisualMaterialNodeVectorFunc::GenerateCode(const std::vector<std::string>& inputVars) const
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

}
