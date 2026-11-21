#include "EditorContext.h"
#include "AssetChooser.h"

#include "AnimationPanel.h"

#include <variant>

namespace Kaidel {

	struct SelectedAnimationData
	{
		AnimationFrame* SelectedAnimationFrame;
		AnimationValueType SelectedAnimationValueType;
	};

	struct EditorContextData
	{
		std::variant<Entity, SelectedAnimationData, Asset> Data;

		struct {
			AssetFilter ActiveFilter;
			OnAssetChosen ActiveFunc;
		} AssetChooserData;

	};

	static EditorContextData* s_Data;

	void EditorContext::Init()
	{
		s_Data = new EditorContextData;
	}

	void EditorContext::Shutdown()
	{
		delete s_Data;
	}

	Entity EditorContext::SelectedEntity()
	{
		Entity* entity = std::get_if<0>(&s_Data->Data);
		if (entity)
			return *entity;
		return {};
	}


	void EditorContext::SelectedEntity(Entity selectedEntity)
	{
		s_Data->Data = selectedEntity;

		AnimationPanel::OnSelectionChanged();
	}

	Kaidel::Asset EditorContext::SelectedAsset()
	{
		Asset* asset = std::get_if<2>(&s_Data->Data);
		if (asset)
			return *asset;
		return {};
	}


	void EditorContext::SelectedAsset(Asset asset)
	{
		s_Data->Data = asset;

		AnimationPanel::OnSelectionChanged();
	}

	AnimationFrame* EditorContext::SelectedAnimationFrame()
	{
		SelectedAnimationData* data = std::get_if<1>(&s_Data->Data);
		if (data)
			return data->SelectedAnimationFrame;
		return nullptr;
	}

	Kaidel::AssetFilter& EditorContext::GetActiveAssetFilter()
	{
		return s_Data->AssetChooserData.ActiveFilter;
	}

	Kaidel::OnAssetChosen& EditorContext::GetActiveAssetFunc()
	{
		return s_Data->AssetChooserData.ActiveFunc;
	}

	void EditorContext::SetAssetChooserData(const AssetFilter& filter, const OnAssetChosen& onChosen)
	{
		s_Data->AssetChooserData.ActiveFilter = filter;
		s_Data->AssetChooserData.ActiveFunc = onChosen;
	}

	AnimationValueType EditorContext::SelectedAnimationFrameValueType()
	{
		SelectedAnimationData* data = std::get_if<1>(&s_Data->Data);
		if (data)
			return data->SelectedAnimationValueType;
		return AnimationValueType::MaxValue;
	}

	void EditorContext::SelectedAnimationFrame(AnimationFrame* selectedAnimationFrame, AnimationValueType type)
	{
		s_Data->Data = SelectedAnimationData{ selectedAnimationFrame, type };
	}
}
