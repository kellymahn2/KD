#include "KDpch.h"
#include "Kaidel/Core/UUID.h"
#include "Kaidel/Scene/Entity.h"
#include "Kaidel/Scene/Scene.h"
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

namespace Kaidel {
	std::unordered_map<MonoType*, std::function<bool(Entity)>> s_HasComponentFuncs;
#define KD_ADD_INTERNAL(Name) mono_add_internal_call("KaidelCore.Internals::"#Name,&Name)

	static void NativeLog(MonoString* string) {
		char* cstr = mono_string_to_utf8(string);
		std::string s(cstr);
		mono_free(cstr);
		KD_CORE_INFO("{}", s);
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
	static void TransformComponent_GetPosition(UUID id, glm::vec3* outPos) {
		Scene* scene = ScriptEngine::GetSceneContext();
		Entity entity = scene->GetEntity(id);
		auto& pos = entity.GetComponent<TransformComponent>().Translation;
		memcpy(outPos, &pos, sizeof(glm::vec3));
	}
	static void TransformComponent_SetPosition(UUID id, glm::vec3* setPos) {
		Scene* scene = ScriptEngine::GetSceneContext();
		Entity entity = scene->GetEntity(id);
		auto& pos = entity.GetComponent<TransformComponent>().Translation;
		memcpy(&pos, setPos, sizeof(glm::vec3));
	}

	static void TransformComponent_GetRotation(UUID id, glm::vec3* outRos) {
		Scene* scene = ScriptEngine::GetSceneContext();
		Entity entity = scene->GetEntity(id);
		auto& rot = entity.GetComponent<TransformComponent>().Rotation;
		memcpy(outRos, &rot, sizeof(glm::vec3));
	}
	static void TransformComponent_SetRotation(UUID id, glm::vec3* setRot) {
		Scene* scene = ScriptEngine::GetSceneContext();
		Entity entity = scene->GetEntity(id);
		auto& rot = entity.GetComponent<TransformComponent>().Rotation;
		memcpy(&rot, setRot, sizeof(glm::vec3));
	}

	static void TransformComponent_GetScale(UUID id, glm::vec3* outScale) {
		Scene* scene = ScriptEngine::GetSceneContext();
		Entity entity = scene->GetEntity(id);
		auto& scale = entity.GetComponent<TransformComponent>().Scale;
		memcpy(outScale, &scale, sizeof(glm::vec3));
	}
	static void TransformComponent_SetScale(UUID id, glm::vec3* setScale) {
		Scene* scene = ScriptEngine::GetSceneContext();
		Entity entity = scene->GetEntity(id);
		auto& scale = entity.GetComponent<TransformComponent>().Scale;
		memcpy(&scale, setScale, sizeof(glm::vec3));
	}
	static void SpriteRendererComponent_GetColor(UUID id, glm::vec4* outColor) {
		Scene* scene = ScriptEngine::GetSceneContext();
		Entity entity = scene->GetEntity(id);
		auto& color = entity.GetComponent<SpriteRendererComponent>().Color;
		memcpy(outColor, &color,sizeof(glm::vec4));
	}
	static void SpriteRendererComponent_SetColor(UUID id,glm::vec4* setColor){
		Scene* scene = ScriptEngine::GetSceneContext();
		Entity entity = scene->GetEntity(id);
		auto& color = entity.GetComponent<SpriteRendererComponent>().Color;
		memcpy(&color, setColor, sizeof(glm::vec4));
	}
	static void Rigidbody2DComponent_ApplyLinearImpulse(UUID id, glm::vec2* impulse,glm::vec2* point,bool wake) {
		Scene* scene = ScriptEngine::GetSceneContext();
		Entity entity = scene->GetEntity(id);
		auto& rb2d= entity.GetComponent<Rigidbody2DComponent>();
		auto body = (b2Body*)rb2d.RuntimeBody;
		body->ApplyLinearImpulse(*(b2Vec2*)impulse, *(b2Vec2*)point,wake);
	}
	static void Rigidbody2DComponent_ApplyForce(UUID id, glm::vec2* force, glm::vec2* point, bool wake) {
		Scene* scene = ScriptEngine::GetSceneContext();
		Entity entity = scene->GetEntity(id);
		auto& rb2d = entity.GetComponent<Rigidbody2DComponent>();
		auto body = (b2Body*)rb2d.RuntimeBody;
		body->ApplyForce(*(b2Vec2*)force, *(b2Vec2*)point, wake);
	}
	static bool Input_IsKeyDown(KeyCode* keyCode) {
		return Input::IsKeyDown(*keyCode);
	}
	static bool Input_IsMouseDown(MouseCode* mouseCode) {
		return Input::IsMouseButtonDown(*mouseCode);
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
		return v->length();
	}
#pragma endregion
#pragma region Vector3
	static void Vector3_AddVec(glm::vec3* a, glm::vec3* b, glm::vec3* outRes) {
		*outRes = *a + *b;
	}
	static void Vector3_SubtractVec(glm::vec3* a, glm::vec3*b, glm::vec3* outRes) {
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
		return v->length();
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
		return v->length();
	}
#pragma endregion

	template<typename Component>
	static void RegisterComponent() {
		std::string_view typeName= typeid(Component).name();
		size_t pos = typeName.find_last_of(':');
		std::string_view componentName = typeName.substr(pos+1);
		std::string managedTypeName = fmt::format("KaidelCore.{}", componentName);
		MonoType* managedType = mono_reflection_type_from_name(managedTypeName.data(), ScriptEngine::GetCoreAssemblyImage());
		s_HasComponentFuncs[managedType] = [](Entity entity) {return entity.HasComponent<Component>(); };
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
		KD_ADD_INTERNAL(NativeLog);
		KD_ADD_INTERNAL(Entity_GetName);
		KD_ADD_INTERNAL(Entity_HasComponent);
		KD_ADD_INTERNAL(TransformComponent_GetPosition);
		KD_ADD_INTERNAL(TransformComponent_SetPosition);
		KD_ADD_INTERNAL(TransformComponent_GetRotation);
		KD_ADD_INTERNAL(TransformComponent_SetRotation);
		KD_ADD_INTERNAL(TransformComponent_GetScale);
		KD_ADD_INTERNAL(TransformComponent_SetScale);
		KD_ADD_INTERNAL(SpriteRendererComponent_GetColor);
		KD_ADD_INTERNAL(SpriteRendererComponent_SetColor);
		KD_ADD_INTERNAL(Rigidbody2DComponent_ApplyLinearImpulse);
		KD_ADD_INTERNAL(Rigidbody2DComponent_ApplyForce);
		KD_ADD_INTERNAL(Input_IsKeyDown);

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

	}
}
