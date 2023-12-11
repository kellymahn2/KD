#pragma once
#include "Kaidel/Renderer/Texture.h"
#include <glm/glm.hpp>
#include <vector>
#include <map>
#include <stack>
namespace Kaidel
{

	struct _MaterialInternal{
		glm::vec4 Color = glm::vec4{ 1.0f };
		glm::vec3 Ambient = glm::vec3{ .2f };
		glm::vec3 Diffuse = glm::vec3{ 1.0f };
		glm::vec3 Specular = glm::vec3{ .5f };
		float Shininess = 32.0f;
	};
    class Material{
    public:
		Material();
		~Material();
		static inline const std::vector<_MaterialInternal>& GetMaterials(){return s_InternalData;}
		const glm::vec4& GetColor() const{return s_InternalData[m_MaterialIndex].Color;}
		const glm::vec3& GetAmbient() const{return s_InternalData[m_MaterialIndex].Ambient;}
		const glm::vec3& GetDiffuse() const{return s_InternalData[m_MaterialIndex].Diffuse;}
		const glm::vec3& GetSpecular() const{return s_InternalData[m_MaterialIndex].Specular;}
		float GetShininess()const{return s_InternalData[m_MaterialIndex].Shininess;}
		void SetColor(const glm::vec4& color) {s_InternalData[m_MaterialIndex].Color = color;}
		void SetAmbient(const glm::vec3& ambient) {s_InternalData[m_MaterialIndex].Ambient = ambient;}
		void SetDiffuse(const glm::vec3& diffuse) {s_InternalData[m_MaterialIndex].Diffuse = diffuse;}
		void SetSpecular(const glm::vec3& specular) {s_InternalData[m_MaterialIndex].Specular = specular;}
		void SetShininess (float shininess){s_InternalData[m_MaterialIndex].Shininess = shininess;}
		uint64_t GetIndex()const { return m_MaterialIndex; }
    private:
		static uint64_t GetMaterialCount() { return s_InternalData.size(); }
		static void SetMaterials();
		static std::vector<_MaterialInternal> s_InternalData;
		static std::vector<Material*> s_Materials;
		uint64_t m_MaterialIndex;
		friend class SceneRenderer;
    }; 
} 
