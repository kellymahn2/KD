#include "KDpch.h"
#include "AssetChooser.h"
#include "EditorContext.h"
#include "Kaidel/Asset/AssetManager.h"
#include "Kaidel/Renderer/Sprite.h"

#include <imgui/imgui.h>

namespace Kaidel {
	
	void AssetChooser::Render(const AssetFilter& filter, const OnAssetChosen& onChosen)
	{
		EditorContext::SetAssetChooserData(filter, onChosen);
	}

	void AssetChooser::OnImGuiRender()
	{
		auto& filter = EditorContext::GetActiveAssetFilter();
		auto& onChosen = EditorContext::GetActiveAssetFunc();

		if (!filter.ActiveCount)
			return;

		bool open = true;

		if (!ImGui::Begin("Asset chooser", &open))
			goto EarlyOut;

		for (auto& [handle, asset] : AssetManager::BeginTotal())
		{
			AssetType type = asset->GetAssetType();

			if (!filter.Passes(type))
				continue;

			const auto& metadata = AssetManager::GetAssetMetadata(handle);

			if (RenderAssetChooserElement(asset, metadata, onChosen))
			{
				open = false;
				break;
			}
		}

	EarlyOut:
		ImGui::End();

		if (!open) {
			filter = {};
			onChosen = {};
			return;
		}
	}


	bool AssetChooser::RenderAssetChooserElement(Ref<AssetBase> asset, const AssetMetadata& metadata, const OnAssetChosen& onChosen)
	{
		uint32_t sCount = asset->GetSubAssetCount();

		ImGuiTreeNodeFlags flags =
			(sCount == 0 ? ImGuiTreeNodeFlags_Leaf : 0) |
			ImGuiTreeNodeFlags_SpanAvailWidth;

		if (ImGui::TreeNodeEx((const void*)(uint64_t)metadata.Handle, flags, metadata.Path.stem().string().c_str()))
		{

			if (ImGui::IsItemHovered() && ImGui::IsMouseDoubleClicked(ImGuiMouseButton_Left) && asset)
			{
				onChosen(asset);
				ImGui::TreePop();
				return true;
			}


			if (sCount)
			{
				SubAsset subAsset;

				if (asset->GetAssetType() == AssetType::SpriteSheet)
				{
					Ref<SpriteSheet> sheet = DynamicPointerCast<SpriteSheet>(asset);
					for (uint32_t i = 0; i < sCount; ++i)
					{
						auto& sprite = sheet->GetSprites()[i];

						if (ImGui::TreeNodeEx(sprite.Name.c_str(), flags | ImGuiTreeNodeFlags_Leaf))
						{
							ImGui::TreePop();
						}


					}
				}
				else
				{
					//TODO: implement.
					KD_CORE_ASSERT(false);
				}
			}

			ImGui::TreePop();
		}

		

		return false;
	}

}


