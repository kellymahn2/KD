#include "KDpch.h"
#include "SceneSerializer.h"

#include "Entity.h"
#include "Kaidel/Core/YAMLHelpers.h"
#include "Components.h"
#include "Kaidel/Project/Project.h"
#include "Kaidel/Scripting/ScriptEngine.h"
#include "Kaidel/Asset/AssetManager.h"
#include "Kaidel/Scene/ModelLibrary.h"

#include <fstream>
#include <yaml-cpp/yaml.h>

namespace Kaidel {


	

	static std::string ScriptFieldTypeToString(ScriptFieldType type) {
		switch (type)
		{
		case ScriptFieldType::Float:return "Float";
		case ScriptFieldType::Double:return "Double";
		case ScriptFieldType::Short:return "Short";
		case ScriptFieldType::UShort:return "UShort";
		case ScriptFieldType::Int:return "Int";
		case ScriptFieldType::UInt:return "UInt";
		case ScriptFieldType::Long:return "Long";
		case ScriptFieldType::ULong:return "ULong";
		case ScriptFieldType::Byte:return "Byte";
		case ScriptFieldType::SByte:return "SByte";
		case ScriptFieldType::Char:return "Char";
		case ScriptFieldType::String:return "String";
		case ScriptFieldType::Bool:return "Bool";
		case ScriptFieldType::Entity:return "Entity";
		case ScriptFieldType::Vector2:return "Vector2";
		case ScriptFieldType::Vector3:return "Vector3";
		case ScriptFieldType::Vector4:return "Vector4";
		}
		KD_CORE_ASSERT(false, "Not a valid field type");
		return "";
	}
	static ScriptFieldType ScriptFieldTypeFromString(std::string_view type) {
#define Temp(T) if(type==#T) return ScriptFieldType::T;

		Temp(Float);
		Temp(Double);
		Temp(Short);
		Temp(UShort);
		Temp(Int);
		Temp(UInt);
		Temp(Long);
		Temp(ULong);
		Temp(Byte);
		Temp(SByte);
		Temp(Char);
		Temp(String);
		Temp(Bool);
		Temp(Entity);
		Temp(Vector2);
		Temp(Vector3);
		Temp(Vector4);
		KD_CORE_ASSERT(false, "Type does not exist");
		return ScriptFieldType::None;
	}
	static std::string RigidBody2DBodyTypeToString(Rigidbody2DComponent::BodyType bodyType) {
		switch (bodyType)
		{
		case Rigidbody2DComponent::BodyType::Static:return "Static";
		case Rigidbody2DComponent::BodyType::Dynamic:return "Dynamic";
		case Rigidbody2DComponent::BodyType::Kinematic:return "Kinematic";
		}
		KD_CORE_ASSERT(false, "Invalid Body Type");
		return "";
	}
	static Rigidbody2DComponent::BodyType RigidBody2DBodyTypeFromString(const std::string& bodyType) {
		if (bodyType == "Static")
			return Rigidbody2DComponent::BodyType::Static;
		else if (bodyType == "Dynamic")
			return Rigidbody2DComponent::BodyType::Dynamic;
		else if (bodyType == "Kinematic")
			return Rigidbody2DComponent::BodyType::Kinematic;
		KD_CORE_ASSERT(false, "Invalid Body Type");
		return Rigidbody2DComponent::BodyType::Static;
	}
	SceneSerializer::SceneSerializer(const Ref<Scene>& scene)
		: m_Scene(scene)
	{
	}

