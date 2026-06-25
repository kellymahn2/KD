#include "ContentBrowserPanel.h"
#include "Kaidel/Core/UsedFileExtensions.h"
#include "UI/UIHelper.h"
#include "Kaidel/Project/Project.h"
#include "UI/IconLibrary.h"
#include "UI/AssetPreview.h"
#include "Kaidel/Asset/AssetManager.h"
#include "Kaidel/Core/DebugUtils.h"
#include <imgui/imgui.h>
#include <glm/gtx/compatibility.hpp>

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
			if (extension == Extensions::Scene)
				return "CONTENT_BROWSER_ITEM";
				if (extension == ".png")
					return "ASSET_TEXTURE";
				if (extension == Extensions::Material)
					return "MAT";
				return nullptr;
		}

		static std::string CreateNewFileName(const std::string& path, const std::string& baseName, const std::string& extension) {
			uint32_t i = 0;
			while (i < 100) {
				std::string name = baseName + (i ? std::to_string(i) : "") + extension;
				FileSystem::directory_entry de = FileSystem::directory_entry(path + "/" + name);
				if (!de.exists())
					return name;
				++i;
			}
			return "";
		}

	}

	template<typename T>
	static void HandleDrag(DragDropSource& dragdropSource, const Path& path, const std::string& extension, const char* dragDropName, const T& data) {
		if (path.extension().string() == extension) {
			dragdropSource.Send(dragDropName, &data);
		}
	}

	static bool ShouldSkip(const Path& path)
	{
		return AssetManager::GetSupportedAssets().find(path.extension().string()) == 
			AssetManager::GetSupportedAssets().end();
	}

	static struct ContentBrowserPanelData {
		std::filesystem::path CurrentPath;
		std::vector<std::string> CurrentPathSplit;
		char m_SelectedFileName[100] = { 0 };
		bool m_WasNew = false;
	} s_Data;

	static int editingIndex = -1;

	static void GoToPath(const std::string& fileName)
	{
		s_Data.CurrentPath /= fileName;
		s_Data.CurrentPathSplit.push_back(fileName);
		editingIndex = -1;
	}

	
	constexpr static float padding = 32.0f;
	constexpr static float minThumbnailSize = 60.0f;
	constexpr static float maxThumbnailSize = 100.0f;


	static float s_Zoom = 0.2f;

	void ContentBrowserPanel::OnImGuiRender()
	{

		

		ImGui::Begin("Content Browser");

		auto regionAvail = ImGui::GetContentRegionAvail();

		ImGui::BeginChild("##DirectoryContents", ImVec2(0, -regionAvail.y * 0.1f), ImGuiChildFlags_AutoResizeY);
		RenderCurrentDirectory();
		ImGui::EndChild();

		ImGui::Separator();
		
		ImGui::PushStyleVar(ImGuiStyleVar_FrameRounding, 12.0f);
		ImGui::PushStyleVar(ImGuiStyleVar_GrabMinSize, 20.0f);
		ImGui::PushStyleVar(ImGuiStyleVar_GrabRounding, 12.0f);

		ImGui::SetNextItemWidth(140.0f);
		ImGui::SetCursorPosX(ImGui::GetContentRegionAvail().x - ImGui::CalcItemWidth());

		ImGui::SliderFloat("##Zoom", &s_Zoom, 0.0f, 1.0f, "");

		ImGui::PopStyleVar(3);
		ImGui::End();
	}

	void ContentBrowserPanel::RenderCurrentDirectory()
	{
		const auto& startPath = Project::GetProjectDirectory();

		if (s_Data.CurrentPath.empty())
		{
			s_Data.CurrentPath = startPath;
		}

		static bool f = false;

		if (!f)
		{
			GoToPath("Assets");
			GoToPath("Gun");
			f = true;
		}

		if (startPath.empty())
		{
			ImGui::End();
			return;
		}

		ShowFileNavigator();


		if (s_Zoom != 0.0f)
		{
			float panelWidth = ImGui::GetContentRegionAvail().x;

			const float thumbnailSize = glm::lerp(minThumbnailSize, maxThumbnailSize, s_Zoom);

			const float cellSize = thumbnailSize + padding;

			int columnCount = (int)(panelWidth / cellSize);
			if (columnCount < 1)
				columnCount = 1;
			ImGui::Columns(columnCount, 0, false);
		}

		int i = 0;

		for (auto& directoryEntry : std::filesystem::directory_iterator(s_Data.CurrentPath)) {
			if (!directoryEntry.is_directory() && ShouldSkip(directoryEntry.path()))
				continue;
			if (s_Zoom != 0.0f)
			{
				RenderThumbnail(directoryEntry, i);
			}
			else
			{
				RenderListItem(directoryEntry, i);
			}
			++i;
		}

			

		ImGui::Columns(1);
		if (ImGui::BeginPopupContextWindow("##ContentBrowser", ImGuiPopupFlags_MouseButtonRight | ImGuiPopupFlags_NoOpenOverItems)) 
		{
			if (ImGui::BeginMenu("New")) 
			{

				ImGui::EndMenu();
			}

			ImGui::EndPopup();
		}
	}
	

	

	void ContentBrowserPanel::RenderThumbnail(const FileSystem::directory_entry& directoryEntry, uint32_t i) {
		auto& path = directoryEntry.path();

		std::string filename = path.filename().string();
		ImGui::PushID(i);
		bool isDirectory = directoryEntry.is_directory();

		Ref<DescriptorSet> iconSet;
		ImVec2 bottomLeft(0, 0), topRight(1, 1);

		if (isDirectory)
		{
			uint32_t icon = IconLibrary::GetIcon("ContentBrowser/DirectoryIcon.png");
			auto& coords = IconLibrary::GetAtlas()->GetSamplingCoords(icon);
			bottomLeft = ImVec2(coords.BottomLeft.x, coords.BottomLeft.y);
			topRight = ImVec2(coords.TopRight.x, coords.TopRight.y);

			iconSet = IconLibrary::GetDescriptorSet();
		}
		else
		{
			Asset asset = AssetManager::GetAssetAtPath(path);
			
			Ref<Texture2D> texture;
			
			if (asset)
			{
				texture = AssetPreview::GetPreview(asset);
			}

			if (texture)
			{
				DescriptorSetLayoutSpecification specs({ {DescriptorType::SamplerWithTexture, ShaderStage_FragmentShader} });

				iconSet = DescriptorSet::Create(specs);

				iconSet->Update(texture, GLOBAL_SAMPLER_LINEAR_MIPPED, ImageLayout::ShaderReadOnlyOptimal, 0);
			}
			else
			{
				uint32_t icon = IconLibrary::GetIcon("ContentBrowser/FileIcon.png");
				auto& coords = IconLibrary::GetAtlas()->GetSamplingCoords(icon);
				bottomLeft = ImVec2(coords.BottomLeft.x, coords.BottomLeft.y);
				topRight = ImVec2(coords.TopRight.x, coords.TopRight.y);

				iconSet = IconLibrary::GetDescriptorSet();
			}
		}

		ImGui::PushStyleColor(ImGuiCol_Button, { 0,0,0,0 });

		const ImGuiButtonFlags folderFlags = ImGuiButtonFlags_PressedOnDoubleClick;
		const ImGuiButtonFlags fileFlags = 0;

		const ImGuiButtonFlags flags = isDirectory ? folderFlags : fileFlags;

		const float thumbnailSize = glm::lerp(minThumbnailSize, maxThumbnailSize, s_Zoom);

		if (ImGui::ImageButtonEx(
			i + 1, (ImTextureID)iconSet->GetSetID(),
			ImVec2(thumbnailSize, thumbnailSize), bottomLeft, topRight,
			ImVec4(0, 0, 0, 0), ImVec4(1, 1, 1, 1), flags))
		{
			if (isDirectory)
			{
				GoToPath(filename);
			}
			else
			{
				EditorContext::SelectedAsset(AssetManager::GetAssetAtPath(path));
			}
		}
		//if (ImGui::ImageButton((ImTextureID)icon->GetRendererID(), { thumbnailSize,thumbnailSize }, { 0,1 }, { 1,0 })) {
		//	SelectedType type = Utils::SelectedTypeFromExtension(Utils::Lower(path.extension().string()));
		//	if (type == SelectedType::Asset) {
		//		if (asset) {
		//			m_Context->_SelectedAsset = asset;
		//			m_Context->Type = SelectedType::Asset;
		//		}
		//		//Asset doesn't exist
		//		else {
		//		}
		//	}
		//}

		// Drag drops
		//{
		//	DragDropSource dragdropSource(0);

		//	const char* type = Utils::DragDropNameFromExtension(Utils::Lower(path.extension().string()));
		//	/*if (type) {
		//		const wchar_t* itemPath = path.c_str();
		//		dragdropSource.Send<wchar_t>(type, itemPath, wcslen(itemPath) + 1);
		//	}*/
		//}

		//// Mouse clicking
		//{
		//	if (ImGui::IsItemHovered()) {
		//		if (ImGui::IsMouseDoubleClicked(ImGuiMouseButton_Left)) {
		//			// Folder
		//			if (directoryEntry.is_directory()) {
		//				m_CurrentPath /= path.filename();
		//				editingIndex = -1;
		//				strcpy(m_SelectedFileName, "");
		//			}
		//			// File
		//		}
		//		else if (ImGui::IsItemClicked(ImGuiMouseButton_Left) && !ImGui::IsMouseDragging(ImGuiMouseButton_Left)) {
		//			std::cout << "Hello\n";
		//			if (!directoryEntry.is_directory()) {
		//				SelectedType type = Utils::SelectedTypeFromExtension(Utils::Lower(path.extension().string()));
		//				if (type == SelectedType::Asset) {
		//					if (asset) {
		//						m_Context->_SelectedAsset = asset;
		//						m_Context->Type = SelectedType::Asset;
		//					}
		//					//Asset doesn't exist
		//					else {

		//					}

		//				}
		//			}
		//		}

		//	}
		//}
		//

		ImGui::PopStyleColor();
		//Name editing
		{
			if (editingIndex == i || (editingIndex == -1 && filename == s_Data.m_SelectedFileName)) {
				if (ImGui::InputText("##", s_Data.m_SelectedFileName, 24)) {
					editingIndex = i;
				}
				if (ImGui::IsItemDeactivated()) {
					if (std::filesystem::exists(s_Data.CurrentPath / s_Data.m_SelectedFileName))
						KD_WARN("Tried To Rename to {} When It Already Exists", s_Data.m_SelectedFileName);
					else {
						if (s_Data.m_WasNew) {
							s_Data.m_WasNew = false;
						}
						else {
							std::filesystem::rename(s_Data.CurrentPath / filename, s_Data.CurrentPath / s_Data.m_SelectedFileName);
						}
					}
					strcpy(s_Data.m_SelectedFileName, "");
					editingIndex = -1;
				}
			}
			else {
				ImGui::TextWrapped(filename.c_str());
				if (ImGui::IsItemHovered() && ImGui::IsMouseDoubleClicked(ImGuiMouseButton_Left)) {
					strcpy(s_Data.m_SelectedFileName, filename.c_str());
				}
			}
		}
		

		ImGui::NextColumn();
		ImGui::PopID();
	}


	void ContentBrowserPanel::RenderListItem(const FileSystem::directory_entry& directoryEntry, uint32_t i)
	{
		ImGui::PushID(i);

		ImGui::BeginGroup();

		float cursor = ImGui::GetCursorPos().x;
		auto lineHeight = ImGui::GetTextLineHeight();

		float factor = 1.4f;

		ImGui::Dummy(ImVec2(ImGui::GetContentRegionAvail().x, lineHeight * factor));
		
		ImGui::SameLine();
		ImGui::SetCursorPosX(cursor);

		ImGui::Selectable("##Selectable", false, 0, ImVec2(0, lineHeight * factor));
		
		bool isDirectory = directoryEntry.is_directory();
		uint32_t icon = isDirectory ? IconLibrary::GetIcon("ContentBrowser/DirectoryIcon.png") : IconLibrary::GetIcon("ContentBrowser/FileIcon.png");

		auto& coords = IconLibrary::GetAtlas()->GetSamplingCoords(icon);

		ImGui::SameLine();
		ImGui::Image((ImTextureRef)IconLibrary::GetDescriptorSet()->GetSetID(), 
			ImVec2(lineHeight * factor, lineHeight * factor), (ImVec2&)coords.BottomLeft, (ImVec2&)coords.TopRight);
		
		std::string filename = directoryEntry.path().filename().string();

		ImGui::SameLine();

		ImGui::SetCursorPosY(ImGui::GetCursorPosY() + lineHeight * (factor - 1.0f) * 0.5f);
		ImGui::Text(filename.c_str());

		ImGui::EndGroup();

		if (ImGui::IsItemHovered() && ImGui::IsMouseDoubleClicked(ImGuiMouseButton_Left))
		{
			GoToPath(filename);
		}

		ImGui::PopID();
	}

	static int lastHoveredIndex = -1;

	static bool FileNavigatorSingle(uint32_t i, int& currHovered)
	{
		if (i != 0)
		{
			ImGui::SameLine();
			ImGui::Text(" / ");
			ImGui::SameLine();
		}

		const char* str = i == 0 ? "root" : s_Data.CurrentPathSplit[i - 1].c_str();
		
		if (ImGui::Selectable(str, false, 0, ImGui::CalcTextSize(str))) 
		{
			if (i == 0)
			{
				s_Data.CurrentPath = Project::GetProjectDirectory();
				s_Data.CurrentPathSplit.clear();
			}
			else
			{
				while (s_Data.CurrentPathSplit.size() != i)
				{
					s_Data.CurrentPathSplit.pop_back();
					s_Data.CurrentPath = s_Data.CurrentPath.parent_path();
				}
			}
			
			lastHoveredIndex = -1;
			return true;
		}
		if (i == lastHoveredIndex) 
		{
			const ImVec2 cursorPos = ImGui::GetItemRectMin();
			const ImVec2 size = ImGui::GetItemRectSize();

			const ImVec2 linePos = ImVec2(cursorPos.x, cursorPos.y + size.y);

			ImGui::GetWindowDrawList()->AddLine(linePos, ImVec2(linePos.x + size.x, linePos.y), IM_COL32(255, 255, 255, 255));
		}
		if (ImGui::IsItemHovered()) 
		{
			currHovered = i;
		}
		return false;
	}

	void ContentBrowserPanel::ShowFileNavigator()
	{
		int currHovered = -1;
		ImGui::PushStyleColor(ImGuiCol_HeaderHovered, { 0,0,0,0 });
		ImGui::PushStyleColor(ImGuiCol_HeaderActive, { 0,0,0,0 });
		ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, { 0,0 });

		if (!FileNavigatorSingle(0, currHovered))
		{
			for (uint32_t i = 0; i < s_Data.CurrentPathSplit.size(); ++i)
			{
				if (FileNavigatorSingle(i + 1, currHovered))
				{
					break;
				}
			}
		}

		lastHoveredIndex = currHovered;

		ImGui::PopStyleVar();
		ImGui::PopStyleColor(2);
	}
}
