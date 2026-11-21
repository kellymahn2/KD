#pragma once

#include "VisualShaderNode.h"

namespace Kaidel {
	//TODO: Switch, IsNan, IsInf
	struct VisualShaderNodeCompare : VisualShaderNode {
		enum class CompareFunc {
			Equal,
			NotEqual,
			Greater,
			GreaterEqual,
			Less,
			LessEqual,
		};

		enum class CompareType {
			Int, UInt,
			Float, Vec2, Vec3, Vec4
		};

		enum CompareCond {
			Any, All
		};

		CompareType CType = CompareType::Float;
		CompareFunc CFunc = CompareFunc::Equal;
		CompareCond CCond = CompareCond::All;

		union ValueType {
			int32_t Int;
			uint32_t UInt;
			float Float;
			glm::vec2 Vec2;
			glm::vec3 Vec3;
			glm::vec4 Vec4;

			ValueType() {
				Vec4 = glm::vec4(0.0f);
			}
		};

		ValueType ValueA = ValueType(), ValueB = ValueType();

		uint32_t InputConnections[2] = { UINT32_MAX, UINT32_MAX };


		virtual std::string GenerateCode(const std::vector<std::string>& inputVars) const override;


		virtual std::string_view GetCaption() const override;


		virtual uint32_t GetConnectedInputPort(uint32_t port) const override;


		virtual uint32_t GetInputPortCount() const override;


		virtual std::string_view GetInputPortName(uint32_t port) const override;


		virtual VisualShaderNodePortType GetInputPortType(uint32_t port) const override;


		virtual uint32_t GetOutputPortCount() const override;


		virtual std::string_view GetOutputPortName(uint32_t port) const override;


		virtual VisualShaderNodePortType GetOutputPortType(uint32_t port) const override;


		virtual bool IsInputPortConnected(uint32_t port) const override;


		virtual void SetConnectedInputPort(uint32_t port, uint32_t connection) override;

		NODE_CLASS_DECL("Compare");  

		void Serialize(YAML::Emitter& out) const override;


		void Deserialize(YAML::Node& node) override;

	};


	struct VisualShaderNodeSwitch : VisualShaderNode
	{
		enum class SwitchType {
			Int, UInt,
			Float, Vec2, Vec3, Vec4
		};

		SwitchType SType = SwitchType::Float;

		union ValueType {
			int32_t Int;
			uint32_t UInt;
			float Float;
			glm::vec2 Vec2;
			glm::vec3 Vec3;
			glm::vec4 Vec4;

			ValueType() {
				Vec4 = glm::vec4(0.0f);
			}
		};

		ValueType ValueA = ValueType(), ValueB = ValueType();
		bool Condition = false;

		uint32_t InputConnections[3] = { UINT32_MAX, UINT32_MAX, UINT32_MAX };

		virtual std::string GenerateCode(const std::vector<std::string>& inputVars) const override;


		virtual std::string_view GetCaption() const override;


		virtual uint32_t GetConnectedInputPort(uint32_t port) const override;


		virtual uint32_t GetInputPortCount() const override;


		virtual std::string_view GetInputPortName(uint32_t port) const override;


		virtual VisualShaderNodePortType GetInputPortType(uint32_t port) const override;


		virtual uint32_t GetOutputPortCount() const override;


		virtual std::string_view GetOutputPortName(uint32_t port) const override;


		virtual VisualShaderNodePortType GetOutputPortType(uint32_t port) const override;


		virtual bool IsInputPortConnected(uint32_t port) const override;


		virtual void SetConnectedInputPort(uint32_t port, uint32_t connection) override;

		NODE_CLASS_DECL("Switch");

		void Serialize(YAML::Emitter& out) const override;


		void Deserialize(YAML::Node& node) override;

	};



}
