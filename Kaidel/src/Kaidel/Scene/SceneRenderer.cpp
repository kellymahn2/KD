#include "KDpch.h"
#include "SceneRenderer.h"
#include "Components.h"
#include "Scene.h"
#include "Entity.h"

#include "Kaidel/Renderer/GraphicsAPI/PerFrameResource.h"

#include "Kaidel/Renderer/GraphicsAPI/UniformBuffer.h"
#include "Kaidel/Renderer/2D\Renderer2D.h"
#include "Kaidel/Core/JobSystem.h"
#include "Kaidel/Core/Timer.h"
#include "Kaidel/Renderer/RenderCommand.h"
#include "Kaidel/Renderer/GraphicsAPI/VertexBuffer.h"
#include "Kaidel/Renderer/GraphicsAPI/IndexBuffer.h"
#include "Kaidel/Renderer/GraphicsAPI/ShaderLibrary.h"
#include "Kaidel/Renderer/GraphicsAPI/TextureLibrary.h"
#include "Kaidel/Renderer/DescriptorSetPack.h"

#include <assimp/Importer.hpp>
#include <assimp/mesh.h>
#include <assimp/scene.h>
#include <assimp/postprocess.h>

#include <glm/gtx/compatibility.hpp>
#include <chrono>
#include <cmath>
#include <iostream>


#define MAX_LIGHT_COUNT 100

namespace Kaidel {
	static struct Data {
		Ref<VertexBuffer> CubeVertexBuffer;
		Ref<IndexBuffer> CubeIndexBuffer;
		Ref<ComputePipeline> ClusterPipeline;
		PerFrameResource<DescriptorSetPack> ClusterPipelinePack;
		Ref<ComputePipeline> LightCullPipeline;
		PerFrameResource<DescriptorSetPack> LightCullPipelinePack;

		//PerFrameResource<Ref<UniformBuffer>> CameraBuffers;
		//PerFrameResource<Ref<DescriptorSet>> CameraBufferSets;

		PerFrameResource<Ref<StorageBuffer>> Clusters;
		PerFrameResource<Ref<StorageBuffer>> ClusterGrids;

		Ref<RenderPass> GBufferRenderPass;
		PerFrameResource<Ref<Framebuffer>> GBuffers;
		Ref<GraphicsPipeline> GBufferPipeline;
		DescriptorSetPack GBufferPipelinePack;
		Ref<StorageBuffer> GBufferInstances;

		//Screen pass
		Ref<VertexBuffer> ScreenNDC;
		Ref<RenderPass> ScreenRenderPass;
		PerFrameResource<Ref<Framebuffer>> Outputs;
		Ref<GraphicsPipeline> ScreenPipeline;
		PerFrameResource<DescriptorSetPack> ScreenPipelinePack;


		//Material
		Ref<Texture2D> Albedo;
		Ref<Texture2D> Spec;
		Ref<Sampler> GBufferSampler;
		Ref<DescriptorSet> MaterialSet;
		

		Ref<Texture2D> DefaultWhite;

		Ref<Sampler> GlobalSampler;
		uint32_t Width = 1280, Height = 720;
	}*s_Data;

	struct MeshVertex
	{
		glm::vec3 Position;
		glm::vec2 TexCoords;
		glm::vec3 ModelNormal;
		glm::vec3 ModelTangent;
		glm::vec3 ModelBitangent;
	};


	enum class MaterialTextureType {
		Albedo = 0,
		Specular,
		Max
	};

	class Material : public IRCCounter<false> {
	public:
		Material() {
			m_Pipeline = s_Data->GBufferPipeline;
			m_TextureSet = DescriptorSet::Create(ShaderLibrary::GetNamedShader("DeferredGBufferGen"), 1);
			
			for (uint32_t i = 0; i < (uint32_t)MaterialTextureType::Max; ++i) {
				m_TextureSet->Update(s_Data->DefaultWhite, {}, ImageLayout::ShaderReadOnlyOptimal, i);
			}

			m_TextureSet->Update({}, s_Data->GBufferSampler, {}, (uint32_t)MaterialTextureType::Max);
		}
		
		~Material() = default;

		void BindPipeline()const {
		}

		void BindDescriptorSets()const {
		
		}

		Ref<DescriptorSet> GetTextureSet()const { return m_TextureSet; }

		const glm::vec3& GetAlbedoColor()const { return m_Values.AlbedoColor; }
		void SetAlbedoColor(const glm::vec3& color) { m_Values.AlbedoColor = color; }

		const glm::vec3& GetSpecular()const { return m_Values.Specular; }
		void SetSpecular(const glm::vec3& value) { m_Values.Specular = value; }

		const glm::vec3& GetMetallic()const { return m_Values.Metallic; }
		void SetMetallic(const glm::vec3& value) { m_Values.Metallic = value; }

		const glm::vec3& GetRoughness()const { return m_Values.Roughness; }
		void SetRoughness(const glm::vec3& value) { m_Values.Roughness = value; }

		void SetTexture(MaterialTextureType type, Ref<Texture2D> image) {
			m_Textures[(uint32_t)type] = image;
			if(image)
				m_TextureSet->Update(image, {}, ImageLayout::ShaderReadOnlyOptimal, (uint32_t)type);
		}
	private:
		struct MaterialValues {
			glm::vec3 AlbedoColor = {};
			glm::vec3 Specular = {};
			glm::vec3 Metallic = {};
			glm::vec3 Roughness = {};
		};
	private:

