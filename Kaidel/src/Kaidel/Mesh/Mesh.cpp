#include "KDpch.h"
#include "Mesh.h"
#include "Kaidel/Renderer/RenderCommand.h"
namespace Kaidel {

	static std::mutex s_Mutex;

	Mesh::Mesh(const std::vector<MeshVertex>& vertices, const std::vector<uint32_t> indices) 
		:m_Vertices(vertices),m_IndexCount(indices.size())
	{
		Setup(indices);
	}
	Mesh::~Mesh() {
		int x = 3;
	}
	Mesh::Mesh(const Mesh& mesh) {
		m_DrawData = { 0,1024,[&](MeshDrawData* slot, uint64_t size) {
			this->Flush();
			} };
		m_IndexCount = mesh.m_IndexCount;
		m_InstanceCount = mesh.m_InstanceCount;
		m_UAV = mesh.m_UAV;
		m_VAO = mesh.m_VAO;
		m_VBO = mesh.m_VBO;
		m_Vertices = mesh.m_Vertices;
	}

	void Mesh::Setup(const std::vector<uint32_t>& indices) {
		m_VAO = VertexArray::Create();
		m_VBO = VertexBuffer::Create((float*)m_Vertices.data(),m_Vertices.size() * sizeof(MeshVertex));
		Ref<IndexBuffer> ib = IndexBuffer::Create((uint32_t*)indices.data(), indices.size());
		m_VBO->SetLayout({
			{ShaderDataType::Float3,"a_Position"},
			{ShaderDataType::Float3,"a_Normal"},
			{ShaderDataType::Float2,"a_TexCoords"},
			});
		m_VAO->AddVertexBuffer(m_VBO);
		m_VAO->SetIndexBuffer(ib);
		m_UAV = UAVInput::Create(0, sizeof(MeshDrawData), nullptr);
	}
	void Mesh::Draw(const glm::mat4& transform, Ref<Material>& mat) {
		auto bvi = m_DrawData.Reserve(1);
		bvi[0].Transform = transform;
		bvi[0].NormalTransform = glm::transpose(glm::inverse(glm::mat3(transform)));
		bvi[0].MaterialID.x = mat->GetIndex();
		m_InstanceCount++;
	}
	void Mesh::Flush() {
		if (m_InstanceCount == 0)
			return;
		std::unique_lock<std::mutex> lock(s_Mutex);
		m_UAV->SetBufferData(m_DrawData.Get(), m_DrawData.Size());
		m_UAV->Bind(5);
		RenderCommand::DrawIndexedInstanced(m_VAO, m_IndexCount, m_InstanceCount);
		m_InstanceCount = 0;
		m_DrawData.Reset();
	}

}
