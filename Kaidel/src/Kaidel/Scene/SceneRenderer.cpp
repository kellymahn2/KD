#include "KDpch.h"
#include "SceneRenderer.h"
#include "Components.h"
#include "Kaidel/Core/DebugUtils.h"
#include "Scene.h"
#include "Entity.h"
#include "Kaidel/Renderer/Text/Font.h"
#include "Kaidel/Renderer/Text/MSDFData.h"
#include "Kaidel/Renderer/GraphicsAPI/PerFrameResource.h"
#include "Kaidel/Scene/ModelLibrary.h"

#include "Kaidel/Renderer/GraphicsAPI/UniformBuffer.h"
#include "Kaidel/Renderer/EnvironmentMap.h"
#include "Kaidel/Renderer/2D\Renderer2D.h"
#include "Kaidel/Core/JobSystem.h"
#include "Kaidel/Core/Timer.h"
#include "Kaidel/Renderer/RenderCommand.h"
#include "Kaidel/Renderer/GraphicsAPI/VertexBuffer.h"
#include "Kaidel/Renderer/GraphicsAPI/IndexBuffer.h"
#include "Kaidel/Renderer/GraphicsAPI/ShaderLibrary.h"
#include "Kaidel/Renderer/GraphicsAPI/TextureLibrary.h"
#include "Kaidel/Renderer/DescriptorSetPack.h"
#include "Kaidel/Scene/Material.h"
#include "Kaidel/Scene/Model.h"
#include "Visibility.h"

#include <glm/glm.hpp>
#include <glm/gtx/compatibility.hpp>
#include <chrono>
#include <cmath>
#include <iostream>
#include <future>

#include "stb_image.h"
#define MAX_LIGHT_COUNT 100

#define SHADOW_MAP_SIZE 4096
#define SHADOW_MAPPING



namespace Kaidel {
	struct DeferredPassData {
		Ref<RenderPass> RenderPass;
		PerFrameResource<Ref<Framebuffer>> Output;
		//Ref<GraphicsPipeline> Pipeline;
	};
	
	struct BoneData {
		Ref<ComputePipeline> Pipeline;
	};

	struct SkyboxData {
		Ref<GraphicsPipeline> Pipeline;
		Ref<VertexBuffer> SkyboxCubeData;
	};

	static struct Data {

		Ref<Mesh> DirectionalArrow;

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

		
		BoneData Bones;
		SkyboxData Skybox;

		Ref<Sampler> GlobalSampler;
		uint32_t Width = 1280, Height = 720;

	}*s_Data;

	struct Light {
		glm::vec4 Position;
		glm::vec4 Color;
		float Radius;
	};

	std::vector<Light> lights;

