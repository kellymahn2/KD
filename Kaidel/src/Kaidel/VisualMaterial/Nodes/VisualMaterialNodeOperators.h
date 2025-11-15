#pragma once

#include "VisualMaterialNodeVector.h"


namespace Kaidel {
	enum class IntOp {
		Add,
		Subtract,
		Multiply,
		Divide,
		Remainder,
		Max,
		Min,
		BitwiseAnd,
		BitwiseOr,
		BitsiseXor,
		LeftShift,
		RightShift,
	};

	using UIntOp = IntOp;

	enum class FloatOp {
		Add,
		Subtract,
		Multiply,
		Divide,
		Remainder,
		Power,
		Max,
		Min,
		Atan2,
		Step
	};

	enum class VectorOp {
		Add,
		Subtract,
		Multiply,
		Divide,
		Remainder,
		Power,
		Max,
		Min,
		Cross,
		Dot,
		Atan2,
		Reflect,
		Step,
		Lerp,
	};

	struct VisualMaterialNodeOp : VisualMaterialNode
	{
		uint32_t InputConnections[2] = { -1U, -1U };

		virtual uint32_t GetConnectedInputPort(uint32_t port) const override;


		virtual uint32_t GetInputPortCount() const override;


		virtual std::string_view GetInputPortName(uint32_t port) const override;


		virtual uint32_t GetOutputPortCount() const override;


		virtual std::string_view GetOutputPortName(uint32_t port) const override;


		virtual bool IsInputPortConnected(uint32_t port) const override;


		virtual void SetConnectedInputPort(uint32_t port, uint32_t connection) override;

	};

	struct VisualMaterialNodeIntOp : VisualMaterialNodeOp
	{
		IntOp OpType = IntOp::Add;
		int ValueA = 0, ValueB = 0;


		virtual std::string_view GetCaption() const override;


		virtual VisualMaterialNodePortType GetInputPortType(uint32_t port) const override;


		virtual VisualMaterialNodePortType GetOutputPortType(uint32_t port) const override;


		virtual std::string GenerateCode(const std::vector<std::string>& inputVars) const override;

	};

	struct VisualMaterialNodeUIntOp : VisualMaterialNodeOp
	{
		UIntOp OpType = UIntOp::Add;
		uint32_t ValueA = 0, ValueB = 0;


		virtual std::string_view GetCaption() const override;


		virtual VisualMaterialNodePortType GetInputPortType(uint32_t port) const override;


		virtual VisualMaterialNodePortType GetOutputPortType(uint32_t port) const override;


		virtual std::string GenerateCode(const std::vector<std::string>& inputVars) const override;

	};

	struct VisualMaterialNodeFloatOp : VisualMaterialNodeOp
	{
		FloatOp OpType = FloatOp::Add;
		float ValueA = 0.0f, ValueB = 0.0f;


		virtual std::string_view GetCaption() const override;


		virtual VisualMaterialNodePortType GetInputPortType(uint32_t port) const override;


		virtual VisualMaterialNodePortType GetOutputPortType(uint32_t port) const override;


		virtual std::string GenerateCode(const std::vector<std::string>& inputVars) const override;

	};

	struct VisualMaterialNodeVectorOp : VisualMaterialNodeVector, VisualMaterialNodeOp
	{
		VectorOp OpType;
		glm::vec4 ValueA = glm::vec4(0.0f), ValueB = glm::vec4(0.0f);

		virtual uint32_t GetOutputPortCount() const override;


		virtual std::string_view GetOutputPortName(uint32_t port) const override;


		virtual std::string_view GetCaption() const override;


		virtual VisualMaterialNodePortType GetInputPortType(uint32_t port) const override;


		virtual VisualMaterialNodePortType GetOutputPortType(uint32_t port) const override;


		virtual std::string GenerateCode(const std::vector<std::string>& inputVars) const override;

	};

}
