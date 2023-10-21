#include "KDpch.h"
#include "Kaidel/Core/UUID.h"
#include "Kaidel/Scene/Entity.h"
#include "Kaidel/Scene/Scene.h"
#include "Kaidel/Core/Console.h"
#include "Kaidel/Math/Math.h"
#include "ScriptEngine.h"
#include "ScriptRegistry.h"
#include "Kaidel/Core/Input.h"
#include "mono/metadata/assembly.h"
#include "mono/metadata/reflection.h"
#include "mono/jit/jit.h"
#include <glm/glm.hpp>

#include <box2d/b2_body.h>
#include <box2d/b2_polygon_shape.h>
#include <box2d/b2_circle_shape.h>

//TODO : Add local Rotation.

namespace std {
	template<>
	struct hash<glm::vec2>
	{
		size_t operator()(const glm::vec2& _Keyval) const {
			size_t hash = 17;
			hash = hash * 23 + std::hash<float>()(_Keyval.x);
			hash = hash * 23 + std::hash<float>()(_Keyval.y);
			return hash;
		}
	};
	template<>
	struct hash<glm::vec3>
	{
		size_t operator()(const glm::vec3& _Keyval) const {
			size_t hash = 17;
			hash = hash * 23 + std::hash<float>()(_Keyval.x);
			hash = hash * 23 + std::hash<float>()(_Keyval.y);
			hash = hash * 23 + std::hash<float>()(_Keyval.z);
			return hash;
		}
	};
	template<>
	struct hash<glm::vec4>
	{
		size_t operator()(const glm::vec4& _Keyval) const {
			size_t hash = 17;
			hash = hash * 23 + std::hash<float>()(_Keyval.x);
			hash = hash * 23 + std::hash<float>()(_Keyval.y);
			hash = hash * 23 + std::hash<float>()(_Keyval.z);
			hash = hash * 23 + std::hash<float>()(_Keyval.w);
			return hash;
		}
	};
}
namespace Kaidel {
	std::unordered_map<MonoType*, std::function<bool(Entity)>> s_HasComponentFuncs;
	std::unordered_map<MonoType*, std::function<void(Entity)>> s_AddComponentFuncs;



	struct ScriptMaps {

		std::unordered_map<glm::vec2,float> Vector2LengthMap;
		std::unordered_map<glm::vec3,float> Vector3LengthMap;
		std::unordered_map<glm::vec4,float> Vector4LengthMap;
	};


	static ScriptMaps s_ScriptMaps;


#define KD_ADD_INTERNAL(Name) mono_add_internal_call("KaidelCore.Internals::"#Name,&Name)
	static Entity GetEntity(UUID id) {
		Scene* scene = ScriptEngine::GetSceneContext();
		return scene->GetEntity(id);
	}

	template<typename T>
	static T& GetComponent(UUID id) {
		return GetEntity(id).GetComponent<T>();
	}

	static void NativeLog(MonoString* string) {
		char* cstr = mono_string_to_utf8(string);
		std::string s(cstr);
		mono_free(cstr);
		KD_CORE_INFO("{}", s);
	}
	static bool Entity_HasParent(UUID id) {
		Scene* scene = ScriptEngine::GetSceneContext();
		auto entity = scene->GetEntity(id);
		return entity.HasComponent<ChildComponent>();
	}
	static bool Entity_HasChildren(UUID id) {
		Scene* scene = ScriptEngine::GetSceneContext();
		auto entity = scene->GetEntity(id);
		return entity.HasComponent<ParentComponent>() && !entity.GetComponent<ParentComponent>().Children.empty();
	}
	static void Entity_SetActive(UUID id, bool isActive) {
		GetComponent<IDComponent>(id).IsActive = isActive;
	}
	static bool Entity_GetActive(UUID id) {
		return GetComponent<IDComponent>(id).IsActive;
	}
	static void Entity_SetVisible(UUID id, bool isVisible) {
		GetComponent<IDComponent>(id).IsVisible= isVisible;
	}
	static bool Entity_GetVisible(UUID id) {
		return GetComponent<IDComponent>(id).IsVisible;
	}
	
