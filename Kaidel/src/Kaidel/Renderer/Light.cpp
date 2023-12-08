#include "KDpch.h"
#include "Kaidel/Renderer/Shader.h"
#include "Light.h"

namespace Kaidel {

	std::vector<_LightInternal> Light::s_InternalData;
	std::vector<Light*> Light::s_Lights;
	Light::Light() {
		m_MaterialIndex = s_InternalData.size();
		s_InternalData.emplace_back(_LightInternal{});
		s_Lights.push_back(this);
	}
	Light::~Light(){
		std::swap(s_InternalData[m_MaterialIndex], s_InternalData.back());
		std::swap(s_Lights[m_MaterialIndex], s_Lights.back());
		s_Lights[m_MaterialIndex]->m_MaterialIndex = m_MaterialIndex;
		s_InternalData.pop_back();
		s_Lights.pop_back();
	}



	void Light::SetLights()
	{
		static Ref<UAVInput> s_MaterialUAV = UAVInput::Create(s_InternalData.size() * sizeof(_LightInternal));
		s_MaterialUAV->SetBufferData(s_InternalData.data(), s_InternalData.size() * sizeof(_LightInternal));
		s_MaterialUAV->Bind(2);
	}

}
