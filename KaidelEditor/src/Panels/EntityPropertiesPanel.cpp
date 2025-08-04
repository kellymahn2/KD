#include "EntityPropertiesPanel.h"
#include "Kaidel/Scripting/ScriptEngine.h"
#include "Kaidel/Project/Project.h"
#include "UI/UIHelper.h"
#include <imgui/imgui.h>
#include <imgui/imgui_internal.h>
#include <imgui/misc/cpp/imgui_stdlib.h>
#include <glm/glm.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <inttypes.h>
namespace Kaidel {

	template<typename T, typename UIFunction>
	static void DrawComponent(const std::string& name, Entity entity, UIFunction uiFunction, bool removeable = true)
	{
		static std::unordered_map<std::string, bool> s_Map;
		const ImGuiTreeNodeFlags treeNodeFlags = ImGuiTreeNodeFlags_DefaultOpen | ImGuiTreeNodeFlags_OpenOnDoubleClick |
			ImGuiTreeNodeFlags_Framed | ImGuiTreeNodeFlags_SpanAvailWidth | ImGuiTreeNodeFlags_AllowItemOverlap | ImGuiTreeNodeFlags_FramePadding | ImGuiTreeNodeFlags_Leaf;
		if (entity.HasComponent<T>())
		{
			ImVec2 contentRegionAvailable = ImGui::GetContentRegionAvail();
			auto& component = entity.GetComponent<T>();
			float lineHeight = GImGui->Font->FontSize + GImGui->Style.FramePadding.y * 2.0f;
			static float itemHeight = 0.0f;
			static float itemWidth = 0.0f;
			bool open = false;
			bool button = false;
			ImGui::Separator();
			auto it = s_Map.find(name);
			if (it == s_Map.end() || it->second) {
				ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, ImVec2{ 4, 4 });
				ImGui::PushStyleColor(ImGuiCol_Header, ImVec4{ 0,0,0,0 });
				ImGui::PushStyleColor(ImGuiCol_HeaderActive, ImVec4{ 0,0,0,0 });
				ImGui::PushStyleColor(ImGuiCol_HeaderHovered, ImVec4{ 0,0,0,0 });

				open = ImGui::TreeNodeEx((void*)typeid(T).hash_code(), treeNodeFlags, name.c_str());
				itemWidth = ImGui::GetItemRectSize().x;
				itemHeight = ImGui::GetItemRectSize().y;
				ImGui::PopStyleColor(3);
				ImGui::PopStyleVar();
			}
			else {
				ImGui::PushStyleColor(ImGuiCol_Button, ImVec4{ 0,0,0,0 });
				ImGui::PushStyleColor(ImGuiCol_ButtonActive, ImVec4{ 0,0,0,0 });
				ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4{ 0,0,0,0 });
				ImGui::PushStyleVar(ImGuiStyleVar_ButtonTextAlign, { 0,0 });
				ImGui::Button(name.c_str(), { itemWidth,itemHeight });
				ImGui::PopStyleVar();
				ImGui::PopStyleColor(3);

				button = true;
			}
			if (ImGui::IsMouseDoubleClicked(ImGuiMouseButton_Left) && ImGui::IsItemHovered()) {
				if (it == s_Map.end()) {
					s_Map[name] = false;
				}
				else
					s_Map[name] = !s_Map[name];
			}

