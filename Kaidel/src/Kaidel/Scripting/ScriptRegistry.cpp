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
	std::unordered_map<MonoType*, std::function<void(Entity)>> s_AddComponentFuncs;
#define KD_ADD_INTERNAL(Name) mono_add_internal_call("KaidelCore.Internals::"#Name,&Name)
	template<typename T>
	static T& GetComponent(UUID id){
		Scene* scene = ScriptEngine::GetSceneContext();
		Entity entity = scene->GetEntity(id);
		return entity.GetComponent<T>();
	}

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
	static void Entity_AddComponent(UUID id, MonoReflectionType* componentType) {
		Scene* scene = ScriptEngine::GetSceneContext();
		Entity entity = scene->GetEntity(id);
		MonoType* managedType = mono_reflection_type_get_type(componentType);
		KD_CORE_ASSERT(entity, "Entity Doesn't Exist");
		KD_CORE_ASSERT(s_AddComponentFuncs.find(managedType) != s_AddComponentFuncs.end(), "Component Is not Managed");
		return s_AddComponentFuncs.at(managedType)(entity);
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

	static void CircleRendererComponent_GetColor(UUID id, glm::vec4* outColor){
		auto& crc = GetComponent<CircleRendererComponent>(id);
		memcpy(outColor,&crc.Color,sizeof(glm::vec4));
	}
	static void CircleRendererComponent_SetColor(UUID id, glm::vec4* setColor){
		auto& crc = GetComponent<CircleRendererComponent>(id);
		memcpy(&crc.Color,setColor,sizeof(glm::vec4));
	}
	static float CircleRendererComponent_GetThickness(UUID id){
		auto& crc = GetComponent<CircleRendererComponent>(id);
		return crc.Thickness;
	}
	static void CircleRendererComponent_SetThickness(UUID id,float setThickness){
		auto& crc = GetComponent<CircleRendererComponent>(id);
		crc.Thickness = setThickness;
	}
	static float CircleRendererComponent_GetFade(UUID id){
		auto& crc = GetComponent<CircleRendererComponent>(id);
		return crc.Fade;
	}
	static void CircleRendererComponent_SetFade(UUID id, float setFade){
		auto& crc = GetComponent<CircleRendererComponent>(id);
		crc.Fade = setFade;
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
	static int Rigidbody2DComponent_GetBodyType(UUID id){
		auto& rb2d = GetComponent<Rigidbody2DComponent>(id);
		return (int)rb2d.Type;
	}
	static bool Rigidbody2DComponent_GetFixedRotation(UUID id){
		auto& rb2d = GetComponent<Rigidbody2DComponent>(id);
		return rb2d.FixedRotation;
	}
	static void Rigidbody2DComponent_SetFixedRotation(UUID id,bool value){
		auto& rb2d = GetComponent<Rigidbody2DComponent>(id);
		rb2d.FixedRotation = value;
	}
	static void BoxCollider2DComponent_GetOffset(UUID id,glm::vec2* outOffset){
		auto& bc2d = GetComponent<BoxCollider2DComponent>(id);
		memcpy(outOffset,&bc2d.Offset,sizeof(glm::vec2));
	}
	static void BoxCollider2DComponent_SetOffset(UUID id,glm::vec2* setOffset){
		auto& bc2d = GetComponent<BoxCollider2DComponent>(id);
		memcpy(&bc2d.Offset,setOffset,sizeof(glm::vec2));
	}
	static void BoxCollider2DComponent_GetSize(UUID id,glm::vec2* outSize){
		auto& bc2d = GetComponent<BoxCollider2DComponent>(id);
		memcpy(outSize,&bc2d.Size,sizeof(glm::vec2));
	}
	static void BoxCollider2DComponent_SetSize(UUID id,glm::vec2* setSize){
		auto& bc2d = GetComponent<BoxCollider2DComponent>(id);
		memcpy(&bc2d.Size,setSize,sizeof(glm::vec2));
	}
	static float BoxCollider2DComponent_GetDensity(UUID id){
		auto& bc2d = GetComponent<BoxCollider2DComponent>(id);
		return bc2d.Density;
	}
	static void BoxCollider2DComponent_SetDensity(UUID id,float setDensity){
		auto& bc2d = GetComponent<BoxCollider2DComponent>(id);
		bc2d.Density = setDensity;
	}
	static float BoxCollider2DComponent_GetFriction(UUID id){
		auto& bc2d = GetComponent<BoxCollider2DComponent>(id);
		return bc2d.Friction;
	}
	static void BoxCollider2DComponent_SetFriction(UUID id,float setFriction){
		auto& bc2d = GetComponent<BoxCollider2DComponent>(id);
		bc2d.Friction = setFriction;
	}
	static float BoxCollider2DComponent_GetRestitution(UUID id){
		auto& bc2d = GetComponent<BoxCollider2DComponent>(id);
		return bc2d.Restitution;
	}
	static void BoxCollider2DComponent_SetRestitution(UUID id,float setRestitution){
		auto& bc2d = GetComponent<BoxCollider2DComponent>(id);
		bc2d.Restitution = setRestitution;
	}
	static float BoxCollider2DComponent_GetRestitutionThreshold(UUID id){
		auto& bc2d = GetComponent<BoxCollider2DComponent>(id);
		return bc2d.RestitutionThreshold;
	}
	static void BoxCollider2DComponent_SetRestitutionThreshold(UUID id,float setRestitutionThreshold){
		auto& bc2d = GetComponent<BoxCollider2DComponent>(id);
		bc2d.RestitutionThreshold = setRestitutionThreshold;
	}
	static void CircleCollider2DComponent_GetOffset(UUID id,glm::vec2* outOffset){
		auto& cc2D = GetComponent<CircleCollider2DComponent>(id);
		memcpy(outOffset,&cc2D.Offset,sizeof(glm::vec2));
	}
	static void CircleCollider2DComponent_SetOffset(UUID id,glm::vec2* setOffset){
		auto& cc2d = GetComponent<CircleCollider2DComponent>(id);
		memcpy(&cc2d.Offset,setOffset,sizeof(glm::vec2));
	}
	static float CircleCollider2DComponent_GetRadius(UUID id){
		auto&cc2d = GetComponent<CircleCollider2DComponent>(id);
		return cc2d.Radius;
	}
	static void CircleCollider2DComponent_SetRadius(UUID id, float setRadius){
		auto&cc2d = GetComponent<CircleCollider2DComponent>(id);
		cc2d.Radius = setRadius;
	}
	static float CircleCollider2DComponent_GetDensity(UUID id){
		auto&cc2d = GetComponent<CircleCollider2DComponent>(id);
		return cc2d.Density;
	}
	static void CircleCollider2DComponent_SetDensity(UUID id, float setDensity){
		auto&cc2d = GetComponent<CircleCollider2DComponent>(id);
		cc2d.Density = setDensity;
	}
	static float CircleCollider2DComponent_GetFriction(UUID id){
		auto&cc2d = GetComponent<CircleCollider2DComponent>(id);
		return cc2d.Friction;
	}
	static void CircleCollider2DComponent_SetFriction(UUID id,float setFriction){
		auto&cc2d = GetComponent<CircleCollider2DComponent>(id);
		cc2d.Friction = setFriction;
	}
	static float CircleCollider2DComponent_GetRestitution(UUID id){
		auto&cc2d = GetComponent<CircleCollider2DComponent>(id);
		return cc2d.Restitution;
	}
	static void CircleCollider2DComponent_SetRestitution(UUID id,float setRestitution){
		auto&cc2d = GetComponent<CircleCollider2DComponent>(id);
		cc2d.Restitution = setRestitution;
	}
	
	static float CircleCollider2DComponent_GetRestitutionThreshold(UUID id){
		auto&cc2d = GetComponent<CircleCollider2DComponent>(id);
		return cc2d.RestitutionThreshold;
	}
	static void CircleCollider2DComponent_SetRestitutionThreshold(UUID id,float setRestitutionThreshold){
		auto&cc2d = GetComponent<CircleCollider2DComponent>(id);
		cc2d.RestitutionThreshold = setRestitutionThreshold;
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
		KD_ADD_INTERNAL(NativeLog);

		KD_ADD_INTERNAL(Entity_GetName);
		KD_ADD_INTERNAL(Entity_HasComponent);
		KD_ADD_INTERNAL(Entity_AddComponent);
		KD_ADD_INTERNAL(TransformComponent_GetPosition);
		KD_ADD_INTERNAL(TransformComponent_SetPosition);
		KD_ADD_INTERNAL(TransformComponent_GetRotation);
		KD_ADD_INTERNAL(TransformComponent_SetRotation);
		KD_ADD_INTERNAL(TransformComponent_GetScale);
		KD_ADD_INTERNAL(TransformComponent_SetScale);
		KD_ADD_INTERNAL(SpriteRendererComponent_GetColor);
		KD_ADD_INTERNAL(SpriteRendererComponent_SetColor);
		KD_ADD_INTERNAL(CircleRendererComponent_GetColor);
		KD_ADD_INTERNAL(CircleRendererComponent_SetColor);
		KD_ADD_INTERNAL(CircleRendererComponent_GetThickness);
		KD_ADD_INTERNAL(CircleRendererComponent_SetThickness);
		KD_ADD_INTERNAL(CircleRendererComponent_GetFade);
		KD_ADD_INTERNAL(CircleRendererComponent_SetFade);
		//Physics
		KD_ADD_INTERNAL(Rigidbody2DComponent_ApplyLinearImpulse);
		KD_ADD_INTERNAL(Rigidbody2DComponent_ApplyForce);
		KD_ADD_INTERNAL(Rigidbody2DComponent_GetBodyType);
		KD_ADD_INTERNAL(Rigidbody2DComponent_GetFixedRotation);
		KD_ADD_INTERNAL(Rigidbody2DComponent_SetFixedRotation);
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