	DirectionalLightData CalculateDirectionalLightData(
		float zNear, float zFar, 
		const glm::vec3& color, float lightSize, const glm::mat4& rotation, const glm::mat4& viewProj,
		float splits[4], float shadowMapWidth)
	{
		DirectionalLightData lightData{};
		lightData.LightSize = lightSize;

		float maxDistance = zFar;

		float minDistance = zNear;

		lightData.Color = glm::vec4(color, 1.0f);

		glm::mat4 lightRotation = rotation;
		lightRotation = glm::transpose(glm::inverse(glm::mat3(lightRotation)));

		const glm::vec4 to = -glm::normalize(lightRotation[2]);
		const glm::vec4 up = glm::normalize(lightRotation[1]);
		glm::mat4 lightView = glm::lookAt(glm::vec3(0.0f), glm::vec3(to), glm::vec3(up));

		const glm::mat4 unproj = glm::inverse(viewProj);

		glm::vec4 frustum_corners[] =
		{
			unproj * glm::vec4(-1, -1, 0, 1),
			unproj * glm::vec4(-1, -1, 1, 1),
			unproj * glm::vec4(-1, 1, 0, 1),
			unproj * glm::vec4(-1, 1, 1, 1),
			unproj * glm::vec4(1, -1, 0, 1),
			unproj * glm::vec4(1, -1, 1, 1),
			unproj * glm::vec4(1, 1, 0, 1),
			unproj * glm::vec4(1, 1, 1, 1),
		};

		for (uint32_t i = 0; i < 8; ++i)
		{
			frustum_corners[i] /= frustum_corners[i].w;
		}

		lightData.SplitDistances[0] = glm::lerp(minDistance, maxDistance, splits[0]);
		lightData.SplitDistances[1] = glm::lerp(minDistance, maxDistance, splits[1]);
		lightData.SplitDistances[2] = glm::lerp(minDistance, maxDistance, splits[2]);
		lightData.SplitDistances[3] = glm::lerp(minDistance, maxDistance, splits[3]);

		for (int cascade = 0; cascade < 4; ++cascade)
		{
			// Compute cascade bounds in light-view-space from the main frustum corners:
			const float split_near = cascade == 0 ? 0 : splits[cascade - 1];
			const float split_far = splits[cascade];

			const glm::vec4 corners[] =
			{
				lightView * glm::lerp(frustum_corners[0], frustum_corners[1], split_near),
				lightView * glm::lerp(frustum_corners[0], frustum_corners[1], split_far),
				lightView * glm::lerp(frustum_corners[2], frustum_corners[3], split_near),
				lightView * glm::lerp(frustum_corners[2], frustum_corners[3], split_far),
				lightView * glm::lerp(frustum_corners[4], frustum_corners[5], split_near),
				lightView * glm::lerp(frustum_corners[4], frustum_corners[5], split_far),
				lightView * glm::lerp(frustum_corners[6], frustum_corners[7], split_near),
				lightView * glm::lerp(frustum_corners[6], frustum_corners[7], split_far),
			};

			glm::vec3 center = glm::vec3(0.0f);

			// Compute cascade bounding sphere center:
			for (int j = 0; j < 8; ++j)
			{
				center += glm::vec3(corners[j]);
			}
			center = center / 8.0f;

			float radius = 0.0f;

			// Compute cascade bounding sphere radius:
			for (int j = 0; j < 8; ++j)
			{
				radius = std::max(radius, glm::length(glm::vec3(corners[j]) - center));
			}

			// Fit AABB onto bounding sphere:
			glm::vec3 vRadius = glm::vec3(radius);
			glm::vec3 vMin = center - vRadius;
			glm::vec3 vMax = center + radius;

			// Snap cascade to texel grid:
			const glm::vec3 extent = vMax - vMin;
			const glm::vec3 texelSize = extent / float(shadowMapWidth);
			vMin = glm::floor(vMin / texelSize) * texelSize;
			vMax = glm::floor(vMax / texelSize) * texelSize;
			center = (vMin + vMax) * 0.5f;

			glm::vec3 _center = center;

			// clipping extrusion for projection:
			//	Tight Z distribution for precision (16-bit unorm especially) but allowing some extra room for cascade blending in Z
			{
				glm::vec3 _min = vMin;
				glm::vec3 _max = vMax;
				float ext = abs(_center.z - _min.z);

				const glm::mat4 lightProjection =
					glm::ortho(_min.x, _max.x, _min.y, _max.y, _min.z, _max.z);
				lightData.ViewProjection[cascade] = lightProjection * lightView;
			}
		}

		lightData.Direction = to;

		return lightData;
	}

	static Entity GetDirectionalLightEntity(Scene* scene, entt::registry& sceneReg)
	{
		auto view = sceneReg.view<DirectionalLightComponent>();
		entt::entity e = *view.begin();

		return Entity(e, scene);
	}

	static void DrawSkinnedMesh(const SkinnedMeshComponent& smc, Ref<Mesh> mesh, Ref<VertexBuffer> vb, Ref<IndexBuffer> ib)
	{
		if (mesh->GetSubmeshes().size() == 1)
		{
			Ref<Material> mat = smc.UsedMaterial[0];
			KD_CORE_ASSERT(mat);

			Renderer3DRenderParams params;
			params.VB = vb;
			params.IB = ib;
			params.IndexCount = mesh->GetSubmeshes()[0].IndexCount;
			params.IndexOffset = mesh->GetSubmeshes()[0].IndexOffset;
			params.InstanceCount = 1;
			params.InstanceOffset = 0;
			params.VertexCount = mesh->GetSubmeshes()[0].VertexCount;
			params.VertexOffset = mesh->GetSubmeshes()[0].VertexOffset;

			Renderer3D::Draw(params, mat);

			return;
		}


		uint32_t i = 0;

		bool rendered = false;
		for (auto& submesh : mesh->GetSubmeshes())
		{
			Ref<Material> mat = smc.UsedMaterial[i];
			KD_CORE_ASSERT(mat);

			if (!rendered)
				Renderer3D::BeginSubmesh();

			rendered = true;

			Renderer3DRenderParams params;
			params.VB = vb;
			params.IB = ib;
			params.IndexCount = submesh.IndexCount;
			params.IndexOffset = submesh.IndexOffset;
			params.InstanceCount = 1;
			params.InstanceOffset = 0;
			params.VertexCount = submesh.VertexCount;
			params.VertexOffset = submesh.VertexOffset;

			Renderer3D::DrawSubmesh(params, mat);
			++i;
		}

		if (rendered)
			Renderer3D::EndSubmesh();
	}
	
