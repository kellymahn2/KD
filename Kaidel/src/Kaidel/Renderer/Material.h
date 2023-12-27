#pragma once
#include "Kaidel/Renderer/Texture.h"
#include <glm/glm.hpp>
#include <vector>
#include <unordered_map>
#include <stack>
namespace Kaidel
{

	struct _MaterialInternal{
		glm::vec4 Color = glm::vec4{ 1.0f };
		uint32_t Diffuse = 0;
		uint32_t Specular = 1;
		float Shininess = 32.0f;
	};

	static inline constexpr uint32_t _MaterialTexturesBindingSlot = 0;

    class Material{
    public:
		Material();
		~Material();

		static inline const std::vector<_MaterialInternal>& GetMaterials(){return s_InternalData;}

		const glm::vec4& GetColor() const{return s_InternalData[m_MaterialIndex].Color;}
		uint32_t GetDiffuse() { return s_InternalData[m_MaterialIndex].Diffuse; }
		uint32_t GetSpecular() { return s_InternalData[m_MaterialIndex].Specular; }
		float GetShininess()const{return s_InternalData[m_MaterialIndex].Shininess;}



		void SetColor(const glm::vec4& color) {s_InternalData[m_MaterialIndex].Color = color;}
		void SetDiffuse(uint32_t diffuse) { s_InternalData[m_MaterialIndex].Diffuse = diffuse; }
		void SetSpecular(uint32_t specular) { s_InternalData[m_MaterialIndex].Specular = specular; }
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