		MaterialValues m_Values;

		Ref<UniformBuffer> m_MaterialData;
		Ref<DescriptorSet> m_DataSet;
		
		Ref<GraphicsPipeline> m_Pipeline;

		Ref<Texture2D> m_Textures[(uint32_t)MaterialTextureType::Max] = {};
		Ref<DescriptorSet> m_TextureSet;
	};


	class Mesh : public IRCCounter<false> {
	public:
		Mesh(const std::vector<MeshVertex>& vertices,const std::vector<uint16_t>& indices)
			:m_VertexCount(vertices.size()), m_IndexCount(indices.size())
		{
			m_VertexBuffer = VertexBuffer::Create(vertices.data(), vertices.size() * sizeof(MeshVertex));
			m_IndexBuffer = IndexBuffer::Create(indices.data(), indices.size() * sizeof(uint16_t), IndexType::Uint16);
		}

		Ref<VertexBuffer> GetVertexBuffer()const {return m_VertexBuffer; }
		Ref<IndexBuffer> GetIndexBuffer()const { return m_IndexBuffer; }
		Ref<Material> GetDefaultMaterial()const { return m_DefaultMaterial; }
		void SetDefaultMaterial(Ref<Material> mat) { m_DefaultMaterial = mat; }

		uint64_t GetVertexCount()const { return m_VertexCount; }
		uint64_t GetIndexCount()const { return m_IndexCount; }

	private:
		Ref<VertexBuffer> m_VertexBuffer;
		Ref<IndexBuffer> m_IndexBuffer;
		Ref<Material> m_DefaultMaterial;
		uint64_t m_VertexCount, m_IndexCount;
	};

	struct MeshInstance {
		Ref<Kaidel::Mesh> Mesh;
		Ref<Material> OverrideMaterial;
	};

	TextureSamples samples = TextureSamples::x1;

	glm::vec3 pos, rot, scale = glm::vec3(1, 1, 1);

	

	struct Vertex {
		glm::vec3 position;
		glm::vec2 texCoords;
		glm::vec3 normal;
		glm::vec3 tangent;
		glm::vec3 bitangent;
	};

