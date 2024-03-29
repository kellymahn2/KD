#include "KDpch.h"
#include "Material.h"
#include "Kaidel/Renderer/GraphicsAPI/Shader.h"

namespace Kaidel{
    std::vector<_MaterialInternal> Material::s_InternalData;
	std::vector<Material*> Material::s_Materials;

	Material::Material() {
       m_MaterialIndex = s_InternalData.size();
       s_InternalData.emplace_back();
       s_Materials.push_back(this);
    }
	Material::~Material() {
		if (!s_InternalData.empty()) {
			std::swap(s_InternalData[m_MaterialIndex],s_InternalData.back());
			std::swap(s_Materials[m_MaterialIndex],s_Materials.back());
			s_Materials[m_MaterialIndex]->m_MaterialIndex = m_MaterialIndex;
			s_InternalData.pop_back();
			s_Materials.pop_back();
		}
    }
    void Material::SetMaterials(){
        static Ref<UAVInput> s_MaterialUAV = UAVInput::Create(s_InternalData.size(),sizeof(_MaterialInternal));
        s_MaterialUAV->SetBufferData(s_InternalData.data(),s_InternalData.size());
        s_MaterialUAV->Bind(_MaterialInternalBindingSlot);
    }

}
