#include "UIHelper.h"
#include <imgui/imnodes.h>
#include <imgui/imnodes_internal.h>

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
					//current = strings[i];
					return i;
				}

				if (isSelected)
					ImGui::SetItemDefaultFocus();
			}

			ImGui::EndCombo();
		}
		return -1;
	}

	bool ButtonEx(const char* label, const ImVec2& size_arg, 
		ImDrawFlags drawFlags, ImGuiButtonFlags flags)
	{
		using namespace ImGui;
		ImGuiWindow* window = GetCurrentWindow();
		if (window->SkipItems)
			return false;

		ImGuiContext& g = *GImGui;
		const ImGuiStyle& style = g.Style;
		const ImGuiID id = window->GetID(label);
		const ImVec2 label_size = CalcTextSize(label, NULL, true);

		ImVec2 pos = window->DC.CursorPos;
		if ((flags & ImGuiButtonFlags_AlignTextBaseLine) && style.FramePadding.y < window->DC.CurrLineTextBaseOffset) // Try to vertically align buttons that are smaller/have no padding so that text baseline matches (bit hacky, since it shouldn't be a flag)
			pos.y += window->DC.CurrLineTextBaseOffset - style.FramePadding.y;
		ImVec2 size = CalcItemSize(size_arg, label_size.x + style.FramePadding.x * 2.0f, label_size.y + style.FramePadding.y * 2.0f);

		const ImRect bb(pos, { pos.x + size.x,pos.y + size.y });
		ItemSize(size, style.FramePadding.y);
		if (!ItemAdd(bb, id))
			return false;

		bool hovered, held;
		bool pressed = ButtonBehavior(bb, id, &hovered, &held, flags);

		// Render
		const ImU32 col = GetColorU32((held && hovered) ? ImGuiCol_ButtonActive : hovered ? ImGuiCol_ButtonHovered : ImGuiCol_Button);
		RenderNavCursor(bb, id);
		RenderFrame(bb.Min, bb.Max, col, true, style.FrameRounding);

		window->DrawList->AddRectFilled(bb.Min, bb.Max, col, style.FrameRounding, drawFlags);
		const float border_size = g.Style.FrameBorderSize;
		if (true && border_size > 0.0f)
		{
			window->DrawList->AddRect({ bb.Min.x + 1,bb.Min.y + 1 }, { bb.Max.x + 1,bb.Max.y + 1 }, GetColorU32(ImGuiCol_BorderShadow), style.FrameRounding, 0, border_size);
			window->DrawList->AddRect(bb.Min, bb.Max, GetColorU32(ImGuiCol_Border), style.FrameRounding, 0, border_size);
		}
		if (g.LogEnabled)
			LogSetNextTextDecoration("[", "]");
		RenderTextClipped({ bb.Min.x + style.FramePadding.x ,bb.Min.y + style.FramePadding.y },
			{ bb.Max.x - style.FramePadding.x,bb.Max.y - style.FramePadding.y }, label, NULL, &label_size, style.ButtonTextAlign, &bb);

		// Automatically close popups
		//if (pressed && !(flags & ImGuiButtonFlags_DontClosePopups) && (window->Flags & ImGuiWindowFlags_Popup))
		//    CloseCurrentPopup();

		IMGUI_TEST_ENGINE_ITEM_INFO(id, label, g.LastItemData.StatusFlags);
		return pressed;
	}
	
	bool DragScalar(
		const char* label, ImGuiDataType data_type, void* p_data, float v_speed, 
		const void* p_min, const void* p_max, const char* format, ImGuiSliderFlags flags, ImDrawFlags drawFlags)
	{
		using namespace ImGui;
		ImGuiWindow* window = GetCurrentWindow();
		if (window->SkipItems)
			return false;

		ImGuiContext& g = *GImGui;
		const ImGuiStyle& style = g.Style;
		const ImGuiID id = window->GetID(label);
		const float w = CalcItemWidth();

		const ImVec2 label_size = CalcTextSize(label, NULL, true);
		const ImRect frame_bb(window->DC.CursorPos, window->DC.CursorPos + ImVec2(w, label_size.y + style.FramePadding.y * 2.0f));
		const ImRect total_bb(frame_bb.Min, frame_bb.Max + ImVec2(label_size.x > 0.0f ? style.ItemInnerSpacing.x + label_size.x : 0.0f, 0.0f));

		const bool temp_input_allowed = (flags & ImGuiSliderFlags_NoInput) == 0;
		ItemSize(total_bb, style.FramePadding.y);
		if (!ItemAdd(total_bb, id, &frame_bb, temp_input_allowed ? ImGuiItemFlags_Inputable : 0))
			return false;

		// Default format string when passing NULL
		if (format == NULL)
			format = DataTypeGetInfo(data_type)->PrintFmt;

		const bool hovered = ItemHoverable(frame_bb, id, g.LastItemData.ItemFlags);
		bool temp_input_is_active = temp_input_allowed && TempInputIsActive(id);
		if (!temp_input_is_active)
		{
			// Tabbing or CTRL+click on Drag turns it into an InputText
			const bool clicked = hovered && IsMouseClicked(0, ImGuiInputFlags_None, id);
			const bool double_clicked = (hovered && g.IO.MouseClickedCount[0] == 2 && TestKeyOwner(ImGuiKey_MouseLeft, id));
			const bool make_active = (clicked || double_clicked || g.NavActivateId == id);
			if (make_active && (clicked || double_clicked))
				SetKeyOwner(ImGuiKey_MouseLeft, id);
			if (make_active && temp_input_allowed)
				if ((clicked && g.IO.KeyCtrl) || double_clicked || (g.NavActivateId == id && (g.NavActivateFlags & ImGuiActivateFlags_PreferInput)))
					temp_input_is_active = true;

			// (Optional) simple click (without moving) turns Drag into an InputText
			if (g.IO.ConfigDragClickToInputText && temp_input_allowed && !temp_input_is_active)
				if (g.ActiveId == id && hovered && g.IO.MouseReleased[0] && !IsMouseDragPastThreshold(0, g.IO.MouseDragThreshold * 0.5f))
				{
					g.NavActivateId = id;
					g.NavActivateFlags = ImGuiActivateFlags_PreferInput;
					temp_input_is_active = true;
				}

			// Store initial value (not used by main lib but available as a convenience but some mods e.g. to revert)
			if (make_active)
				memcpy(&g.ActiveIdValueOnActivation, p_data, DataTypeGetInfo(data_type)->Size);

			if (make_active && !temp_input_is_active)
			{
				SetActiveID(id, window);
				SetFocusID(id, window);
				FocusWindow(window);
				g.ActiveIdUsingNavDirMask = (1 << ImGuiDir_Left) | (1 << ImGuiDir_Right);
			}
		}

		if (temp_input_is_active)
		{
			// Only clamp CTRL+Click input when ImGuiSliderFlags_AlwaysClamp is set
			const bool is_clamp_input = (flags & ImGuiSliderFlags_AlwaysClamp) != 0 && (p_min == NULL || p_max == NULL || DataTypeCompare(data_type, p_min, p_max) < 0);
			return TempInputScalar(frame_bb, id, label, data_type, p_data, format, is_clamp_input ? p_min : NULL, is_clamp_input ? p_max : NULL);
		}

		// Draw frame
		const ImU32 frame_col = GetColorU32(g.ActiveId == id ? ImGuiCol_FrameBgActive : hovered ? ImGuiCol_FrameBgHovered : ImGuiCol_FrameBg);
		RenderNavHighlight(frame_bb, id);
		//RenderFrame(frame_bb.Min, frame_bb.Max, frame_col, true, style.FrameRounding);
		window->DrawList->AddRectFilled(frame_bb.Min, frame_bb.Max, frame_col, style.FrameRounding, drawFlags);
		const float border_size = g.Style.FrameBorderSize;
		if (true && border_size > 0.0f)
		{
			window->DrawList->AddRect(frame_bb.Min + ImVec2(1, 1), frame_bb.Max + ImVec2(1, 1), GetColorU32(ImGuiCol_BorderShadow), style.FrameRounding, 0, border_size);
			window->DrawList->AddRect(frame_bb.Min, frame_bb.Max, GetColorU32(ImGuiCol_Border), style.FrameRounding, 0, border_size);
		}

		// Drag behavior
		const bool value_changed = DragBehavior(id, data_type, p_data, v_speed, p_min, p_max, format, flags);
		if (value_changed)
			MarkItemEdited(id);

		// Display value using user-provided display format so user can add prefix/suffix/decorations to the value.
		char value_buf[64];
		const char* value_buf_end = value_buf + DataTypeFormatString(value_buf, IM_ARRAYSIZE(value_buf), data_type, p_data, format);
		if (g.LogEnabled)
			LogSetNextTextDecoration("{", "}");
		RenderTextClipped(frame_bb.Min, frame_bb.Max, value_buf, value_buf_end, NULL, ImVec2(0.5f, 0.5f));

		if (label_size.x > 0.0f)
			RenderText(ImVec2(frame_bb.Max.x + style.ItemInnerSpacing.x, frame_bb.Min.y + style.FramePadding.y), label);

		IMGUI_TEST_ENGINE_ITEM_INFO(id, label, g.LastItemData.StatusFlags);
		return value_changed;
	}

	bool DragFloat(
		const char* label, float* v, float v_speed, 
		ImDrawFlags drawFlags, float v_min, float v_max, 
		const char* format, ImGuiSliderFlags flags)
	{
		return DragScalar(label, ImGuiDataType_Float, v, v_speed, &v_min, &v_max, format, flags, drawFlags);
	}

	bool DrawVec2Control(const std::string& label, glm::vec2& values, float resetValue, float columnWidth)
	{
		ImGuiIO& io = ImGui::GetIO();
		auto boldFont = io.Fonts->Fonts[0];

		ImGui::PushID(label.c_str());

		ImGui::Columns(2);
		ImGui::SetColumnWidth(0, columnWidth);
		ImGui::Text(label.c_str());
		ImGui::NextColumn();
		ImGui::PushMultiItemsWidths(2, ImGui::CalcItemWidth());

		float lineHeight = GImGui->Font->LegacySize + GImGui->Style.FramePadding.y * 2.0f;
		ImVec2 buttonSize = { lineHeight + 2.0f, lineHeight };

		bool changed = false;

		auto controlLambda = [&](float* v, const char* buttonText, const char* dragText, const ImVec4* buttonColors) {
			ImGui::PushStyleVar(ImGuiStyleVar_FrameRounding, 8.0f);
			ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, { 0,2.0f });
			ImGui::PushStyleColor(ImGuiCol_Button, buttonColors[0]);
			ImGui::PushStyleColor(ImGuiCol_ButtonHovered, buttonColors[1]);
			ImGui::PushStyleColor(ImGuiCol_ButtonActive, buttonColors[2]);
			ImGui::PushFont(boldFont);
			if (ButtonEx(buttonText, buttonSize, ImDrawFlags_RoundCornersBottomLeft | ImDrawFlags_RoundCornersTopLeft))
			{
				*v = resetValue;
				changed = true;
			}
			ImGui::PopFont();
			ImGui::PopStyleColor(3);
			ImGui::SameLine();
			if (DragFloat(dragText, v, 0.1f, ImDrawFlags_RoundCornersBottomRight | ImDrawFlags_RoundCornersTopRight, 0.0f, 0.0f, "%.2f"))
			{
				changed = true;
			}
			ImGui::PopStyleVar(2);
			ImGui::PopItemWidth();

			};
		const ImVec4 xColors[3] = { ImVec4{0.8f, 0.1f, 0.15f, 1.0f},ImVec4{0.9f, 0.2f, 0.2f, 1.0f},ImVec4{0.8f, 0.1f, 0.15f, 1.0f} };
		const ImVec4 yColors[3] = { ImVec4{ 0.2f, 0.7f, 0.2f, 1.0f},ImVec4{0.3f, 0.8f, 0.3f, 1.0f},ImVec4{0.2f, 0.7f, 0.2f, 1.0f} };

		controlLambda(&values.x, "X", "##X", xColors);
		ImGui::SameLine(0.0f, 5.0f);
		controlLambda(&values.y, "Y", "##Y", yColors);

		ImGui::Columns(1);

		ImGui::PopID();

		return changed;
	}

	bool DrawVec3Control(const std::string& label, glm::vec3& values, float resetValue, float columnWidth)
	{
		ImGuiIO& io = ImGui::GetIO();
		auto boldFont = io.Fonts->Fonts[0];

		ImGui::PushID(label.c_str());

		ImGui::Columns(2);
		ImGui::SetColumnWidth(0, columnWidth);
		ImGui::Text(label.c_str());
		ImGui::NextColumn();
		ImGui::PushMultiItemsWidths(3, ImGui::CalcItemWidth());

		bool changed = false;

		float lineHeight = GImGui->Font->LegacySize + GImGui->Style.FramePadding.y * 2.0f;
		ImVec2 buttonSize = { lineHeight + 2.0f, lineHeight };
		auto controlLambda = [&](float* v, const char* buttonText, const char* dragText, const ImVec4* buttonColors) {
			ImGui::PushStyleVar(ImGuiStyleVar_FrameRounding, 8.0f);
			ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, { 0,2.0f });
			ImGui::PushStyleColor(ImGuiCol_Button, buttonColors[0]);
			ImGui::PushStyleColor(ImGuiCol_ButtonHovered, buttonColors[1]);
			ImGui::PushStyleColor(ImGuiCol_ButtonActive, buttonColors[2]);
			ImGui::PushFont(boldFont);
			if (ButtonEx(buttonText, buttonSize, ImDrawFlags_RoundCornersBottomLeft | ImDrawFlags_RoundCornersTopLeft))
			{
				*v = resetValue;
				changed = true;
			}
			ImGui::PopFont();
			ImGui::PopStyleColor(3);
			ImGui::SameLine();
			if (DragFloat(dragText, v, 0.1f, ImDrawFlags_RoundCornersBottomRight | ImDrawFlags_RoundCornersTopRight, 0.0f, 0.0f, "%.2f"))
			{
				changed = true;
			}
			ImGui::PopStyleVar(2);
			ImGui::PopItemWidth();

			};
		const ImVec4 xColors[3] = { ImVec4{0.8f, 0.1f, 0.15f, 1.0f},ImVec4{0.9f, 0.2f, 0.2f, 1.0f},ImVec4{0.8f, 0.1f, 0.15f, 1.0f} };
		const ImVec4 yColors[3] = { ImVec4{ 0.2f, 0.7f, 0.2f, 1.0f},ImVec4{0.3f, 0.8f, 0.3f, 1.0f},ImVec4{0.2f, 0.7f, 0.2f, 1.0f} };
		const ImVec4 zColors[3] = { ImVec4{0.1f, 0.25f, 0.8f, 1.0f},ImVec4{0.2f, 0.35f, 0.9f, 1.0f},ImVec4{0.1f, 0.25f, 0.8f, 1.0f} };

		controlLambda(&values.x, "X", "##X", xColors);
		ImGui::SameLine(0.0f, 5.0f);
		controlLambda(&values.y, "Y", "##Y", yColors);
		ImGui::SameLine(0.0f, 5.0f);
		controlLambda(&values.z, "Z", "##Z", zColors);


		ImGui::Columns(1);

		ImGui::PopID();

		return changed;
	}

	bool DrawVec4Control(const std::string& label, glm::vec4& values, float resetValue, float columnWidth)
	{
		ImGuiIO& io = ImGui::GetIO();
		auto boldFont = io.Fonts->Fonts[0];

		ImGui::PushID(label.c_str());

		ImGui::Columns(2);
		ImGui::SetColumnWidth(0, columnWidth);
		ImGui::Text(label.c_str());
		ImGui::NextColumn();
		ImGui::PushMultiItemsWidths(4, ImGui::CalcItemWidth());

		bool changed = false;

		float lineHeight = GImGui->Font->LegacySize + GImGui->Style.FramePadding.y * 2.0f;
		ImVec2 buttonSize = { lineHeight + 2.0f, lineHeight };
		auto controlLambda = [&](float* v, const char* buttonText, const char* dragText, const ImVec4* buttonColors) {
			ImGui::PushStyleVar(ImGuiStyleVar_FrameRounding, 8.0f);
			ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, { 0,2.0f });
			ImGui::PushStyleColor(ImGuiCol_Button, buttonColors[0]);
			ImGui::PushStyleColor(ImGuiCol_ButtonHovered, buttonColors[1]);
			ImGui::PushStyleColor(ImGuiCol_ButtonActive, buttonColors[2]);
			ImGui::PushFont(boldFont);
			if (ButtonEx(buttonText, buttonSize, ImDrawFlags_RoundCornersBottomLeft | ImDrawFlags_RoundCornersTopLeft))
			{
				*v = resetValue;
				changed = true;
			}
			ImGui::PopFont();
			ImGui::PopStyleColor(3);
			ImGui::SameLine();
			if (DragFloat(dragText, v, 0.1f, ImDrawFlags_RoundCornersBottomRight | ImDrawFlags_RoundCornersTopRight, 0.0f, 0.0f, "%.2f"))
			{
				changed = true;
			}
			ImGui::PopStyleVar(2);
			ImGui::PopItemWidth();

			};
		const ImVec4 xColors[3] = { ImVec4{0.8f, 0.1f, 0.15f, 1.0f},ImVec4{0.9f, 0.2f, 0.2f, 1.0f},ImVec4{0.8f, 0.1f, 0.15f, 1.0f} };
		const ImVec4 yColors[3] = { ImVec4{ 0.2f, 0.7f, 0.2f, 1.0f},ImVec4{0.3f, 0.8f, 0.3f, 1.0f},ImVec4{0.2f, 0.7f, 0.2f, 1.0f} };
		const ImVec4 zColors[3] = { ImVec4{0.1f, 0.25f, 0.8f, 1.0f},ImVec4{0.2f, 0.35f, 0.9f, 1.0f},ImVec4{0.1f, 0.25f, 0.8f, 1.0f} };
		const ImVec4 wColors[3] = { ImVec4{0.84f, 0.9f, 0.70f, 1.0f},ImVec4{0.94f, 1.0f, 0.80f, 1.0f},ImVec4{0.84f, 0.9f, 0.70f, 1.0f} };


		controlLambda(&values.x, "X", "##X", xColors);
		ImGui::SameLine(0.0f, 5.0f);
		controlLambda(&values.y, "Y", "##Y", yColors);
		ImGui::SameLine(0.0f, 5.0f);
		controlLambda(&values.z, "Z", "##Z", zColors);
		ImGui::SameLine(0.0f, 5.0f);
		controlLambda(&values.w, "W", "##W", wColors);


		ImGui::Columns(1);

		ImGui::PopID();

		return changed;
	}

	bool DrawQuatControl(const std::string& label, glm::quat& values, float columnWidth)
	{
		ImGuiIO& io = ImGui::GetIO();
		auto boldFont = io.Fonts->Fonts[0];

		ImGui::PushID(label.c_str());

		ImGui::Columns(2);
		ImGui::SetColumnWidth(0, columnWidth);
		ImGui::Text(label.c_str());
		ImGui::NextColumn();
		ImGui::PushMultiItemsWidths(4, ImGui::CalcItemWidth());

		bool changed = false;

		float lineHeight = GImGui->Font->LegacySize + GImGui->Style.FramePadding.y * 2.0f;
		ImVec2 buttonSize = { lineHeight + 2.0f, lineHeight };
		auto controlLambda = [&](float* v, const char* buttonText, const char* dragText, const ImVec4* buttonColors, float resetValue) {
			ImGui::PushStyleVar(ImGuiStyleVar_FrameRounding, 8.0f);
			ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, { 0,2.0f });
			ImGui::PushStyleColor(ImGuiCol_Button, buttonColors[0]);
			ImGui::PushStyleColor(ImGuiCol_ButtonHovered, buttonColors[1]);
			ImGui::PushStyleColor(ImGuiCol_ButtonActive, buttonColors[2]);
			ImGui::PushFont(boldFont);
			if (ButtonEx(buttonText, buttonSize, ImDrawFlags_RoundCornersBottomLeft | ImDrawFlags_RoundCornersTopLeft))
			{
				*v = resetValue;
				changed = true;
			}
			ImGui::PopFont();
			ImGui::PopStyleColor(3);
			ImGui::SameLine();
			if (DragFloat(dragText, v, 0.1f, ImDrawFlags_RoundCornersBottomRight | ImDrawFlags_RoundCornersTopRight, 0.0f, 0.0f, "%.2f"))
				changed = true;
			ImGui::PopStyleVar(2);
			ImGui::PopItemWidth();

			};
		const ImVec4 xColors[3] = { ImVec4{0.8f, 0.1f, 0.15f, 1.0f},ImVec4{0.9f, 0.2f, 0.2f, 1.0f},ImVec4{0.8f, 0.1f, 0.15f, 1.0f} };
		const ImVec4 yColors[3] = { ImVec4{ 0.2f, 0.7f, 0.2f, 1.0f},ImVec4{0.3f, 0.8f, 0.3f, 1.0f},ImVec4{0.2f, 0.7f, 0.2f, 1.0f} };
		const ImVec4 zColors[3] = { ImVec4{0.1f, 0.25f, 0.8f, 1.0f},ImVec4{0.2f, 0.35f, 0.9f, 1.0f},ImVec4{0.1f, 0.25f, 0.8f, 1.0f} };
		const ImVec4 wColors[3] = { ImVec4{0.1f, 0.25f, 0.8f, 1.0f},ImVec4{0.2f, 0.35f, 0.9f, 1.0f},ImVec4{0.1f, 0.25f, 0.8f, 1.0f} };

		controlLambda(&values.x, "X", "##X", xColors, 0.0f);
		ImGui::SameLine(0.0f, 5.0f);
		controlLambda(&values.y, "Y", "##Y", yColors, 0.0f);
		ImGui::SameLine(0.0f, 5.0f);
		controlLambda(&values.z, "Z", "##Z", zColors, 0.0f);
		ImGui::SameLine(0.0f, 5.0f);
		controlLambda(&values.w, "W", "##W", wColors, 1.0f);

		ImGui::Columns(1);

		ImGui::PopID();

		return changed;
	}

}
