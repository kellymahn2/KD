#pragma once

#include <string>
#include <Kaidel/Renderer/GraphicsAPI/DescriptorSet.h>
#include "Kaidel/Renderer/Sprite.h"

#include <glm/glm.hpp>
#include <yaml-cpp\yaml.h>
#include "Kaidel/Core/YAMLHelpers.h"

#include <variant>

#define NODE_PORT_NAME_OFFSET(nodeID, portID) fmt::format("nodeOut_{}_{}", nodeID, portID)
#define NODE_PORT_NAME(portID) NODE_PORT_NAME_OFFSET(NodeID, portID)
#define NODE_DESCRIPTOR_NAME(nodeID, descID) fmt::format("node_{}_Descriptor_{}", nodeID, descID)
#define EXPANDED_PORTS_DEFINE_VEC2(portID) fmt::format("float {0} = {2}.r;\nfloat {1} = {2}.g;\n", NODE_PORT_NAME(portID + 1),NODE_PORT_NAME(portID + 2), NODE_PORT_NAME(portID))
#define EXPANDED_PORTS_DEFINE_VEC3(portID) fmt::format("float {0} = {3}.r;\nfloat {1} = {3}.g;\nfloat {2} = {3}.b;\n", NODE_PORT_NAME(portID + 1), NODE_PORT_NAME(portID + 2), NODE_PORT_NAME(portID + 3), NODE_PORT_NAME(portID))
#define EXPANDED_PORTS_DEFINE_VEC4(portID) fmt::format("float {0} = {4}.r;\nfloat {1} = {4}.g;\nfloat {2} = {4}.b;\nfloat {3} = {4}.a;\n", NODE_PORT_NAME(portID + 1),NODE_PORT_NAME(portID + 2), NODE_PORT_NAME(portID + 3), NODE_PORT_NAME(portID + 4), NODE_PORT_NAME(portID))

#define NODE_CLASS_DECL(name) static const char* GetStaticTypename() { return name; }\
								 virtual const char* GetTypename() const override { return GetStaticTypename(); } 

namespace Kaidel
{
	enum class VisualShaderNodePortType
	{
		Bool, //Math types
		Int,  //Math types
		UInt,  //Math types
		Float, Vec2, Vec3, Vec4, //Math types
		Sampler
	};

	enum class VisualShaderParameterType {
		None, Bool, Int, UInt, Float, Vec2, Vec3, Vec4, Texture
	};

	struct VisualShaderParameter
	{
		std::string Name = "";
		VisualShaderParameterType Type = VisualShaderParameterType::None;
		uint32_t ByteOffset = 0;
		bool Active = false;
	};

	struct VisualShaderNodePort
	{
		std::string Name;
		VisualShaderNodePortType PortType;
	};

	struct VisualShaderResourceBinding {
		uint32_t SetID;
		uint32_t Binding;
		
		VisualShaderResourceBinding(uint32_t setID = -1, uint32_t binding = -1)
			: SetID(setID), Binding(binding)
		{}
	};

	struct VisualShaderNode
	{
		glm::vec2 Position = glm::vec2(0.0f);

		uint32_t NodeID;

		VisualShaderParameter Parameter;

		virtual ~VisualShaderNode() = default;

		virtual const char* GetTypename() const 
		{
			KD_CORE_ASSERT("Not implemented");
			return nullptr;
		}

		virtual std::string_view GetCaption() const = 0;

		virtual uint32_t GetInputPortCount() const = 0;

		virtual VisualShaderNodePortType GetInputPortType(uint32_t port) const = 0;

		virtual std::string_view GetInputPortName(uint32_t port) const = 0;

		virtual uint32_t GetOutputPortCount() const = 0;

		virtual VisualShaderNodePortType GetOutputPortType(uint32_t port) const = 0;

		virtual std::string_view GetOutputPortName(uint32_t port) const = 0;

		virtual bool IsInputPortConnected(uint32_t port) const = 0;

		virtual uint32_t GetConnectedInputPort(uint32_t port) const = 0;

		virtual void SetConnectedInputPort(uint32_t port, uint32_t connection) = 0;

		//virtual float GetWidth() const = 0;

		virtual std::string GenerateCode(const std::vector<std::string>& inputVars) const = 0;

		virtual Ref<SpriteSheet> GetDescriptorTexture(uint32_t index) const { return {}; }

		void PromoteToParameter(const std::string& name) {
			Parameter.Name = name;
			Parameter.Active = true;
		}
		void DemoteToParameter() {
			Parameter.Active = false;
		}

		uint32_t GetInputPortID(uint32_t index) const { return PortOffset + index; }
		uint32_t GetOutputPortID(uint32_t index) const { return PortOffset + GetInputPortCount() + index; }

		virtual void Serialize(YAML::Emitter& out) const 
		{
			KD_CORE_ASSERT("Not Implemented");
		}
		virtual void Deserialize(YAML::Node& node) 
		{
			KD_CORE_ASSERT("Not Implemented");
		}

		uint32_t PortOffset = 0;
	};

}
