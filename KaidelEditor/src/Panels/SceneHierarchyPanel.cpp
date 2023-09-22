#include "SceneHierarchyPanel.h"

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
		if (ImGui::BeginPopupContextWindow(0, 1|ImGuiPopupFlags_NoOpenOverItems))
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
		bool popped = false;
		static float defFont = ImGui::GetFont()->FontSize;
		if (currContext == entity) {
			ImGui::GetFont()->FontSize -= .8f;
		}
		else
			ImGui::GetFont()->FontSize = defFont;
		ImGuiTreeNodeFlags flags = ((m_SelectionContext == entity) ? ImGuiTreeNodeFlags_Selected : 0) | ImGuiTreeNodeFlags_OpenOnDoubleClick;
		flags |= ImGuiTreeNodeFlags_SpanAvailWidth;
		bool opened = false;
		if (entity.HasComponent<ParentComponent>()&&!entity.GetComponent<ParentComponent>().Children.empty()) {
			opened = ImGui::TreeNodeEx((void*)(uint64_t)entity.GetUUID(), flags, tag.c_str());
			if (ImGui::BeginDragDropSource()) {
				ImGui::SetDragDropPayload("ENTITY_PARENT", &entity.GetComponent<IDComponent>(), sizeof(IDComponent));
				ImGui::EndDragDropSource();
			}
			if (ImGui::BeginDragDropTarget()) {
				if (auto payload = ImGui::AcceptDragDropPayload("ENTITY_PARENT")) {
					auto childEntityID= (*(IDComponent*)payload->Data).ID;
					auto childEntity = m_Context->GetEntity(childEntityID);

					if ((childEntity.HasChildren()&& childEntity.IsParentOf(entity.GetUUID()))||entity.IsParentOf(childEntityID)){
						if(opened)
							ImGui::TreePop();
						ImGui::PopStyleColor(3);
						ImGui::GetFont()->FontSize = defFont;
						return;
					}
					entity.AddChild(childEntityID);

					if (childEntity.HasComponent<ChildComponent>()) {
						auto& cc = childEntity.GetComponent<ChildComponent>();
						auto oldParent = m_Context->GetEntity(cc.Parent);
						auto& oldpc = oldParent.GetComponent<ParentComponent>();
						auto it = std::find(oldpc.Children.begin(), oldpc.Children.end(), childEntityID);
						if (it != oldpc.Children.end())
							oldpc.Children.erase(it, it + 1);
						cc.Parent = entity.GetUUID();
					}
					else {
						childEntity.AddComponent<ChildComponent>(entity.GetUUID());
					}
				}
				ImGui::EndDragDropTarget();
			}
			if (ImGui::IsItemClicked())
			{
				m_SelectionContext = entity;
			}
			if (opened) {
				for (auto& child : entity.GetComponent<ParentComponent>().Children) {
					DrawEntityNode(m_Context->GetEntity(child));
				}
				ImGui::TreePop();
			}
			ImGui::GetFont()->FontSize = defFont;
		}
		else {
			flags |= ImGuiTreeNodeFlags_Leaf| ImGuiTreeNodeFlags_NoTreePushOnOpen;
			ImGui::PushID((uint64_t)entity.GetUUID());
			opened = ImGui::TreeNodeEx(tag.c_str(),flags);
			if (ImGui::BeginDragDropSource()) {
				ImGui::SetDragDropPayload("ENTITY_PARENT", &entity.GetComponent<IDComponent>(), sizeof(uint64_t));
				ImGui::EndDragDropSource();
			}
			if (ImGui::BeginDragDropTarget()) {
				if (auto payload = ImGui::AcceptDragDropPayload("ENTITY_PARENT")) {
					auto childEntityID= (*(IDComponent*)payload->Data).ID;
					auto childEntity = m_Context->GetEntity(childEntityID);

					if ((childEntity.HasChildren() && childEntity.IsParentOf(entity.GetUUID()))) {
						if (opened)
							ImGui::TreePop();
						ImGui::PopStyleColor(3);
						ImGui::GetFont()->FontSize = defFont;
						return;
					}

					entity.AddChild(childEntityID);

					if (childEntity.HasComponent<ChildComponent>()) {
						auto& cc = childEntity.GetComponent<ChildComponent>();
						auto oldParent = m_Context->GetEntity(cc.Parent);
						auto& oldpc = oldParent.GetComponent<ParentComponent>();
						auto it = std::find(oldpc.Children.begin(), oldpc.Children.end(), childEntityID);
						if (it != oldpc.Children.end())
							oldpc.Children.erase(it, it + 1);
						cc.Parent = entity.GetUUID();

					}
					else {
						childEntity.AddComponent<ChildComponent>(entity.GetUUID());
					}
				}
				ImGui::EndDragDropTarget();
			}
			if (ImGui::IsItemClicked()) {
				m_SelectionContext = entity;
			}
			
			ImGui::PopID();
		}
		if (currContext == entity)
			ImGui::GetFont()->FontSize =defFont;
		ImGui::PopStyleColor(3);
		bool entityDeleted = false;
		if (ImGui::BeginPopupContextItem())
		{
			if (ImGui::MenuItem("Delete Entity"))
				entityDeleted = true;
			if (ImGui::MenuItem("Remove Parent", nullptr, nullptr, entity.HasParent())) {
				//TODO: Remove Parent
			}
			ImGui::EndPopup();
		}

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

			//TODO: Remove Parent
			m_Context->DestroyEntity(entity);
			if (m_SelectionContext == entity)
				m_SelectionContext = {};
		}
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
		ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, ImVec2{ 0, 0 });

		float lineHeight = GImGui->Font->FontSize + GImGui->Style.FramePadding.y * 2.0f;
		ImVec2 buttonSize = { lineHeight + 3.0f, lineHeight };

		ImGui::PushStyleColor(ImGuiCol_Button, ImVec4{ 0.8f, 0.1f, 0.15f, 1.0f });
		ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4{ 0.9f, 0.2f, 0.2f, 1.0f });
		ImGui::PushStyleColor(ImGuiCol_ButtonActive, ImVec4{ 0.8f, 0.1f, 0.15f, 1.0f });
		ImGui::PushFont(boldFont);
		if (ImGui::Button("X", buttonSize))
			values.x = resetValue;
		ImGui::PopFont();
		ImGui::PopStyleColor(3);

		ImGui::SameLine();
		ImGui::DragFloat("##X", &values.x, 0.1f, 0.0f, 0.0f, "%.2f");
		ImGui::PopItemWidth();
		ImGui::SameLine();

		ImGui::PushStyleColor(ImGuiCol_Button, ImVec4{ 0.2f, 0.7f, 0.2f, 1.0f });
		ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4{ 0.3f, 0.8f, 0.3f, 1.0f });
		ImGui::PushStyleColor(ImGuiCol_ButtonActive, ImVec4{ 0.2f, 0.7f, 0.2f, 1.0f });
		ImGui::PushFont(boldFont);
		if (ImGui::Button("Y", buttonSize))
			values.y = resetValue;
		ImGui::PopFont();
		ImGui::PopStyleColor(3);

		ImGui::SameLine();
		ImGui::DragFloat("##Y", &values.y, 0.1f, 0.0f, 0.0f, "%.2f");
		ImGui::PopItemWidth();
		ImGui::SameLine();

		ImGui::PushStyleColor(ImGuiCol_Button, ImVec4{ 0.1f, 0.25f, 0.8f, 1.0f });
		ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4{ 0.2f, 0.35f, 0.9f, 1.0f });
		ImGui::PushStyleColor(ImGuiCol_ButtonActive, ImVec4{ 0.1f, 0.25f, 0.8f, 1.0f });
		ImGui::PushFont(boldFont);
		if (ImGui::Button("Z", buttonSize))
			values.z = resetValue;
		ImGui::PopFont();
		ImGui::PopStyleColor(3);

		ImGui::SameLine();
		ImGui::DragFloat("##Z", &values.z, 0.1f, 0.0f, 0.0f, "%.2f");
		ImGui::PopItemWidth();

		ImGui::PopStyleVar();

		ImGui::Columns(1);

		ImGui::PopID();
	}

	template<typename T, typename UIFunction>
	static void DrawComponent(const std::string& name, Entity entity, UIFunction uiFunction)
	{
		const ImGuiTreeNodeFlags treeNodeFlags = ImGuiTreeNodeFlags_DefaultOpen | ImGuiTreeNodeFlags_Framed | ImGuiTreeNodeFlags_SpanAvailWidth | ImGuiTreeNodeFlags_AllowItemOverlap | ImGuiTreeNodeFlags_FramePadding;
		if (entity.HasComponent<T>())
		{
			auto& component = entity.GetComponent<T>();
			ImVec2 contentRegionAvailable = ImGui::GetContentRegionAvail();

			ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, ImVec2{ 4, 4 });
			float lineHeight = GImGui->Font->FontSize + GImGui->Style.FramePadding.y * 2.0f;
			ImGui::Separator();
			bool open = ImGui::TreeNodeEx((void*)typeid(T).hash_code(), treeNodeFlags, name.c_str());
			ImGui::PopStyleVar(
			);
			ImGui::SameLine(contentRegionAvailable.x - lineHeight * 0.5f);
			if (ImGui::Button("+", ImVec2{ lineHeight, lineHeight }))
			{
				ImGui::OpenPopup("ComponentSettings");
			}

			bool removeComponent = false;
			if (ImGui::BeginPopup("ComponentSettings"))
			{
				if (ImGui::MenuItem("Remove component"))
					removeComponent = true;
				
				ImGui::EndPopup();
			}

			if (open)
			{
				uiFunction(component);
				ImGui::TreePop();
			}

			if (removeComponent)
				entity.RemoveComponent<T>();
		}
	}
	template<typename T>
	static void DrawAddComponentItems(Entity& entity,const std::string& text) {
		if (!entity.HasComponent<T>()) {
			if (ImGui::MenuItem(text.c_str())) {
				entity.AddComponent<T>();
				ImGui::CloseCurrentPopup();
			}
		}
	}
	class AssetChooserScriptItem {
	public:
		AssetChooserScriptItem() = default;
		AssetChooserScriptItem(const std::unordered_map<std::string,Ref<ScriptClass>>& map) {
			Timer timer("Construction");
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
					if (ImGui::TreeNode(name.c_str())) {
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
			DrawAddComponentItems<ScriptComponent>(m_SelectionContext, "Script Component");
			DrawAddComponentItems<CameraComponent>(m_SelectionContext, "Camera");
			DrawAddComponentItems<SpriteRendererComponent>(m_SelectionContext, "Sprite Renderer");
			DrawAddComponentItems<CircleRendererComponent>(m_SelectionContext, "Circle Renderer");
			DrawAddComponentItems<Rigidbody2DComponent>(m_SelectionContext, "Rigidbody 2D");
			DrawAddComponentItems<BoxCollider2DComponent>(m_SelectionContext, "Box Collider 2D");
			DrawAddComponentItems<CircleCollider2DComponent>(m_SelectionContext, "Circle Collider 2D");
			DrawAddComponentItems<ParentComponent>(m_SelectionContext, "Parent");
			DrawAddComponentItems<ChildComponent>(m_SelectionContext, "Child");
			ImGui::EndPopup();
		}

		ImGui::PopItemWidth();

		DrawComponent<TransformComponent>("Transform", entity, [](auto& component)
			{
				DrawVec3Control("Translation", component.Translation);
				glm::vec3 rotation = glm::degrees(component.Rotation);
				DrawVec3Control("Rotation", rotation);
				component.Rotation = glm::radians(rotation);
				DrawVec3Control("Scale", component.Scale, 1.0f);
			});

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

		//Scripts
		DrawComponent<ScriptComponent>("Script", entity, [entity, scene = m_Context](auto& component) mutable
			{
				bool scriptClassExists = ScriptEngine::ClassExists(component.Name);
				if (!scriptClassExists)
					ImGui::PushStyleColor(ImGuiCol_TextDisabled, ImVec4(0.9f, 0.2f, 0.3f, 1.0f));

				/*static char buffer[64];
				strcpy_s(buffer, sizeof(buffer), component.Name.c_str());


				if (ImGui::InputText("Class", buffer, sizeof(buffer)))
					component.Name = buffer;*/
				auto& entityScripts = ScriptEngine::GetClasses();
				static bool IsChoosingScript = false;
				ImGui::TextDisabled(component.Name.c_str());
				ImGui::SameLine();
				IsChoosingScript = !entityScripts.empty() && (ImGui::Button("##ScriptChooser", { 15,0 }) || IsChoosingScript);
				if (IsChoosingScript) {
					AssetChooserScriptItem acsi(entityScripts);
					ImGui::Begin("Please Choose A Script", &IsChoosingScript);
					{
						std::string res = acsi.Draw();
						if (!res.empty()) {
							component.Name = res;
							IsChoosingScript = false;
						}
					}
					ImGui::End();
				}




				// Fields
				bool sceneRunning = scene->IsRunning();
				if (sceneRunning)
				{
					Ref<ScriptInstance> scriptInstance = ScriptEngine::GetEntityScriptInstance(entity.GetUUID());
					if (scriptInstance)
					{
						const auto& fields = scriptInstance->GetScriptClass()->GetFields();
						for (const auto& [name, field] : fields)
						{
							if (field.Type == ScriptFieldType::Float)
							{
								float data = scriptInstance->GetFieldValue<float>(field);
								if (ImGui::DragFloat(name.c_str(), &data))
								{
									scriptInstance->SetFieldValue<float>(field, data);
								}
							}
						}
					}
				}
				else
				{
					if (scriptClassExists)
					{
						Ref<ScriptClass> entityClass = ScriptEngine::GetEntityClass(component.Name);
						const auto& fields = entityClass->GetFields();

						auto& entityFields = ScriptEngine::GetScriptFieldMap(entity.GetUUID());
						for (const auto& [name, field] : fields)
						{
							// Field has been set in editor
							if (entityFields.find(name) != entityFields.end())
							{
								const ScriptFieldInstance& scriptField = entityFields.at(name);

								// Display control to set it maybe
								if (field.Type == ScriptFieldType::Float)
								{
									float data = scriptField.GetValue<float>();
									if (ImGui::DragFloat(name.c_str(), &data))
										scriptField.SetValue(data);
								}
							}
							else
							{
								// Display control to set it maybe
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
				}

				if (!scriptClassExists)
					ImGui::PopStyleColor();
			});
	}
}
