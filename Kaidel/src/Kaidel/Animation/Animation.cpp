#include "KDpch.h"
#include "Animation.h"
#include "Kaidel/Scene/Entity.h"
#include "Kaidel/Scene/Scene.h"
namespace Kaidel {

	static std::unordered_map<UUID, glm::vec3> s_StartTranslationMap;
	

	void ApplyTranslation(glm::vec3* current, glm::vec3* target, float t, Entity& entity)
	{
		auto& tc = entity.GetComponent<TransformComponent>();
		glm::vec3 newT;
		if (current) {
			newT = glm::mix(*current, *target, t);
		}
		else {
			auto it = s_StartTranslationMap.find(entity.GetUUID());
			if(it!=s_StartTranslationMap.end())
				newT = glm::mix(it->second, *target, t);
			else
				newT = glm::mix({0,0,0}, *target, t);
		}
		auto deltaT = newT - tc.Translation;
		MoveEntity(entity, entity.GetScene(), deltaT, glm::vec3{ 0 });
	}

	void AddDefaultTranslation(const glm::vec3& default, UUID id)
	{
		s_StartTranslationMap[id] = default;
	}

	void SetDefaultTranslation(Entity& entity)
	{
		auto it = s_StartTranslationMap.find(entity.GetUUID());
		if (it == s_StartTranslationMap.end())
			return;
		entity.GetComponent<TransformComponent>().Translation = it->second;
	}

}
