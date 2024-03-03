#pragma once
#include "Kaidel/Scene/Entity.h"
#include "Kaidel/Scene/Scene.h"
#include "Kaidel/Animation/Animation.h"
#include "Kaidel/Renderer/GraphicsAPI/Framebuffer.h"
namespace Kaidel {
	class AnimationPanel
	{
	public:

		AnimationPanel();
		void OnImGuiRender();
		void SetSelectedAnimation(Asset<Animation> anim) { m_SelectedAnimation = anim; }
		Asset<Animation> GetSelectedAnimation() { return m_SelectedAnimation; }
		

	private:
		Asset<Animation> m_SelectedAnimation;
		glm::vec2 m_ViewportSize{ 1280.0f,720.0f };
		glm::vec2 m_ImagePos{ 0,0 };
		glm::vec2 m_LastFrameMousePos{ 0,0 };
		Ref<Framebuffer> outputBuffer;
		uint32_t m_CurrentControlPointBeingEdited = -1;
	};
}