	static void DrawMesh(const MeshComponent& mc, Ref<Mesh> mesh, Ref<VertexBuffer> vb, Ref<IndexBuffer> ib)
	{
		if (mesh->GetSubmeshes().size() == 1)
		{
			Ref<Material> mat = mc.UsedMaterial[0];
			KD_CORE_ASSERT(mat);

			if (!mc.VisibilityResults[0])
				return;

			Renderer3DRenderParams params;
			params.VB = vb;
			params.IB = ib;
			params.IndexCount = mesh->GetSubmeshes()[0].IndexCount;
			params.IndexOffset = mesh->GetSubmeshes()[0].IndexOffset;
			params.InstanceCount = 1;
			params.InstanceOffset = 0;
			params.VertexCount = mesh->GetSubmeshes()[0].VertexCount;
			params.VertexOffset = mesh->GetSubmeshes()[0].VertexOffset;

			Renderer3D::Draw(params, mat);

			return;
		}


		uint32_t i = 0;

		bool rendered = false;
		for (auto& submesh : mesh->GetSubmeshes())
		{
			Ref<Material> mat = mc.UsedMaterial[i];
			KD_CORE_ASSERT(mat);

			if (!mc.VisibilityResults[i++])
				continue;

			if (!rendered)
				Renderer3D::BeginSubmesh();

			rendered = true;

			Renderer3DRenderParams params;
			params.VB = vb;
			params.IB = ib;
			params.IndexCount = submesh.IndexCount;
			params.IndexOffset = submesh.IndexOffset;
			params.InstanceCount = 1;
			params.InstanceOffset = 0;
			params.VertexCount = submesh.VertexCount;
			params.VertexOffset = submesh.VertexOffset;

			Renderer3D::DrawSubmesh(params, mat);
		}

		if (rendered)
			Renderer3D::EndSubmesh();
	}

#pragma region Deferred Pass
	static void DeferredPass(Visibility& vis, const glm::mat4& viewProj, const entt::registry& sceneReg) {
		Ref<VertexBuffer> lastSetVertexBuffer = {};
		Ref<IndexBuffer> lastSetIndexBuffer = {};

		Ref<Material> lastSetMaterial = {};

		Renderer3D::BeginColor(RendererGlobals::GetEnvironmentMap());

		{
			for (uint32_t entity = 0;  entity < vis.Count; ++entity) {
				bool isSkinned = sceneReg.try_get<SkinnedMeshComponent>(vis.VisibleEntities[entity]);

				Ref<Mesh> mesh;

				const void* component = nullptr;
					 
				if (isSkinned)
				{
					auto& smc = sceneReg.get<SkinnedMeshComponent>(vis.VisibleEntities[entity]);
					component = &smc;
					mesh = smc.UsedMesh;
				}
				else
				{
					auto& mc = sceneReg.get<MeshComponent>(vis.VisibleEntities[entity]);
					component = &mc;
					mesh = mc.UsedMesh;
				}

				if (!mesh)
					continue;
		
				Ref<VertexBuffer> vb = mesh->GetVertexBuffer();
				Ref<IndexBuffer> ib = mesh->GetIndexBuffer();

				if (isSkinned)
					DrawSkinnedMesh(*(const SkinnedMeshComponent*)component, mesh, vb, ib);
				else
					DrawMesh(*(const MeshComponent*)component, mesh, vb, ib);
			}
		}
		
		Renderer3D::EndColor();
	}
	
	#pragma endregion

