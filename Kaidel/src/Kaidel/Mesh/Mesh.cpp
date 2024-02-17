#include "KDpch.h"
#include "Mesh.h"
#include "Kaidel/Renderer/RenderCommand.h"
#include "Kaidel/Renderer/3D/Renderer3D.h"
namespace Kaidel {


	Mesh::Mesh(const std::string& meshName,const std::vector<MeshVertex>& vertices, const std::vector<uint32_t> indices,Ref<Material> mat,glm::vec3 center)
		:m_Vertices(vertices),m_IndexCount(indices.size()),m_Material(mat),m_MeshName(meshName)
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
	
	
	Mesh::Mesh(const Mesh& mesh) 
	{
		m_DrawData = { 0,1024,[&](MeshDrawData* slot, uint64_t size) {
			this->Flush();
			} };
		m_IndexCount = mesh.m_IndexCount;
		m_InstanceCount = mesh.m_InstanceCount;
		m_UAV = mesh.m_UAV;
		m_VAO = mesh.m_VAO;
		m_VBO = mesh.m_VBO;
		m_Vertices = mesh.m_Vertices;
		m_BoundingBox = mesh.m_BoundingBox;
		m_Center = mesh.m_Center;
		m_MeshName = mesh.m_MeshName;
		m_Material = mesh.m_Material;
		m_PerInstanceVBO = mesh.m_PerInstanceVBO;
		m_IBO = mesh.m_IBO;
	}
	void Mesh::Setup(const std::vector<uint32_t>& indices) {
		m_VBO = VertexBuffer::Create((float*)m_Vertices.data(),m_Vertices.size() * sizeof(MeshVertex));
		m_PerInstanceVBO = VertexBuffer::Create(1024*sizeof(MeshDrawData));
		Ref<IndexBuffer> ib = IndexBuffer::Create((uint32_t*)indices.data(), indices.size());
		m_VBO->SetLayout({
			{ShaderDataType::Float3,"a_Position"},
			{ShaderDataType::Float3,"a_Normal"},
			{ShaderDataType::Float2,"a_TexCoords"},
			});
		m_PerInstanceVBO->SetLayout({
			{ShaderDataType::Mat4,"a_Transform",1},
			{ShaderDataType::Mat3,"a_NormalTransform",1},
			{ShaderDataType::Int,"a_MaterialID",1}
			});
		
		VertexArraySpecification spec;
		spec.VertexBuffers = { m_VBO,m_PerInstanceVBO };
		spec.IndexBuffer = ib;
		spec.UsedShader = Renderer3D::GetMeshShader();
		m_VAO = VertexArray::Create(spec);
		m_UAV = UAVInput::Create(0, sizeof(MeshDrawData), nullptr);
		m_IBO = ib;
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
		m_UAV->SetBufferData(m_DrawData.Get(), m_DrawData.Size());
		m_UAV->Bind(5);
		RenderCommand::DrawIndexedInstanced(m_VAO, m_IndexCount, m_InstanceCount);
		m_InstanceCount = 0;
		m_DrawData.Reset();
		m_Flushed = true;
	}
}
