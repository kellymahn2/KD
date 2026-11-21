#pragma once
#include "Kaidel/Asset/AssetManager.h"
#include "Panels/AssetChooser.h"
#include <imgui/imgui.h>
#include <imgui/imgui_internal.h>
#include <imgui/implot.h>
#include <vector>
#include <string>
#include <functional>
#include <unordered_map>

#include <imgui/imnodes.h>
#include <imgui/imnodes_internal.h>

#include <glm/glm.hpp>
#include <glm/gtc/quaternion.hpp>


namespace Kaidel {

	class PlotStyler {
	public:
		void PushStyle(ImPlotStyleVar index, const ImVec2& val) {
			ImPlot::PushStyleVar(index, val);
			++m_StyleCount;
		}

		void PushStyle(ImPlotStyleVar index, float val) {
			ImPlot::PushStyleVar(index, val);
			++m_StyleCount;
		}


		void PushColor(ImPlotCol index, const ImVec4& col) {
			ImPlot::PushStyleColor(index, col);
			++m_ColorCount;
		}


		void PushColor(ImPlotCol index, ImU32 col) {
			ImPlot::PushStyleColor(index, col);
			++m_ColorCount;
		}


		void PopColor(uint64_t count) {
			if (count > m_ColorCount)
				return;
			ImPlot::PopStyleColor((int)count);
			m_ColorCount -= count;
		}
		void PopStyle(uint64_t count) {
			if (count > m_StyleCount)
				return;
			ImPlot::PopStyleVar((int)count);
			m_StyleCount -= count;
		}

		~PlotStyler() {
			PopStyle(m_StyleCount);
			PopColor(m_ColorCount);
		}

	private:
		uint64_t m_ColorCount = 0;
		uint64_t m_StyleCount = 0;
	};

	class Styler {
	public:
		void PushStyle(ImGuiStyleVar index, const ImVec2& val) {
			ImGui::PushStyleVar(index, val);
			++m_StyleCount;
		}

		void PushStyle(ImGuiStyleVar index, float val) {
			ImGui::PushStyleVar(index, val);
			++m_StyleCount;
		}


		void PushColor(ImGuiCol index, const ImVec4& col) {
			ImGui::PushStyleColor(index, col);
			++m_ColorCount;
		}


		void PushColor(ImGuiCol index, ImU32 col) {
			ImGui::PushStyleColor(index, col);
			++m_ColorCount;
		}
		

		void PopColor(uint64_t count) {
			if (count > m_ColorCount)
				return;
			ImGui::PopStyleColor((int)count);
			m_ColorCount -= count;
		}
		void PopStyle(uint64_t count) {
			if (count > m_StyleCount)
				return;
			ImGui::PopStyleVar((int)count);
			m_StyleCount -= count;
		}

		~Styler() {
			PopStyle(m_StyleCount);
			PopColor(m_ColorCount);
		}

	private:
		uint64_t m_ColorCount = 0;
		uint64_t m_StyleCount = 0;
	};

	template<typename T>
	struct DragDropDelivery {
		T*const Data = nullptr;
		const bool Delivered = false;

		operator bool()const { return Delivered; }
	};

	class DragDropTarget {
	public:

		DragDropTarget() {
			m_Active = ImGui::BeginDragDropTarget();
		}

		/*template<typename T>
		static void Send(const char* name,const T* obj) {
			if (ImGui::BeginDragDropSource()) {
				ImGui::SetDragDropPayload(name, obj, sizeof(T));
				ImGui::EndDragDropSource();
			}
		}*/
		template<typename T>
		DragDropDelivery<T> Receive(const char* name,ImGuiDragDropFlags flags = 0) {
			if (m_Active) {
				if (auto payload = ImGui::AcceptDragDropPayload(name,flags)) {
					return { (T*)payload->Data,true };
				}
			}
			return {};
		}

		~DragDropTarget() {
			if (m_Active)
				ImGui::EndDragDropTarget();
		}

	private:
		bool m_Active;
	};

	class DragDropSource {
	public:

		DragDropSource(ImGuiDragDropFlags flags = 0) {
			m_Active = ImGui::BeginDragDropSource(flags);
		}

		~DragDropSource() {
			if (m_Active)
				ImGui::EndDragDropSource();
		}
		template<typename T>
		void Send(const char* name,const T* obj,uint64_t count = 1) {
			if (!m_Sent && m_Active) {
				ImGui::SetDragDropPayload(name, obj, count * sizeof(T));
				m_Sent = true;
			}
		}

		template<typename T>
		void Send(const char* name, const T& obj)
		{
			if (!m_Sent && m_Active)
			{
				static T cached;
				cached = obj;
				KD_INFO("{}", (uint64_t)cached);
				ImGui::SetDragDropPayload(name, &cached, sizeof(T));
				m_Sent = true;
			}
		}

	private:
		bool m_Sent = false;
		bool m_Active = false;
	};

	uint32_t Combo(const char* name, const char* strings[], uint64_t stringCount, const char*& current);
	
	bool ButtonEx(const char* label, const ImVec2& size_arg = { 0,0 },
		ImDrawFlags drawFlags = 0, ImGuiButtonFlags flags = 0);

	bool DragScalar(
		const char* label, ImGuiDataType data_type, void* p_data, float v_speed,
		const void* p_min, const void* p_max, const char* format, ImGuiSliderFlags flags, ImDrawFlags drawFlags);

	bool DragFloat(
		const char* label, float* v, float v_speed = 1.0f,
		ImDrawFlags drawFlags = 0, float v_min = 0.0f, float v_max = 0.0f,
		const char* format = "%.3f", ImGuiSliderFlags flags = 0);