	static AABB CalculateSkinAABB(Scene* scene, SkinTree& currBoneNode, Entity currBoneEntity)
	{
		if (!currBoneEntity)
			return {};

		AABB boundingBox = currBoneNode.BoundingBox.Transform(currBoneEntity.GetComponent<TransformComponent>().GlobalTransform);

		if (!currBoneEntity.HasChildren())
			return boundingBox;

		auto& pc = currBoneEntity.GetComponent<ParentComponent>();

		uint32_t i = 0;
		for (auto& childBoneNode : currBoneNode.Children)
		{
			Entity childBoneEntity = scene->GetEntity(pc.Children[i++]);

			AABB childBoundingBox = CalculateSkinAABB(scene, childBoneNode, childBoneEntity);

			boundingBox.Merge(childBoundingBox);
		}

		return boundingBox;
	}

	static void VisibilityTests(Scene* scene, Visibility& vis)
	{
		auto meshView = scene->GetRegistry().view<MeshComponent>();

		auto skinnedView = scene->GetRegistry().view<SkinnedMeshComponent>();

		const entt::sparse_set& meshSet = meshView.handle();
		const entt::sparse_set& skinnedSet = skinnedView.handle();

		uint32_t meshSize = meshView.size();
		uint32_t skinnedSize = skinnedView.size();

		uint32_t size = meshSize + skinnedSize;

		struct SharedMemory {
			
			struct ShadowCascadeData {
				std::vector<InstanceData> ShadowInstances;
				std::vector<entt::entity> ShadowEntities;
			};

			struct ShadowData {
				ShadowCascadeData Cascades[4];
			};

			std::vector<InstanceData> Instances;
			std::vector<entt::entity> Entites;

			ShadowData Shadows;
		};

		vis.Instances.resize(size);
		vis.VisibleEntities.resize(size);
		vis.Count = 0;

		for (uint32_t i = 0; i < 4; ++i)
		{
			vis.Directional.Cascades[i].Instances.resize(size);
			vis.Directional.Cascades[i].VisibleEntities.resize(size);

			vis.Directional.Cascades[i].Count = 0;
		}

		auto visibilityTest = 
			[](
				entt::entity entity, Ref<Mesh> mesh, const glm::mat4& transform, 
				SharedMemory* sharedMemory, Visibility& vis, std::vector<bool>& visibilityResults) 
		{
			auto& submeshes = mesh->GetSubmeshes();

			bool visible = false;

			for (uint32_t i = 0; i < submeshes.size(); ++i)
			{
				visibilityResults[i] = vis.Culler.IsVisiblePlanes(transform, submeshes[i].BoundingBox.Min, submeshes[i].BoundingBox.Max);
				if (visibilityResults[i])
				{
					visible = true;
				}
			}

			if (visible)
			{
				InstanceData instanceData;
				instanceData.Transform = transform;
				sharedMemory->Instances.emplace_back(instanceData);
				sharedMemory->Entites.push_back(entity);
			}

			auto& totalAABB = mesh->GetBoundingBox();

			for (uint32_t i = 0; i < 4; ++i)
			{
				if (vis.Directional.Cascades[i].Culler.IsVisiblePlanes(transform, totalAABB.Min, totalAABB.Max))
				{
					InstanceData instanceData;
					instanceData.Transform = transform;
					sharedMemory->Shadows.Cascades[i].ShadowInstances.push_back(instanceData);
					sharedMemory->Shadows.Cascades[i].ShadowEntities.push_back(entity);
				}
			}
		};

		auto skinnedVisibilityTest =
			[scene](
				entt::entity entity, SkinnedMeshComponent& smc, const glm::mat4& transform,
				SharedMemory* sharedMemory, Visibility& vis, std::vector<bool>& visibilityResults)
		{

			Entity rootBone = scene->GetEntity(smc.RootBone);
			if (!rootBone)
				return;
			
			AABB mergedAABB = CalculateSkinAABB(scene, smc.UsedMesh->GetSkin()->Tree, rootBone);

			if (vis.Culler.IsVisiblePlanes(glm::mat4(1.0f), mergedAABB.Min, mergedAABB.Max))
			{
				visibilityResults.assign(visibilityResults.size(), true);
				InstanceData instanceData;
				instanceData.Transform = glm::mat4(1.0f);
				sharedMemory->Instances.emplace_back(instanceData);
				sharedMemory->Entites.push_back(entity);
			}
			else
			{
				visibilityResults.assign(visibilityResults.size(), false);
			}
		};

		JobSystem::GetMainJobSystem().Dispatch(size, 128, [&](JobDispatchArgs args) {
			
			if (args.GlobalID >= size)
				return;

			SharedMemory* sharedMemory = (SharedMemory*)args.SharedMemory;

			if (args.FirstJobInGroup)
			{
				*sharedMemory = SharedMemory();
			}

			Entity entity;

			if (args.GlobalID < meshSize)
			{
				entity = Entity(meshSet[args.GlobalID], scene);
				auto& [tc, mc] = entity.GetComponent<TransformComponent, MeshComponent>();

				visibilityTest(entity, mc.UsedMesh, tc.GlobalTransform, sharedMemory, vis, mc.VisibilityResults);
			}
			else
			{
				entity = Entity(skinnedSet[args.GlobalID - meshSize], scene);
				auto& [tc, smc] = entity.GetComponent<TransformComponent, SkinnedMeshComponent>();
				skinnedVisibilityTest(entity, smc, tc.GlobalTransform, sharedMemory, vis, smc.VisibilityResults);
			}

			if (args.LastJobInGroup)
			{
				if (sharedMemory->Instances.size())
				{
					uint32_t prevCount = vis.Count.fetch_add(sharedMemory->Instances.size());

					for (uint32_t i = 0; i < sharedMemory->Instances.size(); ++i)
					{
						vis.Instances[prevCount + i] = sharedMemory->Instances[i];
						vis.VisibleEntities[prevCount + i] = sharedMemory->Entites[i];
					}
				}

				for (uint32_t i = 0; i < 4; ++i)
				{
					ShadowVisibilityCascade& cascade = vis.Directional.Cascades[i];

					SharedMemory::ShadowCascadeData& cascadeData = sharedMemory->Shadows.Cascades[i];

					if (cascadeData.ShadowInstances.size())
					{
						uint32_t prevCount = cascade.Count.fetch_add(cascadeData.ShadowInstances.size());

						for (uint32_t j = 0; j < cascadeData.ShadowInstances.size(); ++j)
						{
							cascade.Instances[prevCount + j] = cascadeData.ShadowInstances[j];
							cascade.VisibleEntities[prevCount + j] = cascadeData.ShadowEntities[j];
						}
					}
				}
			}

		}, sizeof(SharedMemory));

		JobSystem::GetMainJobSystem().Wait();
	}

