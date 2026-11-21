#include "KDpch.h"
#include "MaterialSerializer.h"
#include "Kaidel/Core/YAMLHelpers.h"
#include "Kaidel/Asset/AssetManager.h"

#include <yaml-cpp/yaml.h>

namespace Kaidel {

	namespace Utils {
		static const char* TextureChannelToString(StandardMaterial::MaterialTextureChannel channel)
		{
			switch (channel)
			{
			case StandardMaterial::MaterialTextureChannel::Red: return "Red";
			case StandardMaterial::MaterialTextureChannel::Green: return "Green";
			case StandardMaterial::MaterialTextureChannel::Blue: return "Blue";
			}

			KD_CORE_ASSERT(false);
			
			return nullptr;
		}

		static StandardMaterial::MaterialTextureChannel StringToTextureChannel(const std::string& channel)
		{
			if ("Red")
			{
				return StandardMaterial::MaterialTextureChannel::Red;
			}
			if ("Green")
			{
				return StandardMaterial::MaterialTextureChannel::Green;
			}
			if ("Blue")
			{
				return StandardMaterial::MaterialTextureChannel::Blue;
			}


			KD_CORE_ASSERT(false);
			
			return StandardMaterial::MaterialTextureChannel::Red;
		}

		static const char* TextureTypeToString(StandardMaterial::MaterialTextureType type)
		{
			switch (type)
			{
			case StandardMaterial::MaterialTextureType::Albedo: return "Albedo";
			case StandardMaterial::MaterialTextureType::Normal: return "Normal";
			case StandardMaterial::MaterialTextureType::Metallic: return "MetallicT";
			case StandardMaterial::MaterialTextureType::Roughness: return "RoughnessT";
			case StandardMaterial::MaterialTextureType::Emissive: return "Emissive";
			}

			KD_CORE_ASSERT(false);
			
			return nullptr;
		}

		static StandardMaterial::MaterialTextureType StringToTextureType(const std::string& type)
		{
			if (type == "Albedo")
			{
				return StandardMaterial::MaterialTextureType::Albedo;
			}

			if (type == "Normal")
			{
				return StandardMaterial::MaterialTextureType::Normal;
			}

			if (type == "Metallic")
			{
				return StandardMaterial::MaterialTextureType::Metallic;
			}

			if (type == "Roughness")
			{
				return StandardMaterial::MaterialTextureType::Roughness;
			}

			if (type == "Emissive")
			{
				return StandardMaterial::MaterialTextureType::Emissive;
			}

			KD_CORE_ASSERT(false);

			return StandardMaterial::MaterialTextureType::Max;
		}
	
		static void SerializeConnectionEnd(YAML::Emitter& out, uint32_t node, uint32_t port)
		{
			out << YAML::BeginMap;

			out << YAML::Key << "Node" << YAML::Value << node;

			out << YAML::Key << "Port" << YAML::Value << port;

			out << YAML::EndMap;
		}
	}

	void MaterialSerializer::Serialize(const Path& path, Ref<StandardMaterial> mat, MetadataFile* mtdFile)
	{
		YAML::Emitter out;
		
		auto& uniformData = mat->GetUniformData();

		out << YAML::BeginMap;
		
		out << YAML::Key << "BaseColor" << YAML::Value << uniformData.BaseColor;
		
		out << YAML::Key << "Metalness" << YAML::Value << uniformData.Metalness;
		out << YAML::Key << "MetalnessChannel" << YAML::Value <<  Utils::TextureChannelToString(uniformData.MetalnessChannel);

		out << YAML::Key << "Roughness" << YAML::Value << uniformData.Roughness;
		out << YAML::Key << "RoughnessChannel" << YAML::Value << Utils::TextureChannelToString(uniformData.RoughnessChannel);

					
		for (uint32_t i = 0; i < (uint32_t)StandardMaterial::MaterialTextureType::Max; ++i)
		{
			auto type = (StandardMaterial::MaterialTextureType)i;
			
			const char* keyName = Utils::TextureTypeToString(type);

			Ref<SpriteSheet> texture = mat->GetTexture(type);

			if(!texture->GetHandle())
				continue;

			out << YAML::Key << keyName << YAML::Value << (uint64_t)texture->GetHandle();
		}

		out << YAML::EndMap;


		std::ofstream file(path);

		file << out.c_str();
		file.close();
	}

