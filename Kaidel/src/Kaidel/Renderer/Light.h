#pragma once
#include <glm/glm.hpp>
#include <vector>
#include <Kaidel/Renderer/Shader.h>
namespace Kaidel
{
	//struct _LightInternal{
	//	glm::vec3 Position = glm::vec3{ 0.0f };
 //   	//glm::vec3 Direction;
	//	glm::vec3 AmbientIntensity = glm::vec3{.1f};
 //   	glm::vec3 DiffuseIntensity = glm::vec3{1.0f};
	//	glm::vec3 SpecularIntensity = glm::vec3{ .5f };
	//	/*float ConstantAttenuation;
 //   	float LinearAttenuation;
 //   	float QuadraticAttenuation;*/
 //   };

	struct _DirectionalLightInternal {
		glm::vec3 Direction = { -.2f,-1.0f,-.3f };
		glm::vec3 Ambient{ .6f };
		glm::vec3 Diffuse{1.0f};
		glm::vec3 Specular{ 1.0f };
	};
		
	static inline constexpr uint32_t _DirectionalLightBindingSlot = 3;
	static inline constexpr uint32_t _LightBindingSlot = 2;



	template<typename T,uint32_t BindingSlot>
	class Light {
	public:
		Light() {
			m_LightIndex = s_InternalData.size();
			s_InternalData.emplace_back(T{});
			s_Lights.push_back(this);
		}
		~Light() {
			std::swap(s_InternalData[m_LightIndex], s_InternalData.back());
			std::swap(s_Lights[m_LightIndex], s_Lights.back());
			s_Lights[m_LightIndex]->m_LightIndex = m_LightIndex;
			s_InternalData.pop_back();
			s_Lights.pop_back();
		}
		T& GetLight(){ return s_InternalData[m_LightIndex]; }

	private:
		static uint64_t GetLightCount() { return s_InternalData.size(); }
		static void SetLights() {
			static Ref<UAVInput> s_MaterialUAV = UAVInput::Create(s_InternalData.size(), sizeof(T));
			s_MaterialUAV->SetBufferData(s_InternalData.data(), s_InternalData.size());
			s_MaterialUAV->Bind(BindingSlot);
		}
		static inline std::vector<T> s_InternalData{};
		static inline std::vector<Light*> s_Lights{};
		uint64_t m_LightIndex;
		friend class SceneRenderer;
	};

	using DirectionalLight = Light<_DirectionalLightInternal, _DirectionalLightBindingSlot>;
} 