	static uint32_t VisibleObjectsCount(entt::registry& sceneReg)
	{
		uint32_t count = 0;

		auto view = sceneReg.view<MeshComponent>();
		for (auto e : view)
		{
			auto& [mc] = view.get(e);

			count += std::count(mc.VisibilityResults.begin(), mc.VisibilityResults.end(), true);
		}

		return count;
	}

	static void ShadowPass(const SceneData& sceneData, Scene* scene, const DirectionalLightData& lightData, const Visibility& vis) {
		//SCOPED_ACCU_TIMER("SceneRenderer::ShadowPass");
		Ref<VertexBuffer> lastSetVertexBuffer = {};
		Ref<IndexBuffer> lastSetIndexBuffer = {};
		
		auto& sceneReg = scene->GetRegistry();

		Renderer3D::BeginShadow(lightData);

		for (uint32_t t = 0; t < 4; ++t)
		{
			const ShadowVisibilityCascade& cascade = vis.Directional.Cascades[t];

			Renderer3D::BeginDirectionalShadow(t);

			for (uint32_t entity = 0; entity < cascade.Count; ++entity) {
				auto& [tc, smc] = sceneReg.get<TransformComponent, MeshComponent>(cascade.VisibleEntities[entity]);

				if (!smc.UsedMesh)
					continue;

				const glm::mat4& model = tc.GlobalTransform;

				Ref<Mesh> mesh = smc.UsedMesh;
				Ref<VertexBuffer> vb = mesh->GetVertexBuffer();
				Ref<IndexBuffer> ib = mesh->GetIndexBuffer();

				{
					Renderer3DRenderParams params;
					params.VB = vb;
					params.IB = ib;
					params.IndexCount = mesh->GetIndexCount();
					params.IndexOffset = 0;
					params.InstanceCount = 1;
					params.InstanceOffset = 0;
					params.VertexCount = mesh->GetVertexCount();
					params.VertexOffset = 0;

					Renderer3D::Draw(params, {});

					continue;
				}
			}

			Renderer3D::EndShadow();
		}
	}
	
