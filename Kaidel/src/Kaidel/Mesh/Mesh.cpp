#include "KDpch.h"
#include "Mesh.h"
#include "Kaidel/Renderer/RenderCommand.h"
#include "Kaidel/Renderer/3D/Renderer3D.h"
namespace Kaidel {


	Mesh::Mesh(const std::string& meshName,const std::vector<MeshVertex>& vertices, const std::vector<uint32_t> indices,glm::vec3 center)
		:m_Vertices(vertices),m_IndexCount(indices.size()),m_MeshName(meshName)
	{
		KD_CORE_ASSERT(!vertices.empty());

		for (auto& vertex : m_Vertices) {
			vertex.Position -= center;	
		}

		glm::vec3 minPoint = vertices[0].Position;
		glm::vec3 maxPoint = vertices[0].Position;

		for (const auto& vertex : m_Vertices) {
			minPoint = glm::min(minPoint, vertex.Position);
			maxPoint = glm::max(maxPoint, vertex.Position);
		}

		m_BoundingBox.Min = minPoint;
		m_BoundingBox.Max = maxPoint;
		
		m_Center = center;
		Setup(indices);
	}
	
	
	void Mesh::Setup(const std::vector<uint32_t>& indices) {
		m_VBO = VertexBuffer::Create((float*)m_Vertices.data(),m_Vertices.size() * sizeof(MeshVertex));
		m_PerInstanceVBO = VertexBuffer::Create(1024*sizeof(MeshDrawData));
		Ref<IndexBuffer> ib = IndexBuffer::Create((uint32_t*)indices.data(), indices.size());


		BufferLayout vboLayout = {
		{ShaderDataType::Float3,"a_Position"},
		{ShaderDataType::Float3,"a_Normal"},
		{ShaderDataType::Float2,"a_TexCoords"},
		};


		m_VBO->SetLayout(vboLayout);

		BufferLayout pivboLayout = {
			{ShaderDataType::Mat4,"a_Transform",1},
			{ShaderDataType::Mat3,"a_NormalTransform",1},
			{ShaderDataType::Int,"a_MaterialID",1}
		};

		m_PerInstanceVBO->SetLayout(pivboLayout);

		VertexArraySpecification spec;
		spec.VertexBuffers = { m_VBO,m_PerInstanceVBO };
		spec.IndexBuffer = ib;
		spec.UsedShader = Renderer3D::GetMeshShader();
		m_VAO = VertexArray::Create(spec);
		m_IBO = ib;


		uint32_t prevStride = vboLayout.GetStride();

		vboLayout = {
		{ShaderDataType::Float3,"a_Position"}
		};
		vboLayout.SetStride(prevStride);

		prevStride = pivboLayout.GetStride();
		pivboLayout = {
			{ShaderDataType::Mat4,"a_Transform",1}
		};
		pivboLayout.SetStride(prevStride);
		m_VBO->SetLayout(vboLayout);
		m_PerInstanceVBO->SetLayout(pivboLayout);


		
		m_ShadowVAO = VertexArray::Create(spec);

	}
	bool Mesh::Draw(const glm::mat4& transform, Ref<Material>& mat) {
		if (!m_DrawData.CanReserveWithoutOverflow(1))
			return false;

		glm::mat3 normalTransform = glm::transpose(glm::inverse(glm::mat3(transform)));
		auto bvi = m_DrawData.Reserve(1);
		bvi[0].Transform = transform;
		bvi[0].NormalTransform = normalTransform;
		if (mat)
			bvi[0].MaterialID= mat->GetIndex();
		else
			bvi[0].MaterialID= 0;
		m_InstanceCount++;
		return true;
	}
	void Mesh::Flush() {
		if (m_InstanceCount == 0)
			return;
		RenderCommand::DrawIndexedInstanced(m_VAO, m_IndexCount, m_InstanceCount);
		m_InstanceCount = 0;
		m_DrawData.Reset();
		m_Flushed = true;
	}
}
