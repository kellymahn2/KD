#include "KDpch.h"
#include "Kaidel/Core/UUID.h"
#include "Kaidel/Scene/Entity.h"
#include "Kaidel/Scene/Scene.h"
#include "ScriptEngine.h"
#include "ScriptRegistry.h"
#include "Kaidel/Core/Input.h"

#include "mono/metadata/assembly.h"
#include "mono/jit/jit.h"
#include <glm/glm.hpp>


namespace Kaidel {
#define KD_ADD_INTERNAL(Name) mono_add_internal_call("KaidelCore.Internals::"#Name,&Name)

	static void NativeLog(MonoString* string) {
		char* cstr = mono_string_to_utf8(string);
		std::string s(cstr);
		mono_free(cstr);
		KD_CORE_INFO("{}", s);
	}
	static void Entity_GetPosition(UUID id, glm::vec3* outPos) {
		Scene* scene = ScriptEngine::GetSceneContext();
		Entity entity = scene->GetEntity(id);
		auto& pos = entity.GetComponent<TransformComponent>().Translation;
		memcpy(outPos, &pos, sizeof(glm::vec3));
	}
	static void Entity_SetPosition(UUID id, glm::vec3* setPos) {
		Scene* scene = ScriptEngine::GetSceneContext();
		Entity entity = scene->GetEntity(id);
		auto& pos = entity.GetComponent<TransformComponent>().Translation;
		memcpy(&pos, setPos, sizeof(glm::vec3));
	}

	static bool Input_IsKeyDown(KeyCode* keyCode) {
		return Input::IsKeyPressed(*keyCode);
	}
	static bool Input_IsMouseDown(MouseCode* mouseCode) {
		return Input::IsMouseButtonPressed(*mouseCode);
	}



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



	
	void ScriptRegistry::RegisterFunctions()
	{
		KD_ADD_INTERNAL(NativeLog);
		KD_ADD_INTERNAL(Entity_GetPosition);
		KD_ADD_INTERNAL(Entity_SetPosition);
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
