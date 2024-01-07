#pragma once
#include <glm/glm.hpp>
#include <vector>
#include "Kaidel/Renderer/VertexArray.h"
#include "Kaidel/Renderer/Buffer.h"
#include "Kaidel/Renderer/Texture.h"
#include "Kaidel/Renderer/Material.h"
namespace Kaidel {


	struct MeshVertex {
		glm::vec3 Position;
		glm::vec3 Normal;
		glm::vec2 TexCoords;
		int MaterialIndex;
	};

	class Mesh {
	public:
		Mesh(const std::vector<MeshVertex>& vertices, const std::vector<uint32_t> indices,Ref<Material> mat);

	private:
		std::vector<MeshVertex> m_Vertices;
		Ref<VertexArray> m_VAO;
		Ref<VertexBuffer> m_VBO;
		Ref<Material> m_Material;
		uint32_t m_IndexCount;
		void Setup(const std::vector<uint32_t>& indices);
		void Draw();
		friend class Model;
	};
}