	static MonoString* Entity_GetName(UUID id) {
		Scene* scene = ScriptEngine::GetSceneContext();
		Entity entity = scene->GetEntity(id);
		auto& name = entity.GetComponent<TagComponent>().Tag;
		MonoString* res = mono_string_new(mono_domain_get(), name.c_str());
		return res;
	}
	static bool Entity_HasComponent(UUID id, MonoReflectionType* componentType) {
		Scene* scene = ScriptEngine::GetSceneContext();
		Entity entity = scene->GetEntity(id);
		MonoType* managedType = mono_reflection_type_get_type(componentType);
		KD_CORE_ASSERT(entity, "Entity Doesn't Exist");
		KD_CORE_ASSERT(s_HasComponentFuncs.find(managedType) != s_HasComponentFuncs.end(), "Component Is not Managed");
		return s_HasComponentFuncs.at(managedType)(entity);
	}
	static void Entity_AddComponent(UUID id, MonoReflectionType* componentType) {
		Scene* scene = ScriptEngine::GetSceneContext();
		Entity entity = scene->GetEntity(id);
		MonoType* managedType = mono_reflection_type_get_type(componentType);
		KD_CORE_ASSERT(entity, "Entity Doesn't Exist");
		KD_CORE_ASSERT(s_AddComponentFuncs.find(managedType) != s_AddComponentFuncs.end(), "Component Is not Managed");
		return s_AddComponentFuncs.at(managedType)(entity);
	}
	static UUID Entity_GetParentID(UUID ID)
	{
		auto& cc = GetComponent<ChildComponent>(ID);
		return cc.Parent;
	}
	static UUID Entity_GetChildEntityIDWithIndex(UUID ID, uint32_t index)
	{
		auto& pc = GetComponent<ParentComponent>(ID);
		return pc.Children[index];
	}
	static UUID Entity_GetChildEntityIDWithName(UUID ID, MonoString* name)
	{
		char* cStr = mono_string_to_utf8(name);
		std::string str(cStr);
		mono_free(cStr);
		auto& pc = GetComponent<ParentComponent>(ID);
		for (auto& entity : pc.Children) {
			if (GetComponent<TagComponent>(entity).Tag == str)
				return entity;
		}
		KD_CORE_ASSERT(false);
		return { 0 };
	}
	static uint64_t Entity_FindEntityByName(MonoString* name) {
		char* nameCStr = mono_string_to_utf8(name);
		Scene* scene = ScriptEngine::GetSceneContext();;
		auto entity = scene->FindEntityByName(nameCStr);
		mono_free(nameCStr);
		if (!entity)
			return 0;
		return (uint64_t)entity.GetUUID();
	}
	static void TransformComponent_GetPosition(UUID id, glm::vec3* outPos) {
		auto& pos = GetComponent<TransformComponent>(id).Translation;
		memcpy(outPos, &pos, sizeof(glm::vec3));
	}
	static void TransformComponent_SetPosition(UUID id, glm::vec3* setPos) {
		auto& pos = GetComponent<TransformComponent>(id).Translation;
		memcpy(&pos, setPos, sizeof(glm::vec3));
	}

	static void TransformComponent_GetRotation(UUID id, glm::vec3* outRos) {
		auto& rot = GetComponent<TransformComponent>(id).Rotation;
		memcpy(outRos, &rot, sizeof(glm::vec3));
	}
	static void TransformComponent_SetRotation(UUID id, glm::vec3* setRot) {
		auto& rot = GetComponent<TransformComponent>(id).Rotation;
		memcpy(&rot, setRot, sizeof(glm::vec3));
	}

