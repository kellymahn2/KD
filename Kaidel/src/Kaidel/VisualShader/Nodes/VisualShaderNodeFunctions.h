#pragma once

#include "VisualShaderNodeVector.h"

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

	struct VisualShaderNodeFunc : VisualShaderNode
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


	struct VisualShaderNodeIntFunc : VisualShaderNodeFunc
	{
		IntFunc Func = IntFunc::Abs;
		int Value = 0;


		virtual std::string_view GetCaption() const override;


		virtual VisualShaderNodePortType GetInputPortType(uint32_t port) const override;


		virtual VisualShaderNodePortType GetOutputPortType(uint32_t port) const override;


		virtual std::string GenerateCode(const std::vector<std::string>& inputVars) const override;

		NODE_CLASS_DECL("IntFunc");

		void Serialize(YAML::Emitter& out) const override;


		void Deserialize(YAML::Node& node) override;

	};

	struct VisualShaderNodeUIntFunc : VisualShaderNodeFunc
	{
		UIntFunc Func = UIntFunc::BitwiseNot;
		uint32_t Value = 0;


		virtual std::string_view GetCaption() const override;


		virtual VisualShaderNodePortType GetInputPortType(uint32_t port) const override;


		virtual VisualShaderNodePortType GetOutputPortType(uint32_t port) const override;


		virtual std::string GenerateCode(const std::vector<std::string>& inputVars) const override;

		NODE_CLASS_DECL("UIntFunc");

		void Serialize(YAML::Emitter& out) const override;


		void Deserialize(YAML::Node& node) override;

	};

	struct VisualShaderNodeFloatFunc : VisualShaderNodeFunc
	{
		FloatFunc Func = FloatFunc::Sin;
		float Value = 0.0f;


		virtual std::string_view GetCaption() const override;


		virtual VisualShaderNodePortType GetInputPortType(uint32_t port) const override;


		virtual VisualShaderNodePortType GetOutputPortType(uint32_t port) const override;


		virtual std::string GenerateCode(const std::vector<std::string>& inputVars) const override;

		NODE_CLASS_DECL("FloatFunc");

		void Serialize(YAML::Emitter& out) const override;


		void Deserialize(YAML::Node& node) override;

	};

	struct VisualShaderNodeVectorFunc : VisualShaderNodeVector, VisualShaderNodeFunc
	{
		VectorFunc Func = VectorFunc::Normalize;
		glm::vec4 Value = glm::vec4(0.0f);

		VisualShaderNodeVectorFunc() { Type = VectorType::Vec3; }

		virtual uint32_t GetOutputPortCount() const override;


		virtual std::string_view GetOutputPortName(uint32_t port) const override;


		virtual std::string_view GetCaption() const override;


		virtual VisualShaderNodePortType GetInputPortType(uint32_t port) const override;


		virtual VisualShaderNodePortType GetOutputPortType(uint32_t port) const override;


		virtual std::string GenerateCode(const std::vector<std::string>& inputVars) const override;

		NODE_CLASS_DECL("VecFunc");

		void Serialize(YAML::Emitter& out) const override;


		void Deserialize(YAML::Node& node) override;

	};
}
