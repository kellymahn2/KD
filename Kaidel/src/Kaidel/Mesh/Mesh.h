#pragma once

#include "Kaidel/Renderer/VertexArray.h"
#include "Kaidel/Renderer/Buffer.h"
#include "Kaidel/Renderer/Texture.h"
#include "Kaidel/Renderer/Material.h"
#include "Kaidel/Core/BoundedVector.h"
#include "Kaidel/Math/BoundingBox.h"

#include <glm/glm.hpp>
#include <vector>
namespace Kaidel {


	struct MeshVertex {
		glm::vec3 Position;
		glm::vec3 Normal;
		glm::vec2 TexCoords;
	};

	struct MeshDrawData {
		glm::mat4 Transform;
		glm::mat3x4 NormalTransform;
		glm::ivec4 MaterialID;
	};

	class Mesh {
	public:
		Mesh() = default;
		Mesh(const std::vector<MeshVertex>& vertices, const std::vector<uint32_t> indices);
		Mesh(const Mesh& mesh);
		const Math::BoundingBox& GetBoundingBox()const { return m_BoundingBox; }
		~Mesh();
	private:
		std::vector<MeshVertex> m_Vertices;
		Ref<VertexArray> m_VAO;
		Ref<VertexBuffer> m_VBO;
		uint32_t m_IndexCount;
		uint32_t m_InstanceCount = 0;
		BoundedVector<MeshDrawData> m_DrawData = { 0,1024,[&](MeshDrawData* slot, uint64_t size) {
			this->Flush();
			} };;
		Ref<UAVInput> m_UAV;
		void Setup(const std::vector<uint32_t>& indices);
		void Draw(const glm::mat4& transform,Ref<Material>& mat);
		void Flush();
		Math::BoundingBox m_BoundingBox;

		friend class Model;
	};
}
