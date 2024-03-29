#include "KDpch.h"
#include "TextureArraySerializer.h"

#include "yaml-cpp/yaml.h"

namespace Kaidel {


	bool TextureArraySerializer::Serialize(Ref<Texture2DArray> array,const FileSystem::path& path){
		YAML::Emitter out;
		bool res = SerializeTo(array,out);

		if (!res)
			return false;

		std::ofstream file(path);
		file << out.c_str();
	}
	void TextureArraySerializer::Deserialize(Ref<Texture2DArray> array, const FileSystem::path& path) {
		std::ifstream file(path);

		YAML::Node node = YAML::Load(file);

		DeserializeFrom(array,node);

	}


	bool TextureArraySerializer::SerializeLayer(Ref<Texture2DArray> array,const ArraySlotSpecification& spec, YAML::Emitter& out) {
		out << YAML::BeginMap;
		
		if (spec.Data!="") {
			out << YAML::Key << "Data" << YAML::Value << spec.Data;
			out << YAML::Key << "Width" << YAML::Value << spec.LoadedWidth;
			out << YAML::Key << "Height" << YAML::Value << spec.LoadedHeight;
			out << YAML::Key << "ShouldFlip" << YAML::Value << spec.ShouldFlip;
		}
		else {
			out << YAML::Key << "Path" << YAML::Value << spec.Source;
			out << YAML::Key << "ShouldFlip" << YAML::Value << spec.ShouldFlip;
		}

		out << YAML::EndMap;

		return true;
	}

	bool TextureArraySerializer::SerializeTo(Ref<Texture2DArray> array,YAML::Emitter& out){

		const auto& specs = array->GetSpecifications();

		out << YAML::BeginMap;

		out << YAML::Key << "Width" << YAML::Value << array->GetWidth();
		out << YAML::Key << "Height" << YAML::Value << array->GetHeight();

		out << YAML::Key << "Layers" << YAML::Value << YAML::BeginSeq;
	
		uint32_t count = m_Count;
		if (count == -1)
			count = array->GetLayerCount()-m_Start;

		for (uint32_t i = m_Start; i < m_Start + count; ++i) {
			SerializeLayer(array,specs[i], out);
		}

		out << YAML::EndSeq;

		out << YAML::EndMap;
		return true;
	}


	void TextureArraySerializer::DeserializeFrom(Ref<Texture2DArray> array, YAML::Node& node) {

		auto layers = node["Layers"];
		if (!layers)
			return;
		for (auto layerNode : layers) {

			{
				auto dataNode = layerNode["Data"];
				if (dataNode) {

					uint32_t w = layerNode["Width"].as<uint32_t>();
					uint32_t h = layerNode["Height"].as<uint32_t>();
					std::string data = dataNode.as<std::string>();
					bool shouldFlip = layerNode["ShouldFlip"].as<bool>();
					array->PushTexture((void*)data.c_str(), w, h, shouldFlip);
					continue;
				}
			}
			{
				auto pathNode = layerNode["Path"];

				if (pathNode) {
					std::string path = pathNode.as<std::string>();
					bool shouldFlip = layerNode["ShouldFlip"].as<bool>();
					array->PushTexture(path, shouldFlip);
					continue;
				}
			}
		}
	
	}
}
