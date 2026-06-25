#pragma once

#include "EditorContext.h"

namespace Kaidel {

	class AssetPropertiesPanel {
	public:

		AssetPropertiesPanel(Asset asset)
			: m_SelectedAsset(asset)
		{
		}

		void OnImGuiRender();
		void SetContext(Asset asset) {
			m_SelectedAsset = asset;
		}
	public:
		bool StandardMaterialProperties();
		bool VisualMaterialProperties();

	private:
		Asset m_SelectedAsset;
	};
}
