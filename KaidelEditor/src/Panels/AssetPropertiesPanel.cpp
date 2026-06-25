#include "AssetPropertiesPanel.h"

#include "Kaidel/Utils/PlatformUtils.h"
#include "EditorViewport.h"
#include "UI/UIHelper.h"
#include "UI/IconLibrary.h"
#include "UI/AssetPreview.h"

#include <imgui/imgui.h>

namespace Kaidel {

	namespace Utils {
		static void DrawVisualMaterialTextureUI(const char* strID, Ref<VisualMaterialInstance> mat, uint32_t setID, uint32_t binding, const std::string& name,
			const ImVec2 imageSize, Ref<Texture2D> fallback)
		{
			ImGuiTreeNodeFlags flags =
				ImGuiTreeNodeFlags_SpanAvailWidth;

			if (ImGui::TreeNodeEx(strID, flags, name.c_str()))
			{
				Ref<Texture> texture = mat->GetTexture(setID, binding);

				if (texture == RendererGlobals::GetSingleColorTexture(glm::vec4(1.0f)))
				{
					texture = fallback;
				}

				Ref<DescriptorSet> set = DescriptorSet::Create({ {DescriptorType::SamplerWithTexture, ShaderStage_FragmentShader} });

				set->Update(texture, RendererGlobals::GetSampler(SamplerFilter::Linear, SamplerMipMapMode::Linear), ImageLayout::ShaderReadOnlyOptimal, 0);

				ImTextureID id = (ImTextureID)set->GetSetID();

				ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, ImVec2(0, 0));

				if (ImGui::ImageButton("##Image", id, imageSize))
				{
					auto path = FileDialogs::OpenFile("jpg (*.jpg)\0*.jpg\0png (*.png)\0*.png\0");
					if (path)
					{
						Ref<Texture2D> t = TextureLibrary::Load(*path, ImageLayout::ShaderReadOnlyOptimal, Format::RGBA8UN);
						if (t)
						{
							mat->SetTexture(t, setID, binding);
						}
					}
				}

				if (ImGui::IsItemHovered() && texture != fallback)
				{
					ImGui::BeginTooltip();
					ImGui::Text(TextureLibrary::GetPath(texture).string().c_str());
					ImGui::EndTooltip();
				}

				ImGui::PopStyleVar();

				ImGui::TreePop();
			}
		}

		static bool DrawMaterialTextureUI(
			const char* strID, Ref<StandardMaterialInstance> mat, StandardMaterialInstance::MaterialTextureType type,
			Ref<DescriptorSet> set, const ImVec2& imageSize, const ImVec2& iconSize, Ref<Texture2D> fallback, Format format,
			std::function<bool()>&& func)
		{
			ImGuiTreeNodeFlags flags =
				ImGuiTreeNodeFlags_SpanAvailWidth;

			bool changed = false;

			if (ImGui::TreeNodeEx(strID, flags))
			{
				float offset = ImGui::GetCursorPosY();
				ImGui::PushID(strID);
				Ref<Texture2D> texture = mat->GetTexture(type);
				if (!texture)
					texture = fallback;

				Ref<Sampler> sampler = RendererGlobals::GetSampler(SamplerFilter::Linear, SamplerMipMapMode::Linear);
				set->Update(texture, sampler, ImageLayout::ShaderReadOnlyOptimal, 0);


				if (mat->GetTexture(type))
				{
					ImGui::SetCursorPosY(ImGui::GetCursorPosY() + imageSize.y * 0.5f - ImGui::GetFrameHeight() * 0.5f);
					bool isUsed = mat->IsTextureUsed(type);
					if (ImGui::Checkbox("##Used", &isUsed))
					{
						mat->SetTextureUsed(type, isUsed);
						changed = true;
					}

					ImGui::SameLine();
					ImGui::SetCursorPosY(offset);
				}
				ImTextureID id = (ImTextureID)set->GetSetID();

				ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, ImVec2(0, 0));

				if (ImGui::ImageButton("##Image", id, imageSize))
				{
					AssetChooser::Render(AssetFilter({ AssetType::StandardMaterial, AssetType::VisualMaterial }), [mat, type](Asset asset) {
						mat->SetTexture(type, asset.As<Texture2D>());

						AssetManager::SetDirty(mat->GetHandle());
					});
				}

				if (texture != fallback && ImGui::IsItemHovered())
				{
					ImGui::BeginTooltip();
					auto& mtd = AssetManager::GetAssetMetadata(mat->GetTexture(type)->GetHandle());
					ImGui::Text(mtd.Path.string().c_str());
					ImGui::EndTooltip();
				}

				if (ImGui::IsItemHovered())
				{
					ImGui::SetMouseCursor(ImGuiMouseCursor_Hand);
				}

				ImGui::SameLine();
				ImGui::SetCursorPosY(offset + imageSize.y * 0.5f - iconSize.y * 0.5f);
				uint32_t crossIcon = IconLibrary::GetIcon("TextureClearCross.png");
				auto& coords = IconLibrary::GetAtlas()->GetSamplingCoords(crossIcon);

				if (ImGui::ImageButton("##Clear", (ImTextureID)IconLibrary::GetDescriptorSet()->GetSetID(),
					iconSize, (ImVec2&)coords.BottomLeft, (ImVec2&)coords.TopRight))
				{
					mat->SetTexture(type, {});
					changed = true;
				}

				if (func)
					changed |= func();

				ImGui::PopStyleVar(1);
				ImGui::PopID();
				ImGui::TreePop();
			}

