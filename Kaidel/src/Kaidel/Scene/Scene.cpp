#include "KDpch.h"

#include "Scene.h"
#include "Components.h"
#include "ScriptableEntity.h"
#include "Entity.h"
#include "SceneRenderer.h"
#include "Kaidel/Scripting/ScriptEngine.h"
#include "ModelLibrary.h"

#include <glm/glm.hpp>
#include <box2d/b2_world.h>
#include <box2d/b2_body.h>
#include <box2d/b2_fixture.h>
#include <box2d/b2_polygon_shape.h>
#include <box2d/b2_circle_shape.h>



#define DEF_COMPONENT_ADD(Component) template<>\
void Scene::OnComponentAdded<##Component>(Entity entity, ##Component& component){}

namespace Kaidel {
	
	b2BodyType	Rigidbody2DTypeToBox2DBodyType(Rigidbody2DComponent::BodyType type) {

		switch (type)
		{
		case Rigidbody2DComponent::BodyType::Static:return b2_staticBody;
		case Rigidbody2DComponent::BodyType::Dynamic:return b2_dynamicBody;
		case Rigidbody2DComponent::BodyType::Kinematic:return b2_kinematicBody;
		}
		KD_CORE_ASSERT(false, "Not Supported");
		return b2_staticBody;
	}

	Scene::Scene()
	{
	}

	Scene::~Scene()
	{
	}
	template<typename T> 
	static bool CopyComponent(Entity& entity, entt::registry& srcReg, entt::entity srcID) {
		//Entity Has Component in src
		if (srcReg.try_get<T>(srcID)!=nullptr) {
			auto& component= entity.HasComponent<T>()?entity.GetComponent<T>():entity.AddComponent<T>();
			component = srcReg.get<T>(srcID);
		}
		return true;
	}
	//Helper For Expansion of Args template parameter in Functions
	static void helper(...) {}
	template<typename ...Args>
	static void CopyComponents(Entity& entity, entt::registry& srcReg, entt::entity srcID) {
		helper(CopyComponent<Args>(entity, srcReg, srcID)...);
	}
	Ref<Scene> Scene::Copy(const Ref<Scene>& rhs)
	{
		Ref<Scene> newScene = CreateRef<Scene>();
		newScene->m_ViewportWidth = rhs->m_ViewportWidth;
		newScene->m_ViewportHeight = rhs->m_ViewportHeight;
		auto& srcReg = rhs->m_Registry;
		std::vector<entt::entity> V;
		srcReg.each([&V](auto& e) {
			V.push_back(e);
			});
		std::reverse(V.begin(), V.end());
		std::for_each(V.begin(), V.end(), [&newScene, &srcReg](auto& e) {
			UUID uuid = srcReg.get<IDComponent>(e).ID;
			const auto& name = srcReg.get<TagComponent>(e).Tag;
			Entity entity = newScene->CreateEntity(uuid, name);
			CopyComponents<TagComponent, TransformComponent,
				SpriteRendererComponent, 
				CircleRendererComponent ,CircleCollider2DComponent,
				CameraComponent,
				BoxCollider2DComponent, Rigidbody2DComponent,
				NativeScriptComponent, ScriptComponent,ParentComponent,ChildComponent 
				,LineRendererComponent>
				(entity, srcReg, e); });
		newScene->m_IDMap = rhs->m_IDMap;
		return newScene;
	}

