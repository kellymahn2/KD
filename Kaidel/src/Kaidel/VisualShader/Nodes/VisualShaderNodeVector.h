#pragma once

#include "VisualShaderNode.h"

#include "Kaidel/Core/BiMap.h"

namespace Kaidel {


	enum class VectorType {
		Vec2, Vec3, Vec4
	};

	struct VisualShaderNodeVector {
		VectorType Type;

		static void Serialize(YAML::Emitter& out, VectorType type)
		{
			switch (type)
			{
			case Kaidel::VectorType::Vec2: out << YAML::Key << "VType" << YAML::Value << "Vec2"; break;
			case Kaidel::VectorType::Vec3: out << YAML::Key << "VType" << YAML::Value << "Vec3"; break;
			case Kaidel::VectorType::Vec4: out << YAML::Key << "VType" << YAML::Value << "Vec4"; break;
			}

			KD_CORE_ASSERT(false);
		}

		static VectorType Deserialize(YAML::Node& node)
		{
			std::string type = node["VType"].as<std::string>();

			if (type == "Vec2")
			{
				return VectorType::Vec2;
			}
			
			if (type == "Vec3")
			{
				return VectorType::Vec3;
			}
			
			if (type == "Vec4")
			{
				return VectorType::Vec4;
			}

			KD_CORE_ASSERT(false);
			return VectorType::Vec2;
		}
	};

	struct VisualShaderNodeVectorCompose : VisualShaderNodeVector, VisualShaderNode
	{
		uint32_t InputConnections[4] = { UINT32_MAX, UINT32_MAX, UINT32_MAX, UINT32_MAX };

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


		virtual VisualShaderNodePortType GetInputPortType(uint32_t port) const override
		{
			return VisualShaderNodePortType::Float;
		}


		virtual uint32_t GetOutputPortCount() const override
		{
			return 1;
		}


		virtual std::string_view GetOutputPortName(uint32_t port) const override
		{
			return "Vector";
		}


		virtual VisualShaderNodePortType GetOutputPortType(uint32_t port) const override
		{
			switch (Type)
			{
			case VectorType::Vec2: return VisualShaderNodePortType::Vec2;
			case VectorType::Vec3: return VisualShaderNodePortType::Vec3;
			case VectorType::Vec4: return VisualShaderNodePortType::Vec4;
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

		NODE_CLASS_DECL("VecCompose");

		void Serialize(YAML::Emitter& out) const override
		{
			VisualShaderNodeVector::Serialize(out, Type);
		}


		void Deserialize(YAML::Node& node) override
		{
			Type = VisualShaderNodeVector::Deserialize(node);
		}

	};

}
