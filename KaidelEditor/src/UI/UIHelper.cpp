#include "UIHelper.h"

namespace Kaidel {
	uint64_t Combo(const char* name, const char* strings[], uint64_t stringCount, const char*& current) {
		if (ImGui::BeginCombo(name, current))
		{
			for (int i = 0; i < stringCount; i++)
			{
				bool isSelected = current == strings[i];
				if (ImGui::Selectable(strings[i], isSelected))
				{
					ImGui::EndCombo();
					current = strings[i];
					return i;
				}

				if (isSelected)
					ImGui::SetItemDefaultFocus();
			}

			ImGui::EndCombo();
		}
		return -1;
	}
}
