#include "KDpch.h"
#include "Mesh.h"
#include "Kaidel/Renderer/RenderCommand.h"

namespace Kaidel {
	static Ref<Shader> ModelRenderingShader;
	Mesh::Mesh(const std::vector<MeshVertex>& vertices, const std::vector<uint32_t> indices,Ref<Material> mat) 
		:m_Vertices(vertices),m_Material(mat),m_IndexCount(indices.size())
	{
		Setup(indices);
		if (!ModelRenderingShader)
			ModelRenderingShader = Shader::Create("assets/shaders/Mesh/VS.glsl", "assets/shaders/Mesh/FS.glsl");
	}
	void Mesh::Setup(const std::vector<uint32_t>& indices) {
		m_VAO = VertexArray::Create();
		m_VBO = VertexBuffer::Create((float*)m_Vertices.data(),m_Vertices.size() * sizeof(MeshVertex));
		Ref<IndexBuffer> ib = IndexBuffer::Create((uint32_t*)indices.data(), indices.size());
		m_VBO->SetLayout({
			{ShaderDataType::Float3,"a_Position"},
			{ShaderDataType::Float3,"a_Normal"},
			{ShaderDataType::Float2,"a_TexCoords"},
			{ShaderDataType::Int,"a_MaterialIndex"},
			});
		m_VAO->AddVertexBuffer(m_VBO);
		m_VAO->SetIndexBuffer(ib);
	}
	void Mesh::Draw() {
		ModelRenderingShader->Bind();
		ModelRenderingShader->SetInt("u_MaterialTextures", 0);
		RenderCommand::DrawIndexed(m_VAO, m_IndexCount);
	}
}
