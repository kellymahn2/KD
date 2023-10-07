#include "ConsolePanel.h"
#include <imgui/imgui.h>

namespace Kaidel {


	ConsolePanel::ConsolePanel()
	{
		m_Icons.ErrorIcon = Texture2D::Create("Resources/Icons/ConsolePanel/ErrorIcon.png");
		m_Icons.WarnIcon = Texture2D::Create("Resources/Icons/ConsolePanel/WarnIcon.png");
		m_Icons.InfoIcon = Texture2D::Create("Resources/Icons/ConsolePanel/InfoIcon.png");

	}
	static int GetFilters(int filter) {
		if (ImGui::Button("Filters", { 50,0 })) {
			ImGui::OpenPopup("##Filters");
		}
		if (ImGui::BeginPopup("##Filters")) {
			/*bool all = ImGui::MenuItem("All");
			if (all) {
				ImGui::EndPopup();
				return ConsolePanel::Filter_All;
			}
			bool log = ImGui::MenuItem("Log");
			bool info = ImGui::MenuItem("Info");
			bool warn = ImGui::MenuItem("Warn");
			bool error = ImGui::MenuItem("Error");
			int ret = 0;

			ret |= (log * ConsolePanel::Filter_Log);
			ret |= (info * ConsolePanel::Filter_Info);
			ret |= (warn * ConsolePanel::Filter_Warn);
			ret |= (error * ConsolePanel::Filter_Error);
			return ret;
			ImGui::EndPopup();*/
			bool all = filter == ConsolePanel::Filter_All;
			if(all)
				ImGui::PushStyleColor(ImGuiCol_Text, { .2f,.8f,.5f,1 });
			if (ImGui::MenuItem("All")) {
				ImGui::EndPopup();
				if (all) {
					ImGui::PopStyleColor();
					return 0;
				}
				return ConsolePanel::Filter_All;
			}
			if(all)
				ImGui::PopStyleColor();

			bool log = !all && filter & ConsolePanel::Filter_Log;
			if (log)
				ImGui::PushStyleColor(ImGuiCol_Text, { .2f,.8f,.5f,1 });
			if (ImGui::MenuItem("Log")) {
				ImGui::EndPopup();
				if (log) {
					ImGui::PopStyleColor();
					return filter & (~ConsolePanel::Filter_Log);
				}
				return (filter*!all) | ConsolePanel::Filter_Log;
			}
			if (log)
				ImGui::PopStyleColor();


			bool info = !all && filter & ConsolePanel::Filter_Info;
			if (info)
				ImGui::PushStyleColor(ImGuiCol_Text, { .2f,.8f,.5f,1 });
			if (ImGui::MenuItem("Info")) {
				ImGui::EndPopup();
				if (info) {
					ImGui::PopStyleColor();
					return filter & (~ConsolePanel::Filter_Info);
				}
				return (filter * !all) | ConsolePanel::Filter_Info;
			}
			if (info)
				ImGui::PopStyleColor();


			bool warn = !all && filter & ConsolePanel::Filter_Warn;
			if (warn)
				ImGui::PushStyleColor(ImGuiCol_Text, { .2f,.8f,.5f,1 });
			if (ImGui::MenuItem("Warn")) {
				ImGui::EndPopup();
				if (warn) {
					ImGui::PopStyleColor();
					return filter & (~ConsolePanel::Filter_Warn);
				}
				return (filter * !all) | ConsolePanel::Filter_Warn;
			}
			if (warn)
				ImGui::PopStyleColor();

			bool error = !all && filter & ConsolePanel::Filter_Error;
			if (error)
				ImGui::PushStyleColor(ImGuiCol_Text, { .2f,.8f,.5f,1 });
			if (ImGui::MenuItem("Error")) {
				ImGui::EndPopup();
				if (error) {
					ImGui::PopStyleColor();
					return filter & (~ConsolePanel::Filter_Error);
				}
				return (filter * !all) | ConsolePanel::Filter_Error;
			}
			if (error)
				ImGui::PopStyleColor();
			ImGui::EndPopup();
		}
		return filter;
	}
	void ConsolePanel::OnImGuiRender()
	{
		static float maxScroll=0.0f;
		ImGui::SetNextWindowScroll({ 0,maxScroll });
		ImGui::Begin("Console",nullptr, ImGuiWindowFlags_MenuBar);
		if (!m_ConsoleContext)
			ImGui::BeginDisabled();
	
		if (ImGui::BeginMenuBar()) {
			float defX = ImGui::GetCursorPosX();
			ImGui::SetCursorPosX(ImGui::GetWindowWidth() - 50.0f);
			if (ImGui::Button("Clear", { 50,0 })) {
				m_ConsoleContext->Clear();
			}
			ImGui::SetCursorPosX(ImGui::GetWindowWidth() - 102.0f);
			m_CurrentFilter = GetFilters(m_CurrentFilter);




			ImGui::EndMenuBar();
		}
		
		

		if (!m_ConsoleContext)
			ImGui::EndDisabled();

		if (m_ConsoleContext) {

			for (auto& message : m_ConsoleContext->GetMessages()) {
				ImVec4 messageColor{ 1,1,1,1 };
				Ref<Texture2D> icon;
				if (message.Level == MessageLevel::Log && (m_CurrentFilter & Filter_Log) == 0)
					continue;
				switch (message.Level)
				{
				case MessageLevel::Info:
				{
					if ((m_CurrentFilter & Filter_Info) == 0)
						continue;
					messageColor = { .24f,.71f,.78f,1.0f };
					icon = m_Icons.InfoIcon;
				}
				break;
				case MessageLevel::Warn:
				{
					if ((m_CurrentFilter & Filter_Warn) == 0)
						continue;
					messageColor = { .79f,.78f,.32f,1.0f };
					icon = m_Icons.WarnIcon;
				}
				break;
				case MessageLevel::Error:
				{
					if ((m_CurrentFilter & Filter_Error) == 0)
						continue;
					messageColor = { .65f,.31f,.29f,1.0f };
					icon = m_Icons.ErrorIcon;
				}
				break;
				default:
					break;				}
				std::time_t time = std::chrono::system_clock::to_time_t(message.Time);
				std::tm tm = *std::localtime(&time);
				char buf[80] = { 0 };
				std::strftime(buf, sizeof(buf), "%d/%m/%Y-%H:%M:%S", &tm);
				if (icon) {
					ImGui::Image((ImTextureID)icon->GetRendererID(), { (float)icon->GetWidth(),(float)icon->GetHeight() },
						{ 0,1 }, { 1,0 });
					ImGui::SameLine();
				}
				ImGui::PushStyleColor(ImGuiCol_Text, messageColor);
				ImGui::TextWrapped("Engine [%s] : %s", buf, message.Text.c_str());
				ImGui::PopStyleColor();
			}
		}
		maxScroll = ImGui::GetScrollMaxY();
		ImGui::End();
	}

}