	void Scene::OnPhysics2DStart()
	{
		m_PhysicsWorld = new b2World(b2Vec2{ 0.0f,-9.8f });
		auto view = m_Registry.view<Rigidbody2DComponent>();
		for (auto e : view)
		{
			Entity entity = { e, this };
			auto& transform = entity.GetComponent<TransformComponent>();
			auto& rb2d = entity.GetComponent<Rigidbody2DComponent>();

			b2BodyDef bodyDef;
			bodyDef.type = Rigidbody2DTypeToBox2DBodyType(rb2d.Type);
			bodyDef.position.Set(transform.Translation.x, transform.Translation.y);
			bodyDef.angle = transform.Rotation.z;
			b2Body* body = m_PhysicsWorld->CreateBody(&bodyDef);
		
			body->SetFixedRotation(rb2d.FixedRotation);
			rb2d.RuntimeBody = body;

			if (entity.HasComponent<BoxCollider2DComponent>())
			{
				auto& bc2d = entity.GetComponent<BoxCollider2DComponent>();

				b2PolygonShape boxShape;
				boxShape.SetAsBox(bc2d.Size.x * transform.Scale.x, bc2d.Size.y * transform.Scale.y,b2Vec2(bc2d.Offset.x, bc2d.Offset.y),0.0f);

				b2FixtureDef fixtureDef;
				fixtureDef.shape = &boxShape;
				fixtureDef.density = bc2d.Density;
				fixtureDef.friction = bc2d.Friction;
				fixtureDef.restitution = bc2d.Restitution;
				fixtureDef.restitutionThreshold = bc2d.RestitutionThreshold;
				body->CreateFixture(&fixtureDef);
			}

			if (entity.HasComponent<CircleCollider2DComponent>())
			{
				auto& cc2d = entity.GetComponent<CircleCollider2DComponent>();

				b2CircleShape circleShape;
				circleShape.m_p.Set(cc2d.Offset.x, cc2d.Offset.y);
				circleShape.m_radius = transform.Scale.x * cc2d.Radius;

				b2FixtureDef fixtureDef;
				fixtureDef.shape = &circleShape;
				fixtureDef.density = cc2d.Density;
				fixtureDef.friction = cc2d.Friction;
				fixtureDef.restitution = cc2d.Restitution;
				fixtureDef.restitutionThreshold = cc2d.RestitutionThreshold;
				body->CreateFixture(&fixtureDef);
			}
		}
	}


	void Scene::OnPhysics2DUpdate(Timestep& ts)
	{
		{
			const int32_t velocityIterations = 6;
			const int32_t positionIterations = 2;
			auto view = m_Registry.view<Rigidbody2DComponent>();
			m_PhysicsWorld->Step(ts, velocityIterations, positionIterations);
			for (auto e : view)
			{
				Entity entity{ e,this };
				auto& transform = entity.GetComponent<TransformComponent>();
				auto& rb2d = entity.GetComponent<Rigidbody2DComponent>();

				b2Body* body = (b2Body*)rb2d.RuntimeBody; 
				const auto& position = body->GetPosition();

				glm::vec3 deltaPos = -transform.Translation;
				deltaPos.z = 0.0f;
				deltaPos.x += position.x;
				deltaPos.y += position.y;
				glm::vec3 deltaRot = -transform.Rotation;
				deltaRot.x = 0.0f;
				deltaRot.y = 0.0f;
				deltaRot.z += body->GetAngle();
				MoveEntity(entity, this, deltaPos, deltaRot);
			}
		}
	}

	void Scene::OnPhysics2DStop()
	{
		delete m_PhysicsWorld;
		m_PhysicsWorld = nullptr;
	}

	void Scene::UpdateAnimations(float ts)
	{
		/*for (auto e : m_Registry.view<AnimationComponent>()) {
			Entity entity{ e,this };
			auto& ac = entity.GetComponent<AnimationComponent>();
			if (ac.Animation && ac.AnimationController && ac.Animation->GetState() == AnimationState::Playing)
				ac.AnimationController->Update(entity,ts,ac.Animation);
		}*/
	}


	void Scene::RenderScene()
	{
		
	}

