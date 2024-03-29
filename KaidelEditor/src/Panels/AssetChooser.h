#pragma once
#include "PanelContext.h"
#include <imgui/imgui.h>

namespace Kaidel {

	class AssetChooser {
	public:
		AssetChooser(Ref<PanelContext> context)
			:m_Context(context) 
		{}

		template<typename... Assets, typename... Funcs>
		void OnImGuiRender(Funcs&&... funcs) {
			static bool isOpen = false;

			bool isActive = isOpen;

			OnImGuiRenderAsset<Funcs,Assets>(funcs)...;
		}

	private:
		template<typename Func,typename Asset>
		void OnImGuiRenderAsset(Func&& func) {
			AssetManager::Each<Asset>([](Ref<Asset> asset) {
				func(asset);
				});
		}

		Ref<PanelContext> m_Context;
	};

}