	std::vector<Vertex> vertices = {
		// Front face  
		{glm::vec3(-0.5f, -0.5f,  0.5f),{0.0f,0.0f}, glm::vec3(0.0f, 0.0f, 1.0f),{1.0f, 0.0f, 0.0f}, {0.0f, 1.0f, 0.0f}}, // v0  
		{glm::vec3(0.5f, -0.5f,  0.5f), {1.0f,0.0f},glm::vec3(0.0f, 0.0f, 1.0f), {1.0f, 0.0f, 0.0f}, {0.0f, 1.0f, 0.0f}}, // v1  
		{glm::vec3(0.5f,  0.5f,  0.5f), {1.0f,1.0f},glm::vec3(0.0f, 0.0f, 1.0f), {1.0f, 0.0f, 0.0f}, {0.0f, 1.0f, 0.0f}}, // v2  
		{glm::vec3(-0.5f,  0.5f,  0.5f),{0.0f,1.0f}, glm::vec3(0.0f, 0.0f, 1.0f),{1.0f, 0.0f, 0.0f}, {0.0f, 1.0f, 0.0f}}, // v3  

		// Back face  
		{glm::vec3(-0.5f, -0.5f, -0.5f),{1,0}, glm::vec3(0.0f, 0.0f, -1.0f), {-1.0f, 0.0f, 0.0f}, {0.0f, 1.0f, 0.0f}}, // v4  
		{glm::vec3(0.5f, -0.5f, -0.5f), {0,0},glm::vec3(0.0f, 0.0f, -1.0f), {-1.0f, 0.0f, 0.0f}, {0.0f, 1.0f, 0.0f}}, // v5  
		{glm::vec3(0.5f,  0.5f, -0.5f), {0,1},glm::vec3(0.0f, 0.0f, -1.0f), {-1.0f, 0.0f, 0.0f}, {0.0f, 1.0f, 0.0f}}, // v6  
		{glm::vec3(-0.5f,  0.5f, -0.5f),{1,1}, glm::vec3(0.0f, 0.0f, -1.0f), {-1.0f, 0.0f, 0.0f}, {0.0f, 1.0f, 0.0f}}, // v7  

		// Left face  
		{glm::vec3(-0.5f, -0.5f,  0.5f), {1,0}, glm::vec3(-1.0f, 0.0f, 0.0f), {0.0f, 0.0f, 1.0f}, {0.0f, 1.0f, 0.0f}}, // v8  
		{glm::vec3(-0.5f, -0.5f, -0.5f), {0,0}, glm::vec3(-1.0f, 0.0f, 0.0f), {0.0f, 0.0f, 1.0f}, {0.0f, 1.0f, 0.0f}}, // v9  
		{glm::vec3(-0.5f,  0.5f, -0.5f), {0,1}, glm::vec3(-1.0f, 0.0f, 0.0f), {0.0f, 0.0f, 1.0f}, {0.0f, 1.0f, 0.0f}}, // v10  
		{glm::vec3(-0.5f,  0.5f,  0.5f), {1,1}, glm::vec3(-1.0f, 0.0f, 0.0f), {0.0f, 0.0f, 1.0f}, {0.0f, 1.0f, 0.0f}}, // v11  

		// Right face  
		{glm::vec3(0.5f, -0.5f,  0.5f), {0,0},glm::vec3(1.0f, 0.0f, 0.0f), {0.0f, 0.0f, -1.0f}, {0.0f, 1.0f, 0.0f}}, // v12  
		{glm::vec3(0.5f, -0.5f, -0.5f), {1,0},glm::vec3(1.0f, 0.0f, 0.0f), {0.0f, 0.0f, -1.0f}, {0.0f, 1.0f, 0.0f}}, // v13  
		{glm::vec3(0.5f,  0.5f, -0.5f), {1,1},glm::vec3(1.0f, 0.0f, 0.0f), {0.0f, 0.0f, -1.0f}, {0.0f, 1.0f, 0.0f}}, // v14  
		{glm::vec3(0.5f,  0.5f,  0.5f), {0,1},glm::vec3(1.0f, 0.0f, 0.0f), {0.0f, 0.0f, -1.0f}, {0.0f, 1.0f, 0.0f}}, // v15  
		
		// Top face  
		{glm::vec3(-0.5f,  0.5f,  0.5f),{0,0}, glm::vec3(0.0f, 1.0f, 0.0f), {1.0f, 0.0f, 0.0f}, {0.0f, 0.0f, -1.0f}}, // v20  
		{glm::vec3(0.5f,  0.5f,  0.5f), {1,0},glm::vec3(0.0f, 1.0f, 0.0f), {1.0f, 0.0f, 0.0f}, {0.0f, 0.0f, -1.0f}}, // v21  
		{glm::vec3(0.5f,  0.5f, -0.5f), {1,1},glm::vec3(0.0f, 1.0f, 0.0f), {1.0f, 0.0f, 0.0f}, {0.0f, 0.0f, -1.0f}}, // v22  
		{glm::vec3(-0.5f,  0.5f, -0.5f),{0,1}, glm::vec3(0.0f, 1.0f, 0.0f), {1.0f, 0.0f, 0.0f}, {0.0f, 0.0f, -1.0f}},  // v23  

		// Bottom face  
		{glm::vec3(-0.5f, -0.5f,  0.5f),{0,1}, glm::vec3(0.0f, -1.0f, 0.0f), {1.0f, 0.0f, 0.0f}, {0.0f, 0.0f, 1.0f}}, // v16  
		{glm::vec3(0.5f, -0.5f,  0.5f), {1,1},glm::vec3(0.0f, -1.0f, 0.0f), {1.0f, 0.0f, 0.0f}, {0.0f, 0.0f, 1.0f}}, // v17  
		{glm::vec3(0.5f, -0.5f, -0.5f), {1,0},glm::vec3(0.0f, -1.0f, 0.0f), {1.0f, 0.0f, 0.0f}, {0.0f, 0.0f, 1.0f}}, // v18  
		{glm::vec3(-0.5f, -0.5f, -0.5f),{0,0}, glm::vec3(0.0f, -1.0f, 0.0f), {1.0f, 0.0f, 0.0f}, {0.0f, 0.0f, 1.0f}}, // v19  

	};

	// Indices for the cube (using the new vertex count)  
	std::vector<unsigned short> indices = {
		// Front face
		0, 1, 2,
		2, 3, 0,
		// Back face
		5, 4, 7,
		7, 6, 5,
		// Left face
		9, 8, 11,
		11, 10, 9,
		// Right face
		12, 13, 14,
		14, 15, 12,
		// Top face
		16, 17, 18,
		18, 19, 16,
		// Bottom face
		23, 22, 21,
		21, 20, 23,
	};

	struct Light {
		glm::vec4 Position;
		glm::vec4 Color;
		float Radius;
	};

	std::vector<Light> lights;

	struct Cluster {
		glm::vec4 Min;
		glm::vec4 Max;
	};

	struct ClusterGrid {
		uint32_t Count;
		uint32_t Indices[50];
		uint32_t Padding;
	};

	static glm::uvec3 s_ClusterDimensions = { 16,9,24 };
	static uint64_t s_ClusterGridLength = s_ClusterDimensions.x * s_ClusterDimensions.y * s_ClusterDimensions.z;

	struct Cube {
		glm::vec3 position, rotation, scale = { 1,1,1 };
	};

	std::vector<glm::mat4> cubes;

	class Model {
	public:
		Model(const std::string& path)
			:m_ModelPath(path), m_ModelDir(m_ModelPath.parent_path())
		{
			LoadModel(path);
		}
		const auto& GetMeshes()const { return m_Meshes; }


	private:
		void LoadModel(const std::string& path) {
			Assimp::Importer importer;
			const aiScene* scene =
				importer.ReadFile("assets/models/Sponza/Sponza.gltf",
					aiProcess_Triangulate | aiProcess_OptimizeMeshes | aiProcess_CalcTangentSpace | aiProcess_FlipUVs);
			if (!scene || scene->mFlags & AI_SCENE_FLAGS_INCOMPLETE || !scene->mRootNode)
			{
				KD_CORE_ASSERT(importer.GetErrorString());
			}

			ProcessNode(scene->mRootNode, scene);
		}
		void ProcessNode(const aiNode* node, const aiScene* scene) {
			for (uint32_t i = 0; i < node->mNumMeshes; i++) {
				aiMesh* mesh = scene->mMeshes[node->mMeshes[i]];
				m_Meshes.push_back(ProcessMesh(mesh, scene));
			}

			//process all the node children recursively
			for (uint32_t i = 0; i < node->mNumChildren; i++) {
				ProcessNode(node->mChildren[i], scene);
			}
		}

