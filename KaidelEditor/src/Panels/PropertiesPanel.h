#pragma once
#include "Kaidel/Scene/Entity.h"
#include "PanelContext.h"


namespace Kaidel {


	class PropertiesPanel {
	public:

		void OnImGuiRender();
		void DrawComponents();
		void DrawAsset() {}
		void SetContext(Ref<PanelContext> context) {
			m_Context = context;
		}



	private:

		std::string m_AssetSelectingName;

		Ref<PanelContext> m_Context;
	};


}
