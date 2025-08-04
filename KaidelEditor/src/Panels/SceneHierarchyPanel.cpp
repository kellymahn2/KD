#include "SceneHierarchyPanel.h"
#include "PropertiesPanel.h"
#include "EditorContext.h"
#include "UI/UIHelper.h"
#include "Kaidel/Math/Math.h"
#include <imgui/imgui.h>
#include <imgui/imgui_internal.h>

#include <glm/gtc/type_ptr.hpp>
#include "Kaidel/Core/Timer.h"
#include "Kaidel/Scene/Components.h"
#include <cstring>
#include <sstream>
#include "Kaidel/Scripting/ScriptEngine.h"


namespace Kaidel {
	
	void SceneHierarchyPanel::OnImGuiRender(Ref<Scene> scene)
	{
		ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, { 0,0 });
		ImGui::Begin("Scene Hierarchy",nullptr,ImGuiWindowFlags_NoNavInputs);
		scene->m_Registry.each([&](auto entityID)
			{
				Entity entity{entityID , scene.Get() };
				if (entity.HasComponent<ChildComponent>())
					return;
				DrawEntityNode(entity, scene);
		});

		if (ImGui::IsMouseClicked(0) && ImGui::IsWindowHovered() && EditorContext::SelectedEntity()) {
			EditorContext::SelectedEntity({});
		}

		// Right-click on blank space
		if (ImGui::BeginPopupContextWindow(0, 1|ImGuiPopupFlags_NoOpenOverItems|ImGuiPopupFlags_NoOpenOverExistingPopup))
		{
			if (ImGui::MenuItem("Create Empty Entity"))
				scene->CreateEntity("Empty Entity");
			if (ImGui::BeginMenu("New")) {
				if (ImGui::MenuItem("Cube")) {
					scene->CreateCube("Cube");
				}
				ImGui::EndMenu();
			}

			ImGui::EndPopup();
		}

		ImGui::End();
		ImGui::PopStyleVar();
	}

	void SceneHierarchyPanel::DrawEntityNode(Entity entity, Ref<Scene> scene)
	{

		Styler styler;

		auto& tag = entity.GetComponent<TagComponent>().Tag;
		auto currContext = EditorContext::SelectedEntity();
		styler.PushColor(ImGuiCol_HeaderHovered, { 0,0,0,0 });
		styler.PushColor(ImGuiCol_HeaderActive, { 0,0,0,0 });
		styler.PushColor(ImGuiCol_Header, { 0,0,0,0 });
		static float defFont = ImGui::GetFont()->FontSize;
		if (currContext == entity) {
			ImGui::GetFont()->FontSize -= .8f;
			styler.PushColor(ImGuiCol_HeaderHovered, { .2f,.2f,.2f,1.0f });
			styler.PushColor(ImGuiCol_HeaderActive, { .2f,.2f,.2f,1.0f });
			styler.PushColor(ImGuiCol_Header, { .2f,.2f,.2f,1.0f });
		}
		else
			ImGui::GetFont()->FontSize = defFont;
		ImGuiTreeNodeFlags flags = ((currContext == entity) ? ImGuiTreeNodeFlags_Selected : 0) | ImGuiTreeNodeFlags_OpenOnDoubleClick;
		flags |= ImGuiTreeNodeFlags_SpanAvailWidth;
		bool opened = false;
		bool entityDeleted = false;
		if (entity.HasChildren()) {
			opened = ImGui::TreeNodeEx((void*)(uint64_t)entity.GetUUID(), flags, tag.c_str());

			{
				DragDropSource dragdropSrc;
				dragdropSrc.Send<IDComponent>("ENTITY_PARENT", &entity.GetComponent<IDComponent>());
			}

			{
				DragDropTarget dragdropTarget;
				if (auto payload = dragdropTarget.Receive<IDComponent>("ENTITY_PARENT");payload) {
					auto childEntityID = (*(IDComponent*)payload.Data).ID;

					entity.AddChild(childEntityID);

					auto childEntity = scene->GetEntity(childEntityID);
					if (childEntity.HasComponent<ChildComponent>()) {
						auto& cc = childEntity.GetComponent<ChildComponent>();
						auto oldParent = scene->GetEntity(cc.Parent);
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
				EditorContext::SelectedEntity(entity);
			}
			if (opened) {
				for (auto& child : entity.GetComponent<ParentComponent>().Children) {
					DrawEntityNode(scene->GetEntity(child), scene);
				}
				ImGui::TreePop();
			}
		}
		else {
			flags |= ImGuiTreeNodeFlags_Leaf| ImGuiTreeNodeFlags_NoTreePushOnOpen;
			ImGui::PushID((int)(uint64_t)entity.GetUUID());
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

					auto childEntity = scene->GetEntity(childEntityID);
					if (childEntity.HasComponent<ChildComponent>()) {
						auto& cc = childEntity.GetComponent<ChildComponent>();
						auto oldParent = scene->GetEntity(cc.Parent);
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
				EditorContext::SelectedEntity(entity);
			}
			
			ImGui::PopID();
		}
		if (currContext == entity) {
			ImGui::GetFont()->FontSize =defFont;
		}
		
		if (entityDeleted)
		{
			scene->DestroyEntity(entity);

			if (EditorContext::SelectedEntity() == entity)
			{
				EditorContext::SelectedEntity({});
			}
		}
	}
}
