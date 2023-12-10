#pragma once
#include <glm/glm.hpp>
#include <vector>
namespace Kaidel
{
	struct _LightInternal{
		glm::vec3 Color = glm::vec3{ 1.0f };
		glm::vec3 Position = glm::vec3{ 0.0f };
    	//glm::vec3 Direction;
		float AmbientIntensity = .1f;
    	float DiffuseIntensity = 1.0f;
    	float SpecularIntensity = .5f;
		/*float ConstantAttenuation;
    	float LinearAttenuation;
    	float QuadraticAttenuation;*/
    };
    class Light{
    public:
		Light();
		
		~Light();
		static inline const std::vector<_LightInternal>& GetLights(){return s_InternalData;}
		const glm::vec3& GetColor()		const {return s_InternalData[m_MaterialIndex].Color;}
		const glm::vec3& GetPosition()	const { return s_InternalData[m_MaterialIndex].Position;}
		//const glm::vec3& GetDirection()	const { return s_InternalData[m_MaterialIndex].Direction;}
		float GetAmbientIntensity()		const { return s_InternalData[m_MaterialIndex].AmbientIntensity;}
		float GetDiffuseIntensity()		const { return s_InternalData[m_MaterialIndex].DiffuseIntensity;}
		float GetSpecularIntensity()	const { return s_InternalData[m_MaterialIndex].SpecularIntensity;}
		/*float GetConstantAttenuation()	const { return s_InternalData[m_MaterialIndex].ConstantAttenuation;}
		float GetLinearAttenuation()	const { return s_InternalData[m_MaterialIndex].LinearAttenuation;}
		float GetQuadraticAttenuation()	const { return s_InternalData[m_MaterialIndex].QuadraticAttenuation;}*/

		void SetColor				(const glm::vec3& color )		const {s_InternalData[m_MaterialIndex].Color = color; }
		void SetPosition			(const glm::vec3& position)		const {s_InternalData[m_MaterialIndex].Position = position; }
		//void SetDirection			(const glm::vec3& direction)	const {s_InternalData[m_MaterialIndex].Direction = direction; }
		void SetAmbientIntensity	(float ambientIntensity)		const {s_InternalData[m_MaterialIndex].AmbientIntensity = ambientIntensity; }
		void SetDiffuseIntensity	(float diffuseIntensity)		const {s_InternalData[m_MaterialIndex].DiffuseIntensity = diffuseIntensity; }
		void SetSpecularIntensity	(float specularIntensity)		const {s_InternalData[m_MaterialIndex].SpecularIntensity = specularIntensity; }
		/*void SetConstantAttenuation	(float constantAttenuation)		const {s_InternalData[m_MaterialIndex].ConstantAttenuation = constantAttenuation; }
		void SetLinearAttenuation	(float linearAttenuation)		const {s_InternalData[m_MaterialIndex].LinearAttenuation = linearAttenuation; }
		void SetQuadraticAttenuation(float quadraticAttenuation)	const {s_InternalData[m_MaterialIndex].QuadraticAttenuation = quadraticAttenuation; }*/

    private:
		static uint64_t GetLightCount() { return s_InternalData.size(); }
		static void SetLights();
		static std::vector<_LightInternal> s_InternalData;
		static std::vector<Light*> s_Lights;
		uint64_t m_MaterialIndex;
		friend class SceneRenderer;
    }; 
} 