	void Scene::DuplicateEntity(Entity& entity)
	{
		if (!entity)
			return;
		auto newEntity = CreateEntity(entity.GetComponent<TagComponent>().Tag);
		CopyComponents < TransformComponent,
			SpriteRendererComponent, CameraComponent,
			BoxCollider2DComponent, Rigidbody2DComponent,
			CircleRendererComponent ,CircleCollider2DComponent,
			NativeScriptComponent,ScriptComponent,LineRendererComponent,ChildComponent>(newEntity, m_Registry, entity);

		if (entity.HasComponent<ChildComponent>()) {
			auto& parentID = entity.GetComponent<ChildComponent>().Parent;
			auto parent = GetEntity(parentID);
			parent.AddChild(newEntity.GetUUID());
		}
	}


	Entity Scene::CreateEntity(const std::string& name)
	{
		return CreateEntity(UUID{}, name);
	}
	Entity Scene::CreateEntity(UUID uuid, const std::string& name /*= std::string()*/)
	{
		Entity entity = { m_Registry.create(), this };
		entity.AddComponent<TransformComponent>();
		entity.AddComponent<IDComponent>(uuid);
		auto& tag = entity.AddComponent<TagComponent>();
		tag.Tag = name.empty() ? "Entity" : name;
		m_IDMap[uuid] = entity.operator entt::entity();
		return entity;
	}
	

	Entity Scene::CreateCube(const std::string& name, UUID uuid) {
		Entity entity = CreateEntity(uuid, name);
		auto& mc = entity.AddComponent<ModelComponent>();
		mc.UsedModel = ModelLibrary::GetBaseCube();
		return entity;
	}

	Entity Scene::GetEntity(UUID id)
	{
		if (m_IDMap.find(id) != m_IDMap.end())
			return { m_IDMap.at(id),this };
		return Entity{};
	}

	Entity Scene::FindEntityByName(std::string_view name)
	{
		auto view = m_Registry.view<TagComponent>();
		for (auto e : view) {
			const auto& tc = view.get<TagComponent>(e);
			if (tc.Tag == name)
				return { e,this };
		}
		return {};
	}

	void Scene::DestroyEntity(Entity entity)
	{
		m_Registry.destroy(entity);
	}

	void Scene::OnRuntimeStart()
	{
		m_SceneIsRunning = true;
		//Scripts
		ScriptEngine::OnRuntimeStart(this);

		/*for (auto e : m_Registry.view<AnimationComponent>()) {
			auto& ac = m_Registry.get<AnimationComponent>(e);
			if (ac.Animation)
				ac.Animation->Reset();
		}*/
		/*{
			for (auto e : m_Registry.view<AnimationComponent>()) {
				auto& ac = m_Registry.get<AnimationComponent>(e);
				Entity entity{ e,this };
				SetDefaultTranslation(entity);
				if (ac.Animation&& ac.AnimationController) {
					ac.AnimationController->Play(ac.Animation);
				}
			}
		}*/

		//Instantiate all scripts
		auto view = m_Registry.view<ScriptComponent>();
		for (auto e : view) {
			Entity entity = { e,this };
			const auto& sc = entity.GetComponent<ScriptComponent>();
			ScriptEngine::OnCreateEntity(entity);
		}
		OnPhysics2DStart();
	}

	void Scene::OnRuntimeStop()
	{
		m_SceneIsRunning = false;
		m_IsPaused = false;
		OnPhysics2DStop();
		ScriptEngine::OnRuntimeStop();
	}
	
	void Scene::OnSimulationStart()
	{
		OnPhysics2DStart();
	}

	void Scene::OnSimulationStop()
	{
		OnPhysics2DStop();
	}

