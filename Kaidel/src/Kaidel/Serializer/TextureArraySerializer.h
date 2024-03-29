#pragma once
#include "Kaidel/Renderer/GraphicsAPI/TextureArray.h"

#include "yaml-cpp/yaml.h"

namespace Kaidel {
	class TextureArraySerializer {
	public:
		TextureArraySerializer(uint32_t start = 0,uint32_t count = -1)
			:m_Start(start), m_Count(count)
		{}

		bool Serialize(Ref<Texture2DArray> array,const FileSystem::path& path);

		void Deserialize(Ref<Texture2DArray> array,const FileSystem::path& path);

		bool SerializeTo(Ref<Texture2DArray> array,YAML::Emitter& out);

		void DeserializeFrom(Ref<Texture2DArray> array,YAML::Node& node);

	private:
		bool SerializeLayer(Ref<Texture2DArray> array,const ArraySlotSpecification& spec, YAML::Emitter& out);
	private:
		uint32_t m_Start;
		uint32_t m_Count;



	};
}
