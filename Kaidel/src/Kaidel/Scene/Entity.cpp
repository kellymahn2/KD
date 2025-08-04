#include "KDpch.h"
#include "Entity.h"
#include "Kaidel/Math/Math.h"
namespace Kaidel {

	Entity::Entity(entt::entity handle, Scene* scene)
		: m_EntityHandle(handle), m_Scene(scene)
	{
	}
	glm::mat4 GetLocalTransform(Entity entity) {

		if (!entity.HasComponent<ChildComponent>())
			return entity.GetComponent<TransformComponent>().GetTransform();

		auto& origin = entity.GetParent().GetComponent<TransformComponent>();
		//return glm::inverse(Math::Rotate(origin.Translation,origin.Rotation))*entity.GetComponent<TransformComponent>().GetTransform();
	}
	
}
void Kaidel::RotateEntity(Kaidel::Entity entity, Kaidel::Scene* scene, const glm::vec3& delta, Kaidel::Entity top) {
	auto& tc = entity.GetComponent<Kaidel::TransformComponent>();

	if (entity.HasChildren()) {
		for (auto& childID : entity.GetComponent<Kaidel::ParentComponent>().Children) {
			auto child = scene->GetEntity(childID);
			RotateEntity(child, scene, delta, top ? top : entity);
		}
	}
	if (top) {
		Math::Rotate(entity, top, delta);
	}
	else {
		if (entity.HasComponent<Kaidel::ChildComponent>()) {
			auto& cc = entity.GetComponent<Kaidel::ChildComponent>();
			cc.LocalRotation += delta;
			//Math::DecomposeTransform(glm::translate(glm::mat4(1.0f), tc.Translation) * glm::toMat4(glm::quat(tc.Rotation))
			//	/** glm::toMat4(glm::quat(delta)) */* glm::scale(glm::mat4(1.0f), tc.Scale)
			//	, tc.Translation, tc.Rotation, tc.Scale);
		}
		//tc.Rotation += delta;
	}
}
void Kaidel::TranslateEntity(Kaidel::Entity entity, Kaidel::Scene* scene, const glm::vec3& delta, Kaidel::Entity top) {
	auto& tc = entity.GetComponent<Kaidel::TransformComponent>();

	if (entity.HasChildren()) {
		for (auto& childID : entity.GetComponent<Kaidel::ParentComponent>().Children) {
			auto child = scene->GetEntity(childID);
			TranslateEntity(child, scene, delta, top ? top : entity);
		}
	}
	if (top) {
		tc.Translation += delta;
	}
	else {
		if (entity.HasComponent<Kaidel::ChildComponent>()) {
			auto& cc = entity.GetComponent<Kaidel::ChildComponent>();
			cc.LocalPosition += delta;
		}
		tc.Translation += delta;
	}

}

void Kaidel::MoveEntity(Kaidel::Entity entity, Kaidel::Scene* scene, const glm::vec3& deltaT, const glm::vec3& deltaR) {
	//if (glm::dot(deltaR, deltaR)) {
	//	RotateEntity(entity, scene, deltaR);
	//}
	//if (glm::dot(deltaT, deltaT)) {
	//	TranslateEntity(entity, scene, deltaT);
	//}
}