	bool SceneRenderer::NeedsRecreation(Ref<Texture2D> output) {
		const auto& specs = output->GetTextureSpecification();
		return specs.Width != s_Data->Width ||
			specs.Height != s_Data->Height;
	}

#pragma region Bone pass
	static void CreateBonePassResources()
	{
		s_Data->Bones.Pipeline = ComputePipeline::Create(ShaderLibrary::LoadOrGetNamedShader("BonePass", "assets/_shaders/BonePass.glsl"));
	}

	static void CalculateBoneTransforms(Scene* scene, SkinTree& currBoneNode, Entity currBoneEntity, std::vector<glm::mat4>& boneTransforms)
	{
		if (!currBoneEntity)
			return;
		boneTransforms[currBoneNode.ID] = currBoneEntity.GetComponent<TransformComponent>().GlobalTransform;

		if (!currBoneEntity.HasChildren())
			return;

		auto& pc = currBoneEntity.GetComponent<ParentComponent>();

		uint32_t i = 0;
		for (auto& childBoneNode : currBoneNode.Children)
		{
			Entity childBoneEntity = scene->GetEntity(pc.Children[i++]);

			CalculateBoneTransforms(scene, childBoneNode, childBoneEntity, boneTransforms);
		}
	}

	static void BonePass(Scene* scene, const SceneData& sceneData, entt::registry& sceneReg)
	{
		RenderCommand::BindComputePipeline(s_Data->Bones.Pipeline);

		{
			auto view = sceneReg.view<TransformComponent, SkinnedMeshComponent>();
			
			for (auto& e : view)
			{
				auto& [tc, smc] = view.get(e);
				
				if(!smc.UsedMesh)
					continue;
				
				Entity rootBone = scene->GetEntity(smc.RootBone);
				if (!rootBone)
					continue;

				
				Ref<Skin> skin = smc.UsedMesh->GetSkin();

				std::vector<glm::mat4>& boneTransforms = skin->BoneTransforms;

				if((uint64_t)skin->LastRoot != (uint64_t)smc.RootBone) 
				{
					//{
					//	SCOPED_ACCU_TIMER("Bone transforms ST");
					//	CalculateBoneTransforms(scene, skin->Tree, rootBone, boneTransforms);
					//}
					{
						//SCOPED_ACCU_TIMER("Bone transforms MT");
						boneTransforms[skin->Tree.ID] = rootBone.GetComponent<TransformComponent>().GlobalTransform;

						if (!rootBone.HasChildren())
							return;

						auto& pc = rootBone.GetComponent<ParentComponent>();

						uint32_t i = 0;
						for (auto& childBoneNode : skin->Tree.Children)
						{
							Entity childBoneEntity = scene->GetEntity(pc.Children[i++]);

							JobSystem::GetMainJobSystem().Execute(
								[scene, 
								&childBoneNode, 
								childBoneEntity, 
								&boneTransforms]() { 
									CalculateBoneTransforms(scene, childBoneNode, childBoneEntity, boneTransforms); 
								});
						}

						JobSystem::GetMainJobSystem().Wait();
					}

					skin->LastRoot = smc.RootBone;
					skin->BoneTransformsBuffer->SetData(boneTransforms.data(), boneTransforms.size() * sizeof(glm::mat4));
					Application::Get().SubmitToMainThread([skin]() {skin->LastRoot = UUID();});
				}

				RenderCommand::BindDescriptorSet(ShaderLibrary::GetNamedShader("BonePass"), smc.UsedMesh->GetSkinnedBufferSet(), 0);
				RenderCommand::BindDescriptorSet(ShaderLibrary::GetNamedShader("BonePass"), skin->SkinSet, 1);

				uint64_t sizeX = smc.UsedMesh->GetVertexCount() / 64;
				RenderCommand::Dispatch(sizeX + 1, 1, 1);

				BufferMemoryBarrier barrier;
				barrier.Buffer = smc.UsedMesh->GetVertexBuffer();
				barrier.Src = AccessFlags_ShaderWrite;
				barrier.Dst = AccessFlags_VertexAttribureRead;

				RenderCommand::PipelineBarrier(PipelineStages_ComputeShader, PipelineStages_VertexInput, {}, { barrier }, {});
			}
		}
	}

#pragma endregion

