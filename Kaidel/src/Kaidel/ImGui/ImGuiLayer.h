#pragma once

#include "Kaidel/Core/Layer.h"

#include "Kaidel/Events/ApplicationEvent.h"
#include "Kaidel/Events/KeyEvent.h"
#include "Kaidel/Events/MouseEvent.h"

namespace Kaidel {

	class ImGuiLayer : public Layer
	{
	public:
		ImGuiLayer();
		~ImGuiLayer() = default;

		virtual void OnAttach() override;
		virtual void OnDetach() override;
		virtual void OnEvent(Event& e) override;

		void Begin();
		void End();

		void BlockEvents(bool block) { m_BlockEvents = block; }
		
		void SetDarkThemeColors();
	private:
		bool m_BlockEvents = true;
		float m_Time = 0.0f;
	};

}
