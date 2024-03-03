#pragma once
#include "Kaidel/Scene/Entity.h"
#include "PanelContext.h"
#include "Kaidel/Assets/AssetManager.h"


namespace Kaidel {


	class PropertiesPanel {
	public:

		void OnImGuiRender();
		void DrawComponents();
		void DrawAsset();
		void DrawMaterialUI(Ref<Material> mat);
		void SetContext(Ref<PanelContext> context) {
			m_Context = context;
		}

	private:
		Ref<PanelContext> m_Context;
	};


}