	bool DrawVec2Control(const std::string& label, glm::vec2& values, float resetValue = 0.0f, float columnWidth = 100.0f);
	bool DrawVec3Control(const std::string& label, glm::vec3& values, float resetValue = 0.0f, float columnWidth = 100.0f);
	bool DrawVec4Control(const std::string& label, glm::vec4& values, float resetValue = 0.0f, float columnWidth = 100.0f);
	bool DrawQuatControl(const std::string& label, glm::quat& values, float columnWidth = 100.0f);


	template<typename T, typename DrawItemFn>
	bool ReorderableList(const char* label, std::vector<T>& items, uint64_t minItems,
		DrawItemFn drawItem, float height = 160.0f, const T& newItem = T{},
		bool noSelected = false)
	{
		struct Payload { ImGuiID List; int Index; };
		bool changed = false;
		int moveFrom = -1, moveTo = -1;

		ImGui::PushID(label);
		ImGui::SeparatorText(label);
		ImGuiID listId = ImGui::GetID("##list");

		// The storage is the only source of truth. Read into a local and write back at the end,
		// since the storage can reallocate whenever anything else touches it.
		ImGuiStorage* storage = ImGui::GetStateStorage();
		ImGuiID selKey = ImGui::GetID("##selected");
		int sel = storage->GetInt(selKey, -1);

		if (noSelected)
			sel = -1;                                       // external reset
		else if (sel >= (int)items.size())
			sel = (int)items.size() - 1;                    // list may have shrunk elsewhere

		if (ImGui::BeginChild("##list", ImVec2(0, height), ImGuiChildFlags_Borders))
		{
			for (int i = 0; i < (int)items.size(); i++)
			{
				ImGui::PushID(i);
				ImVec2 rowStart = ImGui::GetCursorPos();

				if (ImGui::Selectable("##row", sel == i,
					ImGuiSelectableFlags_AllowOverlap, ImVec2(0, ImGui::GetFrameHeight())))
					sel = i;

				if (ImGui::BeginDragDropSource(ImGuiDragDropFlags_SourceAllowNullID))
				{
					Payload p{ listId, i };
					ImGui::SetDragDropPayload("KD_LIST_REORDER", &p, sizeof(p));
					drawItem(items[i], i);
					ImGui::EndDragDropSource();
				}

				if (ImGui::BeginDragDropTarget())
				{
					if (const ImGuiPayload* pl = ImGui::AcceptDragDropPayload("KD_LIST_REORDER"))
					{
						Payload p;
						memcpy(&p, pl->Data, sizeof(p));
						if (p.List == listId) { moveFrom = p.Index; moveTo = i; }
					}
					ImGui::EndDragDropTarget();
				}

				ImGui::SetCursorPos(rowStart);
				ImGui::TextDisabled("::");
				ImGui::SameLine();
				drawItem(items[i], i);

				ImGui::PopID();
			}
		}
		ImGui::EndChild();

		float btn = ImGui::GetFrameHeight();
		float w = btn * 2 + ImGui::GetStyle().ItemSpacing.x;
		ImGui::SetCursorPosX(ImGui::GetCursorPosX() + ImGui::GetContentRegionAvail().x - w);

		if (ImGui::Button("+", ImVec2(btn, btn)))
		{
			items.push_back(newItem);
			sel = (int)items.size() - 1;
			changed = true;
		}
		ImGui::SameLine();
		ImGui::BeginDisabled(items.size() <= minItems || sel < 0 || sel >= (int)items.size());
		if (ImGui::Button("-", ImVec2(btn, btn)))
		{
			items.erase(items.begin() + sel);
			sel = std::min(sel, (int)items.size() - 1);
			changed = true;
		}
		ImGui::EndDisabled();

		if (moveFrom >= 0 && moveTo >= 0 && moveFrom != moveTo)
		{
			T tmp = std::move(items[moveFrom]);
			items.erase(items.begin() + moveFrom);
			items.insert(items.begin() + moveTo, std::move(tmp));
			sel = moveTo;
			changed = true;
		}

		storage->SetInt(selKey, sel);

		ImGui::PopID();
		return changed;
	}

	inline int GetReorderableListSelection(const char* label)
	{
		ImGui::PushID(label);
		int sel = ImGui::GetStateStorage()->GetInt(ImGui::GetID("##selected"), -1);
		ImGui::PopID();
		return sel;
	}

	inline void HandleAssetBindingDragDrop(OnAssetChosen&, DragDropTarget&) {}

	template<typename T, typename... AssetTypes>
	void HandleAssetBindingDragDrop(OnAssetChosen& assetChosen, DragDropTarget& t)
	{
		if (auto delivery = t.Receive<uint64_t>(T::GetStaticAssetTypeString()); delivery)
		{
			assetChosen(AssetManager::GetAsset(*delivery.Data));   // *delivery.Data with the pointer version
			return;
		}

		if constexpr (sizeof...(AssetTypes) > 0)
			HandleAssetBindingDragDrop<AssetTypes...>(assetChosen, t);
	}

	template<typename... AssetTypes> 
	void AssetBinding(uint64_t id, Asset asset, OnAssetChosen&& assetChosen)
	{
		std::string name;

		if (asset)
		{
			name = AssetManager::GetAssetMetadata(asset->GetHandle()).Path.stem().string();
		}

		ImGui::PushID(id);

		ImGui::BeginDisabled();
		
		ImGui::InputText("##AssetBinding", name.data(), name.size());

		ImGui::EndDisabled();


		DragDropTarget t;

		HandleAssetBindingDragDrop<AssetTypes...>(assetChosen, t);

		ImGui::SameLine();
		if (ImGui::Button("."))
		{
			AssetChooser::Render(AssetFilter({ AssetTypes::GetStaticAssetType()... }), assetChosen);
		}

		ImGui::PopID();
	}
}

