#include "SceneHierarchyPanel.h"
#include "Kaidel/Math/Math.h"
#include <imgui/imgui.h>
#include <imgui/imgui_internal.h>

#include <glm/gtc/type_ptr.hpp>
#include "Kaidel/Core/Timer.h"
#include "Kaidel/Scene/Components.h"
#include <cstring>
#include <sstream>
#include "Kaidel/Scripting/ScriptEngine.h"
/* The Microsoft C++ compiler is non-compliant with the C++ standard and needs
 * the following definition to disable a security warning on std::strncpy().
 */
#ifdef _MSVC_LANG
  #define _CRT_SECURE_NO_WARNINGS
#endif
namespace Kaidel {
	typedef std::function<void(const std::string& name, const ScriptField& field, Ref<ScriptInstance>& instance)> FieldRendererFunction;
	std::unordered_map <ScriptFieldType, FieldRendererFunction>s_FieldRenderers;
	void RegisterFieldRenderer(ScriptFieldType type, FieldRendererFunction&& func) {
		s_FieldRenderers[type] = func;
	}
	void SceneHierarchyPanel::RegisterFieldRenderers() {
		//RegisterFieldRenderer(ScriptFieldType::Float, [](const std::string& name, const ScriptField& field, Ref<ScriptInstance>& instance) {
		//	float data = instance->GetFieldValue<float>(field);
		//	if (ImGui::DragFloat(name.c_str(), &data)) {
		//		instance->SetFieldValue(field, data);
		//	}
		//});
		///*RegisterFieldRenderer(ScriptFieldType::Double, [](auto& name, auto& field, auto& instance) {
		//	double data = instance->GetFieldValue<double>(field);
		//	if (ImGui::DragFloat(name.c_str(), (float*) & data)) {
		//		instance->SetFieldValue(field, data);
		//	}
		//});*/
		////RegisterFieldRenderer(ScriptFieldType::Short);
		////RegisterFieldRenderer(ScriptFieldType::UShort);
		//RegisterFieldRenderer(ScriptFieldType::Int, [](auto& name, auto& field, auto& instance) {
		//	int data = instance->GetFieldValue<int>(field);
		//	if (ImGui::DragInt(name.c_str(), &data)) {
		//		instance->SetFieldValue(field, data);
		//	}
		//});
		//RegisterFieldRenderer(ScriptFieldType::UInt, [](auto& name, auto& field, auto& instance) {
		//	uint32_t data = instance->GetFieldValue<uint32_t>(field);
		//	if (ImGui::DragInt(name.c_str(), (int*) & data)) {
		//		instance->SetFieldValue(field, data);
		//	}
		//});
		////RegisterFieldRenderer(ScriptFieldType::Long);
		////RegisterFieldRenderer(ScriptFieldType::ULong);
		////RegisterFieldRenderer(ScriptFieldType::Byte);
		///*RegisterFieldRenderer(ScriptFieldType::SByte, [](auto& name, auto& field, auto& instance) {

		//	std::string data = instance->GetFieldValue<char*>(field);
		//	char buff[256];
		//	memcpy(buff, data.data(), data.size() + 1);
		//	if (ImGui::InputText(name.c_str(), buff, 256)) {
		//		data = buff;
		//		instance->SetFieldValue(field, data);
		//	}

		//});*/
		////RegisterFieldRenderer(ScriptFieldType::Char);
		//RegisterFieldRenderer(ScriptFieldType::String, [](auto& name, auto& field, auto& instance){
		//	std::string data = instance->GetFieldValue<char*>(field);
		//	char buff[256];
		//	memcpy(buff, data.data(), data.size() + 1);
		//	if (ImGui::InputText(name.c_str(), buff, 256)) {
		//		data = buff;
		//		instance->SetFieldValue(field, data);
		//	}
		//});
		////RegisterFieldRenderer(ScriptFieldType::Bool);
		////RegisterFieldRenderer(ScriptFieldType::Entity);
		//RegisterFieldRenderer(ScriptFieldType::Vector2, [](auto& name, auto& field, auto& instance) {
		//	glm::vec2 data = instance->GetFieldValue<glm::vec2>(field);
		//	if (ImGui::DragFloat2(name.c_str(), glm::value_ptr(data))) {
		//		instance->SetFieldValue(field,data);
		//	}
		//});
		//RegisterFieldRenderer(ScriptFieldType::Vector3, [](auto& name, auto& field, auto& instance) {
		//	glm::vec3 data = instance->GetFieldValue<glm::vec3>(field);
		//	if (ImGui::DragFloat3(name.c_str(), glm::value_ptr(data))) {
		//		instance->SetFieldValue(field, data);
		//	}
		//});
		//RegisterFieldRenderer(ScriptFieldType::Vector4, [](auto& name, auto& field, auto& instance) {
		//	glm::vec4 data = instance->GetFieldValue<glm::vec4>(field);
		//	if (ImGui::DragFloat4(name.c_str(), glm::value_ptr(data))) {
		//		instance->SetFieldValue(field, data);
		//	}
		//});
		////RegisterFieldRenderer(ScriptFieldType::Vector3);
		////RegisterFieldRenderer(ScriptFieldType::Vector4);

		
	}
	SceneHierarchyPanel::SceneHierarchyPanel(const Ref<Scene>& context)
	{
		SetContext(context);
	}

