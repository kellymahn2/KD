#pragma once
#include "Kaidel/Core/Base.h"

#include "Kaidel/Renderer/GraphicsAPI/Shader.h"
#include "Kaidel/Renderer/GraphicsAPI/ShaderLibrary.h"

#include "Nodes/VisualMaterialNodes.h"

#include <glm/glm.hpp>

#include <map>

namespace Kaidel
{
	enum class VisualMaterialNodeType {
		ConstantBool,
		ConstantInt,
		ConstantUInt,
		ConstantFloat,
		ConstantVec2,
		ConstantVec3, 
		ConstantVec4,
		IntOp,
		UIntOp,
		FloatOp,
		VectorOp,

		IntFunc,
		UIntFunc,
		FloatFunc,
		VectorFunc,

		FragmentOutput
	};

	struct VisualMaterialNodeFragmentOutput : VisualMaterialNode {
			
		glm::vec4 Albedo = glm::vec4(1.0f);
		float Metallic = 0.0f;
		float Roughness = 1.0f;
		glm::vec4 Emissive = glm::vec4(0.0f);
		glm::vec3 Normal = glm::vec3(0.0f, 0.0f, 1.0f);

		uint32_t InputConnections[5] = { -1U, -1U, -1U, -1U, -1U };

		virtual std::string_view GetCaption() const override;

		virtual uint32_t GetConnectedInputPort(uint32_t port) const override;


		virtual uint32_t GetInputPortCount() const override;


		virtual std::string_view GetInputPortName(uint32_t port) const override;


		virtual VisualMaterialNodePortType GetInputPortType(uint32_t port) const override;


		virtual uint32_t GetOutputPortCount() const override;


		virtual std::string_view GetOutputPortName(uint32_t port) const override;


		virtual VisualMaterialNodePortType GetOutputPortType(uint32_t port) const override;


		virtual bool IsInputPortConnected(uint32_t port) const override;


		virtual void SetConnectedInputPort(uint32_t port, uint32_t connection) override;


		virtual std::string GenerateCode(const std::vector<std::string>& inputVars) const override;

	};

	struct VisualMaterialNodeConnection
	{
		uint32_t FromNode = -1;
		uint32_t FromPort = -1;

		uint32_t ToNode = -1;
		uint32_t ToPort = -1;
	};

	struct PortCacheID {
		uint32_t Node = 0;
		uint32_t Port = 0;

		PortCacheID() = default;

		PortCacheID(const VisualMaterialNodeConnection& connection)
		{
			Node = connection.FromNode;
			Port = connection.FromPort;
		}

		PortCacheID(uint32_t node, uint32_t port)
			: Node(node), Port(port)
		{
		}


		bool operator==(const PortCacheID& rhs) const { return Node == rhs.Node && Port == rhs.Port; }
	};

	struct VisualMaterial : IRCCounter<false> 
	{

		struct CodeSegment {
			std::string Code;
			std::string MainCode;
			std::string TextureCode;
			std::vector<std::pair<VisualMaterialParameterType, std::string>> Uniforms;
			uint32_t UniformBinding, UniformSet;
		};

		struct UniformBufferLayout {
			uint32_t Size;
			uint32_t Set, Binding;
		};

		struct Parameter {
			uint32_t ByteOffset = 0;
			uint32_t Set = 0, Binding = 0;
			VisualMaterialParameterType Type;
		};

		CodeSegment Code;

		uint32_t NodesCreated = 0;

		std::unordered_map<uint32_t, Scope<VisualMaterialNode>> Nodes;
		std::vector<VisualMaterialNodeConnection> Connections;

		Ref<Kaidel::Shader> Shader = {};

		Ref<GraphicsPipeline> Pipeline = {};
		
		uint32_t Version = 0;

		std::unordered_map<std::string, Ref<Texture>> DefaultTextureValues;

		std::unordered_map<std::string, Parameter> Parameters;
		
		UniformBufferLayout UniformLayout;

		void Recompile();

		VisualMaterial();
		
		CodeSegment GenerateCode();

		bool IsValidConnection(uint32_t connectionID) const
		{
			return connectionID < Connections.size();
		}

		void RemoveNode(uint32_t nodeID)
		{
			VisualMaterialNode* node = Nodes[nodeID].get();


			//Remove connections
			for (uint32_t i = 0; i < node->GetInputPortCount(); ++i)
			{
				uint32_t portConnection = node->GetConnectedInputPort(i);

				if (portConnection != -1)
				{
					RemoveConnection(portConnection);
				}
			}

			//Remove from nodes array
			Nodes.erase(nodeID);
		}

		void AddConnection(uint32_t fromNode, uint32_t fromPort, uint32_t toNode, uint32_t toPort)
		{
			VisualMaterialNodeConnection connection;
			connection.FromNode = fromNode;
			connection.FromPort = fromPort;
			connection.ToNode = toNode;
			connection.ToPort = toPort;

			VisualMaterialNode& from = *Nodes[fromNode];
			VisualMaterialNode& to = *Nodes[toNode];

			if (to.IsInputPortConnected(toPort))
			{
				uint32_t oldConnection = to.GetConnectedInputPort(toPort);

				RemoveConnection(oldConnection);
			}

			to.SetConnectedInputPort(toPort, Connections.size());

			Connections.emplace_back(connection);
		}

		void RemoveConnection(uint32_t connectionID)
		{
			VisualMaterialNodeConnection connection = Connections[connectionID];

			VisualMaterialNode& to = *Nodes[connection.ToNode];

			//Remove from the nodes
			to.SetConnectedInputPort(connection.ToPort, -1);

			//Remove from the connections array

			//no work to do if the last element 
			if (connectionID == Connections.size() - 1)
			{
				Connections.pop_back();
				return;
			}

			uint32_t last = Connections.size() - 1;

			Connections[connectionID] = Connections[last];
			
			UpdateConnection(connectionID);

			Connections.pop_back();
		}

		template<typename T, typename ...Args>
		Scope<T>& CreateNode(Args&&... args)
		{
			uint32_t nodeID = NodesCreated++;
			Scope<VisualMaterialNode>& node = Nodes[nodeID] = (CreateScope<T>(std::forward<Args>(args)...));
			node->NodeID = nodeID;
			return (Scope<T>&)node;
		}

	private:

		void UpdateConnection(uint32_t connectionID)
		{
			VisualMaterialNodeConnection connection = Connections[connectionID];

			VisualMaterialNode& from = *Nodes[connection.FromNode];
			VisualMaterialNode& to = *Nodes[connection.ToNode];

			//Remove from the nodes
			to.SetConnectedInputPort(connection.ToPort, connectionID);
		}

	};

	const char* VisualMaterialNodeTypeToName(VisualMaterialNodeType type);
}