	static void SerializeEntity(YAML::Emitter& out, Entity entity)
	{
		KD_CORE_ASSERT(entity.HasComponent<IDComponent>());
		out << YAML::BeginMap; // Entity
		out << YAML::Key << "Entity" << YAML::Value << (uint64_t)entity.GetUUID();

		if (entity.HasComponent<TagComponent>())
		{
			out << YAML::Key << "TagComponent";
			out << YAML::BeginMap; // TagComponent

			auto& tag = entity.GetComponent<TagComponent>().Tag;
			out << YAML::Key << "Tag" << YAML::Value << tag;

			out << YAML::EndMap; // TagComponent
		}
		if (entity.HasComponent<ChildComponent>()) {
			out << YAML::Key << "ChildComponent";
			out << YAML::BeginMap; // ChildComponent

			auto& cc = entity.GetComponent<ChildComponent>();
			out << YAML::Key << "Parent" << YAML::Value << (uint64_t)Entity(cc.Parent, entity.GetScene()).GetUUID();
			out << YAML::Key << "LocalPosition" << YAML::Value << cc.LocalPosition;
			out << YAML::Key << "LocalRotation" << YAML::Value << cc.LocalRotation;

			out << YAML::EndMap; // ChildComponent
		}
		if (entity.HasComponent<TransformComponent>())
		{
			out << YAML::Key << "TransformComponent";
			out << YAML::BeginMap; // TransformComponent

			auto& tc = entity.GetComponent<TransformComponent>();
			out << YAML::Key << "Translation" << YAML::Value << tc.Translation;
			out << YAML::Key << "Rotation" << YAML::Value << tc.Rotation;
			out << YAML::Key << "Scale" << YAML::Value << tc.Scale;

			out << YAML::EndMap; // TransformComponent
		}
		if (entity.HasComponent<MeshComponent>())
		{
			out << YAML::Key << "MeshComponent";
			out << YAML::BeginMap; // MeshComponent

			auto& mc = entity.GetComponent<MeshComponent>();


			out << YAML::Key << "Mesh" << YAML::Value;

			out << (uint64_t)mc.UsedMesh->GetHandle();

			out << YAML::Key << "Materials" << YAML::Value;

			out << YAML::BeginSeq;
			for (auto& mat : mc.Instance.UsedMaterial)
			{
				out << (uint64_t)mat->GetHandle();
			}
			out << YAML::EndSeq;

			out << YAML::EndMap; // MeshComponent
		}
		if (entity.HasComponent<CameraComponent>())
		{
			out << YAML::Key << "CameraComponent";
			out << YAML::BeginMap; // CameraComponent

			auto& cameraComponent = entity.GetComponent<CameraComponent>();
			auto& camera = cameraComponent.Camera;

			out << YAML::Key << "Camera" << YAML::Value;
			out << YAML::BeginMap; // Camera
			out << YAML::Key << "ProjectionType" << YAML::Value << (int)camera.GetProjectionType();
			out << YAML::Key << "PerspectiveFOV" << YAML::Value << camera.GetPerspectiveVerticalFOV();
			out << YAML::Key << "PerspectiveNear" << YAML::Value << camera.GetPerspectiveNearClip();
			out << YAML::Key << "PerspectiveFar" << YAML::Value << camera.GetPerspectiveFarClip();
			out << YAML::Key << "OrthographicSize" << YAML::Value << camera.GetOrthographicSize();
			out << YAML::Key << "OrthographicNear" << YAML::Value << camera.GetOrthographicNearClip();
			out << YAML::Key << "OrthographicFar" << YAML::Value << camera.GetOrthographicFarClip();
			out << YAML::EndMap; // Camera

			out << YAML::Key << "Primary" << YAML::Value << cameraComponent.Primary;
			out << YAML::Key << "FixedAspectRatio" << YAML::Value << cameraComponent.FixedAspectRatio;

			out << YAML::EndMap; // CameraComponent
		}
		if (entity.HasComponent<SpriteRendererComponent>())
		{
			out << YAML::Key << "SpriteRendererComponent";
			out << YAML::BeginMap; // SpriteRendererComponent

			//auto& spriteRendererComponent = entity.GetComponent<SpriteRendererComponent>();
			//out << YAML::Key << "Material" << YAML::Value << spriteRendererComponent.Material;
			out << YAML::EndMap; // SpriteRendererComponent
		}
		if (entity.HasComponent<CircleRendererComponent>())
		{
			out << YAML::Key << "CircleRendererComponent";
			out << YAML::BeginMap; // CircleRendererComponent

			auto& circleRendererComponent = entity.GetComponent<CircleRendererComponent>();
			out << YAML::Key << "Color" << YAML::Value << circleRendererComponent.Color;
			out << YAML::Key << "Thickness" << YAML::Value << circleRendererComponent.Thickness;
			out << YAML::Key << "Fade" << YAML::Value << circleRendererComponent.Fade;

			out << YAML::EndMap; // CircleRendererComponent
		}
		if (entity.HasComponent<Rigidbody2DComponent>())
		{
			out << YAML::Key << "Rigidbody2DComponent";
			out << YAML::BeginMap; // Rigidbody2DComponent

			auto& rigidbody2DComponent = entity.GetComponent<Rigidbody2DComponent>();
			out << YAML::Key << "BodyType" << YAML::Value << RigidBody2DBodyTypeToString(rigidbody2DComponent.Type);
			out << YAML::Key << "FixedRotation" << YAML::Value << rigidbody2DComponent.FixedRotation;

			out << YAML::EndMap; // Rigidbody2DComponent
		}
		if (entity.HasComponent<BoxCollider2DComponent>())
		{
			out << YAML::Key << "BoxCollider2DComponent";
			out << YAML::BeginMap; // BoxCollider2DComponent

			auto& boxCollider2DComponent = entity.GetComponent<BoxCollider2DComponent>();
			out << YAML::Key << "Offset" << YAML::Value << boxCollider2DComponent.Offset;
			out << YAML::Key << "Size" << YAML::Value << boxCollider2DComponent.Size;
			out << YAML::Key << "Density" << YAML::Value << boxCollider2DComponent.Density;
			out << YAML::Key << "Friction" << YAML::Value << boxCollider2DComponent.Friction;
			out << YAML::Key << "Restitution" << YAML::Value << boxCollider2DComponent.Restitution;
			out << YAML::Key << "RestitutionThreshold" << YAML::Value << boxCollider2DComponent.RestitutionThreshold;

			out << YAML::EndMap; // BoxCollider2DComponent
		}
		if (entity.HasComponent<CircleCollider2DComponent>())
		{
			out << YAML::Key << "CircleCollider2DComponent";
			out << YAML::BeginMap; // CircleCollider2DComponent

			auto& circleCollider2DComponent = entity.GetComponent<CircleCollider2DComponent>();
			out << YAML::Key << "Offset" << YAML::Value << circleCollider2DComponent.Offset;
			out << YAML::Key << "Radius" << YAML::Value << circleCollider2DComponent.Radius;
			out << YAML::Key << "Density" << YAML::Value << circleCollider2DComponent.Density;
			out << YAML::Key << "Friction" << YAML::Value << circleCollider2DComponent.Friction;
			out << YAML::Key << "Restitution" << YAML::Value << circleCollider2DComponent.Restitution;
			out << YAML::Key << "RestitutionThreshold" << YAML::Value << circleCollider2DComponent.RestitutionThreshold;



			out << YAML::EndMap; // CircleCollider2DComponent
		}
		if (entity.HasComponent<LineRendererComponent>()) {
			out << YAML::Key << "LineRendererComponent";
			out << YAML::BeginMap; // LineRendererComponent

			auto& lineRendererComponent = entity.GetComponent<LineRendererComponent>();
			out << YAML::Key << "Tesselation" << YAML::Value << lineRendererComponent.Tesselation;
			out << YAML::Key << "Color" << YAML::Value << lineRendererComponent.Color;
			out << YAML::Key << "Points";
			out << YAML::BeginSeq;
			for (auto& point : lineRendererComponent.Points) {
				out << point.Position;
			}
			out << YAML::EndSeq;
			out << YAML::EndMap; // LineRendererComponent
		}
		if (entity.HasComponent<ScriptComponent>()) {
			out << YAML::Key << "ScriptComponent";
			out << YAML::BeginMap; // ScriptComponent
			auto& src = entity.GetComponent<ScriptComponent>();
			out << YAML::Key << "Scripts";
			out << YAML::BeginSeq;
			const auto& scriptFields = ScriptEngine::GetScriptFieldMaps(entity.GetUUID());
			for (auto& script : src.ScriptNames) {
				auto klass = ScriptEngine::GetEntityClass(script);
				if (!klass)
					continue;
				out << YAML::Key << "Name" << YAML::Value << script;
				if (scriptFields.find(klass) != scriptFields.end()) {
					auto& fields = scriptFields.at(klass);
					if (fields.empty())
						continue;
					out << YAML::Key << "ScriptFields" << YAML::Value;
					out << YAML::BeginSeq;
					const auto& entityFields = scriptFields.at(klass);
					for (const auto& [fieldName, field] : entityFields) {
						out << YAML::BeginMap;//ScriptFields
						out << YAML::Key << "Name" << YAML::Value << fieldName;
						out << YAML::Key << "Type" << YAML::Value << ScriptFieldTypeToString(field.Field.Type);
						out << YAML::Key << "Data" << YAML::Value;
						// Field has been set in editor
						switch (field.Field.Type)
						{
						case ScriptFieldType::Float:
							out<< field.GetValue<float>();
							break;
						}
						out << YAML::EndMap;//ScriptFields
					}
					out << YAML::EndSeq;
				}
			}
			out << YAML::EndSeq;
			out << YAML::EndMap; // ScriptComponent
		}
		if (entity.HasComponent<DirectionalLightComponent>()) {
			out << YAML::Key << "DirectionalLightComponent";
			
			out << YAML::BeginMap; // DirectionalLightComponent

			auto& dlc = entity.GetComponent<DirectionalLightComponent>();
			out << YAML::Key << "Color" << YAML::Value << dlc.Color;
			out << YAML::Key << "MaxDistance" << YAML::Value << dlc.MaxDistance;

			out << YAML::EndMap; // DirectionalLightComponent
		}

		if (entity.HasComponent<TextComponent>()) {
			out << YAML::Key << "TextComponent";

			out << YAML::BeginMap; // TextComponent

			auto& tc = entity.GetComponent<TextComponent>();
			out << YAML::Key << "Text" << YAML::Value << tc.TextContent;
			out << YAML::Key << "Color" << YAML::Value << tc.Color;
			out << YAML::Key << "BorderThickness" << YAML::Value << tc.BorderThickness;
			out << YAML::Key << "BorderColor" << YAML::Value << tc.BorderColor;
			out << YAML::Key << "Kerning" << YAML::Value << tc.Kerning;
			//TODO: Add fonts.

			out << YAML::EndMap; // TextComponent
		}

		if (entity.HasComponent<AnimationPlayerComponent>()) {
			out << YAML::Key << "AnimationPlayerComponent";

			out << YAML::BeginMap; // AnimationPlayerComponent

			auto& apc = entity.GetComponent<AnimationPlayerComponent>();
			out << YAML::Key << "Animation" << YAML::Value << (apc.Animation && apc.Animation->GetHandle() != 0 ? (uint64_t)apc.Animation->GetHandle() : 0);

			out << YAML::EndMap; // AnimationPlayerComponent
		}


		out << YAML::EndMap; // Entity
	}

