#pragma once

#include "VisualMaterialNode.h"

namespace Kaidel
{
	struct VisualMaterialNodeConstant : VisualMaterialNode
	{
		virtual uint32_t GetConnectedInputPort(uint32_t port) const override;


		virtual uint32_t GetInputPortCount() const override;


		virtual std::string_view GetInputPortName(uint32_t port) const override;


		virtual VisualMaterialNodePortType GetInputPortType(uint32_t port) const override;


		virtual bool IsInputPortConnected(uint32_t port) const override;


		virtual void SetConnectedInputPort(uint32_t port, uint32_t connection) override;
		
		virtual uint32_t GetOutputPortCount() const override;
	};

	struct VisualMaterialNodeBoolConstant : VisualMaterialNodeConstant
	{
		bool Value = false;
		
		virtual std::string_view GetCaption() const override;

		virtual std::string_view GetOutputPortName(uint32_t port) const override;

		virtual VisualMaterialNodePortType GetOutputPortType(uint32_t port) const override;

		virtual std::string GenerateCode(const std::vector<std::string>& inputVars) const override;

		VisualMaterialNodeBoolConstant();

	};
	struct VisualMaterialNodeIntConstant : VisualMaterialNodeConstant
	{
		int Value = 0;
		
		virtual std::string_view GetCaption() const override;

		virtual std::string_view GetOutputPortName(uint32_t port) const override;

		virtual VisualMaterialNodePortType GetOutputPortType(uint32_t port) const override;

		virtual std::string GenerateCode(const std::vector<std::string>& inputVars) const override;

		VisualMaterialNodeIntConstant();

	};
	struct VisualMaterialNodeUIntConstant : VisualMaterialNodeConstant
	{
		uint32_t Value = 0;
		
		virtual std::string_view GetCaption() const override;

		virtual std::string_view GetOutputPortName(uint32_t port) const override;

		virtual VisualMaterialNodePortType GetOutputPortType(uint32_t port) const override;

		virtual std::string GenerateCode(const std::vector<std::string>& inputVars) const override;

		VisualMaterialNodeUIntConstant();

	};
	struct VisualMaterialNodeFloatConstant : VisualMaterialNodeConstant
	{
		float Value = 0.0f;

		virtual std::string_view GetCaption() const override;

		virtual std::string_view GetOutputPortName(uint32_t port) const override;

		virtual VisualMaterialNodePortType GetOutputPortType(uint32_t port) const override;

		virtual std::string GenerateCode(const std::vector<std::string>& inputVars) const override;

		VisualMaterialNodeFloatConstant();

	};
	struct VisualMaterialNodeVec2Constant : VisualMaterialNodeConstant
	{
		glm::vec2 Value{ 0.0f };
		
		virtual std::string_view GetCaption() const override;

		virtual uint32_t GetOutputPortCount() const override;

		virtual std::string_view GetOutputPortName(uint32_t port) const override;

		virtual VisualMaterialNodePortType GetOutputPortType(uint32_t port) const override;

		virtual std::string GenerateCode(const std::vector<std::string>& inputVars) const override;

		VisualMaterialNodeVec2Constant();

	};
	struct VisualMaterialNodeVec3Constant : VisualMaterialNodeConstant
	{
		glm::vec3 Value{ 0.0f };

		virtual std::string_view GetCaption() const override;

		virtual uint32_t GetOutputPortCount() const override;

		virtual std::string_view GetOutputPortName(uint32_t port) const override;

		virtual VisualMaterialNodePortType GetOutputPortType(uint32_t port) const override;

		virtual std::string GenerateCode(const std::vector<std::string>& inputVars) const override;

		VisualMaterialNodeVec3Constant();

	};
	struct VisualMaterialNodeVec4Constant : VisualMaterialNodeConstant
	{
		glm::vec4 Value{ 0.0f };

		virtual std::string_view GetCaption() const override;

		virtual uint32_t GetOutputPortCount() const override;

		virtual std::string_view GetOutputPortName(uint32_t port) const override;

		virtual VisualMaterialNodePortType GetOutputPortType(uint32_t port) const override;

		virtual std::string GenerateCode(const std::vector<std::string>& inputVars) const override;

		VisualMaterialNodeVec4Constant();

	};
}
