#pragma once
#include <imgui/imgui.h>
#include <imgui/imgui_internal.h>
#include <vector>
#include <string>


namespace Kaidel {
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
			ImGui::PopStyleColor(count);
			m_ColorCount -= count;
		}
		void PopStyle(uint64_t count) {
			if (count > m_StyleCount)
				return;
			ImGui::PopStyleVar(count);
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

	uint64_t Combo(const char* name, const char* strings[], uint64_t stringCount, const char*& current);

}