	void SceneSerializer::Serialize(const std::string& filepath)
	{

		YAML::Emitter out;
		out << YAML::BeginMap;
		out << YAML::Key << "Scene" << YAML::Value << "Untitled";
		out << YAML::Key << "Entities" << YAML::Value << YAML::BeginSeq;
		m_Scene->m_Registry.each([&](auto entityID)
		{
			Entity entity = { entityID, m_Scene.Get() };
			if (!entity)
				return;
			SerializeEntity(out, entity);
		});
		out << YAML::EndSeq;
		out << YAML::EndMap;

		std::ofstream fout(filepath);
		fout << out.c_str();
	}

	void SceneSerializer::SerializeRuntime(const std::string& filepath)
	{
		// Not implemented
		KD_CORE_ASSERT(false);
	}
	template<typename T>
	static void DeserializeComponent(Entity& entity, const char* name,YAML::Node& entityData, std::function<void(T&, Entity&, YAML::Node&)> && func) {
		try {
			auto componentData = entityData[name];
			if (componentData) {
				auto& component = entity.AddComponent<T>();
				func(component, entity, componentData);
			}
		}
		catch (std::exception& error)
		{
			KD_ERROR("Couldn't deserialize component: {} error: {}", name, error.what());
			KD_CORE_ASSERT(false);
			
			return;
		}
	}