	static void TransformComponent_GetScale(UUID id, glm::vec3* outScale) {
		auto& scale = GetComponent<TransformComponent>(id).Scale;
		memcpy(outScale, &scale, sizeof(glm::vec3));
	}
	static void TransformComponent_SetScale(UUID id, glm::vec3* setScale) {
		auto& scale = GetComponent<TransformComponent>(id).Scale;
		memcpy(&scale, setScale, sizeof(glm::vec3));
	}
	static void TransformComponent_GetLocalPosition(UUID id, glm::vec3* outLocalPosition) {
		Entity entity = GetEntity(id);
		if (!entity.HasComponent<ChildComponent>()) {
			*outLocalPosition = { 0,0,0 };
			return;
		}
		auto& localPosition = entity.GetComponent<ChildComponent>().LocalPosition;
		memcpy(outLocalPosition, &localPosition, sizeof(glm::vec3));
	}
	static void TransformComponent_SetLocalPosition(UUID id, glm::vec3* setLocalPosition) {
		Entity entity = GetEntity(id);
		if (!entity.HasComponent<ChildComponent>()) {
			auto& pos = entity.GetComponent<TransformComponent>().Translation;
			MoveEntity(entity,ScriptEngine::GetSceneContext(), *setLocalPosition - pos,{0,0,0});
			return;
		}
		auto& localPosition = entity.GetComponent<ChildComponent>().LocalPosition;
		glm::vec3 delta = *setLocalPosition - localPosition;
		MoveEntity(entity, ScriptEngine::GetSceneContext(), delta, { 0,0,0 });
	}
	static void TransformComponent_GetLocalRotation(UUID id, glm::vec3* outLocalRotation) {
		Entity entity = GetEntity(id);
		if (!entity.HasComponent<ChildComponent>()) {
			*outLocalRotation = { 0,0,0 };
			return;
		}
		auto& localRotation = entity.GetComponent<ChildComponent>().LocalRotation;
		memcpy(outLocalRotation, &localRotation, sizeof(glm::vec3));
	}
	static void TransformComponent_SetLocalRotation(UUID id, glm::vec3* setLocalRotation) {
		Entity entity = GetEntity(id);
		if (!entity.HasComponent<ChildComponent>()) {
			auto& rot = entity.GetComponent<TransformComponent>().Rotation;
			MoveEntity(entity, ScriptEngine::GetSceneContext(), { 0,0,0 }, *setLocalRotation - rot);
			return;
		}
		auto& localRotation = entity.GetComponent<ChildComponent>().LocalRotation;
		glm::vec3 delta = *setLocalRotation - localRotation;
		MoveEntity(entity, ScriptEngine::GetSceneContext(), { 0,0,0 }, delta);
	}
	static void TransformComponent_RotateAround(UUID id, UUID parentID, glm::vec3* rotation) {
		Scene* scene = ScriptEngine::GetSceneContext();
		Math::Rotate(scene->GetEntity(id), scene->GetEntity(parentID), *rotation);
	}
	static void SpriteRendererComponent_GetColor(UUID id, glm::vec4* outColor) {
		auto& color = GetComponent<SpriteRendererComponent>(id).Color;
		memcpy(outColor, &color, sizeof(glm::vec4));
	}
	static void SpriteRendererComponent_SetColor(UUID id, glm::vec4* setColor) {
		auto& color = GetComponent<SpriteRendererComponent>(id).Color;
		memcpy(&color, setColor, sizeof(glm::vec4));
	}
	static void CircleRendererComponent_GetColor(UUID id, glm::vec4* outColor) {
		auto& crc = GetComponent<CircleRendererComponent>(id);
		memcpy(outColor, &crc.Color, sizeof(glm::vec4));
	}
	static void CircleRendererComponent_SetColor(UUID id, glm::vec4* setColor) {
		auto& crc = GetComponent<CircleRendererComponent>(id);
		memcpy(&crc.Color, setColor, sizeof(glm::vec4));
	}
	static float CircleRendererComponent_GetThickness(UUID id) {
		auto& crc = GetComponent<CircleRendererComponent>(id);
		return crc.Thickness;
	}
	static void CircleRendererComponent_SetThickness(UUID id, float setThickness) {
		auto& crc = GetComponent<CircleRendererComponent>(id);
		crc.Thickness = setThickness;
	}
	static float CircleRendererComponent_GetFade(UUID id) {
		auto& crc = GetComponent<CircleRendererComponent>(id);
		return crc.Fade;
	}
	static void CircleRendererComponent_SetFade(UUID id, float setFade) {
		auto& crc = GetComponent<CircleRendererComponent>(id);
		crc.Fade = setFade;
	}
	static void Rigidbody2DComponent_ApplyLinearImpulse(UUID id, glm::vec2* impulse, glm::vec2* point, bool wake) {
		auto& rb2d = GetComponent<Rigidbody2DComponent>(id);
		auto body = (b2Body*)rb2d.RuntimeBody;
		body->ApplyLinearImpulse(*(b2Vec2*)impulse, *(b2Vec2*)point, wake);
	}
	static void Rigidbody2DComponent_ApplyForce(UUID id, glm::vec2* force, glm::vec2* point, bool wake) {
		auto& rb2d = GetComponent<Rigidbody2DComponent>(id);
		auto body = (b2Body*)rb2d.RuntimeBody;
		body->ApplyForce(*(b2Vec2*)force, *(b2Vec2*)point, wake);
	}
	static int Rigidbody2DComponent_GetBodyType(UUID id) {
		auto& rb2d = GetComponent<Rigidbody2DComponent>(id);
		return (int)rb2d.Type;
	}
	static bool Rigidbody2DComponent_GetFixedRotation(UUID id) {
		auto& rb2d = GetComponent<Rigidbody2DComponent>(id);
		return rb2d.FixedRotation;
	}
	static void Rigidbody2DComponent_SetFixedRotation(UUID id, bool value) {
		auto& rb2d = GetComponent<Rigidbody2DComponent>(id);
		rb2d.FixedRotation = value;
	}
	static void BoxCollider2DComponent_GetOffset(UUID id, glm::vec2* outOffset) {
		auto& bc2d = GetComponent<BoxCollider2DComponent>(id);
		memcpy(outOffset, &bc2d.Offset, sizeof(glm::vec2));
	}
	static void BoxCollider2DComponent_SetOffset(UUID id, glm::vec2* setOffset) {
		auto& bc2d = GetComponent<BoxCollider2DComponent>(id);
		memcpy(&bc2d.Offset, setOffset, sizeof(glm::vec2));
	}
	static void BoxCollider2DComponent_GetSize(UUID id, glm::vec2* outSize) {
		auto& bc2d = GetComponent<BoxCollider2DComponent>(id);
		memcpy(outSize, &bc2d.Size, sizeof(glm::vec2));
	}
	static void BoxCollider2DComponent_SetSize(UUID id, glm::vec2* setSize) {
		auto& bc2d = GetComponent<BoxCollider2DComponent>(id);
		memcpy(&bc2d.Size, setSize, sizeof(glm::vec2));
	}
	static float BoxCollider2DComponent_GetDensity(UUID id) {
		auto& bc2d = GetComponent<BoxCollider2DComponent>(id);
		return bc2d.Density;
	}
	static void BoxCollider2DComponent_SetDensity(UUID id, float setDensity) {
		auto& bc2d = GetComponent<BoxCollider2DComponent>(id);
		bc2d.Density = setDensity;
	}
	static float BoxCollider2DComponent_GetFriction(UUID id) {
		auto& bc2d = GetComponent<BoxCollider2DComponent>(id);
		return bc2d.Friction;
	}
	static void BoxCollider2DComponent_SetFriction(UUID id, float setFriction) {
		auto& bc2d = GetComponent<BoxCollider2DComponent>(id);
		bc2d.Friction = setFriction;
	}
	static float BoxCollider2DComponent_GetRestitution(UUID id) {
		auto& bc2d = GetComponent<BoxCollider2DComponent>(id);
		return bc2d.Restitution;
	}
	static void BoxCollider2DComponent_SetRestitution(UUID id, float setRestitution) {
		auto& bc2d = GetComponent<BoxCollider2DComponent>(id);
		bc2d.Restitution = setRestitution;
	}
	static float BoxCollider2DComponent_GetRestitutionThreshold(UUID id) {
		auto& bc2d = GetComponent<BoxCollider2DComponent>(id);
		return bc2d.RestitutionThreshold;
	}
	static void BoxCollider2DComponent_SetRestitutionThreshold(UUID id, float setRestitutionThreshold) {
		auto& bc2d = GetComponent<BoxCollider2DComponent>(id);
		bc2d.RestitutionThreshold = setRestitutionThreshold;
	}
	static void CircleCollider2DComponent_GetOffset(UUID id, glm::vec2* outOffset) {
		auto& cc2D = GetComponent<CircleCollider2DComponent>(id);
		memcpy(outOffset, &cc2D.Offset, sizeof(glm::vec2));
	}
	static void CircleCollider2DComponent_SetOffset(UUID id, glm::vec2* setOffset) {
		auto& cc2d = GetComponent<CircleCollider2DComponent>(id);
		memcpy(&cc2d.Offset, setOffset, sizeof(glm::vec2));
	}
	static float CircleCollider2DComponent_GetRadius(UUID id) {
		auto& cc2d = GetComponent<CircleCollider2DComponent>(id);
		return cc2d.Radius;
	}
	static void CircleCollider2DComponent_SetRadius(UUID id, float setRadius) {
		auto& cc2d = GetComponent<CircleCollider2DComponent>(id);
		cc2d.Radius = setRadius;
	}
	static float CircleCollider2DComponent_GetDensity(UUID id) {
		auto& cc2d = GetComponent<CircleCollider2DComponent>(id);
		return cc2d.Density;
	}
	static void CircleCollider2DComponent_SetDensity(UUID id, float setDensity) {
		auto& cc2d = GetComponent<CircleCollider2DComponent>(id);
		cc2d.Density = setDensity;
	}
	static float CircleCollider2DComponent_GetFriction(UUID id) {
		auto& cc2d = GetComponent<CircleCollider2DComponent>(id);
		return cc2d.Friction;
	}
	static void CircleCollider2DComponent_SetFriction(UUID id, float setFriction) {
		auto& cc2d = GetComponent<CircleCollider2DComponent>(id);
		cc2d.Friction = setFriction;
	}
	static float CircleCollider2DComponent_GetRestitution(UUID id) {
		auto& cc2d = GetComponent<CircleCollider2DComponent>(id);
		return cc2d.Restitution;
	}
	static void CircleCollider2DComponent_SetRestitution(UUID id, float setRestitution) {
		auto& cc2d = GetComponent<CircleCollider2DComponent>(id);
		cc2d.Restitution = setRestitution;
	}

