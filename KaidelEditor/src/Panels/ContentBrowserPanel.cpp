#include "ContentBrowserPanel.h"
#include <imgui/imgui.h>
#include <stack>
namespace Kaidel {
	constexpr char* s_Origin = "assets";
	const static std::filesystem::path s_AssetPath = s_Origin;
	ContentBrowserPanel::ContentBrowserPanel() 
		:m_CurrentPath(s_Origin)
	{
		m_Icons.DirectoryIcon = Texture2D::Create("Resources/Icons/ContentBrowser/DirectoryIcon.png");
		m_Icons.FileIcon = Texture2D::Create("Resources/Icons/ContentBrowser/FileIcon.png");
	}

	void ContentBrowserPanel::OnImGuiRender()
	{

		ImGui::Begin("Content Browser");


		ShowFileNavigator();
		auto relative = std::filesystem::relative(m_CurrentPath, s_AssetPath);
		static float padding = 32.0f;
		static float thumbnailSize = 50.0f;
		float cellSize = thumbnailSize + padding;
		
		float panelWidth = ImGui::GetContentRegionAvail().x;
		int columnCount = (int)(panelWidth / cellSize);
		if (columnCount < 1)
			columnCount = 1;
		ImGui::Columns(columnCount, 0, true);
		
		int i = 0;
		for (auto& directoryEntry : std::filesystem::directory_iterator(m_CurrentPath)) {
			auto path = directoryEntry.path();
			std::string filename = path.filename().string();

			auto icon = directoryEntry.is_directory() ? m_Icons.DirectoryIcon : m_Icons.FileIcon;
			ImGui::PushStyleColor(ImGuiCol_Button, { 0,0,0,0 });
			
			ImGui::ImageButton(filename.c_str(), (ImTextureID)icon->GetRendererID(), {thumbnailSize,thumbnailSize}, {0,1}, {1,0});
			if (ImGui::BeginDragDropSource()) {
				if (path.extension() == ".kaidel"|| path.extension() == ".Kaidel") {
					const wchar_t* itemPath = path.c_str();
					ImGui::SetDragDropPayload("CONTENT_BROWSER_ITEM", itemPath, (wcslen(itemPath) + 1) * sizeof(wchar_t));
				}
				if (path.extension() == ".png") {
					const wchar_t* itemPath = path.c_str();
					ImGui::SetDragDropPayload("ASSET_TEXTURE", itemPath, (wcslen(itemPath) + 1) * sizeof(wchar_t));
				}
				ImGui::EndDragDropSource();
			}

			ImGui::PopStyleColor();

			if (ImGui::IsItemHovered() && ImGui::IsMouseDoubleClicked(ImGuiMouseButton_Left)) {
				if (directoryEntry.is_directory())
					m_CurrentPath /= path.filename();
			}

			ImGui::TextWrapped(filename.c_str());
			ImGui::NextColumn();
		}
		
		ImGui::Columns(1);
		ImGui::End();
	}


	void ContentBrowserPanel::ShowFileNavigator()
	{
		auto curr = m_CurrentPath;
			std::stack<std::filesystem::path> folderPaths;

			while (curr >= s_AssetPath && !curr.empty()) {
				folderPaths.push(curr);
				curr = curr.parent_path();
			}
			int i = 0;
			static int lastHoveredIndex = -1;
			int currHoveredIndex = -1;
			auto& fontSize = ImGui::GetFont()->FontSize;
			auto defFontSize = fontSize;
			ImGui::PushStyleColor(ImGuiCol_HeaderHovered, { 0,0,0,0 });
			ImGui::PushStyleColor(ImGuiCol_HeaderActive, { 0,0,0,0 });
			ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, { 0,0 });
			while (!folderPaths.empty()) {
				if (i != 0) {
					ImGui::SameLine();
					ImGui::Text("/");
					ImGui::SameLine();
				}

				auto cstr = folderPaths.top().filename().string();
				if (i == lastHoveredIndex) {
					fontSize -= .5f;
				}
				if (ImGui::Selectable(cstr.c_str(), false, 0, ImGui::CalcTextSize(cstr.c_str()))) {
					m_CurrentPath = folderPaths.top();
				}
				if (ImGui::IsItemHovered()) {
					currHoveredIndex = i;
				}
				if (i == lastHoveredIndex)
					fontSize = defFontSize;
				folderPaths.pop();
				++i;
			}
			lastHoveredIndex = currHoveredIndex;
			ImGui::PopStyleVar();
			ImGui::PopStyleColor(2);
	}

}
