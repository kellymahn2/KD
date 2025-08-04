#pragma once
#include <imgui/imgui.h>
#include <imgui/imgui_internal.h>
#include <imgui/implot.h>
#include <vector>
#include <string>

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

	private:
		bool m_Sent = false;
		bool m_Active = false;
	};

	ImVec2 operator + (const ImVec2& lhs, const ImVec2& rhs);

	uint64_t Combo(const char* name, const char* strings[], uint64_t stringCount, const char*& current);
	
	bool ButtonEx(const char* label, const ImVec2& size_arg = { 0,0 },
		ImDrawFlags drawFlags = 0, ImGuiButtonFlags flags = 0);

	bool DragScalar(
		const char* label, ImGuiDataType data_type, void* p_data, float v_speed,
		const void* p_min, const void* p_max, const char* format, ImGuiSliderFlags flags, ImDrawFlags drawFlags);

	bool DragFloat(
		const char* label, float* v, float v_speed = 1.0f,
		ImDrawFlags drawFlags = 0, float v_min = 0.0f, float v_max = 0.0f,
		const char* format = "%.3f", ImGuiSliderFlags flags = 0);

	void DrawVec3Control(const std::string& label, glm::vec3& values, float resetValue = 0.0f, float columnWidth = 100.0f);
	void DrawQuatControl(const std::string& label, glm::quat& values, float columnWidth = 100.0f);
}

