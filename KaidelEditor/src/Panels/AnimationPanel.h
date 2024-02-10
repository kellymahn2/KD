#pragma once
#include "Kaidel/Scene/Entity.h"
#include "Kaidel/Scene/Scene.h"
#include "Kaidel/Animation/Animation.h"
#include "Kaidel/Renderer/Framebuffer.h"
namespace Kaidel {
	class AnimationPanel
	{
	public:

		AnimationPanel();
		void OnImGuiRender();
		void SetSelectedAnimation(AssetHandle<Animation> anim) { m_SelectedAnimation = anim; }
		AssetHandle<Animation> GetSelectedAnimation() { return m_SelectedAnimation; }
		
	private:
		AssetHandle<Animation> m_SelectedAnimation;
		glm::vec2 m_ViewportSize{ 1280.0f,720.0f };
		Ref<Framebuffer> outputBuffer;
	};
}
