#pragma once

#include "VisualMaterialNodeVector.h"

namespace Kaidel {
	enum class IntFunc {
		Abs,
		Negate,
		Sign,
		BitwiseNot
	};

	enum class UIntFunc {
		BitwiseNot
	};

	enum class FloatFunc {
		Sin,
		Cos,
		Tan,
		ASin,
		ACos,
		ATan,
		Sinh,
		Cosh,
		Tanh,
		Log,
		Exp,
		Sqrt,
		Abs,
		Sign,
		Floor,
		Round,
		Ceil,
		Fract,
		Saturate,
		Negate,
		ASinh,
		ACosh,
		ATanh,
		Degrees,
		Exp2,
		InvSqrt,
		Log2,
		Radians,
		Reciprocal,
		RoundEven,
		Trunc,
		OneMinus
	};

	enum class VectorFunc {
		Normalize,
		Sin,
		Cos,
		Tan,
		ASin,
		ACos,
		ATan,
		Sinh,
		Cosh,
		Tanh,
		Log,
		Exp,
		Sqrt,
		Abs,
		Sign,
		Floor,
		Round,
		Ceil,
		Fract,
		Saturate,
		Negate,
		ASinh,
		ACosh,
		ATanh,
		Degrees,
		Exp2,
		InvSqrt,
		Log2,
		Radians,
		Reciprocal,
		RoundEven,
		Trunc,
		OneMinus
	};

	struct VisualMaterialNodeFunc : VisualMaterialNode
	{
		uint32_t InputConnection = -1;

		virtual uint32_t GetConnectedInputPort(uint32_t port) const override;


		virtual uint32_t GetInputPortCount() const override;


		virtual std::string_view GetInputPortName(uint32_t port) const override;


		virtual uint32_t GetOutputPortCount() const override;


		virtual std::string_view GetOutputPortName(uint32_t port) const override;


		virtual bool IsInputPortConnected(uint32_t port) const override;


		virtual void SetConnectedInputPort(uint32_t port, uint32_t connection) override;

	};


	struct VisualMaterialNodeIntFunc : VisualMaterialNodeFunc
	{
		IntFunc Func = IntFunc::Abs;
		int Value = 0;


		virtual std::string_view GetCaption() const override;


		virtual VisualMaterialNodePortType GetInputPortType(uint32_t port) const override;


		virtual VisualMaterialNodePortType GetOutputPortType(uint32_t port) const override;


		virtual std::string GenerateCode(const std::vector<std::string>& inputVars) const override;

	};

	struct VisualMaterialNodeUIntFunc : VisualMaterialNodeFunc
	{
		UIntFunc Func = UIntFunc::BitwiseNot;
		uint32_t Value = 0;


		virtual std::string_view GetCaption() const override;


		virtual VisualMaterialNodePortType GetInputPortType(uint32_t port) const override;


		virtual VisualMaterialNodePortType GetOutputPortType(uint32_t port) const override;


		virtual std::string GenerateCode(const std::vector<std::string>& inputVars) const override;

	};

	struct VisualMaterialNodeFloatFunc : VisualMaterialNodeFunc
	{
		FloatFunc Func = FloatFunc::Sin;
		float Value = 0.0f;


		virtual std::string_view GetCaption() const override;


		virtual VisualMaterialNodePortType GetInputPortType(uint32_t port) const override;


		virtual VisualMaterialNodePortType GetOutputPortType(uint32_t port) const override;


		virtual std::string GenerateCode(const std::vector<std::string>& inputVars) const override;

	};

	struct VisualMaterialNodeVectorFunc : VisualMaterialNodeVector, VisualMaterialNodeFunc
	{
		VectorFunc Func = VectorFunc::Normalize;
		glm::vec4 Value = glm::vec4(0.0f);

		VisualMaterialNodeVectorFunc() { Type = VectorType::Vec3; }

		virtual uint32_t GetOutputPortCount() const override;


		virtual std::string_view GetOutputPortName(uint32_t port) const override;


		virtual std::string_view GetCaption() const override;


		virtual VisualMaterialNodePortType GetInputPortType(uint32_t port) const override;


		virtual VisualMaterialNodePortType GetOutputPortType(uint32_t port) const override;


		virtual std::string GenerateCode(const std::vector<std::string>& inputVars) const override;

	};
}
