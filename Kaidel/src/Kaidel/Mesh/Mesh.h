#pragma once

#include "Kaidel/Renderer/GraphicsAPI/VertexArray.h"
#include "Kaidel/Renderer/GraphicsAPI/Buffer.h"
#include "Kaidel/Renderer/GraphicsAPI/Texture.h"
#include "Kaidel/Renderer/3D/Material.h"
#include "Kaidel/Core/BoundedVector.h"
#include "Kaidel/Math/BoundingBox.h"
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
		glm::mat3 NormalTransform;
		int MaterialID;
	};



	class Mesh : public IRCCounter<false> {
	public:
		Mesh() = default;
		Mesh(const std::string& meshName,const std::vector<MeshVertex>& vertices, const std::vector<uint32_t> indices,glm::vec3 center);
		~Mesh() {
			int x = 3;
		}
		const Math::AABB& GetBoundingBox()const { return m_BoundingBox; }
		const std::string& GetMeshName()const { return m_MeshName; }

		const glm::vec3& GetCenter()const { return m_Center; }
		
		Ref<Material> GetMaterial() const { return {}; }

		const auto& GetVertices() const { return m_Vertices; }
		auto GetVertexArray()const { return m_VAO; }
		auto GetVertexBuffer()const { return m_VBO; }
		auto GetPerInstanceBuffer()const { return m_PerInstanceVBO; }
		auto GetIndexBuffer()const { return m_IBO; }
		auto& GetDrawData() { return m_DrawData; }
		auto& GetShadowDrawData() { return m_ShadowDrawData; }
		auto GetShadowVAO()const { return m_ShadowVAO; }
		auto GetShadowPerInstanecBuffer()const { return m_ShadowPerInstanceVBO; }
	private:
		
		std::vector<MeshVertex> m_Vertices;
		Ref<VertexArray> m_VAO;
		Ref<VertexArray> m_ShadowVAO;

		Ref<VertexBuffer> m_VBO;
		Ref<VertexBuffer> m_PerInstanceVBO;
		Ref<VertexBuffer> m_ShadowPerInstanceVBO;
		Ref<IndexBuffer> m_IBO;
		
		uint32_t m_IndexCount = 0;

		BoundedVector<MeshDrawData> m_DrawData = { 0,1024,[](MeshDrawData* slot, uint64_t size){}};
		BoundedVector<glm::mat4> m_ShadowDrawData = { 0,1024,[](auto,auto) {} };



		void Setup(const std::vector<uint32_t>& indices);

		Math::AABB m_BoundingBox{};
		std::string m_MeshName;
		glm::vec3 m_Center;

		friend class Model;
		friend class Renderer3D;
	};

}
