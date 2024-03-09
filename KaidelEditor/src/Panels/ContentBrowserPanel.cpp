#include "ContentBrowserPanel.h"
#include "Kaidel/Renderer/3D/MaterialSerializer.h"
#include "UI/UIHelper.h"
#include "Kaidel/Project/Project.h"
#include <imgui/imgui.h>
#include <stack>
namespace Kaidel {

	namespace Utils {
		static std::string Lower(const std::string& string) {
			std::string s = string;
			for (auto& c : s)
				c = std::tolower(c);
			return s;
		}

		static const char* DragDropNameFromExtension(const std::string& extension) {
			if (extension == ".kaidel")
				return "CONTENT_BROWSER_ITEM";
			if (extension == ".png")
				return "ASSET_TEXTURE";
			if(extension == ".mat")
			return nullptr;
		}

		static SelectedType SelectedTypeFromExtension(const std::string& extension) {
			if (extension == ".mat")
				return SelectedType::Asset;
			if (extension == ".tex")
				return SelectedType::Asset;
			return SelectedType::None;
		}

		static std::string CreateNewFileName(const std::string& path,const std::string& baseName, const std::string& extension) {
			uint32_t i = 0;
			while (i<100) {
				std::string name = baseName + (i ? std::to_string(i) : "") + extension;
				FileSystem::directory_entry de = FileSystem::directory_entry(path +"/"+ name);
				if (!de.exists())
					return name;
				++i;
			}
			return "";
		}
	}

	ContentBrowserPanel::ContentBrowserPanel() 
	{
		m_Icons.DirectoryIcon = Texture2D::Create("Resources/Icons/ContentBrowser/DirectoryIcon.png");
		m_Icons.FileIcon = Texture2D::Create("Resources/Icons/ContentBrowser/FileIcon.png");
	}

