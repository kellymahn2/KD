#pragma once
#include "Kaidel/Scene/Entity.h"
#include "EditorContext.h"


namespace Kaidel {

	class EntityPropertiesPanel {
	public:

		EntityPropertiesPanel(Entity context)
			: m_SelectedEntity(context)
		{
		}

		void OnImGuiRender();
		void DrawComponents();
		void DrawAsset() {}
		void SetContext(Entity context) {
			m_SelectedEntity = context;
		}

	private:
		std::string m_AssetSelectingName;
		
		Entity m_SelectedEntity;
	};


}
