#pragma once

#include "Kaidel/Renderer/GraphicsAPI/UniformBuffer.h"
#include "Kaidel/Renderer/GraphicsAPI/VertexBuffer.h"
#include "Kaidel/Renderer/GraphicsAPI/IndexBuffer.h"
#include "Kaidel/Renderer/GraphicsAPI/ShaderLibrary.h"
#include "Kaidel/Renderer/GraphicsAPI/TextureLibrary.h"
#include "Kaidel/Renderer/DescriptorSetPack.h"
#include "Material.h"
#include "Kaidel/Animation/Animation.h"
#include "Kaidel/Core/UUID.h"

#include <assimp/Importer.hpp>
#include <assimp/mesh.h>
#include <assimp/scene.h>
#include <assimp/postprocess.h>

#include "glm/glm.hpp"
namespace Kaidel {

	static const constexpr uint32_t MaxBoneCount = 4;

	struct AABB {
		glm::vec3 Min;
		glm::vec3 Max;

		AABB(const AABB& aabb)
		{
			Min = aabb.Min;
			Max = aabb.Max;
		}
			

		AABB()
			: Min(glm::vec3(std::numeric_limits<float>::infinity())),
			Max(glm::vec3(-std::numeric_limits<float>::infinity()))
		{}

		AABB(const glm::vec3& minPoint, const glm::vec3& maxPoint)
			: Min(minPoint), Max(maxPoint)
		{}

		void Merge(const AABB& aabb) 
		{
			if (!std::isfinite(Min.x) || !std::isfinite(Min.y) || !std::isfinite(Min.z) ||
				!std::isfinite(Max.x) || !std::isfinite(Max.y) || !std::isfinite(Max.z)) 
			{
				Min = aabb.Min;
				Max = aabb.Max;
				return;
			}

			Min = glm::min(Min, aabb.Min);
			Max = glm::max(Max, aabb.Max);
		}

		void Merge(const glm::vec3& point) 
		{
			Min = glm::min(Min, point);
			Max = glm::max(Max, point);
		}

		AABB Transform(const glm::mat4& M) const
		{
			if (!std::isfinite(Min.x) || !std::isfinite(Min.y) || !std::isfinite(Min.z) ||
				!std::isfinite(Max.x) || !std::isfinite(Max.y) || !std::isfinite(Max.z))
			{
				return AABB();
			}

			glm::vec3 corners[8] = {
				glm::vec3(Min.x, Min.y, Min.z),
				glm::vec3(Max.x, Min.y, Min.z),
				glm::vec3(Min.x, Max.y, Min.z),
				glm::vec3(Max.x, Max.y, Min.z),
				glm::vec3(Min.x, Min.y, Max.z),
				glm::vec3(Max.x, Min.y, Max.z),
				glm::vec3(Min.x, Max.y, Max.z),
				glm::vec3(Max.x, Max.y, Max.z)
			};

			glm::vec3 newMin(std::numeric_limits<float>::infinity());
			glm::vec3 newMax(-std::numeric_limits<float>::infinity());

			for (int i = 0; i < 8; ++i) {
				glm::vec4 p = M * glm::vec4(corners[i], 1.0f);
				p /= p.w;
				
				glm::vec3 tp = glm::vec3(p);

				newMin = glm::min(newMin, tp);
				newMax = glm::max(newMax, tp);
			}

			return AABB(newMin, newMax);
		}
	};

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

	struct SkinTree {
		AABB BoundingBox;
		glm::mat4 BindMatrix;
		uint32_t ID;
		std::string Name;
		std::vector<SkinTree> Children;
		bool IsDirty = true;
	};

	struct Skin : public IRCCounter<false> {
		SkinTree Tree;
		Ref<StorageBuffer> OffsetBuffer;
		Ref<StorageBuffer> BoneTransformsBuffer;
		std::vector<glm::mat4> Offsets;
		std::vector<glm::mat4> BoneTransforms;
		Ref<DescriptorSet> SkinSet;
		UUID LastRoot;
	};

	

	struct Submesh 
	{
		AABB BoundingBox;
		uint32_t VertexOffset;
		uint32_t IndexOffset;
		uint32_t VertexCount;
		uint32_t IndexCount;
		Ref<MaterialInstance> DefaultMaterial;
	};