	void SceneHierarchyPanel::SetContext(const Ref<Scene>& context)
	{
		m_Context = context;
		m_SelectionContext = {};
	}

	void SceneHierarchyPanel::OnImGuiRender()
	{
		ImGui::Begin("Scene Hierarchy",nullptr,ImGuiWindowFlags_NoNavInputs);
		m_Context->m_Registry.each([&](auto entityID)
			{
				
				Entity entity{entityID , m_Context.get()};
				if (entity.HasComponent<ChildComponent>())
					return;
				DrawEntityNode(entity);
			});

		if (ImGui::IsMouseDown(0) && ImGui::IsWindowHovered()) {
			//if((m_SelectionContext&&!(m_SelectionContext.HasComponent<ParentComponent>() && !m_SelectionContext.GetComponent<ParentComponent>().Children.empty())))
				m_SelectionContext = {};
		}

		// Right-click on blank space
		if (ImGui::BeginPopupContextWindow(0, 1|ImGuiPopupFlags_NoOpenOverItems|ImGuiPopupFlags_NoOpenOverExistingPopup))
		{
			if (ImGui::MenuItem("Create Empty Entity"))
				m_Context->CreateEntity("Empty Entity");

			ImGui::EndPopup();
		}

		ImGui::End();
		ImGui::Begin("Properties");
		if (m_SelectionContext)
		{
			DrawComponents(m_SelectionContext);
		}

		ImGui::End();
	}

	void SceneHierarchyPanel::SetSelectedEntity(Entity entity)
	{
		m_SelectionContext = entity;
	}

	void SceneHierarchyPanel::DrawEntityNode(Entity entity)
	{
		auto& tag = entity.GetComponent<TagComponent>().Tag;
		auto currContext = m_SelectionContext;
		ImGui::PushStyleColor(ImGuiCol_HeaderHovered, { 0,0,0,0 });
		ImGui::PushStyleColor(ImGuiCol_HeaderActive, { 0,0,0,0 });
		ImGui::PushStyleColor(ImGuiCol_Header, { 0,0,0,0 });
		static float defFont = ImGui::GetFont()->FontSize;
		if (currContext == entity) {
			ImGui::GetFont()->FontSize -= .8f;
			ImGui::PushStyleColor(ImGuiCol_HeaderHovered, { .2,.2,.2,1.0f });
			ImGui::PushStyleColor(ImGuiCol_HeaderActive, { .2,.2,.2,1.0f });
			ImGui::PushStyleColor(ImGuiCol_Header, { .2,.2,.2,1.0f });
		}
		else
			ImGui::GetFont()->FontSize = defFont;
		ImGuiTreeNodeFlags flags = ((m_SelectionContext == entity) ? ImGuiTreeNodeFlags_Selected : 0) | ImGuiTreeNodeFlags_OpenOnDoubleClick;
		flags |= ImGuiTreeNodeFlags_SpanAvailWidth;
		bool opened = false;
		bool entityDeleted = false;
		if (entity.HasChildren()) {
			opened = ImGui::TreeNodeEx((void*)(uint64_t)entity.GetUUID(), flags, tag.c_str());
			if (ImGui::BeginDragDropSource()) {
				ImGui::SetDragDropPayload("ENTITY_PARENT", &entity.GetComponent<IDComponent>(), sizeof(uint64_t));
				ImGui::EndDragDropSource();
			}
			if (ImGui::BeginDragDropTarget()) {
				if (auto payload = ImGui::AcceptDragDropPayload("ENTITY_PARENT")) {
					auto childEntityID = (*(IDComponent*)payload->Data).ID;

					entity.AddChild(childEntityID);

					auto childEntity = m_Context->GetEntity(childEntityID);
					if (childEntity.HasComponent<ChildComponent>()) {
						auto& cc = childEntity.GetComponent<ChildComponent>();
						auto oldParent = m_Context->GetEntity(cc.Parent);
						auto& oldpc = oldParent.GetComponent<ParentComponent>();
						auto it = std::find(oldpc.Children.begin(), oldpc.Children.end(), childEntityID);
						if (it != oldpc.Children.end())
							oldpc.Children.erase(it, it + 1);
					}
					else {
						childEntity.AddComponent<ChildComponent>(entity.GetUUID());
					}
					childEntity.GetComponent<ChildComponent>().LocalPosition = childEntity.GetComponent<TransformComponent>().Translation 
						- entity.GetComponent<TransformComponent>().Translation;
				}
			}
			if (ImGui::IsItemClicked())
			{
				m_SelectionContext = entity;
			}
			if (opened) {
				for (auto& child : entity.GetComponent<ParentComponent>().Children) {
					DrawEntityNode(m_Context->GetEntity(child));
					ImGui::TreePop();
				}
			}
		}
		else {
			flags |= ImGuiTreeNodeFlags_Leaf| ImGuiTreeNodeFlags_NoTreePushOnOpen;
			ImGui::PushID((uint64_t)entity.GetUUID());
			opened = ImGui::TreeNodeEx(tag.c_str(),flags);
			const char* id = "PopupID";
			if (ImGui::IsItemClicked(ImGuiMouseButton_Right))
				ImGui::OpenPopup(id,ImGuiPopupFlags_NoOpenOverExistingPopup);
			if (ImGui::BeginPopup(id)) {
				if (ImGui::MenuItem("RemoveEntity"))
					entityDeleted = true;
				ImGui::EndPopup();
			}
			if (ImGui::BeginDragDropSource()) {
				ImGui::SetDragDropPayload("ENTITY_PARENT", &entity.GetComponent<IDComponent>(), sizeof(uint64_t));
				ImGui::EndDragDropSource();
			}
			if (ImGui::BeginDragDropTarget()) {
				if (auto payload = ImGui::AcceptDragDropPayload("ENTITY_PARENT")) {
					auto childEntityID= (*(IDComponent*)payload->Data).ID;

					entity.AddChild(childEntityID);

					auto childEntity = m_Context->GetEntity(childEntityID);
					if (childEntity.HasComponent<ChildComponent>()) {
						auto& cc = childEntity.GetComponent<ChildComponent>();
						auto oldParent = m_Context->GetEntity(cc.Parent);
						auto& oldpc = oldParent.GetComponent<ParentComponent>();
						auto it = std::find(oldpc.Children.begin(), oldpc.Children.end(), childEntityID);
						if (it != oldpc.Children.end())
							oldpc.Children.erase(it, it + 1);
					}
					else {
						childEntity.AddComponent<ChildComponent>(entity.GetUUID());
					}
					childEntity.GetComponent<ChildComponent>().LocalPosition = childEntity.GetComponent<TransformComponent>().Translation
						- entity.GetComponent<TransformComponent>().Translation;
				}
			}
			if (ImGui::IsItemClicked()) {
				m_SelectionContext = entity;
			}
			
			ImGui::PopID();
		}
		if (currContext == entity) {
			ImGui::GetFont()->FontSize =defFont;
			ImGui::PopStyleColor(3);
		}
		ImGui::PopStyleColor(3);
		
		/*if (opened)
		{
			ImGuiTreeNodeFlags flags = ImGuiTreeNodeFlags_OpenOnArrow | ImGuiTreeNodeFlags_SpanAvailWidth;
			bool opened = ImGui::TreeNodeEx((void*)9817239, flags, tag.c_str());
			if (opened)
				ImGui::TreePop();
			ImGui::TreePop();
		}*/

		if (entityDeleted)
		{
			m_Context->DestroyEntity(entity);
			if (m_SelectionContext == entity)
				m_SelectionContext = {};
		}
	}
	bool ButtonEx(const char* label, const ImVec2& size_arg = { 0,0 },ImDrawFlags drawFlags=0, ImGuiButtonFlags flags=0)
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

