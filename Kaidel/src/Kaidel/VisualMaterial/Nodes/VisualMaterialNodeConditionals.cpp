#include "KDpch.h"
#include "VisualMaterialNodeConditionals.h"

namespace Kaidel {

	static const char* s_CompareFormats[] = {
		"({} == {})",
		"({} != {})",
		"({} > {})",
		"({} >= {})",
		"({} < {})",
		"({} <= {})"
	};

	static std::string FormatValue(const VisualMaterialNodeCompare::ValueType& value, VisualMaterialNodeCompare::CompareType type)
	{
		switch (type)
		{
		case Kaidel::VisualMaterialNodeCompare::CompareType::Int: return fmt::format("{}", value.Int);
		case Kaidel::VisualMaterialNodeCompare::CompareType::UInt: return fmt::format("{}U", value.UInt);
		case Kaidel::VisualMaterialNodeCompare::CompareType::Float: return fmt::format("{:.3f}", value.Float);
		case Kaidel::VisualMaterialNodeCompare::CompareType::Vec2: return fmt::format("vec2({:.3f}, {:.3f})", value.Vec2.x, value.Vec2.y);
		case Kaidel::VisualMaterialNodeCompare::CompareType::Vec3: return fmt::format("vec3({:.3f}, {:.3f}, {:.3f})", value.Vec3.x, value.Vec3.y, value.Vec3.z);
		case Kaidel::VisualMaterialNodeCompare::CompareType::Vec4: return fmt::format("vec4({:.3f}, {:.3f}, {:.3f}, {:.3f})", value.Vec4.x, value.Vec4.y, value.Vec4.z, value.Vec4.w);
		}
	}

	static std::string FormatValue(const VisualMaterialNodeSwitch::ValueType& value, VisualMaterialNodeSwitch::SwitchType type)
	{
		switch (type)
		{
		case Kaidel::VisualMaterialNodeSwitch::SwitchType::Int: return fmt::format("{}", value.Int);
		case Kaidel::VisualMaterialNodeSwitch::SwitchType::UInt: return fmt::format("{}U", value.UInt);
		case Kaidel::VisualMaterialNodeSwitch::SwitchType::Float: return fmt::format("{:.3f}", value.Float);
		case Kaidel::VisualMaterialNodeSwitch::SwitchType::Vec2: return fmt::format("vec2({:.3f}, {:.3f})", value.Vec2.x, value.Vec2.y);
		case Kaidel::VisualMaterialNodeSwitch::SwitchType::Vec3: return fmt::format("vec3({:.3f}, {:.3f}, {:.3f})", value.Vec3.x, value.Vec3.y, value.Vec3.z);
		case Kaidel::VisualMaterialNodeSwitch::SwitchType::Vec4: return fmt::format("vec4({:.3f}, {:.3f}, {:.3f}, {:.3f})", value.Vec4.x, value.Vec4.y, value.Vec4.z, value.Vec4.w);
		}
	}

