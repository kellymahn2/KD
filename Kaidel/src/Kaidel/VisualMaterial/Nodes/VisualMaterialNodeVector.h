#pragma once

#include "VisualMaterialNode.h"

namespace Kaidel {
	enum class VectorType {
		Vec2, Vec3, Vec4
	};

	struct VisualMaterialNodeVector {
		VectorType Type;
	};

	struct VisualMaterialNodeVectorCompose : VisualMaterialNodeVector, VisualMaterialNode
	{
		uint32_t InputConnections[4] = { -1U, -1U, -1U, -1U };

		virtual std::string_view GetCaption() const override
		{
			return "Vector compose";
		}


		virtual uint32_t GetConnectedInputPort(uint32_t port) const override
		{
			return InputConnections[port];
		}


		virtual uint32_t GetInputPortCount() const override
		{
			return (uint32_t)Type + 2;
		}


		virtual std::string_view GetInputPortName(uint32_t port) const override
		{
			static std::string_view InputNames[] = { "R", "G", "B", "A" };
			
			return InputNames[port];
		}


		virtual VisualMaterialNodePortType GetInputPortType(uint32_t port) const override
		{
			return VisualMaterialNodePortType::Float;
		}


		virtual uint32_t GetOutputPortCount() const override
		{
			return 1;
		}


		virtual std::string_view GetOutputPortName(uint32_t port) const override
		{
			return "Vector";
		}


		virtual VisualMaterialNodePortType GetOutputPortType(uint32_t port) const override
		{
			switch (Type)
			{
			case VectorType::Vec2: return VisualMaterialNodePortType::Vec2;
			case VectorType::Vec3: return VisualMaterialNodePortType::Vec3;
			case VectorType::Vec4: return VisualMaterialNodePortType::Vec4;
			}
		}


		virtual bool IsInputPortConnected(uint32_t port) const override
		{
			return InputConnections[port] != -1;
		}


		virtual void SetConnectedInputPort(uint32_t port, uint32_t connection) override
		{
			InputConnections[port] = connection;
		}


		virtual std::string GenerateCode(const std::vector<std::string>& inputVars) const override
		{
			std::string v0, v1, v2, v3;
			v0 = v1 = v2 = v3 = "0.0";

			if (IsInputPortConnected(0))
				v0 = inputVars[0];
			if (IsInputPortConnected(1))
				v1 = inputVars[1];
			if (IsInputPortConnected(2))
				v2 = inputVars[2];
			if (IsInputPortConnected(3))
				v3 = inputVars[3];

			const char* Formats[] = { "vec2 {0} = vec2({1}, {2});\n", "vec3 {0} = vec3({1}, {2}, {3});\n", "vec4 {0} = vec4({1}, {2}, {3}, {4});\n" };

			return fmt::format(Formats[(uint32_t)Type], NODE_PORT_NAME(0), v0, v1, v2, v3);
		}

	};

}
