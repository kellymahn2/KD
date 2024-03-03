#include "SceneHierarchyPanel.h"
#include "PropertiesPanel.h"
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
	}
	SceneHierarchyPanel::SceneHierarchyPanel(const Ref<Scene>& context)
	{
	}


	void SceneHierarchyPanel::OnImGuiRender()
	{
		ImGui::Begin("Scene Hierarchy",nullptr,ImGuiWindowFlags_NoNavInputs);
		m_Context->Scene->m_Registry.each([&](auto entityID)
			{
				Entity entity{entityID ,m_Context->Scene.Get() };
				if (entity.HasComponent<ChildComponent>())
					return;
				DrawEntityNode(entity);
			});

		if (ImGui::IsMouseDown(0) && ImGui::IsWindowHovered() && m_Context->SelectedEntity()) {
			m_Context->Type = SelectedType::Entity;
			m_Context->_SelectedEntity = {};
		}

		// Right-click on blank space
		if (ImGui::BeginPopupContextWindow(0, 1|ImGuiPopupFlags_NoOpenOverItems|ImGuiPopupFlags_NoOpenOverExistingPopup))
		{
			if (ImGui::MenuItem("Create Empty Entity"))
				m_Context->Scene->CreateEntity("Empty Entity");
			if (ImGui::BeginMenu("New")) {
				if (ImGui::MenuItem("Cube")) {
					m_Context->Scene->CreateCube("Cube");
				}
				ImGui::EndMenu();
			}

			ImGui::EndPopup();
		}

		ImGui::End();
		
	}

	void SceneHierarchyPanel::DrawEntityNode(Entity entity)
	{
		auto& tag = entity.GetComponent<TagComponent>().Tag;
		auto currContext = m_Context->SelectedEntity();
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
		ImGuiTreeNodeFlags flags = ((currContext == entity) ? ImGuiTreeNodeFlags_Selected : 0) | ImGuiTreeNodeFlags_OpenOnDoubleClick;
		flags |= ImGuiTreeNodeFlags_SpanAvailWidth;
		bool opened = false;
		bool entityDeleted = false;
		if (entity.HasChildren()) {
			opened = ImGui::TreeNodeEx((void*)(uint64_t)entity.GetUUID(), flags, tag.c_str());
			if (ImGui::BeginDragDropSource()) {
				ImGui::SetDragDropPayload("ENTITY_PARENT", &entity.GetComponent<IDComponent>(), sizeof(IDComponent));
				ImGui::EndDragDropSource();
			}
			if (ImGui::BeginDragDropTarget()) {
				if (auto payload = ImGui::AcceptDragDropPayload("ENTITY_PARENT")) {
					auto childEntityID = (*(IDComponent*)payload->Data).ID;

					entity.AddChild(childEntityID);

					auto childEntity = m_Context->Scene->GetEntity(childEntityID);
					if (childEntity.HasComponent<ChildComponent>()) {
						auto& cc = childEntity.GetComponent<ChildComponent>();
						auto oldParent = m_Context->Scene->GetEntity(cc.Parent);
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
				m_Context->Type = SelectedType::Entity;
				m_Context->_SelectedEntity = entity;
			}
			if (opened) {
				SCOPED_TIMER(Scene Hierarchy);
				for (auto& child : entity.GetComponent<ParentComponent>().Children) {
					DrawEntityNode(m_Context->Scene->GetEntity(child));
				}
				ImGui::TreePop();
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

					auto childEntity = m_Context->Scene->GetEntity(childEntityID);
					if (childEntity.HasComponent<ChildComponent>()) {
						auto& cc = childEntity.GetComponent<ChildComponent>();
						auto oldParent = m_Context->Scene->GetEntity(cc.Parent);
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
				m_Context->Type = SelectedType::Entity;
				m_Context->_SelectedEntity = entity;
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
			m_Context->Scene->DestroyEntity(entity);
			if (m_Context->SelectedEntity() == entity)
			{
				m_Context->Type = SelectedType::Entity;
				m_Context->_SelectedEntity = {};
			}
		}
	}
	
	
}
