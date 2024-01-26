#pragma once

#include "Kaidel/Renderer/VertexArray.h"
#include "Kaidel/Renderer/Buffer.h"
#include "Kaidel/Renderer/Texture.h"
#include "Kaidel/Renderer/Material.h"
#include "Kaidel/Core/BoundedVector.h"
#include "Kaidel/Math/BoundingBox.h"
#include "Kaidel/Renderer/Material.h"
#include <glm/glm.hpp>
#include <vector>

namespace Kaidel {


	struct MeshVertex {
		glm::vec3 Position{};
		glm::vec3 Normal{};
		glm::vec2 TexCoords{};
	};

	struct MeshDrawData {
		glm::mat4 Transform;
		glm::mat3x4 NormalTransform;
		glm::ivec4 MaterialID;
	};



	class Mesh {
	public:
		Mesh() = default;
		Mesh(const std::string& meshName,const std::vector<MeshVertex>& vertices, const std::vector<uint32_t> indices,Ref<Material> mat,glm::vec3 center);
		Mesh(const Mesh& mesh);
		~Mesh() {
			int x = 3;
		}
		const Math::AABB& GetBoundingBox()const { return m_BoundingBox; }
		const std::string& GetMeshName()const { return m_MeshName; }
		bool Draw(const glm::mat4& transform,Ref<Material>& mat);
		void Flush();
		const glm::vec3& GetCenter()const { return m_Center; }
		
		Ref<Material> GetMaterial() const { return m_Material; }

		


	private:
		std::vector<MeshVertex> m_Vertices;
		Ref<VertexArray> m_VAO;
		Ref<VertexBuffer> m_VBO;
		uint32_t m_IndexCount = 0;
		uint32_t m_InstanceCount = 0;
		BoundedVector<MeshDrawData> m_DrawData = { 0,1024,[&](MeshDrawData* slot, uint64_t size) {
			this->Flush();
			} };;
		Ref<UAVInput> m_UAV;
		void Setup(const std::vector<uint32_t>& indices);

		Math::AABB m_BoundingBox{};
		std::string m_MeshName;
		bool m_Flushed = false;
		glm::vec3 m_Center;

		Ref<Material> m_Material;

		friend class Model;
		friend class Renderer3D;
	};

}
