#pragma once

#include "Kaidel/Renderer/GraphicsAPI/UniformBuffer.h"
#include "Kaidel/Renderer/GraphicsAPI/VertexBuffer.h"
#include "Kaidel/Renderer/GraphicsAPI/IndexBuffer.h"
#include "Kaidel/Renderer/GraphicsAPI/ShaderLibrary.h"
#include "Kaidel/Renderer/GraphicsAPI/TextureLibrary.h"
#include "Kaidel/Renderer/DescriptorSetPack.h"
#include "Material.h"

#include <assimp/Importer.hpp>
#include <assimp/mesh.h>
#include <assimp/scene.h>
#include <assimp/postprocess.h>

#include "glm/glm.hpp"
namespace Kaidel {

	static const constexpr uint32_t MaxBoneCount = 4;

	struct MeshVertex
	{
		glm::vec3 Position;
		glm::vec2 TexCoords;
		glm::vec3 ModelNormal;
		glm::vec3 ModelTangent;
		glm::vec3 ModelBitangent;
	};

	struct SkinnedMeshVertex 
	{
		glm::vec3 Position;
		glm::vec2 TexCoords;
		glm::vec3 ModelNormal;
		glm::vec3 ModelTangent;
		glm::vec3 ModelBitangent;
		int BoneID[MaxBoneCount] = { -1, -1, -1, -1 };
		float BoneWeight[MaxBoneCount] = { 0 };
	};

	struct SkinTree : public IRCCounter<false> {
		glm::mat4 BindMatrix;
		uint32_t ID;
		std::string Name;
		std::vector<SkinTree> Children;
	};

	extern Ref<RenderPass> GetDeferredPassRenderPass();

	class Mesh : public IRCCounter<false> {
	public:
		Mesh() = default;
		virtual ~Mesh() = default;

		Mesh(const std::vector<MeshVertex>& vertices, const std::vector<uint16_t>& indices)
			:m_VertexCount(vertices.size()), m_IndexCount(indices.size())
		{
			m_VertexBuffer = VertexBuffer::Create(vertices.data(), vertices.size() * sizeof(MeshVertex));
			m_IndexBuffer = IndexBuffer::Create(indices.data(), indices.size() * sizeof(uint16_t), IndexType::Uint16);
		}

		Ref<VertexBuffer> GetVertexBuffer()const { return m_VertexBuffer; }
		Ref<IndexBuffer> GetIndexBuffer()const { return m_IndexBuffer; }
		Ref<Material> GetDefaultMaterial()const { return m_DefaultMaterial; }
		void SetDefaultMaterial(Ref<Material> mat) { m_DefaultMaterial = mat; }

		uint64_t GetVertexCount()const { return m_VertexCount; }
		uint64_t GetIndexCount()const { return m_IndexCount; }

		virtual bool IsSkinned() const { return false; }

	protected:
		Ref<VertexBuffer> m_VertexBuffer;
		Ref<IndexBuffer> m_IndexBuffer;
		Ref<Material> m_DefaultMaterial;
		uint64_t m_VertexCount = 0, m_IndexCount = 0;
	};

	class SkinnedMesh : public Mesh {
	public:
		SkinnedMesh(const std::vector<SkinnedMeshVertex>& vertices, const std::vector<uint16_t>& indices, Ref<SkinTree> tree)
			: m_SkinTree(tree)
		{
			m_VertexBuffer = VertexBuffer::Create(nullptr, vertices.size() * sizeof(MeshVertex));
			m_SkinnedBuffer = VertexBuffer::Create(vertices.data(), vertices.size() * sizeof(SkinnedMeshVertex));
			m_IndexBuffer = IndexBuffer::Create(indices.data(), indices.size() * sizeof(uint16_t), IndexType::Uint16);

			m_VertexCount = vertices.size();
			m_IndexCount = indices.size();
			m_BoneCount = CalcBoneCount(*tree);

			{
				DescriptorSetLayoutSpecification specs(
					{
						{DescriptorType::StorageBuffer, ShaderStage_ComputeShader},
						{DescriptorType::StorageBuffer, ShaderStage_ComputeShader}
					});

				m_SkinnedBufferSet = DescriptorSet::Create(specs);

 				m_SkinnedBufferSet->Update(m_SkinnedBuffer, 0).Update(m_VertexBuffer, 1);
			}

			{
				m_BoneTransformsBuffer = StorageBuffer::Create(m_BoneCount * sizeof(glm::mat4));
			}

			{
				DescriptorSetLayoutSpecification specs(
					{ {DescriptorType::StorageBuffer, ShaderStage_ComputeShader} }
				);

				m_BoneTransformsSet = DescriptorSet::Create(specs);

				m_BoneTransformsSet->Update(m_BoneTransformsBuffer, 0);
			}

			m_BoneTransforms.resize(m_BoneCount, glm::mat4(1.0f));
		}

		virtual bool IsSkinned() const override { return true; }

		Ref<SkinTree> GetSkinTree()const { return m_SkinTree; }

		uint64_t GetBoneCount() const { return m_BoneCount; }

		std::vector<glm::mat4>& GetBoneTransforms() { return m_BoneTransforms; }
		Ref<DescriptorSet> GetBoneTransformsSet() const { return m_BoneTransformsSet; }
		Ref<StorageBuffer> GetBoneTransformsBuffer() const { return m_BoneTransformsBuffer; }

		Ref<DescriptorSet> GetSkinnedBufferSet() const { return m_SkinnedBufferSet; }

	private:
		uint64_t CalcBoneCount(const SkinTree& tree) {
			uint64_t total = 0;

			for (uint64_t i = 0; i < tree.Children.size(); ++i)
			{
				total += CalcBoneCount(tree.Children[i]);
			}

			return total + 1;
		}

	private:
		Ref<VertexBuffer> m_SkinnedBuffer;
		Ref<DescriptorSet> m_SkinnedBufferSet;
		Ref<SkinTree> m_SkinTree;
		uint64_t m_BoneCount = 0;
		std::vector<glm::mat4> m_BoneTransforms;
		Ref<DescriptorSet> m_BoneTransformsSet;
		Ref<StorageBuffer> m_BoneTransformsBuffer;
	};
	
	struct MeshTree {
		glm::vec3 Position;
		glm::vec3 Rotation;
		glm::vec3 Scale;

		Ref<Mesh> NodeMesh;
		std::string NodeName;
		std::vector<Scope<MeshTree>> Children;
	};

	

	class Model : public IRCCounter<false> {
	public:
		Model() = default;

		std::vector<Ref<Mesh>> GetMeshes() { return {}; }

		Scope<MeshTree>& GetMeshTree() { return m_Tree; }

	private:
		Path m_ModelPath;
		Path m_ModelDir;
		Scope<MeshTree> m_Tree;
		SkinTree m_SkinTree;

		friend class ModelLibrary;
		friend class ModelLoader;
	};
}