	static float CircleCollider2DComponent_GetRestitutionThreshold(UUID id) {
		auto& cc2d = GetComponent<CircleCollider2DComponent>(id);
		return cc2d.RestitutionThreshold;
	}
	static void CircleCollider2DComponent_SetRestitutionThreshold(UUID id, float setRestitutionThreshold) {
		auto& cc2d = GetComponent<CircleCollider2DComponent>(id);
		cc2d.RestitutionThreshold = setRestitutionThreshold;
	}



	static bool Input_IsKeyDown(KeyCode* keyCode) {
		return Input::IsKeyDown(*keyCode);
	}
	static bool Input_IsMouseDown(MouseCode* mouseCode) {
		return Input::IsMouseButtonDown(*mouseCode);
	}

	static MonoObject* Instance_GetScriptInstance(UUID id) {
		return ScriptEngine::GetEntityScriptInstance(id)->GetInstance();
	}
#pragma region Vector2

	static void Vector2_AddVec(glm::vec2* a, glm::vec2* b, glm::vec2* outRes) {
		*outRes = *a + *b;
	}
	static void Vector2_SubtractVec(glm::vec2* a, glm::vec2* b, glm::vec2* outRes) {
		*outRes = *a - *b;
	}
	static void Vector2_AddNum(glm::vec2* a, float b, glm::vec2* outRes) {
		*outRes = *a + b;
	}
	static void Vector2_SubtractNum(glm::vec2* a, float b, glm::vec2* outRes) {
		*outRes = *a - b;
	}
	static void Vector2_MultNum(glm::vec2* a, float b, glm::vec2* outRes) {
		*outRes = (*a) * b;
	}
	static void Vector2_MultVec(glm::vec2* a, glm::vec2* b, glm::vec2* outRes) {
		*outRes = (*a) * (*b);
	}
	static void Vector2_DivNum(glm::vec2* a, float b, glm::vec2* outRes) {
		*outRes = (*a) / b;
	}
	static void Vector2_DivVec(glm::vec2* a, glm::vec2* b, glm::vec2* outRes) {

		*outRes = (*a) / (*b);
	}
	static float Vector2_DotVec(glm::vec2* a, glm::vec2* b) {
		return glm::dot(*a, *b);
	}
	static float Vector2_LengthVec(glm::vec2* v) {
		glm::vec2& vector = *v;
		auto it = s_ScriptMaps.Vector2LengthMap.find(vector);
		if (it != s_ScriptMaps.Vector2LengthMap.end())
		{
			return static_cast<float>(it->second);
		}
		return s_ScriptMaps.Vector2LengthMap[vector] = glm::length(vector);
	}
#pragma endregion
#pragma region Vector3
	static void Vector3_AddVec(glm::vec3* a, glm::vec3* b, glm::vec3* outRes) {
		*outRes = *a + *b;
	}
	static void Vector3_SubtractVec(glm::vec3* a, glm::vec3* b, glm::vec3* outRes) {
		*outRes = *a - *b;
	}
	static void Vector3_AddNum(glm::vec3* a, float b, glm::vec3* outRes) {
		*outRes = *a + b;
	}
	static void Vector3_SubtractNum(glm::vec3* a, float b, glm::vec3* outRes) {
		*outRes = *a - b;
	}
	static void Vector3_MultNum(glm::vec3* a, float b, glm::vec3* outRes) {
		*outRes = (*a) * b;
	}
	static void Vector3_MultVec(glm::vec3* a, glm::vec3* b, glm::vec3* outRes) {
		*outRes = (*a) * (*b);
	}
	static void Vector3_DivNum(glm::vec3* a, float b, glm::vec3* outRes) {
		*outRes = (*a) / b;
	}
	static void Vector3_DivVec(glm::vec3* a, glm::vec3* b, glm::vec3* outRes) {

		*outRes = (*a) / (*b);
	}
	static float Vector3_DotVec(glm::vec3* a, glm::vec3* b) {
		return glm::dot(*a, *b);
	}
	static void Vector3_CrossVec(glm::vec3* a, glm::vec3* b, glm::vec3* outRes) {
		*outRes = glm::cross(*a, *b);
	}
	static float Vector3_LengthVec(glm::vec3* v) {
		glm::vec3& vector = *v;
		auto it = s_ScriptMaps.Vector3LengthMap.find(vector);
		if (it != s_ScriptMaps.Vector3LengthMap.end())
		{
			return static_cast<float>(it->second);
		}
		return s_ScriptMaps.Vector3LengthMap[vector] = glm::length(vector);
	}
	static void Vector3_RotateAround(glm::vec3* a, glm::vec3* origin, glm::vec3* angle, glm::vec3* outRes) {
		glm::vec4 res = Math::Rotate(*origin, glm::radians(*angle))*(glm::vec4(*a,1.0f));
		memcpy(outRes, &res, sizeof(glm::vec3));
	}
#pragma endregion
#pragma region Vector4

