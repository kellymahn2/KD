#pragma once

#include "Kaidel/Scene/Entity.h"
#include "Kaidel/Animation/Animation.h"
#include "AssetChooser.h"



namespace Kaidel {

	class ModelLoader;


	class EditorContext
	{
	public:
		static void Init();
		static void Shutdown();
		
		static Entity SelectedEntity();
		static void SelectedEntity(Entity selectedEntity);

		static Asset SelectedAsset();
		static void SelectedAsset(Asset asset);

		static AnimationFrame* SelectedAnimationFrame();
		static AnimationValueType SelectedAnimationFrameValueType();
		static void SelectedAnimationFrame(AnimationFrame* selectedAnimationFrame, AnimationValueType type);

		static AssetFilter& GetActiveAssetFilter();
		static OnAssetChosen& GetActiveAssetFunc();

		static void SetAssetChooserData(const AssetFilter& filter, const OnAssetChosen& onChosen);

	};
}
