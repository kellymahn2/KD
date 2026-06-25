#include "AssetPreview.h"
#include "Kaidel/Renderer/RenderCommand.h"
#include "Kaidel/Renderer/Renderer3D.h"
#include "Kaidel/Scene/ModelLibrary.h"
namespace Kaidel {
	glm::mat4 _GetTransform(const glm::vec3& pos, const glm::quat& rot, const glm::vec3& scl);

	namespace Utils {
		struct PreviewCamera {
			glm::vec3 Position;
			glm::vec3 Center;
			glm::mat4 View;
			glm::mat4 Projection;

			float Near, Far;
		};

		static PreviewCamera SetupPreviewCamera(const AABB& bb, float fovDegrees, float aspectRatio,const glm::vec3& viewDirection)
		{
			PreviewCamera camera;

			// Calculate bounding box properties
			glm::vec3 center = (bb.Min + bb.Max) * 0.5f;
			float maxDimension = glm::max(glm::max(bb.Max.x - bb.Min.x,
				bb.Max.y - bb.Min.y),
				bb.Max.z - bb.Min.z);

			// Calculate distance needed to fit the box in view
			float fovRadians = glm::radians(fovDegrees);
			float distance = (maxDimension * 0.5f) / glm::tan(fovRadians * 0.5f);

			// Add some padding (20% is good)
			distance *= 1.2f;

			// Position camera along the view direction
			camera.Position = center - glm::normalize(viewDirection) * distance;
			camera.Center = center;

			// Create view matrix (looking at center)
			camera.View = glm::lookAt(camera.Position, camera.Center, glm::vec3(0, 1, 0));

			// Create projection matrix
			camera.Projection = glm::perspective(fovRadians, aspectRatio, 0.1f, distance * 2.0f);
			camera.Near = 0.1f;
			camera.Far = distance * 2.0f;

			return camera;
		}	

		static void PopulateVisibility(const Scope<MeshTree>& curr, Visibility& vis)
		{
			glm::mat4 model = _GetTransform(curr->Position, curr->Rotation, curr->Scale);
			vis.Instances.push_back(InstanceData{ model });

			for (auto& child : curr->Children)
			{
				PopulateVisibility(child, vis);
			}
		}

		static void RenderNode(const Scope<MeshTree>& curr)
		{
			Ref<Mesh>& mesh = curr->NodeMesh;
			if (mesh->GetSubmeshes().size() == 1)
			{
				Ref<MaterialInstance> mat = mesh->GetSubmeshes()[0].DefaultMaterial;
				if (!mat)
					mat = RendererGlobals::GetDefaultMaterial();

				KD_CORE_ASSERT(mat);

				Renderer3DRenderParams params;
				params.VB = mesh->GetVertexBuffer();
				params.IB = mesh->GetIndexBuffer();
				params.IndexCount = mesh->GetSubmeshes()[0].IndexCount;
				params.IndexOffset = mesh->GetSubmeshes()[0].IndexOffset;
				params.InstanceCount = 1;
				params.InstanceOffset = 0;
				params.VertexCount = mesh->GetSubmeshes()[0].VertexCount;
				params.VertexOffset = mesh->GetSubmeshes()[0].VertexOffset;

				Renderer3D::Draw(params, mat);
			}
			else
			{
				uint32_t i = 0;

				bool rendered = false;
				for (auto& submesh : mesh->GetSubmeshes())
				{
					Ref<MaterialInstance> mat = mesh->GetSubmeshes()[i].DefaultMaterial;
					if (!mat)
						mat = RendererGlobals::GetDefaultMaterial();
					KD_CORE_ASSERT(mat);

					if (!rendered)
						Renderer3D::BeginSubmesh();

					rendered = true;

					Renderer3DRenderParams params;
					params.VB = mesh->GetVertexBuffer();
					params.IB = mesh->GetIndexBuffer();
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

			for (auto& child : curr->Children)
			{
				RenderNode(child);
			}
		}
	}

	static std::unordered_map<AssetHandle, Ref<Texture2D>> s_Previews;

	Kaidel::Ref<Kaidel::Texture2D> AssetPreview::GetPreview(Asset asset)
	{
		AssetType type = asset->GetAssetType();
		AssetHandle handle = asset->GetHandle();

		Ref<Texture2D> texture;

		switch (type)
		{
		//In this case we don't store the preview since the texture is the preview.
		case Kaidel::AssetType::Texture2D:
		{
			return asset.As<Texture2D>();
		}
		case Kaidel::AssetType::Model:
		{
			if (TryGetPreview(handle, texture))
				break;

			//Calculate bounding box of model
			Ref<Model> model = asset.As<Model>();
			const AABB& boundingBox = model->GetBoundingBox();

			glm::vec3 cameraDirection = glm::vec3(1, -1, -1.0f);

			Utils::PreviewCamera camera = Utils::SetupPreviewCamera(boundingBox, 60.0f, 1.0f, cameraDirection);

			//model->GetMeshTree()->

			SceneData sceneData;
			sceneData.AspectRatio = 1.0f;
			sceneData.CameraPos = glm::vec4(camera.Position, 1.0f);
			sceneData.FOV = 60.0f;
			sceneData.Proj = camera.Projection;
			sceneData.ScreenSize = glm::ivec2(100, 100);
			sceneData.View = camera.View;
			sceneData.ViewProj = camera.Projection * camera.View;
			sceneData.zNear = camera.Near;
			sceneData.zFar = camera.Far;

			DirectionalLightData lightData{};
			lightData.Color = glm::vec4(1.0f);
			lightData.Direction =
				-glm::normalize(glm::toMat4(glm::quat(glm::vec3(-glm::pi<float>() / 2.0f, 0.0f, 0.0f)))[2]);

			Visibility vis;

			Utils::PopulateVisibility(model->GetMeshTree(), vis);

			RenderCommand::Submit([&]() 
			{
				Renderer3D::Begin(texture, vis, sceneData, lightData, "ModelPreviewIcon");

				Renderer3D::BeginColor(RendererGlobals::GetSingleColorEnvironmentMap());

				Utils::RenderNode(model->GetMeshTree());

				Renderer3D::EndColor();

				Renderer3D::End();
			});

		} break;
		//case Kaidel::AssetType::StandardMaterial:
		//case Kaidel::AssetType::VisualMaterial: 
		}

		return texture ? texture : Ref<Texture2D>();
	}

	bool AssetPreview::TryGetPreview(AssetHandle handle, Ref<Texture2D>& texture)
	{
		texture = s_Previews[handle];

		if (texture)
		{
			return true;
		}
		
		Texture2DSpecification specs{};
		specs.Width = 100;
		specs.Height = 100;
		specs.Layout = ImageLayout::General;
		specs.Format = Format::RGBA8UN;
		specs.Mipped = false;

		texture = s_Previews[handle] = Texture2D::Create(specs);

		return false;
	}
	
}