		if (g.LastItemData.InFlags & ImGuiItemFlags_ButtonRepeat)
			flags |= ImGuiButtonFlags_Repeat;

		bool hovered, held;
		bool pressed = ButtonBehavior(bb, id, &hovered, &held, flags);

		// Render
		const ImU32 col = GetColorU32((held && hovered) ? ImGuiCol_ButtonActive : hovered ? ImGuiCol_ButtonHovered : ImGuiCol_Button);
		RenderNavHighlight(bb, id);
		//RenderFrame(bb.Min, bb.Max, col, true, style.FrameRounding);

		window->DrawList->AddRectFilled(bb.Min, bb.Max, col, style.FrameRounding, drawFlags);
		const float border_size = g.Style.FrameBorderSize;
		if (true&& border_size > 0.0f)
		{
			window->DrawList->AddRect({bb.Min.x + 1,bb.Min.y + 1}, { bb.Max.x + 1,bb.Max.y + 1 }, GetColorU32(ImGuiCol_BorderShadow), style.FrameRounding, 0, border_size);
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
	ImVec2 operator + (const ImVec2& lhs, const ImVec2& rhs) {
		return { lhs.x + rhs.x,lhs.y + rhs.y };
	}
	bool DragScalar(const char* label, ImGuiDataType data_type, void* p_data, float v_speed, const void* p_min, const void* p_max, const char* format, ImGuiSliderFlags flags,ImDrawFlags drawFlags)
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

		const bool hovered = ItemHoverable(frame_bb, id);
		bool temp_input_is_active = temp_input_allowed && TempInputIsActive(id);
		if (!temp_input_is_active)
		{
			// Tabbing or CTRL-clicking on Drag turns it into an InputText
			const bool input_requested_by_tabbing = temp_input_allowed && (g.LastItemData.StatusFlags & ImGuiItemStatusFlags_FocusedByTabbing) != 0;
			const bool clicked = hovered && IsMouseClicked(0, id);
			const bool double_clicked = (hovered && g.IO.MouseClickedCount[0] == 2 && TestKeyOwner(ImGuiKey_MouseLeft, id));
			const bool make_active = (input_requested_by_tabbing || clicked || double_clicked || g.NavActivateId == id || g.NavActivateInputId == id);
			if (make_active && (clicked || double_clicked))
				SetKeyOwner(ImGuiKey_MouseLeft, id);
			if (make_active && temp_input_allowed)
				if (input_requested_by_tabbing || (clicked && g.IO.KeyCtrl) || double_clicked || g.NavActivateInputId == id)
					temp_input_is_active = true;

			// (Optional) simple click (without moving) turns Drag into an InputText
			if (g.IO.ConfigDragClickToInputText && temp_input_allowed && !temp_input_is_active)
				if (g.ActiveId == id && hovered && g.IO.MouseReleased[0] && !IsMouseDragPastThreshold(0, g.IO.MouseDragThreshold * .5f))
				{
					g.NavActivateId = g.NavActivateInputId = id;
					g.NavActivateFlags = ImGuiActivateFlags_PreferInput;
					temp_input_is_active = true;
				}

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
		window->DrawList->AddRectFilled(frame_bb.Min, frame_bb.Max, frame_col, style.FrameRounding,drawFlags);
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

	bool DragFloat(const char* label, float* v, float v_speed=1.0f, ImDrawFlags drawFlags=0,float v_min=0.0f, float v_max=0.0f
		, const char* format="%.3f", ImGuiSliderFlags flags=0)
	{
		return DragScalar(label, ImGuiDataType_Float, v,v_speed, &v_min, &v_max, format, flags, drawFlags);
	}
	static void DrawVec3Control(const std::string& label, glm::vec3& values, float resetValue = 0.0f, float columnWidth = 100.0f)
	{
		ImGuiIO& io = ImGui::GetIO();
		auto boldFont = io.Fonts->Fonts[0];

		ImGui::PushID(label.c_str());

		ImGui::Columns(2);
		ImGui::SetColumnWidth(0, columnWidth);
		ImGui::Text(label.c_str());
		ImGui::NextColumn();
		ImGui::PushMultiItemsWidths(3, ImGui::CalcItemWidth());

		float lineHeight = GImGui->Font->FontSize + GImGui->Style.FramePadding.y * 2.0f;
		ImVec2 buttonSize = { lineHeight + 3.0f, lineHeight };
		static auto controlLambda = [&](float* v,const char* buttonText,const char* dragText,const ImVec4* buttonColors) {
			ImGui::PushStyleVar(ImGuiStyleVar_FrameRounding, 8.0f);
			ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, { 0,2.0f });
			ImGui::PushStyleColor(ImGuiCol_Button, buttonColors[0]);
			ImGui::PushStyleColor(ImGuiCol_ButtonHovered, buttonColors[1]);
			ImGui::PushStyleColor(ImGuiCol_ButtonActive, buttonColors[2]);
			ImGui::PushFont(boldFont);
			if (ButtonEx(buttonText, buttonSize, ImDrawFlags_RoundCornersBottomLeft | ImDrawFlags_RoundCornersTopLeft))
				*v = resetValue;
			ImGui::PopFont();
			ImGui::PopStyleColor(3);
			ImGui::SameLine();
			DragFloat(dragText, v, 0.1f, ImDrawFlags_RoundCornersBottomRight | ImDrawFlags_RoundCornersTopRight, 0.0f, 0.0f, "%.2f");
			ImGui::PopStyleVar(2);
			ImGui::PopItemWidth();
			
			};
		const ImVec4 xColors[3]= {ImVec4{0.8f, 0.1f, 0.15f, 1.0f},ImVec4{0.9f, 0.2f, 0.2f, 1.0f},ImVec4{0.8f, 0.1f, 0.15f, 1.0f}};
		const ImVec4 yColors[3] = { ImVec4{ 0.2f, 0.7f, 0.2f, 1.0f},ImVec4{0.3f, 0.8f, 0.3f, 1.0f},ImVec4{0.2f, 0.7f, 0.2f, 1.0f} };
		const ImVec4 zColors[3] = { ImVec4{0.1f, 0.25f, 0.8f, 1.0f},ImVec4{0.2f, 0.35f, 0.9f, 1.0f},ImVec4{0.1f, 0.25f, 0.8f, 1.0f} };

		controlLambda(&values.x, "X", "##X",xColors);
		ImGui::SameLine(0.0f,5.0f);
		controlLambda(&values.y, "Y", "##Y",yColors);
		ImGui::SameLine(0.0f, 5.0f);
		controlLambda(&values.z, "Z", "##Z",zColors);
		

		ImGui::Columns(1);

		ImGui::PopID();
	}

	template<typename T, typename UIFunction>
	static void DrawComponent(const std::string& name, Entity entity, UIFunction uiFunction,bool removeable = true)
	{
		static std::unordered_map<std::string, bool> s_Map;
		const ImGuiTreeNodeFlags treeNodeFlags = ImGuiTreeNodeFlags_DefaultOpen |ImGuiTreeNodeFlags_OpenOnDoubleClick| ImGuiTreeNodeFlags_Framed | ImGuiTreeNodeFlags_SpanAvailWidth | ImGuiTreeNodeFlags_AllowItemOverlap | ImGuiTreeNodeFlags_FramePadding|ImGuiTreeNodeFlags_Leaf;
		if (entity.HasComponent<T>())
		{
			ImVec2 contentRegionAvailable = ImGui::GetContentRegionAvail();
			auto& component = entity.GetComponent<T>();
			float lineHeight = GImGui->Font->FontSize + GImGui->Style.FramePadding.y * 2.0f;
			static float itemHeight = 0.0f;
			static float itemWidth = 0.0f;
			bool open = false;
			bool button = false;
			ImGui::Separator();
			auto it = s_Map.find(name);
			if (it == s_Map.end()||it->second) {
				ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, ImVec2{ 4, 4 });
				ImGui::PushStyleColor(ImGuiCol_Header, ImVec4{ 0,0,0,0 });
				ImGui::PushStyleColor(ImGuiCol_HeaderActive, ImVec4{ 0,0,0,0 });
				ImGui::PushStyleColor(ImGuiCol_HeaderHovered, ImVec4{ 0,0,0,0 });

				open = ImGui::TreeNodeEx((void*)typeid(T).hash_code(), treeNodeFlags, name.c_str());
				itemWidth = ImGui::GetItemRectSize().x;
				itemHeight = ImGui::GetItemRectSize().y;
				ImGui::PopStyleColor(3);
				ImGui::PopStyleVar();
			}
			else {
				ImGui::PushStyleColor(ImGuiCol_Button, ImVec4{ 0,0,0,0 });
				ImGui::PushStyleColor(ImGuiCol_ButtonActive, ImVec4{ 0,0,0,0 });
				ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4{ 0,0,0,0 });
				ImGui::PushStyleVar(ImGuiStyleVar_ButtonTextAlign, {0,0});
				ImGui::Button(name.c_str(), { itemWidth,itemHeight });
				ImGui::PopStyleVar();
				ImGui::PopStyleColor(3);

				button = true;
			}
			if (ImGui::IsMouseDoubleClicked(ImGuiMouseButton_Left) && ImGui::IsItemHovered()) {
				if (it == s_Map.end()) {
					s_Map[name] = false;
				}
				else
					s_Map[name] = !s_Map[name];
			}
			
			if (removeable) {
				ImGui::SameLine(contentRegionAvailable.x - ImGui::GetItemRectSize().y*.5f);
				ImGui::PushStyleColor(ImGuiCol_Button, ImVec4{ 0,0,0,0 });
				ImGui::PushStyleColor(ImGuiCol_ButtonActive, ImVec4{ 0,0,0,0 });
				ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4{ 0,0,0,0 });
				if (ImGui::Button("+", ImVec2{ ImGui::GetItemRectSize().y, ImGui::GetItemRectSize().y}))
				{
					ImGui::OpenPopup("ComponentSettings");
				}
				ImGui::PopStyleColor(3);

			}

