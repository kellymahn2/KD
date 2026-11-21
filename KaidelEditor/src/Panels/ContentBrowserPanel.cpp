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
#include <imgui/misc/cpp/imgui_stdlib.h>

#include "Kaidel/Renderer/Sprite.h"

#include <stack>
namespace Kaidel {

	namespace Utils {
		static std::string Lower(const std::string& string) {
			std::string s = string;
			for (auto& c : s)
				c = std::tolower(c);
			return s;
		}

		//static const char* DragDropNameFromExtension(const std::string& extension) {
		//	if (extension == Extensions::Scene)
		//		return "CONTENT_BROWSER_ITEM";
		//		if (extension == ".png")
		//			return "ASSET_TEXTURE";
		//		if (extension == Extensions::Material)
		//			return "MAT";
		//		return nullptr;
		//}

		static void MakeNonExistentPath(Path& path)
		{
			//make new path from it if the one we have already exists.

			std::string filename = path.stem().string();

			std::string extension = path.extension().string();

			uint32_t i = 1;
			while (FileSystem::exists(path))
			{
				path.replace_filename(filename + fmt::format(" ({})", i++) + extension);
			}
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

	struct EditContext {
		std::string OriginalFilename;
		std::string EditedFilename;
		std::string OriginalFileExtension;
		std::string NewFilepath;
		int EditingIndex = -1;
		bool JustStartedEditing = false;


		EditContext() = default;

		EditContext(const std::string& originalFilename, const std::string& originalFileExtension, int editingIndex)
		{
			EditingIndex = editingIndex;
			EditedFilename = originalFilename;
			OriginalFilename = originalFilename;
			OriginalFileExtension = originalFileExtension;
			JustStartedEditing = true;
		}

		EditContext(const std::string& newFilename)
		{
			NewFilepath = newFilename;
		}
	};

	static struct ContentBrowserPanelData {
		std::filesystem::path CurrentPath;
		std::vector<std::string> CurrentPathSplit;

		EditContext EditingContext;
	} s_Data;
	

	static void GoToPath(const std::string& fileName)
	{
		s_Data.CurrentPath /= fileName;
		s_Data.CurrentPathSplit.push_back(fileName);
	}
	
	constexpr static float padding = 16.0f;
	constexpr static float minThumbnailSize = 100.0f;
	constexpr static float maxThumbnailSize = 140.0f;

	static float s_Zoom = 0.2f;

	void ContentBrowserPanel::OnImGuiRender()
	{
		ImGui::Begin("Content Browser");

		const float footerHeight = ImGui::GetFrameHeightWithSpacing();

		ImGui::BeginChild("##DirectoryContents",
			ImVec2(0, -footerHeight),
			ImGuiChildFlags_None);
		RenderCurrentDirectory();
		ImGui::EndChild();

		// Footer
		{
			Styler styler;

			ImGui::Separator();
			styler.PushStyle(ImGuiStyleVar_FrameRounding, 12.0f);
			styler.PushStyle(ImGuiStyleVar_GrabMinSize, 20.0f);
			styler.PushStyle(ImGuiStyleVar_GrabRounding, 12.0f);
			styler.PushStyle(ImGuiStyleVar_FramePadding, ImVec2(0, 0));

			ImGui::SetNextItemWidth(140.0f);
			ImGui::SetCursorPosX(ImGui::GetContentRegionAvail().x - ImGui::CalcItemWidth());
			ImGui::SliderFloat("##Zoom", &s_Zoom, 0.0f, 1.0f, "");
		}

		ImGui::End();
	}

	void ContentBrowserPanel::RenderCurrentDirectory()
	{
		const auto& startPath = Project::GetProjectDirectory();

		if (s_Data.CurrentPath.empty())
		{
			s_Data.CurrentPath = startPath;
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

		auto directoryIterator = std::filesystem::directory_iterator(s_Data.CurrentPath);

		std::vector<FileSystem::directory_entry> entries;

		for (auto& entry : directoryIterator)
			entries.push_back(entry);

		std::sort(entries.begin(), entries.end(),
			[](const FileSystem::directory_entry& a, const FileSystem::directory_entry& b) {
				bool aDir = a.is_directory();
				bool bDir = b.is_directory();
				if (aDir != bDir) return aDir;          // directories before files
				return a.path().stem() > b.path().stem();  // then by name
			});


		for (auto& directoryEntry : entries) {
			if (!directoryEntry.is_directory() && ShouldSkip(directoryEntry.path()))
				continue;

			if (directoryEntry.path() == s_Data.EditingContext.NewFilepath)
			{
				s_Data.EditingContext = EditContext(directoryEntry.path().stem().string(), directoryEntry.path().extension().string(), i);
			}

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

		Styler styler;

		styler.PushStyle(ImGuiStyleVar_PopupRounding, 12.0f);
		styler.PushStyle(ImGuiStyleVar_WindowPadding, ImVec2(14, 14));

		

		if (ImGui::BeginPopupContextWindow("##ContentBrowser", ImGuiPopupFlags_MouseButtonRight | ImGuiPopupFlags_NoOpenOverItems)) 
		{
			if (ImGui::BeginMenu("Create")) 
			{
				if (ImGui::MenuItem("Folder"))
				{
					Path createPath = s_Data.CurrentPath / "Untitled";
					Utils::MakeNonExistentPath(createPath);

					FileSystem::create_directory(createPath);

					s_Data.EditingContext = EditContext(createPath.string());
				}

				if (ImGui::BeginMenu("Materials"))
				{
					if(ImGui::MenuItem("Standard"))
					{
						StandardMaterial::Initializer init;
						init.Metalness = 0.5f;
						init.Roughness = 0.6f;

						init.BaseColor = glm::vec4(0.5f, 0.8f, 1.0f, 1.0f);

						Ref<StandardMaterial> mat = CreateRef<StandardMaterial>(init);

						Path createPath = s_Data.CurrentPath / "Untitled.mat";

						Utils::MakeNonExistentPath(createPath);

						AssetMetadata mtd;
						mtd.Path = createPath;
						mtd.Type = AssetType::StandardMaterial;

						AssetManager::RegisterAsset(mat, mtd);

						AssetManager::Serialize(mtd.Path, Asset(mat), true);

						s_Data.EditingContext = EditContext(mtd.Path.string());
					}

					if (ImGui::MenuItem("Shader"))
					{
						Ref<ShaderMaterial> mat = CreateRef<ShaderMaterial>(Ref<VisualShader>(nullptr));
						
						Path createPath = s_Data.CurrentPath / "Untitled.smat";

						Utils::MakeNonExistentPath(createPath);

						AssetMetadata mtd;
						mtd.Path = createPath;
						mtd.Type = AssetType::ShaderMaterial;

						AssetManager::RegisterAsset(mat, mtd);

						AssetManager::Serialize(mtd.Path, Asset(mat), true);

						s_Data.EditingContext = EditContext(mtd.Path.string());
					}

					ImGui::EndMenu();
				}

				if (ImGui::BeginMenu("Animations"))
				{
					if (ImGui::MenuItem("Animation"))
					{
						Ref<AnimationTree> tree = CreateRef<AnimationTree>();

						Path createPath = s_Data.CurrentPath / "Untitled.anim";

						Utils::MakeNonExistentPath(createPath);

						AssetMetadata mtd;
						mtd.Path = createPath;
						mtd.Type = AssetType::Animation;

						AssetManager::RegisterAsset(tree, mtd);

						AssetManager::Serialize(mtd.Path, Asset(tree), true);

						s_Data.EditingContext = EditContext(mtd.Path.string());
					}

					ImGui::EndMenu();
				}

				if (ImGui::BeginMenu("Shaders"))
				{
					if (ImGui::MenuItem("Visual"))
					{
						Ref<VisualShader> mat = CreateRef<VisualShader>();

						Path createPath = s_Data.CurrentPath / "Untitled.vshader";

						Utils::MakeNonExistentPath(createPath);

						AssetMetadata mtd;
						mtd.Path = createPath;
						mtd.Type = AssetType::VisualShader;

						AssetManager::RegisterAsset(mat, mtd);

						AssetManager::Serialize(mtd.Path, Asset(mat), true);

						s_Data.EditingContext = EditContext(mtd.Path.string());
					}

					ImGui::EndMenu();
				}

				ImGui::EndMenu();
			}

			ImGui::EndPopup();
		}
	}

	Kaidel::Ref<Kaidel::DescriptorSet> GetIconForAsset(Asset asset, ImVec2& bottomLeft, ImVec2& topRight)
	{
		Ref<DescriptorSet> iconSet;

		Ref<Texture2D> texture;

		if (asset)
		{
			texture = AssetPreview::GetPreview(asset);
		}

		if (texture)
		{
			DescriptorSetLayoutSpecification specs({ {DescriptorType::SamplerWithTexture, ShaderStage_FragmentShader} });

			iconSet = DescriptorSet::Create(specs);
			
			Ref<Sampler> sampler = GLOBAL_SAMPLER_LINEAR_MIPPED;

			if (asset->GetAssetType() == AssetType::SpriteSheet)
			{
				sampler = asset.As<SpriteSheet>()->GetSampler();
			}

			iconSet->Update(texture, sampler, ImageLayout::ShaderReadOnlyOptimal, 0);
		}
		else
		{
			uint32_t icon = IconLibrary::GetIcon("ContentBrowser/FileIcon.png");
			auto& coords = IconLibrary::GetAtlas()->GetSamplingCoords(icon);
			bottomLeft = ImVec2(coords.BottomLeft.x, coords.BottomLeft.y);
			topRight = ImVec2(coords.TopRight.x, coords.TopRight.y);

			iconSet = IconLibrary::GetDescriptorSet();
		}

		return iconSet;
	}

	Kaidel::Ref<Kaidel::DescriptorSet> GetIconForPath(const Path& path, bool isDirectory, ImVec2& bottomLeft, ImVec2& topRight)
	{
		Ref<DescriptorSet> iconSet;

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

			iconSet = GetIconForAsset(asset, bottomLeft, topRight);
		}

		return iconSet;
	}

	static void DrawFilenameForThumbnail(const std::string& filename, const std::string& extension, uint32_t i, const ImVec2& textSize)
	{
		Styler styler;
		styler.PushColor(ImGuiCol_Header, 0);
		styler.PushColor(ImGuiCol_HeaderHovered, 0);
		styler.PushColor(ImGuiCol_HeaderActive, 0);

		ImGui::Selectable(filename.c_str(), false, 0, textSize);

		if (ImGui::IsItemHovered() && ImGui::IsMouseDoubleClicked(ImGuiMouseButton_Left))
		{
			s_Data.EditingContext = EditContext(filename, extension, i);
		}
	}

	static void SaveNameEditFile(const Path& oldPath, const Path& newPath)
	{
		AssetHandle asset = AssetManager::GetAssetHandleAtPath(oldPath);

		if (asset != 0)
		{
			AssetManager::ResetAssetPath(asset, oldPath, newPath);
		}
		else
		{
			FileSystem::rename(oldPath, newPath);
		}
	}

	static void SaveNameEdit()
	{
		if (s_Data.EditingContext.EditedFilename.empty())
		{
			return;
		}

		Path oldPath = s_Data.CurrentPath / (s_Data.EditingContext.OriginalFilename + s_Data.EditingContext.OriginalFileExtension);
		
		Path newPath = s_Data.CurrentPath / (s_Data.EditingContext.EditedFilename + s_Data.EditingContext.OriginalFileExtension);
		if (oldPath == newPath)
		{
			return;
		}

		Utils::MakeNonExistentPath(newPath);

		if (FileSystem::is_directory(oldPath))
		{
			//TODO: maybe rethink directories? maybe make it a tree for the reverse registry?
			for (auto& directoryEntry : FileSystem::recursive_directory_iterator(oldPath))
			{
				if (!directoryEntry.is_directory())
				{
					Path relative = FileSystem::relative(directoryEntry.path(), oldPath);

					Path newSubPath = newPath / relative;

					AssetHandle asset = AssetManager::GetAssetHandleAtPath(directoryEntry.path());

					if (asset != 0)
					{
						AssetManager::ResetAssetParentPath(asset, directoryEntry.path(), newSubPath);
					}
				}
			}

			FileSystem::rename(oldPath, newPath);
		}
		else
		{
			SaveNameEditFile(oldPath, newPath);
		}
	}

	static void RemoveAsset(const Path& path)
	{
		AssetManager::RemoveAssetAtPath(path);

		Path mtdPath = path;

		mtdPath.replace_extension(mtdPath.extension().string() + ".mtd");

		Project::ConsumePathByFileWatcher(path, 1);

		FileSystem::remove(mtdPath);
		FileSystem::remove(path);
	}

	static void RemoveDirectory(const Path& path)
	{
		int hasFiles = 0;
		for (auto& entry : FileSystem::recursive_directory_iterator(path))
		{
			if (!entry.is_directory() && AssetManager::GetTypeFromExtension(entry.path().extension().string()) != AssetType::None)
			{
				AssetManager::RemoveAssetAtPath(entry.path());
				Project::ConsumePathByFileWatcher(entry.path(), 1);
				hasFiles = 1;
			}
		}

		Project::ConsumePathByFileWatcher(path, 1 + hasFiles);

		FileSystem::remove_all(path);
	}

	void ContentBrowserPanel::RenderThumbnail(const FileSystem::directory_entry& directoryEntry, uint32_t i) {
		auto& path = directoryEntry.path();

		std::string filename = path.stem().string();
		ImGui::PushID(i);
		bool isDirectory = directoryEntry.is_directory();

		Ref<DescriptorSet> iconSet;
		ImVec2 bottomLeft(0, 0), topRight(1, 1);

		iconSet = GetIconForPath(path, isDirectory, bottomLeft, topRight);

		ImGui::PushStyleColor(ImGuiCol_Button, { 0,0,0,0 });

		const ImGuiButtonFlags folderFlags = ImGuiButtonFlags_PressedOnDoubleClick;
		const ImGuiButtonFlags fileFlags = 0;

		const ImGuiButtonFlags flags = isDirectory ? folderFlags : fileFlags;

		const float thumbnailSize = glm::lerp(minThumbnailSize, maxThumbnailSize, s_Zoom);

		ImGui::BeginGroup();

		Asset asset;
		if (!isDirectory)
		{
			asset = AssetManager::GetAssetAtPath(path);
		}

		{
			float iconWidth = iconSet->GetTextureAtBinding(0)->GetTextureSpecification().Width;
			float iconHeight = iconSet->GetTextureAtBinding(0)->GetTextureSpecification().Height;

			if (iconWidth > thumbnailSize || iconHeight > thumbnailSize)
			{
				const float scale = glm::min(thumbnailSize / iconWidth, thumbnailSize / iconHeight);
				iconWidth *= scale;
				iconHeight *= scale;
			}

			const ImVec2 cursor = ImGui::GetCursorPos();
			ImGui::SetCursorPos(ImVec2(cursor.x + (thumbnailSize - iconWidth) * 0.5f,
				cursor.y + (thumbnailSize - iconHeight) * 0.5f));
			ImGui::SetNextItemAllowOverlap();
			if (ImGui::ImageButtonEx(
				i + 1, (ImTextureID)iconSet->GetSetID(),
				ImVec2(iconWidth, iconHeight), bottomLeft, topRight,
				ImVec4(0, 0, 0, 0), ImVec4(1, 1, 1, 1), flags))
			{
				if (isDirectory)
				{
					GoToPath(filename);
				}
				else
				{
					EditorContext::SelectedAsset(asset);
				}
			}
			ImGui::SetCursorPos(ImVec2(cursor.x, cursor.y + thumbnailSize));
		}

		if (ImGui::BeginPopupContextItem("##ItemPopupContext", ImGuiPopupFlags_MouseButtonRight))
		{
			if (ImGui::MenuItem("Delete"))
			{
				if (isDirectory)
				{
					RemoveDirectory(path);
				}
				else
				{
					if (EditorContext::SelectedAsset() == asset)
					{
						EditorContext::SelectedAsset({});
					}
					RemoveAsset(path);
				}

			}

			ImGui::EndPopup();
		}

		uint32_t sCount = asset ? asset->GetSubAssetCount() : 0;

		bool toggledSubAssets = false;

		if (sCount)
		{
			ImVec2 imgMin = ImGui::GetItemRectMin();
			ImVec2 imgMax = ImGui::GetItemRectMax();

			ImVec2 buttonCenter = ImVec2(imgMax.x, (imgMin.y + imgMax.y) * 0.5f);

			const float overlayExtent = 30.0f;

			ImVec2 savedCursor = ImGui::GetCursorScreenPos();

			ImGui::SetCursorScreenPos(buttonCenter - ImVec2(overlayExtent, 0.5f * overlayExtent));

			auto storage = ImGui::GetStateStorage();

			const ImGuiID id = ImGui::GetID("OverlayToggle");

			toggledSubAssets = storage->GetBool(id, false);

			if (ImGui::ArrowButtonEx("##OverlayButton", toggledSubAssets ? ImGuiDir_Left : ImGuiDir_Right, ImVec2(overlayExtent, overlayExtent)))
			{
				toggledSubAssets = !toggledSubAssets;

				storage->SetBool(id, toggledSubAssets);
			}

			ImGui::SetCursorScreenPos(savedCursor);
		}
		
		if (s_Data.EditingContext.EditingIndex == i)
		{
			ImGuiInputTextFlags flags =
				ImGuiInputTextFlags_ElideLeft |
				ImGuiInputTextFlags_CallbackCompletion |
				ImGuiInputTextFlags_AutoSelectAll | 
				ImGuiInputTextFlags_AlwaysOverwrite;

			if (s_Data.EditingContext.JustStartedEditing)
			{
				ImGui::SetKeyboardFocusHere();
				s_Data.EditingContext.JustStartedEditing = false;
			}

			ImGui::SetNextItemWidth(thumbnailSize);

			ImGui::SetCursorPosX(ImGui::GetCursorPosX() + ImGui::GetStyle().FramePadding.x);

			Styler styler;
			styler.PushStyle(ImGuiStyleVar_FramePadding, ImVec2(0, 0));

			styler.PushColor(ImGuiCol_FrameBg, 0);

			ImGui::InputText("##FilenameInput", &s_Data.EditingContext.EditedFilename, flags);


			if (ImGui::IsItemDeactivated())
			{
				s_Data.EditingContext.EditingIndex = -1;
				SaveNameEdit();
			}
		}
		else
		{
			ImVec2 textSize = ImGui::CalcTextSize(filename.c_str());

			float filenameWidth = textSize.x;

			if (filenameWidth > thumbnailSize)
			{
				filenameWidth = thumbnailSize;
			}

			float cursorPos = ImGui::GetCursorPosX() + (thumbnailSize - filenameWidth) * 0.5f + ImGui::GetStyle().FramePadding.x;

			ImGui::SetCursorPosX(cursorPos);

			DrawFilenameForThumbnail(filename, path.has_extension() ? path.extension().string() : "", i, textSize);
		}
		ImGui::EndGroup();

		if (asset)
		{
			DragDropSource(ImGuiDragDropFlags_SourceAllowNullID).Send(asset->GetAssetTypeString(), (uint64_t)asset->GetHandle());
		}
		
		if (toggledSubAssets)
		{
			const float subAssetThumbnailSize = thumbnailSize;

			ImGui::NextColumn();

			if (asset->GetAssetType() == AssetType::SpriteSheet)
			{
				Ref<SpriteSheet> sheet = asset.As<SpriteSheet>();

				DescriptorSetLayoutSpecification specs({ { DescriptorType::SamplerWithTexture, PipelineStages_FragmentShader } });

				auto set = DescriptorSet::Create(specs);

				set->Update(sheet->GetTexture(), sheet->GetSampler(), ImageLayout::ShaderReadOnlyOptimal, 0);

				for (uint32_t s = 0; s < sCount; ++s)
				{
					ImGui::PushID(s);

					auto& sprite = sheet->GetSprites()[s];

					float iconWidth = sprite.Width;
					float iconHeight = sprite.Height;

					if (iconWidth > subAssetThumbnailSize || iconHeight > subAssetThumbnailSize)
					{
						const float scale = glm::min(subAssetThumbnailSize / iconWidth, subAssetThumbnailSize / iconHeight);
						iconWidth *= scale;
						iconHeight *= scale;
					}

					const ImVec2 cursor = ImGui::GetCursorPos();
					ImGui::SetCursorPos(ImVec2(cursor.x + (subAssetThumbnailSize - iconWidth) * 0.5f,
						cursor.y + (subAssetThumbnailSize - iconHeight) * 0.5f));

					ImGui::ImageWithBg(
						set->GetSetID(), 
						ImVec2(iconWidth, iconHeight),
						ImVec2(sprite.UV0.x, sprite.UV1.y), ImVec2(sprite.UV1.x, sprite.UV0.y),
						ImVec4(0.5f, 0.5f, 0.5f, 1.0f));

					ImVec2 textSize = ImGui::CalcTextSize(sprite.Name.c_str());

					if (textSize.x > subAssetThumbnailSize)
					{
						textSize.x = subAssetThumbnailSize;
					}

					ImVec2 cursorPos = ImGui::GetCursorScreenPos() + ImVec2((subAssetThumbnailSize - textSize.x) * 0.5f + ImGui::GetStyle().FramePadding.x, 0.0f);
					cursorPos.y += (subAssetThumbnailSize - iconHeight) * 0.5f;

					ImGui::SetCursorScreenPos(cursorPos);
					{
						Styler styler;
						styler.PushColor(ImGuiCol_Header, 0);
						styler.PushColor(ImGuiCol_HeaderHovered, 0);
						styler.PushColor(ImGuiCol_HeaderActive, 0);


						ImGui::Selectable(sprite.Name.c_str(), false, 0, textSize);
					}

					ImGui::PopID();

					if (s != sCount - 1)
					{
						ImGui::NextColumn();
					}
				}
			}

			else
			{

			}

			//for (uint32_t s = 0; s < sCount; ++s)
			//{
			//	ImGui::PushID(s);
			//	SubAsset subAsset;
			//	asset->GetSubAssetAt(s, subAsset);
			//	
			//	auto assetType = subAsset.Asset->GetAssetType();
			//
			//	if (assetType == AssetType::SpriteSheet)
			//	{
			//		Ref<SpriteSheet> sheet = asset.As<SpriteSheet>();
			//	}
			//	else
			//	{
			//		//GetIconForPath()
			//	}
			//
			//	ImGui::PopID();
			//}

		}


		// Drag drops
		//{
		//	DragDropSource dragdropSource(0);

		//	const char* type = Utils::DragDropNameFromExtension(Utils::Lower(path.extension().string()));
		//	/*if (type) {
		//		const wchar_t* itemPath = path.c_str();
		//		dragdropSource.Send<wchar_t>(type, itemPath, wcslen(itemPath) + 1);
		//	}*/
		//}

		ImGui::PopStyleColor();

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
