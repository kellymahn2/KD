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

		entt::entity GetHandle() const { return m_EntityHandle; }

		template<typename T, typename... Args>
		T& AddComponent(Args&&... args) const
		{
			KD_CORE_ASSERT(!HasComponent<T>(), "Entity already has component!");
			T& component = m_Scene->m_Registry.emplace<T>(m_EntityHandle, std::forward<Args>(args)...);
			m_Scene->OnComponentAdded<T>(*this, component);
			return component;
		}

		template<typename... Components>
		decltype(auto) AddComponents() const {
			return std::tie(AddComponent<Components>()...);
		}

		const std::string& GetName() const { return GetComponent<TagComponent>().Tag; }

		template<typename Cond>
		Entity WalkUp(Entity current, Cond&& cond) const
		{
			if (cond(current))
			{
				return current;
			}

			auto cc = current.TryGetComponent<ChildComponent>();

			if (cc)
			{
				Entity parent(cc->Parent, m_Scene);

				return WalkUp(parent, std::forward(cond));
			}

			return {};
		}

		template<typename Cond>
		Entity WalkDown(Entity current, Cond&& cond) const
		{
			if (cond(current))
			{
				return current;
			}

			auto pc = current.TryGetComponent<ParentComponent>();

			if (pc)
			{
				for (auto& childID : pc->Children)
				{
					Entity child(childID, m_Scene);

					return WalkDown(child, std::forward(cond));
				}
			}

			return {};
		}
		
		template<typename... T>
		Entity GetComponentInParent() const
		{
			return WalkUp(*this, [](Entity entity) 
			{
				return entity.HasComponents<T...>();
			});
		}

		template<typename... T>
		Entity GetComponentInChildren() const
		{
			return WalkDown(*this, [](Entity entity)
			{
				return entity.HasComponents<T...>();
			});
		}

		template<typename... T>
		decltype(auto) GetComponent() const
		{
			KD_CORE_ASSERT(HasComponents<T...>(), "Entity does not have component!");
			return m_Scene->m_Registry.get<T...>(m_EntityHandle);
		}
		template<typename T>
		bool HasComponent() const {
			return m_Scene->m_Registry.try_get<T>(m_EntityHandle) != nullptr;
		}
		template<typename... T>
		bool HasComponents() const
		{
			return (HasComponent<T>()&&...);
		}

		template<typename... T>
		decltype(auto) TryGetComponent() const
		{
			return m_Scene->m_Registry.try_get<T...>(m_EntityHandle);
		}

		template<typename T,typename... Args>
		T& TryAddComponent(Args&&... args) const {
			{
				T* has = m_Scene->m_Registry.try_get<T>(m_EntityHandle);
				if (has != nullptr)
					return *has;
			}
			return AddComponent<T, Args...>(std::forward<Args>(args)...);
		}


		template<typename T>
		void RemoveComponent() const
		{
			KD_CORE_ASSERT(HasComponent<T>(), "Entity does not have component!");
			m_Scene->m_Registry.remove<T>(m_EntityHandle);
		}
		void AddChild(entt::entity childId) const {
			if (!HasComponent<ParentComponent>())
				AddComponent<ParentComponent>();
			auto& pc=GetComponent<ParentComponent>();
			pc.Children.push_back(childId);
		}
		void AddParent(entt::entity parentID) const {
			if (!HasComponent<ChildComponent>())
				AddComponent<ChildComponent>();
			auto& cc = GetComponent<ChildComponent>();
			cc.Parent = parentID;

			auto& id = GetComponent<IDComponent>();

			uint32_t oldDepth = id.Depth;
			id.Depth = m_Scene->m_Registry.get<IDComponent>(parentID).Depth + 1;

			m_Scene->SetDepth(m_EntityHandle, oldDepth, id.Depth);
		}

		void AddScript() const {
			if (!HasComponent<ScriptComponent>())
				AddComponent<ScriptComponent>();
			auto& sc = GetComponent<ScriptComponent>();
			sc.ScriptNames.push_back({});
		}


		Entity GetParent() const {
			return { GetComponent<ChildComponent>().Parent, m_Scene };
		}

		bool HasParent() const {
			return HasComponent<ChildComponent>() && GetParent();
		}
		bool HasChildren() const {
			return HasComponent<ParentComponent>() && !GetComponent<ParentComponent>().Children.empty();
		}
		inline UUID GetUUID() const { return GetComponent<IDComponent>().ID; }
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
		inline Scene* GetScene() const { return m_Scene; }

	private:
		

	private:
		entt::entity m_EntityHandle{ entt::null };
		Scene* m_Scene = nullptr;
	};
	glm::mat4 GetLocalTransform(Entity entity);
	void RotateEntity(Entity entity, Scene* scene, const glm::vec3& delta, Entity top = {});
	void TranslateEntity(Entity entity, Scene* scene, const glm::vec3& delta, Entity top = {});
	void MoveEntity(Entity entity, Scene* scene, const glm::vec3& deltaT, const glm::vec3& deltaR);
}