		Ref<Mesh> ProcessMesh(const aiMesh* mesh, const aiScene* scene) {
			std::vector<MeshVertex> vertices;
			std::vector<uint16_t> indices;
			for (uint32_t i = 0; i < mesh->mNumVertices; ++i) {
				MeshVertex vertex{};
				glm::vec3 vector{};
				
				vector.x = mesh->mVertices[i].x;
				vector.y = mesh->mVertices[i].y;
				vector.z = mesh->mVertices[i].z;
				vertex.Position = vector;

				vector.x = mesh->mTangents[i].x;
				vector.y = mesh->mTangents[i].y;
				vector.z = mesh->mTangents[i].z;
				vertex.ModelTangent = vector;

				vector.x = mesh->mBitangents[i].x;
				vector.y = mesh->mBitangents[i].y;
				vector.z = mesh->mBitangents[i].z;
				vertex.ModelBitangent = vector;

				vector.x = mesh->mNormals[i].x;
				vector.y = mesh->mNormals[i].y;
				vector.z = mesh->mNormals[i].z;
				vertex.ModelNormal = vector;

				if (mesh->HasTextureCoords(0)) {
					glm::vec2 vec{};
					vec.x = mesh->mTextureCoords[0][i].x;
					vec.y = mesh->mTextureCoords[0][i].y;
					vertex.TexCoords = vec;
				}
				else {
					vertex.TexCoords = glm::vec2(0, 0);
				}

				vertices.push_back(vertex);
			}
			
			for (uint32_t i = 0; i < mesh->mNumFaces; ++i) {
				aiFace face = mesh->mFaces[i];
				for (uint32_t j = 0; j < face.mNumIndices; ++j) {
					indices.push_back(face.mIndices[j]);
				}
			}

			Ref<Material> mat = ProcessMaterial(scene->mMaterials[mesh->mMaterialIndex], scene);

			Ref<Mesh> m = CreateRef<Mesh>(vertices, indices);
			
			m->SetDefaultMaterial(mat);

			return m;
		}

		Ref<Texture2D> GetMaterialTexture(aiTextureType type, const aiMaterial* material, const aiScene* scene) {
			if (!material->GetTextureCount(type))
				return {};

			aiString texturePath;
			KD_CORE_ASSERT(material->GetTexture(type, 0, &texturePath) == aiReturn_SUCCESS);
			KD_CORE_ASSERT(texturePath.length);

			std::string pathStr = texturePath.C_Str();

			return TextureLibrary::Load(m_ModelDir / pathStr, ImageLayout::ShaderReadOnlyOptimal, Format::RGBA8UN);
		}

		glm::vec3 aiColor3DTovec3(const aiColor3D& v) {
			return { v.r,v.g,v.b };
		}

		template<typename T>
		T GetMaterialColor(const char* name, uint32_t type, uint32_t idx, const aiMaterial* material) {
			T value{};
			material->Get(name, type, idx, value);
			return value;
		}

		Ref<Material> ProcessMaterial(const aiMaterial* material, const aiScene* scene) {
			Ref<Material> mat = CreateRef<Material>();
			mat->SetTexture(MaterialTextureType::Albedo, GetMaterialTexture(aiTextureType_DIFFUSE, material, scene));
			mat->SetTexture(MaterialTextureType::Specular, GetMaterialTexture(aiTextureType_SPECULAR, material, scene));
			//mat->SetTexture(MaterialTextureType::Normal, GetMaterialTexture(aiTextureType_NORMALS, material, scene));
			
			mat->SetAlbedoColor(aiColor3DTovec3(GetMaterialColor<aiColor3D>(AI_MATKEY_COLOR_AMBIENT, material)));
			mat->SetSpecular(aiColor3DTovec3(GetMaterialColor<aiColor3D>(AI_MATKEY_COLOR_AMBIENT, material)));
			return mat;
		}	

	private:
		Path m_ModelPath;
		Path m_ModelDir;
		std::vector<Ref<Mesh>> m_Meshes;
	};

	static Model* model;

	static void CreateTestSponzaModel() {
		model = new Model("assets/models/Sponza/Sponza.gltf");
	}


	static void CreateClusterResources() {
		s_Data->ClusterPipeline = ComputePipeline::Create(ShaderLibrary::LoadShader("Cluster", "assets/_shaders/ClusterComp.shader"));
		for (auto& cluster : s_Data->Clusters) {
			cluster =
				StorageBuffer::Create(s_ClusterGridLength * sizeof(Cluster));
		}

		uint32_t i = 0;
		for (auto& pack : s_Data->ClusterPipelinePack) {
			pack =
				DescriptorSetPack(ShaderLibrary::GetNamedShader("Cluster"), {}); 
			pack.GetSet(0)->Update(s_Data->Clusters.GetResources()[i],0);
			++i;
		}
	}

