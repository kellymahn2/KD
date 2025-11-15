#pragma once

#include <string>
#include <Kaidel/Renderer/GraphicsAPI/DescriptorSet.h>

#include <glm/glm.hpp>

#include <variant>

#define NODE_PORT_NAME_OFFSET(portOffset, portID) fmt::format("nodeOut_{}", portOffset + portID)
#define NODE_PORT_NAME(portID) NODE_PORT_NAME_OFFSET(PortOffset, portID)
#define NODE_DESCRIPTOR_NAME(nodeID, descID) fmt::format("node_{}_Descriptor_{}", nodeID, descID)
#define EXPANDED_PORTS_DEFINE_VEC2(portID) fmt::format("float {0} = {2}.r;\nfloat {1} = {2}.g;\n", NODE_PORT_NAME(portID + 1),NODE_PORT_NAME(portID + 2), NODE_PORT_NAME(portID))
#define EXPANDED_PORTS_DEFINE_VEC3(portID) fmt::format("float {0} = {3}.r;\nfloat {1} = {3}.g;\nfloat {2} = {3}.b;\n", NODE_PORT_NAME(portID + 1), NODE_PORT_NAME(portID + 2), NODE_PORT_NAME(portID + 3), NODE_PORT_NAME(portID))
#define EXPANDED_PORTS_DEFINE_VEC4(portID) fmt::format("float {0} = {4}.r;\nfloat {1} = {4}.g;\nfloat {2} = {4}.b;\nfloat {3} = {4}.a;\n", NODE_PORT_NAME(portID + 1),NODE_PORT_NAME(portID + 2), NODE_PORT_NAME(portID + 3), NODE_PORT_NAME(portID + 4), NODE_PORT_NAME(portID))

namespace Kaidel
{
	enum class VisualMaterialNodePortType
	{
		Bool, //Math types
		Int,  //Math types
		UInt,  //Math types
		Float, Vec2, Vec3, Vec4, //Math types
		Sampler
	};

	enum class VisualMaterialParameterType {
		None, Bool, Int, UInt, Float, Vec2, Vec3, Vec4, Texture
	};

	struct VisualMaterialParameter
	{
		std::string Name = "";
		VisualMaterialParameterType Type = VisualMaterialParameterType::None;
		uint32_t ByteOffset = 0;
		bool Active = false;
	};

	struct VisualMaterialNodePort
	{
		std::string Name;
		VisualMaterialNodePortType PortType;
	};

	struct VisualMaterialResourceBinding {
		uint32_t SetID;
		uint32_t Binding;
		
		VisualMaterialResourceBinding(uint32_t setID = -1, uint32_t binding = -1)
			: SetID(setID), Binding(binding)
		{}
	};

	struct VisualMaterialNode
	{
		glm::vec2 Position = glm::vec2(0.0f);

		uint32_t NodeID;

		VisualMaterialParameter Parameter;

		virtual ~VisualMaterialNode() = default;

		virtual std::string_view GetCaption() const = 0;

		virtual uint32_t GetInputPortCount() const = 0;

		virtual VisualMaterialNodePortType GetInputPortType(uint32_t port) const = 0;

		virtual std::string_view GetInputPortName(uint32_t port) const = 0;

		virtual uint32_t GetOutputPortCount() const = 0;

		virtual VisualMaterialNodePortType GetOutputPortType(uint32_t port) const = 0;

		virtual std::string_view GetOutputPortName(uint32_t port) const = 0;

		virtual bool IsInputPortConnected(uint32_t port) const = 0;

		virtual uint32_t GetConnectedInputPort(uint32_t port) const = 0;

		virtual void SetConnectedInputPort(uint32_t port, uint32_t connection) = 0;

		//virtual float GetWidth() const = 0;

		virtual std::string GenerateCode(const std::vector<std::string>& inputVars) const = 0;

		virtual Ref<Texture> GetDescriptorTexture(uint32_t index) const { return {}; }

		void PromoteToParameter(const std::string& name) {
			Parameter.Name = name;
			Parameter.Active = true;
		}
		void DemoteToParameter() {
			Parameter.Active = false;
		}

		uint32_t GetInputPortID(uint32_t index) const { return PortOffset + index; }
		uint32_t GetOutputPortID(uint32_t index) const { return PortOffset + GetInputPortCount() + index; }

		uint32_t PortOffset = 0;
	};

}