	void ContentBrowserPanel::OnImGuiRender()
	{

		ImGui::Begin("Content Browser");

		if (m_CurrentPath.empty() || m_StartPath.empty()) {
			ImGui::End();
			return;
		}
		ShowFileNavigator();


		auto relative = std::filesystem::relative(m_CurrentPath, m_StartPath);
		constexpr static float padding = 32.0f;
		constexpr static float thumbnailSize = 50.0f;
		constexpr static float cellSize = thumbnailSize + padding;
		
		float panelWidth = ImGui::GetContentRegionAvail().x;
		int columnCount = (int)(panelWidth / cellSize);
		if (columnCount < 1)
			columnCount = 1;
		ImGui::Columns(columnCount, 0, false);
		int i = 0;

		for (auto& directoryEntry : std::filesystem::directory_iterator(m_CurrentPath)) {
			auto& path = directoryEntry.path();
			std::string filename = path.filename().string();
			static int editingIndex = -1;
			ImGui::PushID(i);
			auto& icon = directoryEntry.is_directory() ? m_Icons.DirectoryIcon : m_Icons.FileIcon;

			ImGui::PushStyleColor(ImGuiCol_Button, { 0,0,0,0 });
			
			ImGui::Image((ImTextureID)icon->GetRendererID(), {thumbnailSize,thumbnailSize}, {0,1}, {1,0});

			if (ImGui::IsItemHovered()) {
				if (ImGui::IsMouseDoubleClicked(ImGuiMouseButton_Left)) {
					// Folder
					if (directoryEntry.is_directory()) {
						m_CurrentPath /= path.filename();
						editingIndex = -1;
						strcpy(m_SelectedFileName, "");
					}
					// File
				}
				if (ImGui::IsMouseClicked(ImGuiMouseButton_Left)) {
					if (!directoryEntry.is_directory()) {

						SelectedType type = Utils::SelectedTypeFromExtension(Utils::Lower(path.extension().string()));
						if (type == SelectedType::Asset) {
							m_Context->Type = SelectedType::Asset;
							m_Context->_SelectedAsset = AssetManager::AssetsByPath(path);
						}
					}
				}
				
			}

			{
				DragDropSource dragdropSource(ImGuiDragDropFlags_SourceAllowNullID);

				const char* type = Utils::DragDropNameFromExtension(Utils::Lower(path.extension().string()));
				if (type) {
					const wchar_t* itemPath = path.c_str();
					dragdropSource.Send<wchar_t>(type, itemPath, wcslen(itemPath) + 1);
				}

			}


			ImGui::PopStyleColor();
			if (editingIndex==i||(editingIndex==-1&&filename == m_SelectedFileName)) {
				if (ImGui::InputText("##", m_SelectedFileName, 24)) {
					editingIndex = i;
				}
				if (ImGui::IsItemDeactivated()) {
					if (std::filesystem::exists(m_CurrentPath / m_SelectedFileName))
						KD_WARN("Tried To Rename to {} When It Already Exists", m_SelectedFileName);
					else {
						if (m_WasNew) {
							m_WasNew = false;
						}
						else {
							std::filesystem::rename(m_CurrentPath / filename, m_CurrentPath / m_SelectedFileName);
						}
					}
					strcpy(m_SelectedFileName, "");
					editingIndex = -1;
				}
			}
			else {
				ImGui::TextWrapped(filename.c_str());
				if (ImGui::IsItemHovered() && ImGui::IsMouseDoubleClicked(ImGuiMouseButton_Left)) {
					strcpy(m_SelectedFileName, filename.c_str());
				}
			}

			ImGui::NextColumn();
			ImGui::PopID();
			++i;
		}
		
		ImGui::Columns(1);
		if (ImGui::BeginPopupContextWindow("##ContentBrowser", ImGuiPopupFlags_MouseButtonRight | ImGuiPopupFlags_NoOpenOverItems)) {
			if (ImGui::BeginMenu("New")) {
				if (ImGui::MenuItem("Script")) {
					auto x = m_CurrentPath/ Utils::CreateNewFileName(m_CurrentPath.string(), "Script", ".cs");
					std::ofstream file(x);
				}
				if (ImGui::MenuItem("Material")) {
					std::string fileName = Utils::CreateNewFileName(m_CurrentPath.string(), "Material", ".mat");
					auto x = m_CurrentPath / fileName;
					Ref<Material> mat = CreateRef<Material>();
					mat->Name(fileName);
					MaterialSerializer serializer(mat);
					serializer.Serialize(x);
					mat->Path(x);
					AssetManager::Manage(mat);
				}
				ImGui::EndMenu();
			}

			ImGui::EndPopup();
		}
		ImGui::End();
	}


	void ContentBrowserPanel::ShowFileNavigator()
	{
		auto curr = m_CurrentPath;
			std::stack<std::filesystem::path> folderPaths;

			while (curr >= m_StartPath && !curr.empty()) {
				folderPaths.push(curr);
				curr = curr.parent_path();
			}
			int i = 0;
			static int lastHoveredIndex = -1;
			int currHoveredIndex = -1;
			ImGui::PushStyleColor(ImGuiCol_HeaderHovered, { 0,0,0,0 });
			ImGui::PushStyleColor(ImGuiCol_HeaderActive, { 0,0,0,0 });
			ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, { 0,0 });


			while (!folderPaths.empty()) {
				if (i != 0) {
					ImGui::SameLine();
					ImGui::Text(" / ");
					ImGui::SameLine();
				}

				auto cstr = folderPaths.top().filename().string();
				if (ImGui::Selectable(cstr.c_str(), false, 0, ImGui::CalcTextSize(cstr.c_str()))) {
					m_CurrentPath = folderPaths.top();
				}
				if (i == lastHoveredIndex) {
					const ImVec2 cursorPos = ImGui::GetItemRectMin();
					const ImVec2 size = ImGui::GetItemRectSize();

					const ImVec2 linePos = ImVec2(cursorPos.x, cursorPos.y + size.y);

					ImGui::GetWindowDrawList()->AddLine(linePos, ImVec2(linePos.x + size.x, linePos.y), IM_COL32(255, 255, 255, 255));
				}
				if (ImGui::IsItemHovered()) {
					currHoveredIndex = i;
				}
				folderPaths.pop();
				++i;
			}
			lastHoveredIndex = currHoveredIndex;
			ImGui::PopStyleVar();
			ImGui::PopStyleColor(2);
	}



}
