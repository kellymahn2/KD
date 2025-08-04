#pragma once

#include "Kaidel/Core/Timestep.h"
#include "Kaidel/Renderer/EditorCamera.h"
#include "Kaidel/Renderer/GraphicsAPI/Framebuffer.h"
#include "Kaidel/Core/UUID.h"
#include "entt.hpp"
#include "Model.h"
class b2World;

namespace Kaidel {
	class Entity;
	class Scene : public IRCCounter<false>
	{
	public:
		Scene();
		~Scene();

		Entity CreateEntity(const std::string& name = std::string());
		Entity CreateEntity(UUID uuid, const std::string& name = std::string());
		Entity CreateCube(const std::string& name = "", UUID uuid = {});
		Entity CreateModel(Ref<Model> model);


		Entity GetEntity(UUID id);
		bool IsEntity(UUID id);
		Entity FindEntityByName(std::string_view name);
		void DestroyEntity(Entity entity);

		void OnRuntimeStart();
		
		void OnRuntimeStop();

		void OnSimulationStart();
		void OnSimulationStop();

		void DuplicateEntity(Entity& entity);

		void OnUpdateRuntime(Timestep ts);
		void OnUpdateEditor(Timestep ts, EditorCamera& camera, Ref<Texture2D> outputBuffer);
		void OnUpdateSimulation(Timestep ts, EditorCamera& camera);
		void OnViewportResize(uint32_t width, uint32_t height);

		Entity GetPrimaryCameraEntity();
		
		static Ref<Scene> Copy(const Ref<Scene>& rhs);

		void SetPath(const std::string& path) { m_Path = path; }
		const std::string& GetPath()const { return m_Path; }
		template<typename ... Components>
		decltype(auto) GetAllComponentsWith() {
			return m_Registry.view<Components...>();
		}

		bool IsRunning()const { return m_SceneIsRunning; }
		bool IsPaused()const { return m_IsPaused; }
		void ChangePauseState() { m_IsPaused = !m_IsPaused; }
	private:
		template<typename T>
		void OnComponentAdded(Entity entity,T& component);

		void OnPhysics2DStart();
		void OnPhysics2DUpdate(Timestep& ts);
		void OnPhysics2DStop();
		void UpdateAnimations(float ts);
		void RenderScene();
		void CreateModelFromTree(const Scope<MeshTree>& curr, Entity entity, std::unordered_map<std::string, Entity>& nameToEntity);
		void SetSkinnedMeshRootBones(const Scope<MeshTree>& curr, Entity entity, std::unordered_map<std::string, Entity>& nameToEntity);

	private:

		Entity GetMainDirectionalLight();
		entt::registry m_Registry;
		uint32_t m_ViewportWidth = 0, m_ViewportHeight = 0;
		b2World* m_PhysicsWorld = nullptr;
		std::string m_Path;
		bool m_SceneIsRunning=false;
		bool m_IsPaused = false;
		friend class Entity;
		friend class SceneSerializer;
		friend class SceneHierarchyPanel;
		friend class SceneRenderer;
		friend class ShadowPass;
		friend class EditorLayer;
		friend struct GeometryPass;
		friend struct LightingPass;
		friend struct SharedPassData;


		std::unordered_map<UUID, entt::entity> m_IDMap;
	};

}