	extern Ref<RenderPass> GetDeferredPassRenderPass();

	struct MeshInitializer {
		const void* Vertices;
		uint64_t VertexCount;
		uint64_t VertexSize;
		const void* Indices;
		uint64_t IndexCount;
		IndexType IndexFormat;

		const void* ShadowVertices;
		uint64_t ShadowVertexCount;
		uint64_t ShadowVertexSize;
		const void* ShadowIndices;
		uint64_t ShadowIndexCount;
		IndexType ShadowIndexFormat;
		
		std::vector<Submesh> Submeshes;
	};

	class Mesh : public IRCCounter<false> {
	public:
		Mesh() = default;
		virtual ~Mesh() = default;

		Mesh(const MeshInitializer& initializer)
			:m_VertexCount(initializer.VertexCount), m_IndexCount(initializer.IndexCount), m_Submeshes(initializer.Submeshes)
		{
			m_VertexBuffer = VertexBuffer::Create(initializer.Vertices, initializer.VertexCount * initializer.VertexSize);

			uint64_t indexSize = initializer.IndexCount * (uint64_t)initializer.IndexFormat * 2;

			m_IndexBuffer = 
				IndexBuffer::Create(initializer.Indices, indexSize, initializer.IndexFormat);
			
		}

		Ref<VertexBuffer> GetVertexBuffer()const { return m_VertexBuffer; }
		Ref<IndexBuffer> GetIndexBuffer()const { return m_IndexBuffer; }

		uint64_t GetVertexCount()const { return m_VertexCount; }
		uint64_t GetIndexCount()const { return m_IndexCount; }
		std::vector<Submesh>& GetSubmeshes() { return m_Submeshes; }

		virtual bool IsSkinned() const { return false; }

		const AABB& GetBoundingBox() const { return m_BoundingBox; }

		void SetBoundingBox(const AABB& boundingBox) { m_BoundingBox = boundingBox; }


	protected:
		AABB m_BoundingBox;
		Ref<VertexBuffer> m_VertexBuffer;
		Ref<IndexBuffer> m_IndexBuffer;
		Ref<VertexBuffer> m_ShadowVertexBuffer;
		Ref<IndexBuffer> m_ShadowIndexBuffer;
		std::vector<Submesh> m_Submeshes;
		uint64_t m_VertexCount = 0, m_IndexCount = 0;
	};

	class SkinnedMesh : public Mesh {
	public:
		SkinnedMesh(MeshInitializer initializer, Ref<Skin> skin)
			: m_Skin(skin)
		{
			
			m_VertexBuffer = VertexBuffer::Create(nullptr, initializer.VertexCount * sizeof(MeshVertex));

			uint64_t indexSize = initializer.IndexCount * (uint64_t)initializer.IndexFormat * 2;

			m_IndexBuffer = 
				IndexBuffer::Create(initializer.Indices, indexSize, initializer.IndexFormat);

			m_SkinnedBuffer = VertexBuffer::Create(initializer.Vertices, initializer.VertexCount * sizeof(SkinnedMeshVertex));

			m_VertexCount = initializer.VertexCount;
			m_IndexCount = initializer.IndexCount;
			m_BoneCount = CalcBoneCount(skin->Tree);

			m_Submeshes = initializer.Submeshes;

			{
				DescriptorSetLayoutSpecification specs(
					{
						{DescriptorType::StorageBuffer, ShaderStage_ComputeShader},
						{DescriptorType::StorageBuffer, ShaderStage_ComputeShader}
					});

				m_SkinnedBufferSet = DescriptorSet::Create(specs);

 				m_SkinnedBufferSet->Update(m_SkinnedBuffer, 0).Update(m_VertexBuffer, 1);
			}

		}

		virtual bool IsSkinned() const override { return true; }

		Ref<Skin> GetSkin()const { return m_Skin; }

		uint64_t GetBoneCount() const { return m_BoneCount; }

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
		Ref<Skin> m_Skin;
		uint64_t m_BoneCount = 0;
	};
	
	struct MeshTree {
		glm::vec3 Position;
		glm::quat Rotation;
		glm::vec3 Scale;

		Ref<Mesh> NodeMesh;
		Ref<AnimationTree> NodeAnimation;
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
