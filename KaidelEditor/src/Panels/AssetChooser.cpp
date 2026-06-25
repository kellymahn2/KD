#include "KDpch.h"
#include "AssetChooser.h"
#include "EditorContext.h"
#include "Kaidel/Asset/AssetManager.h"

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
		static const constexpr ImGuiTreeNodeFlags flags =
			ImGuiTreeNodeFlags_Leaf |
			ImGuiTreeNodeFlags_SpanAvailWidth;

		if (ImGui::TreeNodeEx((const void*)(uint64_t)metadata.Handle, flags, metadata.AssetName.c_str()))
		{
			if (ImGui::IsItemHovered() && ImGui::IsMouseDoubleClicked(ImGuiMouseButton_Left))
			{
				onChosen(asset);
				ImGui::TreePop();
				return true;
			}

			ImGui::TreePop();
		}

		return false;
	}

}