	static void Vector4_AddVec(glm::vec4* a, glm::vec4* b, glm::vec4* outRes) {
		*outRes = *a + *b;
	}
	static void Vector4_SubtractVec(glm::vec4* a, glm::vec4* b, glm::vec4* outRes) {
		*outRes = *a - *b;
	}
	static void Vector4_AddNum(glm::vec4* a, float b, glm::vec4* outRes) {
		*outRes = *a + b;
	}
	static void Vector4_SubtractNum(glm::vec4* a, float b, glm::vec4* outRes) {
		*outRes = *a - b;
	}
	static void Vector4_MultNum(glm::vec4* a, float b, glm::vec4* outRes) {
		*outRes = (*a) * b;
	}
	static void Vector4_MultVec(glm::vec4* a, glm::vec4* b, glm::vec4* outRes) {
		*outRes = (*a) * (*b);
	}
	static void Vector4_DivNum(glm::vec4* a, float b, glm::vec4* outRes) {
		*outRes = (*a) / b;
	}
	static void Vector4_DivVec(glm::vec4* a, glm::vec4* b, glm::vec4* outRes) {

		*outRes = (*a) / (*b);
	}
	static float Vector4_DotVec(glm::vec4* a, glm::vec4* b) {
		return glm::dot(*a, *b);
	}
	static float Vector4_LengthVec(glm::vec4* v) {
		glm::vec4& vector = *v;
		auto it = s_ScriptMaps.Vector4LengthMap.find(vector);
		if (it != s_ScriptMaps.Vector4LengthMap.end())
		{
			return static_cast<float>(it->second);
		}
		return s_ScriptMaps.Vector4LengthMap[vector] = glm::length(vector);
	}
#pragma endregion
	