	static void CreateLightCullResources() {

		s_Data->LightCullPipeline = ComputePipeline::Create(ShaderLibrary::LoadShader("LightCull", "assets/_shaders/ClusterCullComp.shader"));

		for (auto& grid : s_Data->ClusterGrids) {
			grid = StorageBuffer::Create(s_ClusterGridLength * sizeof(ClusterGrid));
		}

		uint32_t i = 0;
		for (auto& pack : s_Data->LightCullPipelinePack) {
			pack =
				DescriptorSetPack(ShaderLibrary::GetNamedShader("LightCull"), 
					{ {0,s_Data->ClusterPipelinePack.GetResources()[i].GetSet(0)} });
			pack.GetSet(1)->Update(StorageBuffer::Create(MAX_LIGHT_COUNT * sizeof(Light)),0)
				.Update(s_Data->ClusterGrids.GetResources()[i],1);
			++i;
		}
	}

	static void CreateGBufferResources() {
		{
			RenderPassSpecification specs{};
			specs.Colors = {
				//Position
				RenderPassAttachment(Format::RGBA32F,ImageLayout::None,ImageLayout::ColorAttachmentOptimal,samples,
					AttachmentLoadOp::Clear,AttachmentStoreOp::Store),
				//Normals
				RenderPassAttachment(Format::RGBA32F,ImageLayout::None,ImageLayout::ColorAttachmentOptimal,samples,
					AttachmentLoadOp::Clear,AttachmentStoreOp::Store),
				//Albedo + Specular
				RenderPassAttachment(Format::RGBA8UN,ImageLayout::None,ImageLayout::ColorAttachmentOptimal,samples,
					AttachmentLoadOp::Clear,AttachmentStoreOp::Store)
			};
			specs.DepthStencil =
				RenderPassAttachment(Format::Depth32F, ImageLayout::None, ImageLayout::DepthAttachmentOptimal, samples,
					AttachmentLoadOp::Clear, AttachmentStoreOp::Store);
			s_Data->GBufferRenderPass = RenderPass::Create(specs);
		}

		{
			FramebufferSpecification specs{};
			specs.Width = s_Data->Width;
			specs.Height = s_Data->Height;
			specs.RenderPass = s_Data->GBufferRenderPass;
			for (auto& buffer : s_Data->GBuffers) {
				buffer = Framebuffer::Create(specs);
			}
		}

		{
			GraphicsPipelineSpecification specs;

			specs.Input.Bindings.push_back(VertexInputBinding({
				{"a_Position",Format::RGB32F},
				{"a_TexCoords",Format::RG32F},
				{"a_Normal",Format::RGB32F},
				{"a_Tangent",Format::RGB32F},
				{"a_BiTangent",Format::RGB32F},
			}));
			specs.Multisample.Samples = samples;
			specs.Primitive = PrimitiveTopology::TriangleList;
			specs.Rasterization.FrontCCW = true;
			specs.Rasterization.CullMode = PipelineCullMode::None;
			specs.Shader = ShaderLibrary::LoadShader("DeferredGBufferGen", "assets/_shaders/DeferredGBufferGen.shader");
			specs.RenderPass = s_Data->GBufferRenderPass;
			specs.Subpass = 0;
			specs.DepthStencil.DepthTest = true;
			specs.DepthStencil.DepthWrite = true;
			specs.DepthStencil.DepthCompareOperator = CompareOp::LessOrEqual;
			s_Data->GBufferPipeline = GraphicsPipeline::Create(specs);
		}
	}

	static void CreateScreenPassResources() {
		
		{
			RenderPassSpecification specs{};
			specs.Colors =
			{
				RenderPassAttachment(Format::RGBA8UN,ImageLayout::None,ImageLayout::ColorAttachmentOptimal,samples,
					AttachmentLoadOp::Clear, AttachmentStoreOp::Store)
			};
			s_Data->ScreenRenderPass = RenderPass::Create(specs);
		}
		{
			FramebufferSpecification specs;
			specs.RenderPass = s_Data->ScreenRenderPass;
			specs.Width = s_Data->Width;
			specs.Height = s_Data->Height;
			for (auto& fb : s_Data->Outputs) {
				fb = Framebuffer::Create(specs);
			}
		}

		{
			GraphicsPipelineSpecification specs{};
			specs.Input.Bindings = {
				{
					{"a_NDC",Format::RG32F},
					{"a_TexCoords",Format::RG32F}
				}
			};
			specs.Multisample.Samples = samples;
			specs.Primitive = PrimitiveTopology::TriangleList;
			specs.RenderPass = s_Data->ScreenRenderPass;
			specs.Subpass = 0;
			specs.Rasterization.CullMode = PipelineCullMode::None;
			specs.Shader = ShaderLibrary::LoadShader("LightPass", "assets/_shaders/CubeLightPass.shader");
			s_Data->ScreenPipeline = GraphicsPipeline::Create(specs);
		}
		uint32_t i = 0;
		for (auto& pack : s_Data->ScreenPipelinePack) {
			pack = DescriptorSetPack(ShaderLibrary::GetNamedShader("LightPass"), {});
			pack[0]->Update(s_Data->Clusters[i], 0);
			pack[1]->Update(s_Data->LightCullPipelinePack[i].GetSet(1)->GetStorageBufferAtBinding(0), 0).
				Update(s_Data->ClusterGrids[i], 1);
			pack[2]->Update({}, s_Data->GlobalSampler, {}, 0).
				Update(s_Data->GBuffers[i]->GetColorAttachment(0), {}, ImageLayout::ShaderReadOnlyOptimal, 1).
				Update(s_Data->GBuffers[i]->GetColorAttachment(1), {}, ImageLayout::ShaderReadOnlyOptimal, 2).
				Update(s_Data->GBuffers[i]->GetColorAttachment(2), {}, ImageLayout::ShaderReadOnlyOptimal, 3).
				Update(s_Data->GBuffers[i]->GetDepthAttachment(), {}, ImageLayout::ShaderReadOnlyOptimal, 4);
			++i;
		}
	}