	SceneRenderer::SceneRenderer(void* scene)
		:m_Context(scene)
	{
		if (!s_Data) {
			s_Data = new Data;


			{
				s_Data->GlobalSampler = RendererGlobals::GetSamler(SamplerFilter::Linear, SamplerMipMapMode::Linear);
			}

			/*CreateTextPassResources();

			CreateLinePassResources();

			CreateSpriteResources();
			*/
			CreateBonePassResources();

			{
				Ref<Model> m = ModelLibrary::LoadModel("assets/models/EditorDirectionalArrow/untitled.gltf");
				Ref<Mesh> mesh = m->GetMeshTree()->NodeMesh;
				s_Data->DirectionalArrow = mesh;
			}

		}
	}

	static void CalculateTransformFromRoot(Entity entity, Scene* scene, const glm::mat4& parentTransform)
	{
		TransformComponent& tc = entity.GetComponent<TransformComponent>();
		tc.GlobalTransform = parentTransform * tc.GetTransform();

		if (entity.HasChildren())
		{
			const ParentComponent& pc = entity.GetComponent<ParentComponent>();

			for (auto& childID : pc.Children)
			{
				Entity child = scene->GetEntity(childID);
				if (child)
					CalculateTransformFromRoot(child, scene, tc.GlobalTransform);
			}
		}
	}

	static void CalculateTransforms(Scene* scene, entt::registry& m_Registry)
	{
		auto view = m_Registry.view<TransformComponent>();
		if (view.empty())
			return;

		uint64_t offset = 0;

		const entt::sparse_set& dataSet = view.handle();
		uint32_t size = m_Registry.size();

		//Calculate
		JobSystem::GetMainJobSystem().Dispatch(size, std::ceil(size / 16.0f), [view, size, scene, offset](JobDispatchArgs args) {

			if (args.GlobalID >= size)
				return;

			Entity entity = { view.handle()[offset + args.GlobalID], scene };


			if (entity.HasParent())
				return;

			CalculateTransformFromRoot(entity, scene, glm::mat4(1.0f));
		});
		JobSystem::GetMainJobSystem().Wait();
	}

	//TODO: add depth to 2D.
	void SceneRenderer::Render(Ref<Texture2D> outputBuffer, const SceneData& sceneData)
	{
		//SCOPED_ACCU_TIMER("SceneRenderer::Render");

		Scene* scene = (Scene*)m_Context;

		{
			//SCOPED_ACCU_TIMER("SceneRenderer::Transforms");
			CalculateTransforms(scene, scene->m_Registry);
		}

		auto& [tc, dlc] = GetDirectionalLightEntity(scene, scene->m_Registry).GetComponent<TransformComponent, DirectionalLightComponent>();

		DirectionalLightData lightData =
			CalculateDirectionalLightData(
				sceneData.zNear, sceneData.zFar, dlc.Color, dlc.LightSize,
				glm::toMat4(tc.Rotation), sceneData.ViewProj, dlc.SplitDistances, SHADOW_MAP_SIZE);

		static Visibility vis;

		vis.Culler = FrustumCuller(sceneData.ViewProj);

		for (uint32_t i = 0; i < 4; ++i)
		{
			vis.Directional.Cascades[i].Culler = FrustumCuller(lightData.ViewProjection[i]);
		}

		{
			//SCOPED_ACCU_TIMER("SceneRenderer::Visibility");
			VisibilityTests(scene, vis);
		}
		{
			//SCOPED_ACCU_TIMER("SceneRenderer::BonePass");
			BonePass(scene, sceneData, scene->m_Registry);
		}
		{
			
			Renderer3D::Begin(outputBuffer, vis, sceneData, lightData);


			ShadowPass(sceneData, scene, lightData, vis);

			{
				//SCOPED_ACCU_TIMER("SceneRenderer::DeferredPass");
				DeferredPass(vis, sceneData.ViewProj, scene->m_Registry);
			}
			Renderer3D::End();
		}

	}
	
}
