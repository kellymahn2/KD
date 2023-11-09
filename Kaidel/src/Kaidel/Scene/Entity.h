#pragma once

#include "Scene.h"

#include "Components.h"
#include "Kaidel/Core/UUID.h"


#include "entt.hpp"
namespace Kaidel {

	class Entity
	{
	public:
		Entity() = default;
		Entity(entt::entity handle, Scene* scene);
		Entity(const Entity& other) = default;

		template<typename T, typename... Args>
		T& AddComponent(Args&&... args)
		{
			KD_CORE_ASSERT(!HasComponent<T>(), "Entity already has component!");
			T& component = m_Scene->m_Registry.emplace<T>(m_EntityHandle, std::forward<Args>(args)...);
			m_Scene->OnComponentAdded<T>(*this, component);
			return component;
		}

		template<typename T>
		T& GetComponent()
		{
			KD_CORE_ASSERT(HasComponent<T>(), "Entity does not have component!");
			return m_Scene->m_Registry.get<T>(m_EntityHandle);
		}

		template<typename T>
		bool HasComponent()
		{
			return m_Scene->m_Registry.try_get<T>(m_EntityHandle)!=nullptr;
		}

		template<typename T>
		void RemoveComponent()
		{
			KD_CORE_ASSERT(HasComponent<T>(), "Entity does not have component!");
			m_Scene->m_Registry.remove<T>(m_EntityHandle);
		}
		void AddChild(UUID childId) {
			if (!HasComponent<ParentComponent>())
				AddComponent<ParentComponent>();
			auto& pc=GetComponent<ParentComponent>();
			pc.Children.push_back(childId);
		}
		void AddParent(UUID parentID) {
			if (!HasComponent<ChildComponent>())
				AddComponent<ChildComponent>();
			auto& cc = GetComponent<ChildComponent>();
			cc.Parent = parentID;
		}

		void AddScript() {
			if (!HasComponent<ScriptComponent>())
				AddComponent<ScriptComponent>();
			auto& sc = GetComponent<ScriptComponent>();
			sc.ScriptNames.push_back({});
		}


		Entity GetParent() {
			return m_Scene->GetEntity(GetComponent<ChildComponent>().Parent);
		}
		bool HasChildren() {
			return HasComponent<ParentComponent>() && !GetComponent<ParentComponent>().Children.empty();
		}
		inline UUID GetUUID() { return GetComponent<IDComponent>().ID; }
		operator bool() const { return m_EntityHandle != entt::null&&(int)m_EntityHandle!=-1; }
		operator entt::entity() const { return m_EntityHandle; }
		operator uint32_t() const { return (uint32_t)m_EntityHandle; }

		bool operator==(const Entity& other) const
		{
			return m_EntityHandle == other.m_EntityHandle && m_Scene == other.m_Scene;
		}

		bool operator!=(const Entity& other) const
		{
			return !(*this == other);
		}
		inline Scene* GetScene() { return m_Scene; }
	private:
		entt::entity m_EntityHandle{ entt::null };
		Scene* m_Scene = nullptr;
	};
	glm::mat4 GetLocalTransform(Entity entity);
	void RotateEntity(Entity entity, Scene* scene, const glm::vec3& delta, Entity top = {});
	void TranslateEntity(Entity entity, Scene* scene, const glm::vec3& delta, Entity top = {});
	void MoveEntity(Entity entity, Scene* scene, const glm::vec3& deltaT, const glm::vec3& deltaR);
}