	static void CreateMaterialResources() {
		s_Data->Albedo = TextureLibrary::Load("assets/textures/container2.png", ImageLayout::ShaderReadOnlyOptimal, Format::RGBA8UN);
		s_Data->Spec = TextureLibrary::Load("assets/textures/container2_specular.png", ImageLayout::ShaderReadOnlyOptimal, Format::R8UN);
		
		

		s_Data->GBufferPipelinePack = DescriptorSetPack(ShaderLibrary::GetNamedShader("DeferredGBufferGen"), {});
		s_Data->GBufferPipelinePack.GetSet(1)->Update(s_Data->Albedo, {}, ImageLayout::ShaderReadOnlyOptimal, 0)
			.Update(s_Data->Spec, {}, ImageLayout::ShaderReadOnlyOptimal, 1)
			.Update({}, s_Data->GBufferSampler, {}, 2);
		s_Data->GBufferPipelinePack.GetSet(0)->Update(s_Data->GBufferInstances, 0);
	}

	static void DeferredPass(const glm::mat4& viewProj, Scene* scene) {
		RenderCommand::BeginRenderPass(s_Data->GBufferRenderPass, *s_Data->GBuffers,
		{
			AttachmentColorClearValue(glm::vec4(0.0f)),
			AttachmentColorClearValue(glm::vec4(0.0f)),AttachmentColorClearValue(glm::vec4(0.0f)),AttachmentDepthStencilClearValue(1.0f,0),
		});

		Ref<VertexBuffer> lastSetVertexBuffer = s_Data->CubeVertexBuffer;
		Ref<IndexBuffer> lastSetIndexBuffer = s_Data->CubeIndexBuffer;

		RenderCommand::BindGraphicsPipeline(s_Data->GBufferPipeline);
		s_Data->GBufferPipelinePack.Bind();
		RenderCommand::SetViewport(s_Data->Width, s_Data->Height, 0, 0);
		RenderCommand::SetScissor(s_Data->Width, s_Data->Height, 0, 0);
		RenderCommand::BindPushConstants(ShaderLibrary::GetNamedShader("DeferredGBufferGen"), 0, viewProj);
		for (auto& mesh : model->GetMeshes()) {
			RenderCommand::BindVertexBuffers({ mesh->GetVertexBuffer() }, {0});
			RenderCommand::BindIndexBuffer(mesh->GetIndexBuffer(), 0);
			RenderCommand::BindDescriptorSet(ShaderLibrary::GetNamedShader("DeferredGBufferGen"), mesh->GetDefaultMaterial()->GetTextureSet(), 1);
			RenderCommand::DrawIndexed(mesh->GetIndexCount(), mesh->GetVertexCount(), cubes.size(), 0, 0, 0);
		}
		RenderCommand::EndRenderPass();
	}

	//Barrier for sync of Deferred passes
	static void InsertDeferredBarrier() {
		ImageMemoryBarrier positionBarrier(
			s_Data->GBuffers->Get()->GetColorAttachment(0),
			ImageLayout::ShaderReadOnlyOptimal,
			AccessFlags_ColorAttachmentWrite,
			AccessFlags_ShaderRead);
		ImageMemoryBarrier normalBarrier(
			s_Data->GBuffers->Get()->GetColorAttachment(1),
			ImageLayout::ShaderReadOnlyOptimal,
			AccessFlags_ColorAttachmentWrite,
			AccessFlags_ShaderRead);
		ImageMemoryBarrier albedoSpecBarrier(
			s_Data->GBuffers->Get()->GetColorAttachment(2),
			ImageLayout::ShaderReadOnlyOptimal,
			AccessFlags_ColorAttachmentWrite,
			AccessFlags_ShaderRead
		);
		ImageMemoryBarrier depthBarrier(
			s_Data->GBuffers->Get()->GetDepthAttachment(),
			ImageLayout::ShaderReadOnlyOptimal,
			AccessFlags_DepthStencilWrite,
			AccessFlags_ShaderRead
		);
		RenderCommand::PipelineBarrier(
			PipelineStages_ColorAttachmentOutput | PipelineStages_LateFragmentTests | PipelineStages_EarlyFragmentTests,
			PipelineStages_VertexShader,
			{},
			{},
			{ positionBarrier,albedoSpecBarrier,normalBarrier,depthBarrier }
		);
	}

#if 0
	template<typename T>
	void CopyToBuffer(uint8_t* buffer,const T& value) {
		std::memcpy(buffer, &value, sizeof(T));
	}

	template<typename T,typename...Args>
	void CopyToBuffer(uint8_t* buffer, const T& value, const Args&... args) {
		std::memcpy(buffer, &value, sizeof(T));
		CopyToBuffer(buffer + sizeof(T), args...);
	}
	template<typename T,typename... Args>
	const uint8_t* MakePushConstants(const T& value, const Args&... args) {
		static uint8_t Buffer[sizeof(T) + (sizeof(Args) + ...)];
		CopyToBuffer(Buffer, value, args...);
		return Buffer;
	}

