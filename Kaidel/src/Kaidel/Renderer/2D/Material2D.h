#pragma once
#include "Kaidel/Renderer/GraphicsAPI/Texture.h"
#include "Kaidel/Renderer/GraphicsAPI/TextureArray.h"
#include <glm/glm.hpp>
#include <vector>
#include <unordered_map>
#include <filesystem>
namespace Kaidel {
	struct _Material2DInternal {
		glm::vec4 Color = glm::vec4(1.0f);
		uint32_t Albedo = 0;
	};
	static inline constexpr uint32_t _Material2DInternalBindingSlot = 1;

	

	class Material2D : public IRCCounter<false> {
	public:
		Material2D();
		~Material2D();

		static inline const std::vector<_Material2DInternal>& GetMaterials() { return s_InternalData; }

		const glm::vec4& GetColor() const { return s_InternalData[m_MaterialIndex].Color; }
		uint32_t GetAlbedo()const { return s_InternalData[m_MaterialIndex].Albedo; }


		void SetColor(const glm::vec4& color) { s_InternalData[m_MaterialIndex].Color = color; }
		void SetAlbedo(uint32_t albedo) { s_InternalData[m_MaterialIndex].Albedo = albedo; }

		uint64_t GetIndex()const { return m_MaterialIndex; }
		
		static void SetMaterials();
	private:

		static uint64_t GetMaterialCount() { return s_InternalData.size(); }
		static std::vector<_Material2DInternal> s_InternalData;
		static std::vector<Material2D*> s_Materials;
		uint64_t m_MaterialIndex;
		friend class SceneRenderer;
	};

}
