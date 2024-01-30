#pragma once
#include "Kaidel/Scene/Entity.h"
#include "Kaidel/Scene/Scene.h"
#include "Kaidel/Animation/Animation.h"
namespace Kaidel {
	class AnimationPanel
	{
	public:

		AnimationPanel() = default;

		void OnImGuiRender();
		void SetSelectedAnimation(AssetHandle<Animation> anim) { m_SelectedAnimation = anim; }
		AssetHandle<Animation> GetSelectedAnimation() { return m_SelectedAnimation; }

		
	private:
		AssetHandle<Animation> m_SelectedAnimation;
	};
}