	std::string VisualMaterialNodeCompare::GenerateCode(const std::vector<std::string>& inputVars) const
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
			case Kaidel::VisualMaterialNodeCompare::Any: cond = fmt::format("any({})", cond); break;
			case Kaidel::VisualMaterialNodeCompare::All: cond = fmt::format("all({})", cond); break;
			}
		}

		return fmt::format("bool {} = bool({});\n", NODE_PORT_NAME(0), cond);
	}


	std::string_view VisualMaterialNodeCompare::GetCaption() const
	{
		return "Compare";
	}


	uint32_t VisualMaterialNodeCompare::GetConnectedInputPort(uint32_t port) const
	{
		return InputConnections[port];
	}


	uint32_t VisualMaterialNodeCompare::GetInputPortCount() const
	{
		return 2;
	}


	std::string_view VisualMaterialNodeCompare::GetInputPortName(uint32_t port) const
	{
		static std::string_view Names[] = { "a", "b" };

		return Names[port];
	}


	Kaidel::VisualMaterialNodePortType VisualMaterialNodeCompare::GetInputPortType(uint32_t port) const
	{
		switch (CType)
		{
		case Kaidel::VisualMaterialNodeCompare::CompareType::Int: return VisualMaterialNodePortType::Int;
		case Kaidel::VisualMaterialNodeCompare::CompareType::UInt: return VisualMaterialNodePortType::UInt;
		case Kaidel::VisualMaterialNodeCompare::CompareType::Float: return VisualMaterialNodePortType::Float;
		case Kaidel::VisualMaterialNodeCompare::CompareType::Vec2: return VisualMaterialNodePortType::Vec2;
		case Kaidel::VisualMaterialNodeCompare::CompareType::Vec3: return VisualMaterialNodePortType::Vec3;
		case Kaidel::VisualMaterialNodeCompare::CompareType::Vec4: return VisualMaterialNodePortType::Vec4;
		}

		return VisualMaterialNodePortType::Bool;
	}


	uint32_t VisualMaterialNodeCompare::GetOutputPortCount() const
	{
		return 1;
	}


	std::string_view VisualMaterialNodeCompare::GetOutputPortName(uint32_t port) const
	{
		return "Bool";
	}


	Kaidel::VisualMaterialNodePortType VisualMaterialNodeCompare::GetOutputPortType(uint32_t port) const
	{
		return VisualMaterialNodePortType::Bool;
	}


	bool VisualMaterialNodeCompare::IsInputPortConnected(uint32_t port) const
	{
		return InputConnections[port] != -1;
	}


	void VisualMaterialNodeCompare::SetConnectedInputPort(uint32_t port, uint32_t connection)
	{
		InputConnections[port] = connection;
	}


	std::string VisualMaterialNodeSwitch::GenerateCode(const std::vector<std::string>& inputVars) const
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
		case Kaidel::VisualMaterialNodeSwitch::SwitchType::Int: type = "int"; break;
		case Kaidel::VisualMaterialNodeSwitch::SwitchType::UInt: type = "uint"; break;
		case Kaidel::VisualMaterialNodeSwitch::SwitchType::Float: type = "float"; break;
		case Kaidel::VisualMaterialNodeSwitch::SwitchType::Vec2: type = "vec2"; break;
		case Kaidel::VisualMaterialNodeSwitch::SwitchType::Vec3: type = "vec3"; break;
		case Kaidel::VisualMaterialNodeSwitch::SwitchType::Vec4: type = "vec4"; break;
		}

		return fmt::format("{} {} = ({}) ? {} : {};\n", type, NODE_PORT_NAME(0), cond, var0, var1);
	}


	std::string_view VisualMaterialNodeSwitch::GetCaption() const
	{
		return "Switch";
	}


	uint32_t VisualMaterialNodeSwitch::GetConnectedInputPort(uint32_t port) const
	{
		return InputConnections[port];
	}


	uint32_t VisualMaterialNodeSwitch::GetInputPortCount() const
	{
		return 3;
	}


	std::string_view VisualMaterialNodeSwitch::GetInputPortName(uint32_t port) const
	{
		static std::string_view Names[] = 
		{
			"a",
			"b",
			"condition"
		};

		return Names[port];
	}


	Kaidel::VisualMaterialNodePortType VisualMaterialNodeSwitch::GetInputPortType(uint32_t port) const
	{
		if (port == 2)
			return VisualMaterialNodePortType::Bool;

		switch (SType)
		{
		case Kaidel::VisualMaterialNodeSwitch::SwitchType::Int: return VisualMaterialNodePortType::Int;
		case Kaidel::VisualMaterialNodeSwitch::SwitchType::UInt: return VisualMaterialNodePortType::UInt;
		case Kaidel::VisualMaterialNodeSwitch::SwitchType::Float: return VisualMaterialNodePortType::Float;
		case Kaidel::VisualMaterialNodeSwitch::SwitchType::Vec2: return VisualMaterialNodePortType::Vec2;
		case Kaidel::VisualMaterialNodeSwitch::SwitchType::Vec3: return VisualMaterialNodePortType::Vec3;
		case Kaidel::VisualMaterialNodeSwitch::SwitchType::Vec4: return VisualMaterialNodePortType::Vec4;
		}

		return VisualMaterialNodePortType::Bool;
	}


	uint32_t VisualMaterialNodeSwitch::GetOutputPortCount() const
	{
		return 1;
	}


	std::string_view VisualMaterialNodeSwitch::GetOutputPortName(uint32_t port) const
	{
		return "Value";
	}


	Kaidel::VisualMaterialNodePortType VisualMaterialNodeSwitch::GetOutputPortType(uint32_t port) const
	{
		switch (SType)
		{
		case Kaidel::VisualMaterialNodeSwitch::SwitchType::Int: return VisualMaterialNodePortType::Int;
		case Kaidel::VisualMaterialNodeSwitch::SwitchType::UInt: return VisualMaterialNodePortType::UInt;
		case Kaidel::VisualMaterialNodeSwitch::SwitchType::Float: return VisualMaterialNodePortType::Float;
		case Kaidel::VisualMaterialNodeSwitch::SwitchType::Vec2: return VisualMaterialNodePortType::Vec2;
		case Kaidel::VisualMaterialNodeSwitch::SwitchType::Vec3: return VisualMaterialNodePortType::Vec3;
		case Kaidel::VisualMaterialNodeSwitch::SwitchType::Vec4: return VisualMaterialNodePortType::Vec4;
		}

		return VisualMaterialNodePortType::Bool;
	}


	bool VisualMaterialNodeSwitch::IsInputPortConnected(uint32_t port) const
	{
		return InputConnections[port] != -1;
	}


	void VisualMaterialNodeSwitch::SetConnectedInputPort(uint32_t port, uint32_t connection)
	{
		InputConnections[port] = connection;
	}

}
