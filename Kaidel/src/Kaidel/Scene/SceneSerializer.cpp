#include "KDpch.h"
#include "SceneSerializer.h"

#include "Entity.h"
#include "Components.h"
#include "Kaidel/Scripting/ScriptEngine.h"
#include <fstream>

#include <yaml-cpp/yaml.h>

namespace YAML {

	template<>
	struct convert<glm::vec2>
	{
		static Node encode(const glm::vec2& rhs)
		{
			Node node;
			node.push_back(rhs.x);
			node.push_back(rhs.y);
			node.SetStyle(EmitterStyle::Flow);
			return node;
		}

		static bool decode(const Node& node, glm::vec2& rhs)
		{
			if (!node.IsSequence() || node.size() != 2)
				return false;

			rhs.x = node[0].as<float>();
			rhs.y = node[1].as<float>();
			return true;
		}
	};

	template<>
	struct convert<glm::vec3>
	{
		static Node encode(const glm::vec3& rhs)
		{
			Node node;
			node.push_back(rhs.x);
			node.push_back(rhs.y);
			node.push_back(rhs.z);
			node.SetStyle(EmitterStyle::Flow);
			return node;
		}

		static bool decode(const Node& node, glm::vec3& rhs)
		{
			if (!node.IsSequence() || node.size() != 3)
				return false;

			rhs.x = node[0].as<float>();
			rhs.y = node[1].as<float>();
			rhs.z = node[2].as<float>();
			return true;
		}
	};

	template<>
	struct convert<glm::vec4>
	{
		static Node encode(const glm::vec4& rhs)
		{
			Node node;
			node.push_back(rhs.x);
			node.push_back(rhs.y);
			node.push_back(rhs.z);
			node.push_back(rhs.w);
			node.SetStyle(EmitterStyle::Flow);
			return node;
		}

		static bool decode(const Node& node, glm::vec4& rhs)
		{
			if (!node.IsSequence() || node.size() != 4)
				return false;

			rhs.x = node[0].as<float>();
			rhs.y = node[1].as<float>();
			rhs.z = node[2].as<float>();
			rhs.w = node[3].as<float>();
			return true;
		}
	};

}
namespace Kaidel {

	YAML::Emitter& operator<<(YAML::Emitter& out, const glm::vec2& v)
	{
		out << YAML::Flow;
		out << YAML::BeginSeq << v.x << v.y << YAML::EndSeq;
		return out;
	}
	YAML::Emitter& operator<<(YAML::Emitter& out, const glm::vec3& v)
	{
		out << YAML::Flow;
		out << YAML::BeginSeq << v.x << v.y << v.z << YAML::EndSeq;
		return out;
	}

	YAML::Emitter& operator<<(YAML::Emitter& out, const glm::vec4& v)
	{
		out << YAML::Flow;
		out << YAML::BeginSeq << v.x << v.y << v.z << v.w << YAML::EndSeq;
		return out;
	}
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

			auto& parent = entity.GetComponent<ChildComponent>().Parent;
			out << YAML::Key << "Parent" << YAML::Value << (uint64_t)parent;

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

