#pragma once

#include "Kaidel/Core/Timestep.h"
#include "Kaidel/Renderer/EditorCamera.h"
#include "Kaidel/Core/UUID.h"
#include "entt.hpp"
class b2World;

namespace Kaidel {
	class Entity;
	class Scene
	{
	public:
		Scene();
		~Scene();

		Entity CreateEntity(const std::string& name = std::string());
		Entity CreateEntity(UUID uuid, const std::string& name = std::string());
		void DestroyEntity(Entity entity);

		void OnRuntimeStart();

		void OnRuntimeStop();


		void DuplicateEntity(Entity& entity);

		void OnUpdateRuntime(Timestep ts);
		void OnUpdateEditor(Timestep ts, EditorCamera& camera);
		void OnViewportResize(uint32_t width, uint32_t height);

		Entity GetPrimaryCameraEntity();

		static Ref<Scene> Copy(const Ref<Scene>& rhs);

		void SetPath(const std::string& path) { m_Path = path; }
		const std::string& GetPath()const { return m_Path; }
		template<typename ... Components>
		decltype(auto) GetAllComponentsWith() {
			return m_Registry.view<Components...>();
		}
	private:
		template<typename T>
		void OnComponentAdded(Entity entity, T& component);
	private:
		entt::registry m_Registry;
		uint32_t m_ViewportWidth = 0, m_ViewportHeight = 0;
		b2World* m_PhysicsWorld = nullptr;
		std::string m_Path;
		friend class Entity;
		friend class SceneSerializer;
		friend class SceneHierarchyPanel;
	};

}