	void MaterialSerializer::Deserialize(Ref<StandardMaterial> mat, const Path& path, MetadataFile* mtdFile)
	{
		YAML::Node node = YAML::Load(std::ifstream(path));

		StandardMaterial::Initializer init;

		init.BaseColor = node["BaseColor"].as<glm::vec4>();

		init.Metalness = node["Metalness"].as<float>();
		init.MetalnessChannel = Utils::StringToTextureChannel(node["MetalnessChannel"].as<std::string>());

		init.Roughness = node["Roughness"].as<float>();
		init.RoughnessChannel = Utils::StringToTextureChannel(node["RoughnessChannel"].as<std::string>());

		for (uint32_t i = 0; i < (uint32_t)StandardMaterial::MaterialTextureType::Max; ++i)
		{
			auto type = (StandardMaterial::MaterialTextureType)i;

			const char* keyName = Utils::TextureTypeToString(type);
			
			auto texture = node[keyName];

			if (!texture)
				continue;

			AssetHandle textureHandle = texture.as<uint64_t>();

			init.Textures[(uint32_t)type] = AssetManager::ResolveDependency(textureHandle, AssetType::SpriteSheet).As<SpriteSheet>();
		}

		mat->Initialize(init);
	}

	void MaterialSerializer::Serialize(const Path& path, Ref<ShaderMaterial> mat, MetadataFile* mtdFile)
	{
		YAML::Emitter out;

		out << YAML::BeginMap;

		auto vm = mat->GetVisualShader();

		if (vm)
		{
			out << YAML::Key << "Shader" << YAML::Value << (uint64_t)vm->GetHandle();

			out << YAML::Key << "Parameters" << YAML::Value << YAML::BeginMap;

			for (auto& [name, param] : vm->Parameters)
			{

				out << YAML::Key << name << YAML::Value;
				
				bool isTexture = param.Type == VisualShaderParameterType::Texture;

				if (!isTexture)
				{
					void* dataPtr = mat->GetUniformData() + param.ByteOffset;

					switch (param.Type)
					{
					case VisualShaderParameterType::Bool: out << *(bool*)dataPtr; break;
					case VisualShaderParameterType::Int: out << *(int*)dataPtr; break;
					case VisualShaderParameterType::UInt: out << *(uint32_t*)dataPtr; break;
					case VisualShaderParameterType::Float: out << *(float*)dataPtr; break;
					case VisualShaderParameterType::Vec2: out << *(glm::vec2*)dataPtr; break;
					case VisualShaderParameterType::Vec3: out << *(glm::vec3*)dataPtr; break;
					case VisualShaderParameterType::Vec4: out << *(glm::vec4*)dataPtr; break;
					}
				}
				else
				{
					//out << (uint64_t)DynamicPointerCast<Texture2D>(mat->GetTexture(param.Set, param.Binding))->
				}
			}

			out << YAML::EndMap;
		}
		
		out << YAML::EndMap;

		std::ofstream file(path);
		file << out.c_str();
	}

