#pragma once
#include <glm/glm.hpp>
#include <vector>
#include "Kaidel/Renderer/GraphicsAPI/Shader.h""
#include "Kaidel/Renderer/GraphicsAPI/Texture.h"
#include "Kaidel/Renderer/GraphicsAPI/Framebuffer.h"
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
		glm::mat4 LightViewProjection{ 1.0f };
		glm::vec3 Direction = { -.2f,-1.0f,-.3f };
		glm::vec3 Ambient{ .2f };
		glm::vec3 Diffuse{.5f};
		glm::vec3 Specular{ 1.0f };
	};
		
	struct _PointLightInternal {
		glm::mat4 LightViewProjection{ 1.0f };
		glm::vec3 Position{ 0.0f };
		glm::vec3 Ambient{ .2f };
		glm::vec3 Diffuse{ .5f };
		glm::vec3 Specular{ 1.0f };

		float ConstantCoefficient = 1.0f;
		float LinearCoefficient = 0.09f;
		float QuadraticCoefficient = 0.032f;
	};


	struct _SpotLightInternal {
		glm::mat4 LightViewProjection{ 1.0f };
		glm::vec4 Position{ 0.0f };
		glm::vec4 Direction{ 0.0f ,0,0,1};
		glm::vec4 Ambient{ .2f,.2,.2,1 };
		glm::vec4 Diffuse{ .5f ,.5,.5,1};
		glm::vec4 Specular{ 1.0f};
		//Half FOV
		float CutOffAngle = 0.5f;

		float ConstantCoefficient = 1.0f;
		float LinearCoefficient = 0.09f;
		float QuadraticCoefficient = 0.032f;
	};


	static inline constexpr uint32_t _PointLightBindingSlot = 2;
	static inline constexpr uint32_t _DirectionalLightBindingSlot = 3;
	static inline constexpr uint32_t _SpotLightBindingSlot = 4;
	static inline constexpr uint32_t _ShadowMapWidth = 2048;
	static inline constexpr uint32_t _ShadowMapHeight = 2048;

	static inline float CalcLightMaxCoverage(float q,float l,float _c,float epsilon) {
		float a = q;
		float b = l;
		float c = _c - epsilon;
		float d = b * b - 4.0f * a * c;
		if (d < 0) {
			return INFINITY;
		}
		float sqD = glm::sqrt(d);
		float x1 = std::max((-b + sqD) / (2.0f * a), 0.0f);
		float x2 = std::max((-b  - sqD) / (2.0f * a), 0.0f);
		return std::max(x1, x2);
	}

	template<typename T,uint32_t BindingSlot>
	class Light {
	public:
		Light() {
			m_LightIndex = s_InternalData.size();
			s_InternalData.emplace_back(T{});
			s_Lights.push_back(this);
			if (!s_LightDepthMaps)
				s_LightDepthMaps = Depth2DArray::Create(_ShadowMapWidth, _ShadowMapHeight);
			s_LightDepthMaps->PushDepth(_ShadowMapWidth,_ShadowMapHeight);
		}
		~Light() {
			std::swap(s_InternalData[m_LightIndex], s_InternalData.back());
			std::swap(s_Lights[m_LightIndex], s_Lights.back());
			s_Lights[m_LightIndex]->m_LightIndex = m_LightIndex;
			s_InternalData.pop_back();
			s_Lights.pop_back();
			s_LightDepthMaps->PopDepth();
		}
		T& GetLight(){ return s_InternalData[m_LightIndex]; }
		static inline Ref<Depth2DArray> GetDepthMaps() { return s_LightDepthMaps; }
		uint64_t GetIndex()const { return m_LightIndex; }
		static uint64_t GetLightCount() { return s_InternalData.size(); }
		static void SetLights() {
			static Ref<UAVInput> s_MaterialUAV = UAVInput::Create(s_InternalData.size(), sizeof(T));
			s_MaterialUAV->SetBufferData(s_InternalData.data(), s_InternalData.size());
			s_MaterialUAV->Bind(BindingSlot);
			if(!s_LightDepthMaps)
				s_LightDepthMaps = Depth2DArray::Create(_ShadowMapWidth, _ShadowMapHeight);
			for (auto& light : s_Lights) {
				s_LightDepthMaps->ClearLayer(light->m_LightIndex, 1.0f);
			}
			s_LightDepthMaps->Bind(BindingSlot);
		}

		static inline const std::vector<Light*>& GetLights(){ return s_Lights; }

	private:
		static inline std::vector<T> s_InternalData{};
		static inline std::vector<Light*> s_Lights{};
		static inline Ref<Depth2DArray> s_LightDepthMaps;
		uint64_t m_LightIndex;
		friend class SceneRenderer;
		friend static void BindLights(void* m_Context);
	};

	using PointLight = Light<_PointLightInternal, _PointLightBindingSlot>;
	using DirectionalLight = Light<_DirectionalLightInternal, _DirectionalLightBindingSlot>;
	using SpotLight = Light<_SpotLightInternal, _SpotLightBindingSlot>;
} 



