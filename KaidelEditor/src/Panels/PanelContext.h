#pragma once
#include "Kaidel/Scene/Entity.h"
#include "Kaidel/Assets/AssetManager.h"
namespace Kaidel {

	enum class SelectedType {
		None,
		Entity,
		Asset
	};

	union SelectedData {
		Ref<_Asset> Asset;

	};


	class PanelContext : public IRCCounter<false> {
	public:
		SelectedType Type = SelectedType::None;
		Ref<_Asset> _SelectedAsset{};
		Entity _SelectedEntity{};

		Ref<Scene> Scene;

		PanelContext() {

		}

		Entity SelectedEntity() const{
			if (Type == SelectedType::Entity)
				return _SelectedEntity;
			return {};
		}
		Ref<_Asset> SelectedAsset()const {
			if (Type == SelectedType::Asset)
				return _SelectedAsset;
			return {};
		}

		~PanelContext() override {
		}

	};


}
