#include "KDpch.h"
#include "Scene.h"

#include "Components.h"
#include "Kaidel/Renderer/Renderer2D.h"
#include "ScriptableEntity.h"

#include <glm/glm.hpp>
#include "Entity.h"
#include "box2d/b2_world.h"
#include "box2d/b2_body.h"
#include "box2d/b2_fixture.h"
#include "box2d/b2_polygon_shape.h"

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
			CopyComponents<	TagComponent, TransformComponent,
				SpriteRendererComponent, CircleRendererComponent,
				CameraComponent,
				BoxCollider2DComponent, Rigidbody2DComponent,
				NativeScriptComponent					>
				(entity, srcReg, e); });
		return newScene;
	}

	void Scene::DuplicateEntity(Entity& entity)
	{
		if (!entity)
			return;
		auto newEntity = CreateEntity(entity.GetComponent<TagComponent>().Tag);
		CopyComponents < TransformComponent,
			SpriteRendererComponent, CameraComponent,
			BoxCollider2DComponent, Rigidbody2DComponent,
			CircleRendererComponent,
			NativeScriptComponent>(newEntity, m_Registry, entity);
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
		return entity;
	}

	void Scene::DestroyEntity(Entity entity)
	{
		m_Registry.destroy(entity);
	}

	void Scene::OnRuntimeStart()
	{
		m_PhysicsWorld = new b2World(b2Vec2{ 0.0f,-9.8f });
		auto view = m_Registry.view<Rigidbody2DComponent>();
		for (auto e : view) {
			Entity entity{ e,this };
			auto& transform = entity.GetComponent<TransformComponent>();
			auto& rb2d = entity.GetComponent<Rigidbody2DComponent>();
			b2BodyDef bodyDef;
			bodyDef.type = Rigidbody2DTypeToBox2DBodyType(rb2d.Type);
			bodyDef.position.Set(transform.Translation.x, transform.Translation.y);
			bodyDef.angle = transform.Rotation.z;

			b2Body* body = m_PhysicsWorld->CreateBody(&bodyDef);
			body->SetFixedRotation(rb2d.FixedRotation);
			rb2d.RuntimeBody = body;
			if (entity.HasComponent<BoxCollider2DComponent>()) {
				auto& bc2d = entity.GetComponent<BoxCollider2DComponent>();
				b2PolygonShape boxShape;
				boxShape.SetAsBox(bc2d.Size.x * transform.Scale.x, bc2d.Size.y * transform.Scale.y);
				b2FixtureDef fixtureDef;
				fixtureDef.shape = &boxShape;
				fixtureDef.density = bc2d.Density;
				fixtureDef.friction = bc2d.Friction;
				fixtureDef.restitution = bc2d.Restitution;
				fixtureDef.restitutionThreshold = bc2d.RestitutionThreshold;

				body->CreateFixture(&fixtureDef);
			}

			m_PhysicsWorld->CreateBody(&bodyDef);
		}
	}

	void Scene::OnRuntimeStop()
	{
		delete m_PhysicsWorld;
		m_PhysicsWorld = nullptr;
	}

	void Scene::OnUpdateRuntime(Timestep ts)
	{
		// Update scripts
		{
			m_Registry.view<NativeScriptComponent>().each([=](auto entity, auto& nsc)
			{
				// TODO: Move to Scene::OnScenePlay
				if (!nsc.Instance)
				{
					nsc.Instance = nsc.InstantiateScript();
					nsc.Instance->m_Entity = Entity{ entity, this };
					nsc.Instance->OnCreate();
				}

				nsc.Instance->OnUpdate(ts);
			});
		}
		//Physics

		{
			const int32_t velocityIterations = 6;
			const int32_t positionIterations = 2;
			m_PhysicsWorld->Step(ts, velocityIterations, positionIterations);

			auto view = m_Registry.view<Rigidbody2DComponent>();

			for(auto e:view)
			{
				Entity entity{ e,this };
				auto& transform = entity.GetComponent<TransformComponent>();
				auto& rb2d = entity.GetComponent<Rigidbody2DComponent>();

				b2Body* body = (b2Body*)rb2d.RuntimeBody;

				const auto& position = body->GetPosition();
				transform.Translation.x = position.x;
				transform.Translation.y = position.y;
				transform.Rotation.z = body->GetAngle();
			}
		}






		// Render 2D
		Camera* mainCamera = nullptr;
		glm::mat4 cameraTransform;
		{
			auto view = m_Registry.view<TransformComponent, CameraComponent>();
			for (auto entity : view)
			{
				auto [transform, camera] = view.get<TransformComponent, CameraComponent>(entity);
				
				if (camera.Primary)
				{
					mainCamera = &camera.Camera;
					cameraTransform = transform.GetTransform();
					break;
				}
			}
		}

		if (mainCamera)
		{
			Renderer2D::BeginScene(*mainCamera, cameraTransform);

			{
				auto group = m_Registry.group<TransformComponent>(entt::get<SpriteRendererComponent>);
				for (auto entity : group)
				{
					auto [transform, sprite] = group.get<TransformComponent, SpriteRendererComponent>(entity);

					Renderer2D::DrawSprite(transform.GetTransform(), sprite, (int)entity);
				}
			}
			{
				auto view = m_Registry.view<TransformComponent, CircleRendererComponent>();
				for (auto e : view) {
					auto [transform, crc] = view.get<TransformComponent, CircleRendererComponent>(e);

					Renderer2D::DrawCircle(transform.GetTransform(), crc.Color, crc.Thickness, crc.Fade, (int)e);
				}
			}


			Renderer2D::EndScene();
		}

	}

	void Scene::OnUpdateEditor(Timestep ts, EditorCamera& camera)
	{
		Renderer2D::BeginScene(camera);

		{
			auto group = m_Registry.group<TransformComponent>(entt::get<SpriteRendererComponent>);
			for (auto entity : group)
			{
				auto [transform, sprite] = group.get<TransformComponent, SpriteRendererComponent>(entity);

				Renderer2D::DrawSprite(transform.GetTransform(), sprite, (int)entity);
			}
		}
		{
			auto view = m_Registry.view<TransformComponent, CircleRendererComponent>();
			for (auto e : view) {
				auto [transform, crc] = view.get<TransformComponent, CircleRendererComponent>(e);

				Renderer2D::DrawCircle(transform.GetTransform(), crc.Color, crc.Thickness,crc.Fade, (int)e);
			}
		}
		Renderer2D::SetLineWidth(4.0);
		Renderer2D::EndScene();
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
}
