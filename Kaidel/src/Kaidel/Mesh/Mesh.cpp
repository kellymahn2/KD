#include "KDpch.h"
#include "Mesh.h"
#include "Kaidel/Renderer/RenderCommand.h"
#include "Kaidel/Renderer/3D/Renderer3D.h"
namespace Kaidel {


	Mesh::Mesh(const std::string& meshName,const std::vector<MeshVertex>& vertices, const std::vector<uint32_t> indices,glm::vec3 center)
		:m_Vertices(vertices),m_IndexCount((uint32_t)indices.size()),m_MeshName(meshName)
	{
		KD_CORE_ASSERT(!vertices.empty());

		for (auto& vertex : m_Vertices) {
			vertex.Position -= center;	
		}

		
		
		m_Center = center;
		Setup(indices);
	}
	
	
	void Mesh::Setup(const std::vector<uint32_t>& indices) {

		m_VBO = VertexBuffer::Create((float*)m_Vertices.data(),(uint32_t)(m_Vertices.size() * sizeof(MeshVertex)));

		m_PerInstanceVBO = VertexBuffer::Create(1024*sizeof(MeshDrawData));
		Ref<IndexBuffer> ib = IndexBuffer::Create((uint32_t*)indices.data(), (uint32_t)indices.size());


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

		m_ShadowPerInstanceVBO = VertexBuffer::Create(1024 * sizeof(glm::mat4));

		m_ShadowPerInstanceVBO->SetLayout({
			{ShaderDataType::Mat4,"a_Transform",1}
		});
		spec.VertexBuffers = { m_VBO,m_ShadowPerInstanceVBO };

		m_ShadowVAO = VertexArray::Create(spec);
	}
	
}