	static Entity DeserializeEntity(YAML::Node& entityNode, Ref<Scene>& scene)
	{
		UUID uuid = entityNode["Entity"].as<uint64_t>();
		std::string name;
		auto tagComponent = entityNode["TagComponent"];
		if (tagComponent)
			name = tagComponent["Tag"].as<std::string>();

		//KD_CORE_TRACE("Deserialized entity with ID = {0}, name = {1}", uuid, name);

		Entity deserializedEntity = scene->CreateEntity(uuid, name);

		auto transformComponent = entityNode["TransformComponent"];
		if (transformComponent)
		{
			// Entities always have transforms
			auto& tc = deserializedEntity.GetComponent<TransformComponent>();
			tc.Translation = transformComponent["Translation"].as<glm::vec3>();
			tc.Rotation = transformComponent["Rotation"].as<glm::quat>();
			tc.Scale = transformComponent["Scale"].as<glm::vec3>();
		}

		DeserializeComponent<MeshComponent>(deserializedEntity, "MeshComponent", entityNode,
			[](MeshComponent& cc, Entity& entity, YAML::Node& node)
			{
				uint64_t id = node["Mesh"].as<uint64_t>();

				cc.UsedMesh = AssetManager::GetAsset(id).As<Mesh>();

				for (auto matNode : node["Materials"])
				{
					uint64_t id = matNode.as<uint64_t>();

					cc.Instance.UsedMaterial.push_back(AssetManager::GetAsset(id).As<StandardMaterial>());
				}
				cc.Instance.VisibilityResults.resize(cc.UsedMesh->GetSubmeshes().size());
			});

		DeserializeComponent<CameraComponent>(deserializedEntity, "CameraComponent", entityNode,
			[](CameraComponent& cc, Entity& entity, YAML::Node& cameraComponent) {
				auto& cameraProps = cameraComponent["Camera"];

				cc.Camera.SetPerspectiveVerticalFOV(cameraProps["PerspectiveFOV"].as<float>());
				cc.Camera.SetPerspectiveNearClip(cameraProps["PerspectiveNear"].as<float>());
				cc.Camera.SetPerspectiveFarClip(cameraProps["PerspectiveFar"].as<float>());

				cc.Camera.SetOrthographicSize(cameraProps["OrthographicSize"].as<float>());
				cc.Camera.SetOrthographicNearClip(cameraProps["OrthographicNear"].as<float>());
				cc.Camera.SetOrthographicFarClip(cameraProps["OrthographicFar"].as<float>());

				cc.Primary = cameraComponent["Primary"].as<bool>();
				cc.FixedAspectRatio = cameraComponent["FixedAspectRatio"].as<bool>();
				cc.Camera.SetProjectionType((SceneCamera::ProjectionType)cameraProps["ProjectionType"].as<int>());
			}
		);

		DeserializeComponent<SpriteRendererComponent>(deserializedEntity, "SpriteRendererComponent", entityNode,
			[](SpriteRendererComponent& src, Entity& entity, YAML::Node& spriteRendererComponent) {
			}
		);
		DeserializeComponent<CircleRendererComponent>(deserializedEntity, "CircleRendererComponent", entityNode,
			[](CircleRendererComponent& crc, Entity& entity, YAML::Node& circleRendererComponent) {
				crc.Color = circleRendererComponent["Color"].as < glm::vec4>();
				crc.Thickness = circleRendererComponent["Thickness"].as <float>();
				crc.Fade = circleRendererComponent["Fade"].as < float>();

			}
		);
		DeserializeComponent<Rigidbody2DComponent>(deserializedEntity, "Rigidbody2DComponent", entityNode,
			[](Rigidbody2DComponent& rb2d, Entity& entity, YAML::Node& rigidbody2DComponent) {
				rb2d.Type = RigidBody2DBodyTypeFromString(rigidbody2DComponent["BodyType"].as<std::string>());
				rb2d.FixedRotation = rigidbody2DComponent["FixedRotation"].as<bool>();
			}
		);
		DeserializeComponent<BoxCollider2DComponent>(deserializedEntity, "BoxCollider2DComponent", entityNode,
			[](BoxCollider2DComponent& bc2d, Entity& entity, YAML::Node& boxCollider2DComponent) {
				bc2d.Offset = boxCollider2DComponent["Offset"].as<glm::vec2>();
				bc2d.Size = boxCollider2DComponent["Size"].as<glm::vec2>();
				bc2d.Density = boxCollider2DComponent["Density"].as<float>();
				bc2d.Friction = boxCollider2DComponent["Friction"].as<float>();
				bc2d.Restitution = boxCollider2DComponent["Restitution"].as<float>();
				bc2d.RestitutionThreshold = boxCollider2DComponent["RestitutionThreshold"].as<float>();
			}
		);

		DeserializeComponent<CircleCollider2DComponent>(deserializedEntity, "CircleCollider2DComponent", entityNode,
			[](CircleCollider2DComponent& cc2d, Entity& entity, YAML::Node& circleCollider2DComponent) {
				cc2d.Offset = circleCollider2DComponent["Offset"].as<glm::vec2>();
				cc2d.Radius = circleCollider2DComponent["Radius"].as<float>();
				cc2d.Density = circleCollider2DComponent["Density"].as<float>();
				cc2d.Friction = circleCollider2DComponent["Friction"].as<float>();
				cc2d.Restitution = circleCollider2DComponent["Restitution"].as<float>();
				cc2d.RestitutionThreshold = circleCollider2DComponent["RestitutionThreshold"].as<float>();
			}
		);

		DeserializeComponent<LineRendererComponent>(deserializedEntity, "LineRendererComponent", entityNode,
			[](LineRendererComponent& lrc, Entity& entity, YAML::Node& lineRendererComponent) {
				lrc.Tesselation = lineRendererComponent["Tesselation"].as<uint32_t>();
				lrc.Color = lineRendererComponent["Color"].as<glm::vec4>();
				YAML::Node points = lineRendererComponent["Points"];
				for (auto point : points) {
					lrc.Points.push_back({ point.as<glm::vec3>() });
				}
				lrc.RecalculateFinalPoints();
			}
		);

		DeserializeComponent<DirectionalLightComponent>
			(deserializedEntity, "DirectionalLightComponent", entityNode,
				[](DirectionalLightComponent& dlc, Entity& entity, YAML::Node& directionalLightComponent) {
					dlc.Color = directionalLightComponent["Color"].as<glm::vec3>();
					dlc.MaxDistance = directionalLightComponent["MaxDistance"].as<float>();
				});

		DeserializeComponent<ScriptComponent>(deserializedEntity, "ScriptComponent", entityNode,
			[](ScriptComponent& src, Entity& entity, YAML::Node& scriptRendererComponent) {
				YAML::Node scripts = scriptRendererComponent["Scripts"];
				for (auto node : scripts) {
					src.ScriptNames.push_back(node["Name"].as<std::string>());
					auto klass = ScriptEngine::GetEntityClass(node["Name"].as<std::string>());
					if (!klass)
						continue;
					YAML::Node fields = node["ScriptFields"];
					for (auto field : fields) {
						ScriptFieldType type = ScriptFieldTypeFromString(field["Type"].as<std::string>());
						std::string name = field["Name"].as<std::string>();
#define 						FIELD_DATA_SERILIZATION(T,Type) \
								case ScriptFieldType::##T:\
									ScriptEngine::AddSerializedField<Type>(entity.GetUUID(), name,\
										type, klass, field["Data"].as<Type>());\
									break
						switch (type)
						{
							FIELD_DATA_SERILIZATION(Float, float);
						}
					}

				}

			}
		);
		DeserializeComponent<TextComponent>(deserializedEntity, "TextComponent", entityNode,
			[](TextComponent& tc, Entity& entity, YAML::Node& textComponent) {
				tc.TextContent = textComponent["Text"].as<std::string>();
				tc.Color = textComponent["Color"].as<glm::vec4>();
				tc.BorderThickness = textComponent["BorderThickness"].as<float>();
				tc.BorderColor = textComponent["BorderColor"].as<glm::vec4>();
				tc.Kerning = textComponent["Kerning"].as<float>();
				//TODO: Add fonts.
			});

		DeserializeComponent<AnimationPlayerComponent>(deserializedEntity, "AnimationPlayerComponent", entityNode,
			[](AnimationPlayerComponent& apc, Entity& entity, YAML::Node& animComponent) {
				auto animHandle = animComponent["Animation"].as<uint64_t>();

				if (AssetManager::IsAssetHandleValid(animHandle))
				{
					apc.Animation = AssetManager::GetAsset(animHandle).As<AnimationTree>();
				}
				else if (animHandle == 0)
				{
					KD_INFO("Invalid animation.");
				}
			});

		return deserializedEntity;
	}