	void MaterialSerializer::Deserialize(Ref<ShaderMaterial> mat, const Path& path, MetadataFile* mtdFile)
	{
		YAML::Node node = YAML::Load(std::ifstream(path));
		if (node["Shader"])
		{
			AssetHandle shader = node["Shader"].as<uint64_t>();
			
			auto vs = AssetManager::ResolveDependency(shader, AssetType::VisualShader).As<VisualShader>();

			mat->SetVisualShader(vs);

			for (auto param : node["Parameters"])
			{
				auto name = param.first.as<std::string>();
				auto val = param.second;

				auto it = vs->Parameters.find(name);

				if (it == vs->Parameters.end())
				{
					continue;
				}

				void* dataPtr = mat->GetUniformData() + it->second.ByteOffset;

				switch (it->second.Type)
				{
				case VisualShaderParameterType::Bool: *(bool*)dataPtr = val.as<bool>(); break;
				case VisualShaderParameterType::Int: *(int*)dataPtr = val.as<int>(); break;
				case VisualShaderParameterType::UInt: *(uint32_t*)dataPtr = val.as<uint32_t>(); break;
				case VisualShaderParameterType::Float: *(float*)dataPtr = val.as<float>(); break;
				case VisualShaderParameterType::Vec2: *(glm::vec2*)dataPtr = val.as<glm::vec2>(); break;
				case VisualShaderParameterType::Vec3: *(glm::vec3*)dataPtr = val.as<glm::vec3>(); break;
				case VisualShaderParameterType::Vec4: *(glm::vec4*)dataPtr = val.as<glm::vec4>(); break;
				case VisualShaderParameterType::Texture: 
				{
					auto texture = AssetManager::ResolveDependency(val.as<uint64_t>(), AssetType::SpriteSheet).As<SpriteSheet>();

					mat->SetTexture(texture->GetTexture(), it->second.Set, it->second.Binding);
				} break;
				}
				
			}
		}
	}

	
	void MaterialSerializer::Serialize(const Path& path, Ref<VisualShader> mat, MetadataFile* mtdFile)
	{
		YAML::Emitter out;

		out << YAML::BeginMap;

		out << YAML::Key << "MainNodePos" << YAML::Value << mat->Nodes.at(0)->Position;

		out << YAML::Key << "Nodes" << YAML::Value << YAML::BeginSeq;
		
		auto& nodes = mat->Nodes;

		for (auto& [nodeID, node] : nodes)
		{
			if (nodeID == 0)
			{
				continue;
			}

			out << YAML::BeginMap;

			out << YAML::Key << "Type" << YAML::Value << node->GetTypename();
			
			out << YAML::Key << "NodeID" << YAML::Value << node->NodeID;

			out << YAML::Key << "Position" << YAML::Value << node->Position;

			node->Serialize(out);

			if (node->Parameter.Active)
			{
				out << YAML::Key << "ParameterName" << YAML::Value << node->Parameter.Name;
			}

			out << YAML::EndMap;
		}
		
		out << YAML::EndSeq;

		out << YAML::Key << "Connections" << YAML::Value << YAML::BeginSeq;
		
		auto& connections = mat->Connections;

		for (auto& conn : connections)
		{
			out << YAML::BeginMap;

			out << YAML::Key << "From" << YAML::Value;

			Utils::SerializeConnectionEnd(out, conn.FromNode, conn.FromPort);

			out << YAML::Key << "To" << YAML::Value;

			Utils::SerializeConnectionEnd(out, conn.ToNode, conn.ToPort);

			out << YAML::EndMap;
		}

		out << YAML::EndSeq;

		out << YAML::Key << "DefaultTextures" << YAML::Value << YAML::BeginMap;

		for (auto& [name, defTexture] : mat->DefaultTextureValues)
		{
			//out << YAML::Key << name << YAML::Value << (uint64_t)defTexture->GetHandle();
		}

		out << YAML::EndMap;

		out << YAML::EndMap;

		std::ofstream file(path);

		file << out.c_str();
	}