	void Scene::OnUpdateRuntime(Timestep ts)
	{
		// Update native scripts
		/*{
			m_Registry.view<NativeScriptComponent>().each([=](auto entity, auto& nsc)
			{
				if (!nsc.Instance)
				{
					nsc.Instance = nsc.InstantiateScript();
					nsc.Instance->m_Entity = Entity{ entity, this };
					nsc.Instance->OnCreate();
				}

				nsc.Instance->OnUpdate(ts);
			});
		}*/

		UpdateAnimations(ts);

		if (!m_IsPaused) {
			//Physics
			OnPhysics2DUpdate(ts);
			//C# Entity OnUpdate
			auto view = m_Registry.view<ScriptComponent>();
			for (auto e : view) {
				Entity entity = { e,this };
				const auto& sc = entity.GetComponent<ScriptComponent>();
				ScriptEngine::OnUpdateEntity(entity,ts);
			}
		}
	


		// Render 2D
		/*Entity cam = GetPrimaryCameraEntity();
		if (cam)
		{
			auto camTransform = cam.GetComponent<TransformComponent>().GetTransform();
			Renderer2D::BeginScene(cam.GetComponent<CameraComponent>().Camera, camTransform);
			Renderer3D::BeginScene(cam.GetComponent<CameraComponent>().Camera, cam.GetComponent<TransformComponent>().GetTransform());
			RenderScene();
			Renderer3D::EndScene();
			Renderer2D::EndScene();
		}*/

	}

	void Scene::OnUpdateEditor(Timestep ts, EditorCamera& camera, Ref<Texture2D> outputBuffer)
	{
		static SceneRenderer sceneRenderer{ this };
		SceneData data{};
		data.Proj = camera.GetProjection();
		data.View = camera.GetViewMatrix();
		data.ViewProj = camera.GetViewProjection();
		sceneRenderer.Render(outputBuffer, data);
	}

	void Scene::OnUpdateSimulation(Timestep ts, EditorCamera& camera)
	{
		OnPhysics2DUpdate(ts);
		//OnUpdateEditor(ts, camera);
	}

	void Scene::OnViewportResize(uint32_t width, uint32_t height)
	{
		m_ViewportWidth = width;
		m_ViewportHeight = height;

		// Resize our non-FixedAspectRatio cameras
		auto view = m_Registry.view<CameraComponent>();
		for (auto entity : view)
		{
			auto& cameraComponent = view.get<CameraComponent>(entity);
			if (!cameraComponent.FixedAspectRatio)
				cameraComponent.Camera.SetViewportSize(width, height);
		}

	}

	Entity Scene::GetPrimaryCameraEntity()
	{
		auto view = m_Registry.view<CameraComponent>();
		for (auto entity : view)
		{
			const auto& camera = view.get<CameraComponent>(entity);
			if (camera.Primary)
				return Entity{entity, this};
		}
		return {};
	}

	Entity Scene::GetMainDirectionalLight() {
		/*auto view = m_Registry.view<DirectionalLightComponent>();
		for (auto entity : view) {
			const auto& light = view.get<DirectionalLightComponent>(entity);
			if (light.IsPrimary)
				return Entity{ entity,this };
		}*/
		return {};
	}


	template<>
	void Scene::OnComponentAdded<CameraComponent>(Entity entity, CameraComponent& component)
	{
		component.Camera.SetViewportSize(m_ViewportWidth, m_ViewportHeight);
	}
	
	DEF_COMPONENT_ADD(TransformComponent)
	DEF_COMPONENT_ADD(SpriteRendererComponent)
	DEF_COMPONENT_ADD(TagComponent)
	DEF_COMPONENT_ADD(NativeScriptComponent)
	DEF_COMPONENT_ADD(Rigidbody2DComponent)
	DEF_COMPONENT_ADD(BoxCollider2DComponent)
	DEF_COMPONENT_ADD(IDComponent)
	DEF_COMPONENT_ADD(CircleRendererComponent)
	DEF_COMPONENT_ADD(CircleCollider2DComponent)
	DEF_COMPONENT_ADD(ScriptComponent)
	DEF_COMPONENT_ADD(ParentComponent)
	DEF_COMPONENT_ADD(ChildComponent)
	DEF_COMPONENT_ADD(LineRendererComponent)
	DEF_COMPONENT_ADD(AnimationPlayerComponent)
	DEF_COMPONENT_ADD(ModelComponent)
	DEF_COMPONENT_ADD(DirectionalLightComponent)

}