			return changed;
		}

		static void DrawPreview(Ref<MaterialInstance> material)
		{
			ImGuiTreeNodeFlags flags =
				ImGuiTreeNodeFlags_SpanAvailWidth;

			if (ImGui::TreeNodeEx("Preview"))
			{
				Ref<EnvironmentMap> environment = RendererGlobals::GetEnvironmentMap();

				static EditorViewport viewport("Preview", 60.0f, 240, 240, 0.1f, 100.0f);

				static glm::mat4 model = glm::rotate(glm::mat4(1.0f), -glm::pi<float>() / 2.0f, glm::vec3(1.0f, 0.0f, 0.0f));

				viewport.OnImGuiRender(model, material);

				ImGui::TreePop();
			}
		}

		static bool DrawStandardMaterial(Ref<StandardMaterialInstance> material)
		{
			PerFrameResource<Ref<DescriptorSet>> sets[(uint32_t)StandardMaterialInstance::MaterialTextureType::Max] = { {} };
			if (!sets[0][0])
			{
				for (uint32_t i = 0; i < (uint32_t)StandardMaterialInstance::MaterialTextureType::Max; ++i)
				{
					DescriptorSetLayoutSpecification specs({ {DescriptorType::SamplerWithTexture, ShaderStage_FragmentShader} });
					sets[i].Construct([&specs](uint32_t) {
						return DescriptorSet::Create(specs);
						});
				}
			}

			static Ref<Texture2D> fallback =
				TextureLibrary::Load("Resources/Icons/TextureNullCross.png", ImageLayout::ShaderReadOnlyOptimal, Format::RGBA8UN);

			const ImVec2 imageSize = ImVec2(64, 64);
			const ImVec2 iconSize = ImVec2(16, 16);

			static const char* channels[] = { "R", "G", "B", "A" };

			DrawPreview(material);

			bool changed = false;

			changed |= DrawMaterialTextureUI(
				"Albedo", material, StandardMaterialInstance::MaterialTextureType::Albedo,
				sets[(uint32_t)StandardMaterialInstance::MaterialTextureType::Albedo],
				imageSize, iconSize, fallback, Format::RGBA8UN, {});

			changed |= DrawMaterialTextureUI(
				"Normal", material, StandardMaterialInstance::MaterialTextureType::Normal,
				sets[(uint32_t)StandardMaterialInstance::MaterialTextureType::Normal],
				imageSize, iconSize, fallback, Format::RGBA8UN, {});

			changed |= DrawMaterialTextureUI(
				"Metallic", material, StandardMaterialInstance::MaterialTextureType::Metallic,
				sets[(uint32_t)StandardMaterialInstance::MaterialTextureType::Metallic],
				imageSize, iconSize, fallback, Format::RGBA32F, [material]() {
					bool changed = false;
					if (material->GetTexture(StandardMaterialInstance::MaterialTextureType::Metallic))
					{
						uint32_t currIndex = (uint32_t)material->GetUniformData().MetalnessChannel;

						uint32_t combo = Combo("Channel", channels, 3, channels[currIndex]);

						if (combo != -1 && currIndex != combo)
						{
							material->SetMetalnessColorChannel((StandardMaterialInstance::MaterialTextureChannel)combo);
							changed = true;
						}
					}
					else
					{
						float metalness = material->GetUniformData().Metalness;

						if (ImGui::DragFloat("Metalness", &metalness, 0.05f, 0.0f, 1.0f))
						{
							material->SetMetalness(metalness);
							changed = true;
						}
					}

					return changed;
				});

			changed |= DrawMaterialTextureUI(
				"Roughness", material, StandardMaterialInstance::MaterialTextureType::Roughness,
				sets[(uint32_t)StandardMaterialInstance::MaterialTextureType::Roughness],
				imageSize, iconSize, fallback, Format::RGBA32F, [material]() {
					bool changed = false;
					if (material->GetTexture(StandardMaterialInstance::MaterialTextureType::Roughness))
					{
						uint32_t currIndex = (uint32_t)material->GetUniformData().RoughnessChannel;

						uint32_t combo = Combo("Channel", channels, 3, channels[currIndex]);

						if (combo != -1 && currIndex != combo)
						{
							material->SetRoughnessColorChannel((StandardMaterialInstance::MaterialTextureChannel)combo);
							changed = true;
						}
					}
					else
					{
						float roughness = material->GetUniformData().Roughness;

						if (ImGui::DragFloat("Roughness", &roughness, 0.05f, 0.0f, 1.0f))
						{
							material->SetRoughness(roughness);
							changed = true;
						}
					}
					return changed;
				});

			changed |= DrawMaterialTextureUI(
				"Emissive", material, StandardMaterialInstance::MaterialTextureType::Emissive,
				sets[(uint32_t)StandardMaterialInstance::MaterialTextureType::Emissive],
				imageSize, iconSize, fallback, Format::RGBA8UN, {});

			return changed;
		}


		void DrawVisualMaterial(Ref<VisualMaterialInstance> material)
		{

			DrawPreview(material);
			auto& vm = material->GetVisualMaterial();
			bool propertiesChanged = false;

			const ImVec2 imageSize = ImVec2(64, 64);

			static Ref<Texture2D> fallback =
				TextureLibrary::Load("Resources/Icons/TextureNullCross.png", ImageLayout::ShaderReadOnlyOptimal, Format::RGBA8UN);

			for (auto& [name, param] : vm->Parameters)
			{
				ImGui::PushID(&param);

				bool isTexture = param.Type == VisualMaterialParameterType::Texture;

				if (!isTexture)
				{
					void* dataPtr = material->GetUniformData() + param.ByteOffset;

					switch (param.Type)
					{
					case VisualMaterialParameterType::Bool: propertiesChanged = ImGui::Checkbox(name.c_str(), (bool*)dataPtr) || propertiesChanged; break;
					case VisualMaterialParameterType::Int: propertiesChanged = ImGui::DragInt(name.c_str(), (int*)dataPtr) || propertiesChanged; break;
					case VisualMaterialParameterType::UInt:
					{
						uint32_t min = 0, max = UINT32_MAX;
						propertiesChanged = ImGui::DragScalar(name.c_str(), ImGuiDataType_U32, (uint32_t*)dataPtr, 1.0f, &min, &max) || propertiesChanged;
					}
					break;
					case VisualMaterialParameterType::Float: propertiesChanged = ImGui::DragFloat(name.c_str(), (float*)dataPtr, 0.005f) || propertiesChanged; break;
					case VisualMaterialParameterType::Vec2: propertiesChanged = DrawVec2Control(name, *(glm::vec2*)dataPtr, 0.0f, 100.0f) || propertiesChanged; break;
					case VisualMaterialParameterType::Vec3: propertiesChanged = DrawVec3Control(name, *(glm::vec3*)dataPtr, 0.0f, 100.0f) || propertiesChanged; break;
					case VisualMaterialParameterType::Vec4: propertiesChanged = DrawVec4Control(name, *(glm::vec4*)dataPtr, 0.0f, 100.0f) || propertiesChanged; break;
					}
				}
				else
				{
					DrawVisualMaterialTextureUI("##Texture", material, param.Set, param.Binding, name, imageSize, fallback);
				}



				ImGui::PopID();
			}

			if (propertiesChanged)
				material->UploadUniformData();
		}
	}


	void AssetPropertiesPanel::OnImGuiRender()
	{
		ImGui::Text(AssetManager::GetAssetMetadata(m_SelectedAsset->GetHandle()).Dirty ? "Dirty" : "Clean");

		bool changed = false;
		switch (m_SelectedAsset->GetAssetType())
		{
		case AssetType::Texture2D: break;
		case AssetType::StandardMaterial: changed = StandardMaterialProperties(); break;
		}

		if (changed)
			AssetManager::SetDirty(m_SelectedAsset->GetHandle());
	}

	bool AssetPropertiesPanel::StandardMaterialProperties()
	{
		Ref<StandardMaterialInstance> material = m_SelectedAsset.AssetPtr;
		return Utils::DrawStandardMaterial(material);
	}


	bool AssetPropertiesPanel::VisualMaterialProperties()
	{
		Ref<VisualMaterialInstance> material = m_SelectedAsset.AssetPtr;
		Utils::DrawVisualMaterial(material);
		//TODO: fix this.
		return false;
	}

}
