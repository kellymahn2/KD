#include "KDpch.h"
#include "Material2D.h"
#include "Kaidel/Renderer/GraphicsAPI/Shader.h"
namespace Kaidel {
	std::vector<_Material2DInternal> Material2D::s_InternalData;
	std::vector<Material2D*> Material2D::s_Materials;


	Material2D::Material2D(){
		m_MaterialIndex = s_InternalData.size();
		s_InternalData.emplace_back();
		s_Materials.push_back(this);
	}
	Material2D::~Material2D() {
		if (!s_InternalData.empty()) {
			std::swap(s_InternalData[m_MaterialIndex], s_InternalData.back());
			std::swap(s_Materials[m_MaterialIndex], s_Materials.back());
			s_Materials[m_MaterialIndex]->m_MaterialIndex = m_MaterialIndex;
			s_InternalData.pop_back();
			s_Materials.pop_back();
		}
	}
	void Material2D::SetMaterials() {
		static Ref<UAVInput> s_MaterialUAV = UAVInput::Create(s_InternalData.size(), sizeof(_Material2DInternal));
		s_MaterialUAV->SetBufferData(s_InternalData.data(), s_InternalData.size());
		s_MaterialUAV->Bind(_Material2DInternalBindingSlot);
	}

	uint32_t Material2DTextureHandler::LoadTexture(void* data, uint32_t width, uint32_t height) {
		return s_TexturesMap->PushTexture(data, width, height, false);
	}


	void Material2DTextureHandler::Init() {
		s_TexturesMap = Texture2DArray::Create(1024, 1024);
		uint32_t default = 0xffffffff;
		s_TexturesMap->PushTexture(&default, 1, 1);
		s_TextureIndexMap["_Default_Albedo"] = 0;
	}
	uint32_t Material2DTextureHandler::LoadTexture(const std::filesystem::path& texturePath) {
		std::string path = texturePath.string();
		if (s_TextureIndexMap.find(path) != s_TextureIndexMap.end())
			return s_TextureIndexMap.at(path);
		uint32_t index = s_TexturesMap->PushTexture(path, false);
		s_TextureIndexMap[path] = index;
		return index;
	}

}
