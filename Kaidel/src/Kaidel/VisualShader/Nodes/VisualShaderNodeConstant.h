#pragma once

#include "VisualShaderNode.h"

namespace Kaidel
{
	struct VisualShaderNodeConstant : VisualShaderNode
	{
		virtual uint32_t GetConnectedInputPort(uint32_t port) const override;


		virtual uint32_t GetInputPortCount() const override;


		virtual std::string_view GetInputPortName(uint32_t port) const override;


		virtual VisualShaderNodePortType GetInputPortType(uint32_t port) const override;


		virtual bool IsInputPortConnected(uint32_t port) const override;


		virtual void SetConnectedInputPort(uint32_t port, uint32_t connection) override;
		
		virtual uint32_t GetOutputPortCount() const override;
	};

	struct VisualShaderNodeBoolConstant : VisualShaderNodeConstant
	{
		bool Value = false;
		
		virtual std::string_view GetCaption() const override;

		virtual std::string_view GetOutputPortName(uint32_t port) const override;

		virtual VisualShaderNodePortType GetOutputPortType(uint32_t port) const override;

		virtual std::string GenerateCode(const std::vector<std::string>& inputVars) const override;

		VisualShaderNodeBoolConstant();

		NODE_CLASS_DECL("BoolConstant");

		void Serialize(YAML::Emitter& out) const override;


		void Deserialize(YAML::Node& node) override;

	};
	struct VisualShaderNodeIntConstant : VisualShaderNodeConstant
	{
		int Value = 0;
		
		virtual std::string_view GetCaption() const override;

		virtual std::string_view GetOutputPortName(uint32_t port) const override;

		virtual VisualShaderNodePortType GetOutputPortType(uint32_t port) const override;

		virtual std::string GenerateCode(const std::vector<std::string>& inputVars) const override;

		VisualShaderNodeIntConstant();

		NODE_CLASS_DECL("IntConstant");

		void Serialize(YAML::Emitter& out) const override;


		void Deserialize(YAML::Node& node) override;

	};
	struct VisualShaderNodeUIntConstant : VisualShaderNodeConstant
	{
		uint32_t Value = 0;
		
		virtual std::string_view GetCaption() const override;

		virtual std::string_view GetOutputPortName(uint32_t port) const override;

		virtual VisualShaderNodePortType GetOutputPortType(uint32_t port) const override;

		virtual std::string GenerateCode(const std::vector<std::string>& inputVars) const override;

		VisualShaderNodeUIntConstant();

		NODE_CLASS_DECL("UIntConstant");

		void Serialize(YAML::Emitter& out) const override;


		void Deserialize(YAML::Node& node) override;

	};
	struct VisualShaderNodeFloatConstant : VisualShaderNodeConstant
	{
		float Value = 0.0f;

		virtual std::string_view GetCaption() const override;

		virtual std::string_view GetOutputPortName(uint32_t port) const override;

		virtual VisualShaderNodePortType GetOutputPortType(uint32_t port) const override;

		virtual std::string GenerateCode(const std::vector<std::string>& inputVars) const override;

		VisualShaderNodeFloatConstant();

		NODE_CLASS_DECL("FloatConstant");

		void Serialize(YAML::Emitter& out) const override;


		void Deserialize(YAML::Node& node) override;

	};
	struct VisualShaderNodeVec2Constant : VisualShaderNodeConstant
	{
		glm::vec2 Value{ 0.0f };
		
		virtual std::string_view GetCaption() const override;

		virtual uint32_t GetOutputPortCount() const override;

		virtual std::string_view GetOutputPortName(uint32_t port) const override;

		virtual VisualShaderNodePortType GetOutputPortType(uint32_t port) const override;

		virtual std::string GenerateCode(const std::vector<std::string>& inputVars) const override;

		VisualShaderNodeVec2Constant();

		NODE_CLASS_DECL("Vec2Constant");

		void Serialize(YAML::Emitter& out) const override;


		void Deserialize(YAML::Node& node) override;

	};
	struct VisualShaderNodeVec3Constant : VisualShaderNodeConstant
	{
		glm::vec3 Value{ 0.0f };

		virtual std::string_view GetCaption() const override;

		virtual uint32_t GetOutputPortCount() const override;

		virtual std::string_view GetOutputPortName(uint32_t port) const override;

		virtual VisualShaderNodePortType GetOutputPortType(uint32_t port) const override;

		virtual std::string GenerateCode(const std::vector<std::string>& inputVars) const override;

		VisualShaderNodeVec3Constant();

		NODE_CLASS_DECL("Vec3Constant");

		void Serialize(YAML::Emitter& out) const override;


		void Deserialize(YAML::Node& node) override;

	};
	struct VisualShaderNodeVec4Constant : VisualShaderNodeConstant
	{
		glm::vec4 Value{ 0.0f };

		virtual std::string_view GetCaption() const override;

		virtual uint32_t GetOutputPortCount() const override;

		virtual std::string_view GetOutputPortName(uint32_t port) const override;

		virtual VisualShaderNodePortType GetOutputPortType(uint32_t port) const override;

		virtual std::string GenerateCode(const std::vector<std::string>& inputVars) const override;

		VisualShaderNodeVec4Constant();

		NODE_CLASS_DECL("Vec4Constant");

		void Serialize(YAML::Emitter& out) const override;


		void Deserialize(YAML::Node& node) override;

	};
}
