#pragma once
#include "Kaidel/Scene/Entity.h"
namespace Kaidel {

	enum class SelectedType {
		None,
		Entity,
		Asset
	};



	class PanelContext : public IRCCounter<false> {
	public:
		SelectedType Type = SelectedType::None;
		Entity _SelectedEntity{};

		Ref<Scene> Scene;

		PanelContext() {

		}

		Entity SelectedEntity() const{
			if (Type == SelectedType::Entity)
				return _SelectedEntity;
			return {};
		}
		~PanelContext() override {
		}

	};


}