	void MaterialSerializer::Deserialize(Ref<VisualShader> mat, const Path& path, MetadataFile* mtdFile)
	{
		YAML::Node node = YAML::Load(std::ifstream(path));

		mat->Nodes.at(0)->Position = node["MainNodePos"].as<glm::vec2>();

		for (auto& node : node["Nodes"])
		{
			std::string type = node["Type"].as<std::string>();

			uint32_t nodeID = node["NodeID"].as<uint32_t>();

			glm::vec2 position = node["Position"].as<glm::vec2>();

			std::string paramName;
			if (node["ParameterName"])
			{
				paramName = node["ParameterName"].as<std::string>();
			}

#define DESERIALIZE(t) \
			if(t::GetStaticTypename() == type)\
			{\
				auto created = CreateScope<t>();\
				created->Deserialize(node);\
				created->NodeID = nodeID;\
				created->Position = position;\
				created->Parameter.Name = paramName;\
				created->Parameter.Active = !paramName.empty();\
				mat->Nodes[nodeID] = std::move(created);\
				mat->NodesCreated = glm::max(mat->NodesCreated, nodeID + 1);\
				continue;\
			}

			DESERIALIZE(VisualShaderNodeBoolConstant);
			DESERIALIZE(VisualShaderNodeIntConstant);
			DESERIALIZE(VisualShaderNodeUIntConstant);
			DESERIALIZE(VisualShaderNodeFloatConstant);
			DESERIALIZE(VisualShaderNodeVec2Constant);
			DESERIALIZE(VisualShaderNodeVec3Constant);
			DESERIALIZE(VisualShaderNodeVec4Constant);

			DESERIALIZE(VisualShaderNodeCompare);
			DESERIALIZE(VisualShaderNodeSwitch);
			
			DESERIALIZE(VisualShaderNodeEasing);
			
			DESERIALIZE(VisualShaderNodeIntFunc);
			DESERIALIZE(VisualShaderNodeUIntFunc);
			DESERIALIZE(VisualShaderNodeFloatFunc);
			DESERIALIZE(VisualShaderNodeVectorFunc);
			
			DESERIALIZE(VisualShaderNodeIntOp);
			DESERIALIZE(VisualShaderNodeUIntOp);
			DESERIALIZE(VisualShaderNodeFloatOp);
			DESERIALIZE(VisualShaderNodeVectorOp);
			
			if (VisualShaderNodeInput::GetStaticTypename() == type) 
			{
				auto created = CreateScope<VisualShaderNodeInput>(); 
				created->Deserialize(node);
				created->NodeID = nodeID;
				created->Position = position;
				created->Parameter.Name = paramName;
				created->Parameter.Active = !paramName.empty();
				mat->Nodes[nodeID] = std::move(created); 
				mat->NodesCreated = glm::max(mat->NodesCreated, nodeID + 1);
				continue;
			}
			
			DESERIALIZE(VisualShaderNodeTexture2D);
			DESERIALIZE(VisualShaderNodeSampler);

			KD_CORE_ASSERT(false);
 		}


		for (auto& conn : node["Connections"])
		{
			uint32_t fromNode, fromPort, toNode, toPort;

			fromNode = conn["From"]["Node"].as<uint32_t>();
			
			fromPort = conn["From"]["Port"].as<uint32_t>();
			
			toNode = conn["To"]["Node"].as<uint32_t>();
			
			toPort = conn["To"]["Port"].as<uint32_t>();

			mat->AddConnection(fromNode, fromPort, toNode, toPort);
		}

		std::vector<std::pair<std::string, AssetHandle>> defaultTextures;

		for (auto& defTexture : node["DefaultTextures"])
		{
			Asset asset;

			std::string textureName = defTexture.first.as<std::string>();

			AssetHandle textureHandle = defTexture.second.as<uint64_t>();

			if (AssetManager::IsAssetHandleValid(textureHandle))
			{
				//TODO: fix
				asset = AssetManager::GetAsset(textureHandle);
				mat->DefaultTextureValues[textureName] = asset.As<SpriteSheet>()->GetTexture();
			}
			else
			{
				defaultTextures.emplace_back(std::make_pair(textureName, textureHandle));
			}

		}

		if (!defaultTextures.empty())
		{
			Application::Get().SubmitToMainThread([mat, defaultTextures = std::move(defaultTextures)]()
			{
				for (auto& [name, handle] : defaultTextures)
				{
					mat->DefaultTextureValues[name] = AssetManager::GetAsset(handle).As<SpriteSheet>()->GetTexture();
				}
			});
		}

		mat->Recompile();
	}

}