	bool SceneSerializer::Deserialize(const std::string& filepath)
	{
		YAML::Node data = YAML::LoadFile(filepath);
		if (!data["Scene"])
			return false;

		std::string sceneName = data["Scene"].as<std::string>();

		auto entities = data["Entities"];
		if (entities)
		{
			try 
			{
				for (auto entityNode : entities)
				{
					DeserializeEntity(entityNode, m_Scene);
				}

				//Deserialize parent child relationships.
				for (auto entityNode : entities)
				{
					UUID uuid = entityNode["Entity"].as<uint64_t>();
					Entity e = m_Scene->GetEntity(uuid);

					DeserializeComponent<ChildComponent>(e, "ChildComponent", entityNode,
					[](ChildComponent& cc, Entity& entity, YAML::Node& childData)
					{
						UUID parentID = childData["Parent"].as<uint64_t>();

						Entity parent = entity.GetScene()->GetEntity(parentID);

						cc.Parent = parent.GetHandle();

						parent.AddChild(entity);
					});
				}
			}
			catch (std::exception& error)
			{
				KD_ERROR("Couldn't deserialize entity: {}", error.what());
				KD_CORE_ASSERT(false);
				return false;
			}
		}

		if (m_Scene->m_Registry.view<DirectionalLightComponent>().empty())
		{
			Entity e = m_Scene->CreateEntity("Main light");

			e.AddComponent<DirectionalLightComponent>();
		}

		return true;
	}

	bool SceneSerializer::DeserializeRuntime(const std::string& filepath)
	{
		// Not implemented
		KD_CORE_ASSERT(false);
		return false;
	}

}