	static std::unordered_map<UUID, Console> s_ConsolesMap;
	static uint64_t Console_InitNewConsole()
	{
		UUID id;
		s_ConsolesMap.insert({ id,Console{} });
		return (uint64_t)id;
	}
	static void Console_DestroyConsole(UUID id)
	{
		s_ConsolesMap.erase(id);
	}
	static void Console_MessageLog(UUID id, MonoString* s)
	{
		KD_CORE_ASSERT(s_ConsolesMap.find(id) != s_ConsolesMap.end());
		char* cstr = mono_string_to_utf8(s);
		std::string string(cstr);
		mono_free(cstr);
		s_ConsolesMap.at(id).Log(Message{cstr,MessageLevel::Log});
	}
	static void Console_MessageStaticLog(MonoString* s)
	{
		char* cstr = mono_string_to_utf8(s);
		std::string string(cstr);
		mono_free(cstr);
		KD_TRACE(string);
	}
	static void Console_MessageInfo(UUID id, MonoString* s)
	{
		KD_CORE_ASSERT(s_ConsolesMap.find(id) != s_ConsolesMap.end());
		char* cstr = mono_string_to_utf8(s);
		std::string string(cstr);
		mono_free(cstr);
		s_ConsolesMap.at(id).Log(Message{ cstr,MessageLevel::Info });

	}
	static void Console_MessageStaticInfo(MonoString* s)
	{
		char* cstr = mono_string_to_utf8(s);
		std::string string(cstr);
		mono_free(cstr);
		KD_INFO(string);
	}
	static void Console_MessageWarn(UUID id, MonoString* s)
	{
		KD_CORE_ASSERT(s_ConsolesMap.find(id) != s_ConsolesMap.end());
		char* cstr = mono_string_to_utf8(s);
		std::string string(cstr);
		mono_free(cstr);
		s_ConsolesMap.at(id).Log(Message{ cstr,MessageLevel::Warn });
	}
	static void Console_MessageStaticWarn(MonoString* s)
	{
		char* cstr = mono_string_to_utf8(s);
		std::string string(cstr);
		mono_free(cstr);
		KD_WARN(string);
	}
	static void Console_MessageError(UUID id, MonoString* s)
	{
		KD_CORE_ASSERT(s_ConsolesMap.find(id) != s_ConsolesMap.end());
		char* cstr = mono_string_to_utf8(s);
		std::string string(cstr);
		mono_free(cstr);
		s_ConsolesMap.at(id).Log(Message{ cstr,MessageLevel::Error });
	}
	static void Console_MessageStaticError(MonoString* s)
	{
		char* cstr = mono_string_to_utf8(s);
		std::string string(cstr);
		mono_free(cstr);
		KD_ERROR(string);
	}