			auto& spriteRendererComponent = entity.GetComponent<SpriteRendererComponent>();
			out << YAML::Key << "Color" << YAML::Value << spriteRendererComponent.Color;
			if (spriteRendererComponent.Texture)
				out << YAML::Key << "Texture" << YAML::Value << spriteRendererComponent.Texture->GetPath();
			out << YAML::Key << "TilingFactor" << YAML::Value << spriteRendererComponent.TilingFactor;

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
		if (entity.HasComponent<ScriptComponent>()) {
			out << YAML::Key << "ScriptComponent";
			out << YAML::BeginMap;//ScriptComponent
			auto& scriptComponent = entity.GetComponent<ScriptComponent>();
			out << YAML::Key << "Name" << YAML::Value << scriptComponent.Name;

			//Fields
			Ref<ScriptClass> entityClass = ScriptEngine::GetEntityClass(scriptComponent.Name);
			const auto& fields = entityClass->GetFields();
			if (!fields.empty()) {
				out << YAML::Key << "ScriptFields"<<YAML::Value;
				const auto& entityFields = ScriptEngine::GetScriptFieldMap(entity.GetUUID());
				out << YAML::BeginSeq;
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
			out << YAML::EndMap;//ScriptComponent
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
			Entity entity = { entityID, m_Scene.get() };
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
	template<typename T,typename Func>
	static void DeserializeComponent(Entity& entity, const std::string& name,YAML::Node& entityData,Func&& func) {
		auto componentData = entityData[name];
		if (componentData) {
			auto& component = entity.AddComponent<T>();
			func(component, entity, componentData);
		}
	}
	bool SceneSerializer::Deserialize(const std::string& filepath)
	{
		YAML::Node data = YAML::LoadFile(filepath);
		if (!data["Scene"])
			return false;

		std::string sceneName = data["Scene"].as<std::string>();
		//KD_CORE_TRACE("Deserializing scene '{0}'", sceneName);

		auto entities = data["Entities"];
		if (entities)
		{
			for (auto entity : entities)
			{
				UUID uuid = entity["Entity"].as<uint64_t>(); 
				auto& entityFields = ScriptEngine::GetScriptFieldMap(uuid);
				std::string name;
				auto tagComponent = entity["TagComponent"];
				if (tagComponent)
					name = tagComponent["Tag"].as<std::string>();

				//KD_CORE_TRACE("Deserialized entity with ID = {0}, name = {1}", uuid, name);

				Entity deserializedEntity = m_Scene->CreateEntity(uuid,name);

				auto transformComponent = entity["TransformComponent"];
				if (transformComponent)
				{
					// Entities always have transforms
					auto& tc = deserializedEntity.GetComponent<TransformComponent>();
					tc.Translation = transformComponent["Translation"].as<glm::vec3>();
					tc.Rotation = transformComponent["Rotation"].as<glm::vec3>();
					tc.Scale = transformComponent["Scale"].as<glm::vec3>();
				}

				DeserializeComponent<ChildComponent>(deserializedEntity, "ChildComponent", entity,
					[](auto& cc, auto& entity, auto& childComponent) {
						cc.Parent = childComponent["Parent"].as<uint64_t>();
					});
				DeserializeComponent<CameraComponent>(deserializedEntity, "CameraComponent", entity,
					[](auto& cc, auto& entity, auto& cameraComponent) {
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

				DeserializeComponent<SpriteRendererComponent>(deserializedEntity, "SpriteRendererComponent", entity,
					[](auto& src, auto& entity, auto& spriteRendererComponent) {
						src.Color = spriteRendererComponent["Color"].as<glm::vec4>();
						if (spriteRendererComponent["Texture"])
							src.Texture = Texture2D::Create(spriteRendererComponent["Texture"].as<std::string>());
						if (spriteRendererComponent["TilingFactor"])
							src.TilingFactor = spriteRendererComponent["TilingFactor"].as<float>();
					}
				);
				DeserializeComponent<CircleRendererComponent>(deserializedEntity, "CircleRendererComponent", entity,
					[](CircleRendererComponent& crc, auto& entity, auto& circleRendererComponent) {
						crc.Color = circleRendererComponent["Color"].as < glm::vec4>();
						crc.Thickness = circleRendererComponent["Thickness"].as <float>();
						crc.Fade = circleRendererComponent["Fade"].as < float>();

					}
				);
				DeserializeComponent<Rigidbody2DComponent>(deserializedEntity, "Rigidbody2DComponent", entity,
					[](auto& rb2d, auto& entity, auto& rigidbody2DComponent) {
						rb2d.Type = RigidBody2DBodyTypeFromString(rigidbody2DComponent["BodyType"].as<std::string>());
						rb2d.FixedRotation = rigidbody2DComponent["FixedRotation"].as<bool>();
					}
				);
				DeserializeComponent<BoxCollider2DComponent>(deserializedEntity, "BoxCollider2DComponent", entity,
					[](auto& bc2d, auto& entity, auto& boxCollider2DComponent) {
						bc2d.Offset = boxCollider2DComponent["Offset"].as<glm::vec2>();
						bc2d.Size = boxCollider2DComponent["Size"].as<glm::vec2>();
						bc2d.Density = boxCollider2DComponent["Density"].as<float>();
						bc2d.Friction = boxCollider2DComponent["Friction"].as<float>();
						bc2d.Restitution = boxCollider2DComponent["Restitution"].as<float>();
						bc2d.RestitutionThreshold = boxCollider2DComponent["RestitutionThreshold"].as<float>();
					}
				);

				DeserializeComponent<CircleCollider2DComponent>(deserializedEntity, "CircleCollider2DComponent", entity,
					[](auto& cc2d, auto& entity, auto& circleCollider2DComponent) {
						cc2d.Offset = circleCollider2DComponent["Offset"].as<glm::vec2>();
						cc2d.Radius = circleCollider2DComponent["Radius"].as<float>();
						cc2d.Density = circleCollider2DComponent["Density"].as<float>();
						cc2d.Friction = circleCollider2DComponent["Friction"].as<float>();
						cc2d.Restitution = circleCollider2DComponent["Restitution"].as<float>();
						cc2d.RestitutionThreshold = circleCollider2DComponent["RestitutionThreshold"].as<float>();
					}
				);
				DeserializeComponent<ScriptComponent>(deserializedEntity, "ScriptComponent", entity,
					[&entityFields](auto& sc, auto& entity, YAML::Node& scriptComponent) {
						sc.Name = scriptComponent["Name"].as<std::string>();
						if (scriptComponent["ScriptFields"]) {
							Ref<ScriptClass> scriptClass = ScriptEngine::GetEntityClass(sc.Name);
							if (!scriptClass)
								return;
							for (auto v : scriptComponent["ScriptFields"]) {
								auto name = v["Name"].as<std::string>();
								if (scriptClass->GetFields().find(name) == scriptClass->GetFields().end())
									continue;
								auto type = ScriptFieldTypeFromString(v["Type"].as<std::string>());
#define FIELD_DATA_SERILIZATION(T,Type) \
case ScriptFieldType::##T:\
	ScriptEngine::AddSerializedField<Type>(entity.GetUUID(), name,\
		type, ScriptEngine::GetEntityClass(sc.Name), v["Data"].as<Type>());\
	break
								switch (type)
								{
									FIELD_DATA_SERILIZATION(Float,float);
								}
								
							}
						}
					});

			}
		}

		auto view = m_Scene->m_Registry.view<ChildComponent>();
		for (auto e : view) {
			Entity childEntity{ e,m_Scene.get() };
			auto& parentID = childEntity.GetComponent<ChildComponent>().Parent;
			m_Scene->GetEntity(parentID).AddChild(childEntity.GetUUID());
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