	template<typename T>
	const uint8_t* MakePushConstants(const T& value) {
		static uint8_t Buffer[sizeof(T)];
		CopyToBuffer(Buffer, value);
		return Buffer;
	}
#endif
	bool SceneRenderer::NeedsRecreation(Ref<Texture2D> output) {
		const auto& specs = output->GetTextureSpecification();
		return specs.Width != s_Data->Width ||
			specs.Height != s_Data->Height ||
			specs.Samples != samples;
	}
	
	SceneRenderer::SceneRenderer(void* scene)
		:m_Context(scene)
	{
		if (!s_Data) {
			s_Data = new Data;

			s_Data->CubeVertexBuffer = VertexBuffer::Create(vertices.data(), vertices.size() * sizeof(Vertex));
			s_Data->CubeIndexBuffer = IndexBuffer::Create(indices.data(), indices.size() * sizeof(uint16_t), IndexType::Uint16);
			{
				glm::vec2 screenVertices[] =
				{
					//NDC, TexCoords
					{-1.0,-1.0},{0,0},
					{1.0,-1.0},{1,0},
					{1,1},{1,1},
					{1,1},{1,1},
					{-1,1},{0,1},
					{-1.0,-1.0},{0,0},
				};

				s_Data->ScreenNDC = VertexBuffer::Create(screenVertices, sizeof(screenVertices));
			}
			{
				SamplerState state;
				state.Aniso = false;
				state.MagFilter = SamplerFilter::Linear;
				state.MinFilter = SamplerFilter::Linear;
				state.AddressModeU = SamplerAddressMode::Repeat;
				state.AddressModeV = SamplerAddressMode::Repeat;
				state.AddressModeW = SamplerAddressMode::Repeat;
				s_Data->GlobalSampler = Sampler::Create(state);
			}

			{
				SamplerState state;
				state.AddressModeU = SamplerAddressMode::Repeat;
				state.AddressModeV = SamplerAddressMode::Repeat;
				state.AddressModeW = SamplerAddressMode::Repeat;
				state.MagFilter = SamplerFilter::Linear;
				state.MinFilter = SamplerFilter::Linear;
				state.MipFilter = SamplerMipMapMode::Linear;

				s_Data->GBufferSampler = Sampler::Create(state);
			}

			{
				Texture2DSpecification specs;
				specs.Width = 1;
				specs.Height = 1;
				specs.Depth = 1;
				specs.Mips = 1;
				specs.Layers = 1;
				specs.Layout = ImageLayout::ShaderReadOnlyOptimal;
				specs.Samples = TextureSamples::x1;
				specs.Swizzles[0] = TextureSwizzle::Red;
				specs.Swizzles[1] = TextureSwizzle::Green;
				specs.Swizzles[2] = TextureSwizzle::Blue;
				specs.Swizzles[3] = TextureSwizzle::Alpha;

				uint32_t data = 0xFFFFFFFF;

				TextureData init{};
				init.Layer = 0;
				init.Data = &data;
				specs.InitialDatas.push_back(init);
				specs.Format = Format::RGBA8UN;
				s_Data->DefaultWhite = Texture2D::Create(specs);
			}

			CreateClusterResources();
			CreateLightCullResources();
			CreateGBufferResources();
			CreateScreenPassResources();
			CreateTestSponzaModel();
			
			cubes.resize(1);
			cubes[0] = glm::scale(glm::mat4(1.0f),glm::vec3(.1f));

			s_Data->GBufferInstances = StorageBuffer::Create(cubes.size() * sizeof(glm::mat4));
			s_Data->GBufferInstances->SetData(cubes.data(), cubes.size() * sizeof(glm::mat4));
			CreateMaterialResources();	

			Light l{};
			l.Color = glm::vec4(1.0f, 1, 1, 1);
			l.Position = glm::vec4(0.0f);
			l.Radius = 5.0f;

			lights.push_back(l);
		}
	}

	//TODO: normal maps.
	//TODO: implement to ECS, make hierarchy and frustum culling.

	void SceneRenderer::Render(Ref<Texture2D> outputBuffer, const SceneData& sceneData)
	{
		if (NeedsRecreation(outputBuffer)) {
			RenderCommand::DeviceWaitIdle();
			s_Data->Width = outputBuffer->GetTextureSpecification().Width;
			s_Data->Height = outputBuffer->GetTextureSpecification().Height;
			samples = outputBuffer->GetTextureSpecification().Samples;
			RecreateSizeOrSampleDependent();
		}

		glm::mat4 proj = sceneData.Proj;
		glm::mat4 view = sceneData.View;

		glm::mat4 viewProj = sceneData.ViewProj;

		glm::mat4 invProj = glm::inverse(proj);

		float zNear = sceneData.zNear;
		float zFar = sceneData.zFar;

		MakeClusters(invProj,zNear,zFar,sceneData.ScreenSize);
		InsertClusterBarrier();
		MakeLightGrids(view);
		DeferredPass(viewProj,(Scene*)m_Context);
		InsertLightGridBarrier();
		InsertDeferredBarrier();

		ScreenPass(zNear,zFar,sceneData.CameraPos);

		ResolveToOutput(outputBuffer);

	}