	template<typename Component>
	static void RegisterComponent() {
		std::string_view typeName= typeid(Component).name();
		size_t pos = typeName.find_last_of(':');
		std::string_view componentName = typeName.substr(pos+1);
		std::string managedTypeName = fmt::format("KaidelCore.{}", componentName);
		MonoType* managedType = mono_reflection_type_from_name(managedTypeName.data(), ScriptEngine::GetCoreAssemblyImage());
		s_HasComponentFuncs[managedType] = [](Entity entity) {return entity.HasComponent<Component>(); };
		s_AddComponentFuncs[managedType] = [](Entity entity) {entity.AddComponent<Component>(); };
	}
	void ScriptRegistry::RegisterComponents()
	{
		RegisterComponent<TransformComponent>();
		RegisterComponent<SpriteRendererComponent>();
		RegisterComponent<Rigidbody2DComponent>();
		//RegisterComponent<BoxCollider2DComponent>();
	}
	
	void ScriptRegistry::RegisterFunctions()
	{
		#pragma region Entity
		KD_ADD_INTERNAL(Entity_HasParent);
		KD_ADD_INTERNAL(Entity_HasChildren);
		KD_ADD_INTERNAL(Entity_GetName);
		KD_ADD_INTERNAL(Entity_HasComponent);
		KD_ADD_INTERNAL(Entity_AddComponent);
		KD_ADD_INTERNAL(Entity_GetParentID);
		KD_ADD_INTERNAL(Entity_GetChildEntityIDWithIndex);
		KD_ADD_INTERNAL(Entity_GetChildEntityIDWithName);
		KD_ADD_INTERNAL(Entity_FindEntityByName);
		KD_ADD_INTERNAL(Entity_SetActive);
		KD_ADD_INTERNAL(Entity_GetActive);
		KD_ADD_INTERNAL(Entity_SetVisible);
		KD_ADD_INTERNAL(Entity_GetVisible);
#pragma endregion
		#pragma region Components
		#pragma region TransformComponent
		KD_ADD_INTERNAL(TransformComponent_GetPosition);
		KD_ADD_INTERNAL(TransformComponent_SetPosition);
		KD_ADD_INTERNAL(TransformComponent_GetRotation);
		KD_ADD_INTERNAL(TransformComponent_SetRotation);
		KD_ADD_INTERNAL(TransformComponent_GetScale);
		KD_ADD_INTERNAL(TransformComponent_SetScale);
		KD_ADD_INTERNAL(TransformComponent_GetLocalPosition);
		KD_ADD_INTERNAL(TransformComponent_SetLocalPosition);
		KD_ADD_INTERNAL(TransformComponent_GetLocalRotation);
		KD_ADD_INTERNAL(TransformComponent_SetLocalRotation);

#pragma endregion
		#pragma region SpriteRendererComponent
		KD_ADD_INTERNAL(SpriteRendererComponent_GetColor);
		KD_ADD_INTERNAL(SpriteRendererComponent_SetColor);
#pragma endregion
		#pragma region CircleRendererComponent
		KD_ADD_INTERNAL(CircleRendererComponent_GetColor);
		KD_ADD_INTERNAL(CircleRendererComponent_SetColor);
		KD_ADD_INTERNAL(CircleRendererComponent_GetThickness);
		KD_ADD_INTERNAL(CircleRendererComponent_SetThickness);
		KD_ADD_INTERNAL(CircleRendererComponent_GetFade);
		KD_ADD_INTERNAL(CircleRendererComponent_SetFade);
#pragma endregion
		#pragma endregion
		#pragma region Physics
		#pragma region Rigidbody2DComponent
		KD_ADD_INTERNAL(Rigidbody2DComponent_ApplyLinearImpulse);
		KD_ADD_INTERNAL(Rigidbody2DComponent_ApplyForce);
		KD_ADD_INTERNAL(Rigidbody2DComponent_GetBodyType);
		KD_ADD_INTERNAL(Rigidbody2DComponent_GetFixedRotation);
		KD_ADD_INTERNAL(Rigidbody2DComponent_SetFixedRotation);
#pragma endregion
		#pragma region BoxCollider2DComponent
		KD_ADD_INTERNAL(BoxCollider2DComponent_GetOffset);
		KD_ADD_INTERNAL(BoxCollider2DComponent_SetOffset);
		KD_ADD_INTERNAL(BoxCollider2DComponent_GetSize);
		KD_ADD_INTERNAL(BoxCollider2DComponent_SetSize);	
		KD_ADD_INTERNAL(BoxCollider2DComponent_GetDensity);
		KD_ADD_INTERNAL(BoxCollider2DComponent_SetDensity);
		KD_ADD_INTERNAL(BoxCollider2DComponent_GetFriction);
		KD_ADD_INTERNAL(BoxCollider2DComponent_SetFriction);
		KD_ADD_INTERNAL(BoxCollider2DComponent_GetRestitution);
		KD_ADD_INTERNAL(BoxCollider2DComponent_SetRestitution);
		KD_ADD_INTERNAL(BoxCollider2DComponent_GetRestitutionThreshold);
		KD_ADD_INTERNAL(BoxCollider2DComponent_SetRestitutionThreshold);
#pragma endregion
		#pragma region CircleCollider2DComponent
		KD_ADD_INTERNAL(CircleCollider2DComponent_GetOffset);
		KD_ADD_INTERNAL(CircleCollider2DComponent_SetOffset);
		KD_ADD_INTERNAL(CircleCollider2DComponent_GetRadius);
		KD_ADD_INTERNAL(CircleCollider2DComponent_SetRadius);
		KD_ADD_INTERNAL(CircleCollider2DComponent_GetDensity);
		KD_ADD_INTERNAL(CircleCollider2DComponent_SetDensity);
		KD_ADD_INTERNAL(CircleCollider2DComponent_GetFriction);
		KD_ADD_INTERNAL(CircleCollider2DComponent_SetFriction);
		KD_ADD_INTERNAL(CircleCollider2DComponent_GetRestitution);
		KD_ADD_INTERNAL(CircleCollider2DComponent_SetRestitution);
		KD_ADD_INTERNAL(CircleCollider2DComponent_GetRestitutionThreshold);
		KD_ADD_INTERNAL(CircleCollider2DComponent_SetRestitutionThreshold);
#pragma endregion
		#pragma endregion
		#pragma region Math
		#pragma region Vector2
		KD_ADD_INTERNAL(Vector2_AddVec);
		KD_ADD_INTERNAL(Vector2_SubtractVec);
		KD_ADD_INTERNAL(Vector2_AddNum);
		KD_ADD_INTERNAL(Vector2_SubtractNum);
		KD_ADD_INTERNAL(Vector2_MultNum);
		KD_ADD_INTERNAL(Vector2_MultVec);
		KD_ADD_INTERNAL(Vector2_DivNum);
		KD_ADD_INTERNAL(Vector2_DivVec);
		KD_ADD_INTERNAL(Vector2_DotVec);
		KD_ADD_INTERNAL(Vector2_LengthVec);
#pragma endregion
		#pragma region Vector3
		KD_ADD_INTERNAL(Vector3_AddVec);
		KD_ADD_INTERNAL(Vector3_SubtractVec);
		KD_ADD_INTERNAL(Vector3_AddNum);
		KD_ADD_INTERNAL(Vector3_SubtractNum);
		KD_ADD_INTERNAL(Vector3_MultNum);
		KD_ADD_INTERNAL(Vector3_MultVec);
		KD_ADD_INTERNAL(Vector3_DivNum);
		KD_ADD_INTERNAL(Vector3_DivVec);
		KD_ADD_INTERNAL(Vector3_DotVec);
		KD_ADD_INTERNAL(Vector3_CrossVec);
		KD_ADD_INTERNAL(Vector3_LengthVec);
		KD_ADD_INTERNAL(Vector3_RotateAround);
#pragma endregion
		#pragma region Vector4
		KD_ADD_INTERNAL(Vector4_AddVec);
		KD_ADD_INTERNAL(Vector4_SubtractVec);
		KD_ADD_INTERNAL(Vector4_AddNum);
		KD_ADD_INTERNAL(Vector4_SubtractNum);
		KD_ADD_INTERNAL(Vector4_MultNum);
		KD_ADD_INTERNAL(Vector4_MultVec);
		KD_ADD_INTERNAL(Vector4_DivNum);
		KD_ADD_INTERNAL(Vector4_DivVec);
		KD_ADD_INTERNAL(Vector4_DotVec);
		KD_ADD_INTERNAL(Vector4_LengthVec);
#pragma endregion
		#pragma endregion
		#pragma region Misc.
		KD_ADD_INTERNAL(NativeLog);
		KD_ADD_INTERNAL(Input_IsKeyDown);
		KD_ADD_INTERNAL(Instance_GetScriptInstance);
		KD_ADD_INTERNAL(Console_InitNewConsole);
		KD_ADD_INTERNAL(Console_DestroyConsole);
		KD_ADD_INTERNAL(Console_MessageLog);
		KD_ADD_INTERNAL(Console_MessageStaticLog);
		KD_ADD_INTERNAL(Console_MessageInfo);
		KD_ADD_INTERNAL(Console_MessageStaticInfo);
		KD_ADD_INTERNAL(Console_MessageWarn);
		KD_ADD_INTERNAL(Console_MessageStaticWarn);
		KD_ADD_INTERNAL(Console_MessageError);
		KD_ADD_INTERNAL(Console_MessageStaticError);
#pragma endregion
	}
}
