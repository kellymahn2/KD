#pragma once
#include "Kaidel/Renderer/GraphicsAPI/Texture.h"
#include <glm/glm.hpp>
#include <vector>
#include <unordered_map>
#include <stack>
#include <filesystem>
namespace Kaidel
{

	struct _MaterialInternal{
		glm::vec4 Color = glm::vec4{ 1.0f };
		uint32_t Diffuse = 0;
		uint32_t Specular = 1;
		float Shininess = 32.0f;
	};

	static inline constexpr uint32_t _MaterialTexturesBindingSlot = 0;
	static inline constexpr uint32_t _MaterialInternalBindingSlot = 1;


	class MaterialTextureHandler {
	public:
		static void Init();
		static uint32_t LoadTexture(const std::filesystem::path& texturePath);
		static uint32_t LoadTexture(void* data,uint32_t width,uint32_t height);
		static const std::unordered_map<std::string, uint32_t>& GetTextureIndexMap() { return s_TextureIndexMap; }
		static Ref<Texture2DArray> GetTexturesMap() { return s_TexturesMap; }
	private:
		static inline std::unordered_map<std::string, uint32_t> s_TextureIndexMap;
		static inline Ref<Texture2DArray> s_TexturesMap;
	};

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
		static void SetMaterials();
    private:

		static uint64_t GetMaterialCount() { return s_InternalData.size(); }
		static std::vector<_MaterialInternal> s_InternalData;
		static std::vector<Material*> s_Materials;
		uint64_t m_MaterialIndex;
		friend class SceneRenderer;
    }; 
} 
