#pragma once
#include "Kaidel/Core/Console.h"
#include "Kaidel/Renderer/GraphicsAPI/Texture.h"
namespace Kaidel {
	class ConsolePanel {
	public:
		ConsolePanel();
		~ConsolePanel() = default;

		void OnImGuiRender();
		void SetContext(Ref<Console> context) { m_ConsoleContext = context; }

	private:
		enum Filter_ {
			Filter_Log = BIT(0),
			Filter_Info = BIT(1),
			Filter_Warn = BIT(2),
			Filter_Error = BIT(3),
			Filter_All = Filter_Log | Filter_Info | Filter_Warn | Filter_Error
		};
		friend static int GetFilters(int);
		struct Icons {
			Ref<Texture2D> ErrorIcon;
			Ref<Texture2D> WarnIcon;
			Ref<Texture2D> InfoIcon;
		};
		int m_CurrentFilter = Filter_All;
		Icons m_Icons;
		Ref<Console> m_ConsoleContext;
	};

}