			if (removeable) {
				ImGui::SameLine(contentRegionAvailable.x - ImGui::GetItemRectSize().y * .5f);
				ImGui::PushStyleColor(ImGuiCol_Button, ImVec4{ 0,0,0,0 });
				ImGui::PushStyleColor(ImGuiCol_ButtonActive, ImVec4{ 0,0,0,0 });
				ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4{ 0,0,0,0 });
				if (ImGui::Button("+", ImVec2{ ImGui::GetItemRectSize().y, ImGui::GetItemRectSize().y }))
				{
					ImGui::OpenPopup("ComponentSettings");
				}
				ImGui::PopStyleColor(3);

			}

			bool removeComponent = false;
			if (removeable) {
				if (ImGui::BeginPopup("ComponentSettings"))
				{
					if (ImGui::MenuItem("Remove component"))
						removeComponent = true;

					ImGui::EndPopup();
				}
			}

			if (open)
			{
				uiFunction(component);
				ImGui::TreePop();
			}
			if (removeable) {
				if (removeComponent)
					entity.RemoveComponent<T>();
			}
		}
	}
	template<typename T, typename ...Exclude>
	static void DrawAddComponentItems(Entity& entity, const std::string& text, std::function<void(T&)> func = [](T&) {}) {
		if (!entity.HasComponent<T>() && ((!(entity.HasComponent<Exclude>()))&&...)) {
			if (ImGui::MenuItem(text.c_str())) {
				func(entity.AddComponent<T>());
				ImGui::CloseCurrentPopup();
			}
		}
	}
	class AssetChooserScriptItem {
	public:
		AssetChooserScriptItem() = default;
		AssetChooserScriptItem(const std::unordered_map<std::string, Ref<ScriptClass>>& map) {
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
					if (ImGui::IsItemHovered() && ImGui::IsMouseDoubleClicked(ImGuiMouseButton_Left)) {
						return name;
					}
				}
				else {
					if (ImGui::TreeNodeEx(name.c_str(), ImGuiTreeNodeFlags_SpanAvailWidth)) {
						std::string res = acsi.Draw();
						ImGui::TreePop();
						if (!res.empty())
							return name + '.' + res;
					}
				}
			}
			return "";
		}
	private:
		std::unordered_map<std::string, AssetChooserScriptItem> m_ScriptMap;
	};

	void HandleEntityDragDrops(Entity entity) {

		DragDropTarget dragdropTarget;
		// Materials
	}


	void EntityPropertiesPanel::OnImGuiRender() {
		DrawComponents();
	}


	template<typename Func>
	static void DrawAssetChooser(const std::string& chooserName, Func&& func) {
		static bool chooserOpen = false;

		if (ImGui::Button("Choose... "))
			chooserOpen = !chooserOpen;



		bool chooser = chooserOpen;

		if (chooser) {
			if (ImGui::Begin(chooserName.c_str(), &chooser)) {
				bool chooserState = func();
				if (chooserOpen) {
					chooserOpen = chooserState;
				}
			}
			ImGui::End();
		}
		chooserOpen = chooser;
	}

	void EntityPropertiesPanel::DrawComponents()
	{
		Entity entity = m_SelectedEntity;
		if (!entity)
			return;
		Scene* scene = m_SelectedEntity.GetScene();

		//Drag drops
		{
			ImVec2 size = ImGui::GetContentRegionAvail();
			ImVec2 cursorPos = ImGui::GetCursorPos();

			ImGui::PushStyleColor(ImGuiCol_DragDropTarget, { 0,0,0,0 });

			ImGui::Dummy(size);

			HandleEntityDragDrops(entity);

			ImGui::SetCursorPos(cursorPos);
			ImGui::PopStyleColor();
		}

		auto& id = entity.GetComponent<IDComponent>();
		ImGui::Checkbox("##Active", &id.IsActive);
		ImGui::SameLine();
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


		ImGui::Text(std::to_string(entity.GetUUID().operator uint64_t()).c_str());
		DrawComponent<TransformComponent>("Transform", entity, [&entity, scene = scene](TransformComponent& component)
			{
				glm::vec3 rotDeg = glm::degrees(glm::eulerAngles(component.Rotation));

				DrawVec3Control("Translation", component.Translation);
				DrawVec3Control("Rotation", rotDeg);
				DrawVec3Control("Scale", component.Scale, 1.0f);

				component.Rotation = glm::radians(rotDeg);
			}, false);

		DrawComponent<AnimationPlayerComponent>("Animation Player", entity, [&entity, scene = scene](AnimationPlayerComponent& component)
			{
				{
					bool x = component.State == AnimationPlayerComponent::PlayerState::Playing;
					ImGui::Checkbox("Playing", &x);
					if (x)
						component.State = AnimationPlayerComponent::PlayerState::Playing;
					else
						component.State = AnimationPlayerComponent::PlayerState::Paused;
				}

				{
					bool x = component.FinishAction == AnimationPlayerComponent::AnimationOnFinishAction::Repeat;
					ImGui::Checkbox("Repeat", &x);
					if (x)
						component.FinishAction = AnimationPlayerComponent::AnimationOnFinishAction::Repeat;
					else
						component.FinishAction = AnimationPlayerComponent::AnimationOnFinishAction::None;
				}

				ImGui::DragFloat("Playback speed", &component.PlaybackSpeed, 0.05f, 0.0f, FLT_MAX);
				ImGui::Text("%.3f", component.Time);
			});


		DrawComponent<CameraComponent>("Camera", entity, [](auto& component)
			{
				auto& camera = component.Camera;
				ImGui::Checkbox("Primary", &component.Primary);

				const char* projectionTypeStrings[] = { "Perspective", "Orthographic" };
				uint64_t current = (uint64_t)camera.GetProjectionType();
				uint64_t combo = Combo("Projection", projectionTypeStrings, ARRAYSIZE(projectionTypeStrings), projectionTypeStrings[current]);

				if (combo != -1) {
					camera.SetProjectionType((SceneCamera::ProjectionType)combo);
				}

				switch (camera.GetProjectionType())
				{
				case SceneCamera::ProjectionType::Perspective: {
					float perspectiveVerticalFov = glm::degrees(camera.GetPerspectiveVerticalFOV());
					if (ImGui::DragFloat("Vertical FOV", &perspectiveVerticalFov))
						camera.SetPerspectiveVerticalFOV(glm::radians(perspectiveVerticalFov));

					float perspectiveNear = camera.GetPerspectiveNearClip();
					if (ImGui::DragFloat("Near", &perspectiveNear))
						camera.SetPerspectiveNearClip(perspectiveNear);

					float perspectiveFar = camera.GetPerspectiveFarClip();
					if (ImGui::DragFloat("Far", &perspectiveFar))
						camera.SetPerspectiveFarClip(perspectiveFar);
				}break;
				case SceneCamera::ProjectionType::Orthographic:
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
				}break;
				}

			});

		//Renderers
		DrawComponent<SpriteRendererComponent>("Sprite Renderer", entity, [](auto& component)
			{

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
			ImGui::DragFloat2("Offset", glm::value_ptr(component.Offset), .01f);
			ImGui::DragFloat("Radius", &component.Radius, 0.01f);
			ImGui::DragFloat("Density", &component.Density, 0.01f, .0f, 1.f);
			ImGui::DragFloat("Friction", &component.Friction, 0.01f, .0f, 1.f);
			ImGui::DragFloat("Restitution", &component.Restitution, 0.01f, .0f, 1.f);
			ImGui::DragFloat("Restitution Threshold", &component.RestitutionThreshold, 0.01f, .0f, 1.f);

			});


		DrawComponent<LineRendererComponent>("Line Renderer", entity, [](LineRendererComponent& component) {


			ImGui::ColorEdit4("Color", &component.Color.x);
			int32_t cpy = (int32_t)component.Tesselation;
			if (ImGui::InputInt("Tesselation", (int*)&cpy)) {
				if (cpy >= 0) {
					component.Tesselation = cpy;
					component.RecalculateFinalPoints();
				}
			}
			uint32_t size = (uint32_t)component.Points.size();
			uint32_t i = 0;
			static const ImVec2 padding = { 16.0f,0 };
			const ImVec2 addButtonSize = ImGui::CalcTextSize("+") + padding;
			const ImVec2 deleteButtonSize = ImGui::CalcTextSize("-") + padding;
			bool addedRemoved = false;
			std::vector<LineRendererComponent::Point> componentCopy = component.Points;
			for (auto& point : component.Points) {
				ImGui::PushID(i);
				if (ImGui::DragFloat3(std::to_string(i).c_str(), &point.Position.x, .1f)) {
					component.RecalculateFinalPoints();
				}
				ImGui::SameLine();
				if (ImGui::Button("+", addButtonSize)) {
					addedRemoved = true;
					componentCopy.insert(componentCopy.begin() + i + 1, 1, LineRendererComponent::Point{ point.Position });
				}
				if (size <= 2)
					ImGui::BeginDisabled();
				ImGui::SameLine();
				if (ImGui::Button("-", deleteButtonSize)) {
					addedRemoved = true;
					componentCopy.erase(componentCopy.begin() + i, componentCopy.begin() + i + 1);
				}
				if (size <= 2)
					ImGui::EndDisabled();
				++i;
				ImGui::PopID();
			}
			if (addedRemoved)
			{
				component.Points = std::move(componentCopy);
				component.RecalculateFinalPoints();
			}
			});

		DrawComponent<MeshComponent>("Mesh", entity, [entity, scene = scene](MeshComponent& component) {
			bool b = component.UsedMesh->IsSkinned();
			ImGui::Checkbox("IsSkinned", &b);

			ImGui::Text("%" PRIu64, (uint64_t)component.UsedMesh.Get());
			});

		DrawComponent<SkinnedMeshComponent>("Skinned mesh", entity, [entity, scene = scene](SkinnedMeshComponent& component) {
			bool b = component.UsedMesh->IsSkinned();
			ImGui::Checkbox("IsSkinned", &b);
			ImGui::Text("%llu", (uint64_t)component.RootBone);
			ImGui::Text("%s", scene->GetEntity(component.RootBone).GetComponent<TagComponent>().Tag.c_str());
			});

		DrawComponent<DirectionalLightComponent>("Directional light", entity, [entity, scene = scene](DirectionalLightComponent& component) {
			ImGui::ColorEdit3("Color", &component.Color.r);
			ImGui::DragFloat("Max distance", &component.MaxDistance);
			ImGui::DragFloat("Split lambda", &component.SplitLambda, 0.005f, 0.0f, 1.0f);
			ImGui::DragFloat("Fade start", &component.FadeStart, 0.005f, 0.0f, 1.0f);
			});

		//Scripts
		DrawComponent<ScriptComponent>("Script", entity, [entity, scene = scene](ScriptComponent& component) mutable
			{

				auto& entityScripts = ScriptEngine::GetClasses();
				bool sceneRunning = scene->IsRunning();
				auto& entityScriptFields = ScriptEngine::GetScriptFieldMaps(entity.GetUUID());
				auto& entityScriptInstances = ScriptEngine::GetEntityScriptInstances(entity.GetUUID());

				static int64_t currentChoosingIndex = -1;
				int64_t currentIndex = 0;
				for (auto& klassName : component.ScriptNames) {
					bool scriptClassExists = ScriptEngine::ClassExists(klassName);
					if (!scriptClassExists) {
						ImGui::TextDisabled("No Script Found");
					}
					else {
						ImGui::TextDisabled(klassName.c_str());
					}
					static bool IsChoosingScript = false;
					ImGui::SameLine();
					ImGui::PushID((int)currentIndex);
					IsChoosingScript = !entityScripts.empty() && (ImGui::Button("##ScriptChooser", { 15,0 }) || (IsChoosingScript));
					ImGui::PopID();
					if (IsChoosingScript && currentChoosingIndex == -1)
						currentChoosingIndex = currentIndex;
					if (IsChoosingScript && currentChoosingIndex == currentIndex) {
						AssetChooserScriptItem acsi(entityScripts);
						ImGui::Begin("Please Choose A Script", &IsChoosingScript);
						if (!IsChoosingScript)
						{
							currentChoosingIndex = -1;
						}
						{
							std::string res = acsi.Draw();
							if (!res.empty()) {
								klassName = res;
								IsChoosingScript = false;
								currentChoosingIndex = -1;
							}
						}
						ImGui::End();

					}
					if (!scriptClassExists) {
						++currentIndex;
						continue;
					}
					ImGui::PushID((int)currentIndex);
					if (sceneRunning) {
						Ref<ScriptInstance> instance = entityScriptInstances.at(klassName);
						KD_CORE_ASSERT(instance);
						const auto& fields = instance->GetScriptClass()->GetFields();

						for (const auto& [name, field] : fields)
						{
							if (field.Type == ScriptFieldType::Float)
							{
								float data = instance->GetFieldValue<float>(field);
								if (ImGui::DragFloat(name.c_str(), &data))
								{
									instance->SetFieldValue<float>(field, data);
								}
							}
						}
					}
					else {
						Ref<ScriptClass> entityClass = ScriptEngine::GetEntityClass(klassName);

						const auto& fields = entityClass->GetFields();
						auto& entityFields = entityScriptFields[entityClass];

						for (const auto& [name, field] : fields)
						{
							// Field has been set in editor
							if (entityFields.find(name) != entityFields.end())
							{
								const ScriptFieldInstance& scriptField = entityFields.at(name);

								if (field.Type == ScriptFieldType::Float)
								{
									float data = scriptField.GetValue<float>();
									if (ImGui::DragFloat(name.c_str(), &data))
										scriptField.SetValue(data);
								}
							}
							else
							{
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
					ImGui::PopID();
					++currentIndex;
				}
				//TODO: Add functionality for other types

			});

		DrawComponent<TextComponent>("Text Renderer", entity, [entity, scene = scene](TextComponent& component) {
			ImGui::InputTextMultiline("Text content", &component.TextContent);
			ImGui::ColorEdit3("Color", &component.Color.r);
			ImGui::SliderFloat("Border thickness", &component.BorderThickness, 0.0f, 0.35f);
			if (component.BorderThickness != 0.0f)
				ImGui::ColorEdit3("Border Color", &component.BorderColor.r);
			ImGui::DragFloat("Kerning", &component.Kerning, 0.05f, 0.0f, FLT_MAX);
			});

		ImGui::PushItemWidth(-1);

		if (ImGui::Button("Add Component"))
			ImGui::OpenPopup("AddComponent");

		if (ImGui::BeginPopup("AddComponent"))
		{
			//DrawAddComponentItems<ScriptComponent>(entity, "Script Component");
			if (ImGui::MenuItem("Script Component")) {
				entity.AddScript();
			}
			DrawAddComponentItems<CameraComponent>(entity, "Camera");
			DrawAddComponentItems<SpriteRendererComponent, LineRendererComponent, CircleRendererComponent>(entity, "Sprite Renderer");
			DrawAddComponentItems<CircleRendererComponent, LineRendererComponent, SpriteRendererComponent>(entity, "Circle Renderer");
			DrawAddComponentItems<Rigidbody2DComponent>(entity, "Rigidbody 2D");
			DrawAddComponentItems<BoxCollider2DComponent>(entity, "Box Collider 2D");
			DrawAddComponentItems<CircleCollider2DComponent>(entity, "Circle Collider 2D");
			DrawAddComponentItems<LineRendererComponent, CircleRendererComponent, SpriteRendererComponent>(entity, "Line Renderer", [](LineRendererComponent& lrc) {
				lrc.Points = { LineRendererComponent::Point{ {0,0,0} }, LineRendererComponent::Point{ {1,1,0} } };
				lrc.RecalculateFinalPoints();
				});
			//DrawAddComponentItems<LightComponent>(entity, "Light");
			DrawAddComponentItems<LineRendererComponent, CircleRendererComponent, SpriteRendererComponent >(entity, "Cube Renderer");

			//DrawAddComponentItems<ParentComponent>(m_SelectionContext, "Parent");
			//DrawAddComponentItems<ChildComponent>(m_SelectionContext, "Child");
			ImGui::EndPopup();
		}

		ImGui::PopItemWidth();

	}

}