	void SceneRenderer::RecreateSizeOrSampleDependent()
	{
		CreateGBufferResources();
		CreateScreenPassResources();
	}
	void SceneRenderer::ResolveToOutput(Ref<Texture2D> outputBuffer)
	{
		//Screen pass barrier
		{
			ImageMemoryBarrier barrier =
				ImageMemoryBarrier(
					s_Data->Outputs->Get()->GetColorAttachment(0),
					ImageLayout::TransferSrcOptimal,
					AccessFlags_ColorAttachmentWrite,
					AccessFlags_TransferRead
				);
			RenderCommand::PipelineBarrier(
				PipelineStages_ColorAttachmentOutput,
				PipelineStages_Transfer,
				{},
				{},
				{ barrier }
			);
		}

		ImageLayout outputLayout = outputBuffer->GetTextureSpecification().Layout;

		//Transition Output to copy layout
		{
			ImageMemoryBarrier barrier =
				ImageMemoryBarrier(
					outputBuffer,
					ImageLayout::TransferDstOptimal,
					AccessFlags_None,
					AccessFlags_TransferWrite
				);

			barrier.OldLayout = ImageLayout::None;

			RenderCommand::PipelineBarrier(
				PipelineStages_TopOfPipe,
				PipelineStages_Transfer,
				{},
				{},
				{ barrier }
			);
		}

		RenderCommand::CopyTexture(s_Data->Outputs->Get()->GetColorAttachment(0), 0, 0, outputBuffer, 0, 0);

		{
			ImageMemoryBarrier barrier =
				ImageMemoryBarrier(
					outputBuffer,
					ImageLayout::ShaderReadOnlyOptimal,
					AccessFlags_TransferWrite,
					AccessFlags_ShaderRead
				);

			RenderCommand::PipelineBarrier(
				PipelineStages_Transfer,
				PipelineStages_VertexShader,
				{},
				{},
				{ barrier }
			);
	}
	}
	void SceneRenderer::ScreenPass(float zNear, float zFar, const glm::vec3& cameraPos) {
		float scale = 24.0f / (std::log2f(zFar / zNear));
		float bias = -24.0f * (std::log2f(zNear)) / std::log2f(zFar / zNear);
		RenderCommand::BeginRenderPass(s_Data->ScreenRenderPass, *s_Data->Outputs,
			{ AttachmentColorClearValue(glm::vec4{0.0f,0.0f,0.0f,0.0f}) });
		RenderCommand::BindGraphicsPipeline(s_Data->ScreenPipeline);
		RenderCommand::BindVertexBuffers({ s_Data->ScreenNDC }, { 0 });
		s_Data->ScreenPipelinePack->Bind();
		RenderCommand::BindPushConstants(ShaderLibrary::GetNamedShader("LightPass"), 0,
			glm::vec4(cameraPos, 0.0f), glm::vec2{ s_Data->Width,s_Data->Height }, zNear, zFar, scale, bias);
		RenderCommand::Draw(6, 1, 0, 0);
		RenderCommand::EndRenderPass();
	}
	void SceneRenderer::MakeClusters(const glm::mat4& invProj, float zNear, float zFar, const glm::vec2& screenSize) {
		RenderCommand::BindComputePipeline(s_Data->ClusterPipeline);
		s_Data->ClusterPipelinePack->Bind();
		RenderCommand::BindPushConstants(ShaderLibrary::GetNamedShader("Cluster"), 0,
			invProj, zNear, zFar, screenSize);
		RenderCommand::Dispatch(s_ClusterDimensions.x, s_ClusterDimensions.y, s_ClusterDimensions.z);
	}
	//Barrier for sync of ClusterAABB writes
	void SceneRenderer::InsertClusterBarrier() {
		BufferMemoryBarrier barrier{};
		barrier.Buffer = *s_Data->Clusters;
		barrier.Offset = 0;
		barrier.Size = -1;
		barrier.Src = AccessFlags_ShaderWrite;
		barrier.Dst = AccessFlags_ShaderRead;
		RenderCommand::PipelineBarrier(
			PipelineStages_ComputeShader,
			PipelineStages_ComputeShader,
			{},
			{ barrier },
			{}
		);
	}
	void SceneRenderer::MakeLightGrids(const glm::mat4& view) {
		s_Data->LightCullPipelinePack->GetSet(1)->GetStorageBufferAtBinding(0)->SetData(lights.data(), lights.size() * sizeof(Light));

		RenderCommand::BindComputePipeline(s_Data->LightCullPipeline);
		s_Data->LightCullPipelinePack->Bind();
		RenderCommand::BindPushConstants(ShaderLibrary::GetNamedShader("LightCull"), 0,
			view, lights.size());
		RenderCommand::Dispatch(1, 1, 6);
	}
	//Barrier for sync of light grid creation
	void SceneRenderer::InsertLightGridBarrier() {
		BufferMemoryBarrier barrier{};
		barrier.Buffer = s_Data->LightCullPipelinePack->GetSet(1)->GetStorageBufferAtBinding(1);
		barrier.Offset = 0;
		barrier.Size = -1;
		barrier.Src = AccessFlags_ShaderWrite;
		barrier.Dst = AccessFlags_ShaderRead;
		RenderCommand::PipelineBarrier(
			PipelineStages_ComputeShader,
			PipelineStages_FragmentShader,
			{},
			{ barrier },
			{}
		);
	}
}