			bool removeComponent = false;
			if (removeable) {
				if (ImGui::BeginPopup("ComponentSettings"))
				{
					if (ImGui::MenuItem("Remove component"))
						removeComponent = true;

					ImGui::EndPopup();
				}
			}

			if (open)
			{
				uiFunction(component);
				ImGui::TreePop();
			}
			if (removeable) {
				if (removeComponent)
					entity.RemoveComponent<T>();
			}
		}
	}
	template<typename T>
	static void DrawAddComponentItems(Entity& entity, const std::string& text, std::function<void(T&)> func = [](T&){}) {
		if (!entity.HasComponent<T>()) {
			if (ImGui::MenuItem(text.c_str())) {
				func(entity.AddComponent<T>());
				ImGui::CloseCurrentPopup();
			}
		}
	}
	class AssetChooserScriptItem {
	public:
		AssetChooserScriptItem() = default;
		AssetChooserScriptItem(const std::unordered_map<std::string,Ref<ScriptClass>>& map) {
			for (const auto& [name, field] : map) {
				AddScript(name);
			}
		}
		void AddScript(const std::string& script) {
			std::size_t loc = script.find_first_of('.');
			if (loc != std::string::npos) {
				std::string name = script.substr(0, loc);
				m_ScriptMap[name].AddScript(script.substr(loc + 1));
				return;
			}
			m_ScriptMap[script];
		}
		std::string Draw() {
			for (auto& [name, acsi] : m_ScriptMap) {
				if (acsi.m_ScriptMap.empty()) {
					ImGui::MenuItem(name.c_str());
					if (ImGui::IsItemHovered()&&ImGui::IsMouseDoubleClicked(ImGuiMouseButton_Left)) {
						return name;
					}
				}
				else {
					if (ImGui::TreeNodeEx(name.c_str(),ImGuiTreeNodeFlags_SpanAvailWidth)) {
						std::string res = acsi.Draw();
						ImGui::TreePop();
						if(!res.empty())
							return name + '.' + res;
					}
				}
			}
			return "";
		}
	private:
		std::unordered_map<std::string, AssetChooserScriptItem> m_ScriptMap;
	};
	void SceneHierarchyPanel::DrawComponents(Entity entity)
	{
		auto& id = entity.GetComponent<IDComponent>();
		ImGui::Checkbox("##Active", &id.IsActive);
		ImGui::SameLine();
		if (entity.HasComponent<TagComponent>())
		{
			auto& tag = entity.GetComponent<TagComponent>().Tag;
			char buffer[256];
			memset(buffer, 0, sizeof(buffer));
			std::strncpy(buffer, tag.c_str(), sizeof(buffer));
			if (ImGui::InputText("##Tag", buffer, sizeof(buffer)))
			{
				tag = std::string(buffer);
			}
		}

		ImGui::SameLine();
		ImGui::PushItemWidth(-1);

		if (ImGui::Button("Add Component"))
			ImGui::OpenPopup("AddComponent");

		if (ImGui::BeginPopup("AddComponent"))
		{
			//DrawAddComponentItems<ScriptComponent>(entity, "Script Component");
			if (ImGui::MenuItem("Script Component")) {
				entity.AddScript();
			}
			DrawAddComponentItems<CameraComponent>(entity, "Camera");
			DrawAddComponentItems<SpriteRendererComponent>(entity, "Sprite Renderer");
			DrawAddComponentItems<CircleRendererComponent>(entity, "Circle Renderer");
			DrawAddComponentItems<Rigidbody2DComponent>(entity, "Rigidbody 2D");
			DrawAddComponentItems<BoxCollider2DComponent>(entity, "Box Collider 2D");
			DrawAddComponentItems<CircleCollider2DComponent>(entity, "Circle Collider 2D");
			DrawAddComponentItems<LineRendererComponent>(entity, "Line Renderer", [](LineRendererComponent& lrc) {
				lrc.Points = { LineRendererComponent::Point{ {0,0,0} }, LineRendererComponent::Point{ {1,1,0} } };
				lrc.RecalculateFinalPoints();
				});
			//DrawAddComponentItems<LightComponent>(entity, "Light");
			DrawAddComponentItems<CubeRendererComponent>(entity, "Cube Renderer");
			//DrawAddComponentItems<ParentComponent>(m_SelectionContext, "Parent");
			//DrawAddComponentItems<ChildComponent>(m_SelectionContext, "Child");
			ImGui::EndPopup();
		}

		ImGui::PopItemWidth();
		ImGui::Text(std::to_string(m_SelectionContext.GetUUID().operator uint64_t()).c_str());
		DrawComponent<TransformComponent>("Transform", entity, [&entity,scene = m_Context](auto& component)
			{
				bool hasParent = entity.HasComponent<ChildComponent>();
				glm::vec3 pos = component.Translation, rot = glm::degrees(component.Rotation);
				if (hasParent) {
					auto& cc = entity.GetComponent<ChildComponent>();
					pos = cc.LocalPosition;
					rot = glm::degrees(cc.LocalRotation);
				}
				glm::vec3 orgPos = pos, orgRot = rot;
				DrawVec3Control("Translation", pos);
				DrawVec3Control("Rotation", rot);
				DrawVec3Control("Scale", component.Scale, 1.0f);
				MoveEntity(entity, scene.get(), pos - orgPos, glm::radians(rot - orgRot));
			},false);

		DrawComponent<CameraComponent>("Camera", entity, [](auto& component)
			{
				auto& camera = component.Camera;

				ImGui::Checkbox("Primary", &component.Primary);

				const char* projectionTypeStrings[] = { "Perspective", "Orthographic" };
				const char* currentProjectionTypeString = projectionTypeStrings[(int)camera.GetProjectionType()];
				if (ImGui::BeginCombo("Projection", currentProjectionTypeString))
				{
					for (int i = 0; i < 2; i++)
					{
						bool isSelected = currentProjectionTypeString == projectionTypeStrings[i];
						if (ImGui::Selectable(projectionTypeStrings[i], isSelected))
						{
							currentProjectionTypeString = projectionTypeStrings[i];
							camera.SetProjectionType((SceneCamera::ProjectionType)i);
						}

						if (isSelected)
							ImGui::SetItemDefaultFocus();
					}

					ImGui::EndCombo();
				}

				if (camera.GetProjectionType() == SceneCamera::ProjectionType::Perspective)
				{
					float perspectiveVerticalFov = glm::degrees(camera.GetPerspectiveVerticalFOV());
					if (ImGui::DragFloat("Vertical FOV", &perspectiveVerticalFov))
						camera.SetPerspectiveVerticalFOV(glm::radians(perspectiveVerticalFov));

					float perspectiveNear = camera.GetPerspectiveNearClip();
					if (ImGui::DragFloat("Near", &perspectiveNear))
						camera.SetPerspectiveNearClip(perspectiveNear);

					float perspectiveFar = camera.GetPerspectiveFarClip();
					if (ImGui::DragFloat("Far", &perspectiveFar))
						camera.SetPerspectiveFarClip(perspectiveFar);
				}

				if (camera.GetProjectionType() == SceneCamera::ProjectionType::Orthographic)
				{
					float orthoSize = camera.GetOrthographicSize();
					if (ImGui::DragFloat("Size", &orthoSize))
						camera.SetOrthographicSize(orthoSize);

					float orthoNear = camera.GetOrthographicNearClip();
					if (ImGui::DragFloat("Near", &orthoNear))
						camera.SetOrthographicNearClip(orthoNear);

					float orthoFar = camera.GetOrthographicFarClip();
					if (ImGui::DragFloat("Far", &orthoFar))
						camera.SetOrthographicFarClip(orthoFar);

					ImGui::Checkbox("Fixed Aspect Ratio", &component.FixedAspectRatio);
				}
			});

		//Renderers
		DrawComponent<SpriteRendererComponent>("Sprite Renderer", entity, [](auto& component)
			{
				ImGui::ColorEdit4("Color", glm::value_ptr(component.Color));
				ImGui::Button("Texture");
				if (ImGui::BeginDragDropTarget()) {
					if (auto payload = ImGui::AcceptDragDropPayload("ASSET_TEXTURE")) {
						const wchar_t* path = (const wchar_t*)payload->Data;
						std::filesystem::path p = path;
						component.Texture = Texture2D::Create(p.string());
					}
					ImGui::EndDragDropTarget();
				}
				ImGui::DragFloat("Tiling Float", &component.TilingFactor, .1f, 0, 100.f);
			});
		DrawComponent<CircleRendererComponent>("Circle Renderer", entity, [](CircleRendererComponent& component) {
			ImGui::ColorEdit4("Color", glm::value_ptr(component.Color));
			ImGui::DragFloat("Thickness", &component.Thickness, .1f, .0f, 1.f);
			ImGui::DragFloat("Fade", &component.Fade, .1f, .0f, 1.f);

			});

		//Physics
		DrawComponent<Rigidbody2DComponent>("Rigidbody 2D", entity, [](auto& component)
			{
				const char* bodyTypeStrings[] = { "Static", "Dynamic","Kinematic" };
				const char* curretBodyTypeString = bodyTypeStrings[(int)component.Type];
				if (ImGui::BeginCombo("Body Type", curretBodyTypeString))
				{
					for (int i = 0; i < 3; i++)
					{
						bool isSelected = curretBodyTypeString == bodyTypeStrings[i];
						if (ImGui::Selectable(bodyTypeStrings[i], isSelected))
						{
							curretBodyTypeString = bodyTypeStrings[i];
							component.Type = (Rigidbody2DComponent::BodyType)i;
						}

						if (isSelected)
							ImGui::SetItemDefaultFocus();
					}

					ImGui::EndCombo();
				}
				ImGui::Checkbox("Fixed Rotation", &component.FixedRotation);
			});
		DrawComponent<BoxCollider2DComponent>("Box Collider 2D", entity, [](auto& component) {
			ImGui::DragFloat2("Offset", glm::value_ptr(component.Offset));
			ImGui::DragFloat2("Size", glm::value_ptr(component.Size));
			ImGui::DragFloat("Density", &component.Density, 0.01f, .0f, 1.f);
			ImGui::DragFloat("Friction", &component.Friction, 0.01f, .0f, 1.f);
			ImGui::DragFloat("Restitution", &component.Restitution, 0.01f, .0f, 1.f);
			ImGui::DragFloat("Restitution Threshold", &component.RestitutionThreshold, 0.01f, .0f, 1.f);

			});
		DrawComponent<CircleCollider2DComponent>("Box Collider 2D", entity, [](CircleCollider2DComponent& component) {
			ImGui::DragFloat2("Offset", glm::value_ptr(component.Offset),.01f);
			ImGui::DragFloat("Radius", &component.Radius, 0.01f);
			ImGui::DragFloat("Density", &component.Density, 0.01f, .0f, 1.f);
			ImGui::DragFloat("Friction", &component.Friction, 0.01f, .0f, 1.f);
			ImGui::DragFloat("Restitution", &component.Restitution, 0.01f, .0f, 1.f);
			ImGui::DragFloat("Restitution Threshold", &component.RestitutionThreshold, 0.01f, .0f, 1.f);

			});
		DrawComponent<DirectionalLightComponent>("Directional Light", entity, [](DirectionalLightComponent& component) {
			auto& light = component.Light->GetLight();
			ImGui::DragFloat3("Ambient", &light.Ambient.x,.01f,0.0f,1.0f);
			ImGui::DragFloat3("Diffuse", &light.Diffuse.x,.01f,0.0f,1.0f);
			ImGui::DragFloat3("Specular", &light.Specular.x,.01f,0.0f,1.0f);
			});
		DrawComponent<PointLightComponent>("Point Light", entity, [](PointLightComponent& component) {
			auto& light = component.Light->GetLight();
			ImGui::DragFloat3("Ambient", &light.Ambient.x, .01f, 0.0f, 1.0f);
			ImGui::DragFloat3("Diffuse", &light.Diffuse.x, .01f, 0.0f, 1.0f);
			ImGui::DragFloat3("Specular", &light.Specular.x, .01f, 0.0f, 1.0f);
			ImGui::DragFloat("Constant Coefficient", &light.ConstantCoefficient, 0.01f,0.01f);
			ImGui::DragFloat("Linear Coefficient",&light.LinearCoefficient,0.01f, 0.01f);
			ImGui::DragFloat("Quadratic Coefficient", &light.QuadraticCoefficient, 0.01f, 0.01f);

			});
		
		DrawComponent<LineRendererComponent>("Line Renderer", entity, [](LineRendererComponent& component) {
				

			ImGui::ColorEdit4("Color", &component.Color.x);
			int32_t cpy = component.Tesselation;
			if (ImGui::InputInt("Tesselation", (int*)&cpy)) {
				if (cpy >= 0) {
					component.Tesselation = cpy;
					component.RecalculateFinalPoints();
				}
			}
			uint32_t size = component.Points.size();
			uint32_t i = 0;
			static const ImVec2 padding = { 16.0f,0 };
			const ImVec2 addButtonSize = ImGui::CalcTextSize("+")+padding;
			const ImVec2 deleteButtonSize = ImGui::CalcTextSize("-")+padding;
			bool addedRemoved = false;
			std::vector<LineRendererComponent::Point> componentCopy = component.Points;
			for (auto& point : component.Points) {
				ImGui::PushID(i);
				if (ImGui::DragFloat3(std::to_string(i).c_str(),&point.Position.x,.1f)) {
					component.RecalculateFinalPoints();
				}
				ImGui::SameLine();
				if (ImGui::Button("+", addButtonSize)) {
					addedRemoved = true;
					componentCopy.insert(componentCopy.begin() + i + 1, 1, LineRendererComponent::Point{point.Position});
				}
				if (size <= 2)
					ImGui::BeginDisabled();
				ImGui::SameLine();
				if (ImGui::Button("-", deleteButtonSize)) {
					addedRemoved = true;
					componentCopy.erase(componentCopy.begin()+i,componentCopy.begin()+i+1);
				}
				if (size <= 2)
					ImGui::EndDisabled();
				++i;
				ImGui::PopID();
			}
			if (addedRemoved)
			{
				component.Points = std::move(componentCopy);
				component.RecalculateFinalPoints();
			}
			});

		DrawComponent<CubeRendererComponent>("Cube Renderer", entity, [entity, scene = m_Context](CubeRendererComponent& component) {
			});

		//Scripts
		DrawComponent<ScriptComponent>("Script", entity, [entity, scene = m_Context](ScriptComponent& component) mutable
			{
			
				auto& entityScripts = ScriptEngine::GetClasses();
				bool sceneRunning = scene->IsRunning();
				auto& entityScriptFields = ScriptEngine::GetScriptFieldMaps(entity.GetUUID());
				auto& entityScriptInstances = ScriptEngine::GetEntityScriptInstances(entity.GetUUID());

				static int64_t currentChoosingIndex = -1;
				int64_t currentIndex = 0;
				for (auto& klassName : component.ScriptNames) {
					bool scriptClassExists = ScriptEngine::ClassExists(klassName);
					if (!scriptClassExists) {
						ImGui::TextDisabled("No Script Found");
					}
					else {
						ImGui::TextDisabled(klassName.c_str());
					}
					static bool IsChoosingScript = false;
					ImGui::SameLine();
					ImGui::PushID(currentIndex);
					IsChoosingScript = !entityScripts.empty() && (ImGui::Button("##ScriptChooser", { 15,0 }) || (IsChoosingScript));
					ImGui::PopID();
					if (IsChoosingScript && currentChoosingIndex == -1)
						currentChoosingIndex = currentIndex;
					if (IsChoosingScript && currentChoosingIndex == currentIndex) {
						AssetChooserScriptItem acsi(entityScripts);
						ImGui::Begin("Please Choose A Script", &IsChoosingScript);
						if (!IsChoosingScript)
						{
							currentChoosingIndex = -1;
						}
						{
							std::string res = acsi.Draw();
							if (!res.empty()) {
								klassName = res;
								IsChoosingScript = false;
								currentChoosingIndex = -1;
							}
						}
						ImGui::End();
						
					}
					if (!scriptClassExists) {
						++currentIndex;
						continue;
					}
					ImGui::PushID(currentIndex);
					if (sceneRunning) {
						Ref<ScriptInstance> instance = entityScriptInstances.at(klassName);
						KD_CORE_ASSERT(instance);
						const auto& fields = instance->GetScriptClass()->GetFields();

						for (const auto& [name, field] : fields)
						{
							if (field.Type == ScriptFieldType::Float)
							{
								float data = instance->GetFieldValue<float>(field);
								if (ImGui::DragFloat(name.c_str(), &data))
								{
									instance->SetFieldValue<float>(field, data);
								}
							}
						}
					}
					else {
						Ref<ScriptClass> entityClass = ScriptEngine::GetEntityClass(klassName);

						const auto& fields = entityClass->GetFields();
						auto& entityFields = entityScriptFields[entityClass];
						
						for (const auto& [name, field] : fields)
						{
							// Field has been set in editor
							if (entityFields.find(name) != entityFields.end())
							{
								const ScriptFieldInstance& scriptField = entityFields.at(name);

								if (field.Type == ScriptFieldType::Float)
								{
									float data = scriptField.GetValue<float>();
									if (ImGui::DragFloat(name.c_str(), &data))
										scriptField.SetValue(data);
								}
							}
							else
							{
								if (field.Type == ScriptFieldType::Float)
								{
									float data = 0.0f;
									if (ImGui::DragFloat(name.c_str(), &data))
									{
										ScriptFieldInstance& fieldInstance = entityFields[name];
										fieldInstance.Field = field;
										fieldInstance.SetValue(data);
									}
								}
							}
						}
					}
					ImGui::PopID();
					++currentIndex;
				}
				//TODO: Add functionality for other types
				
			});
	}
}
