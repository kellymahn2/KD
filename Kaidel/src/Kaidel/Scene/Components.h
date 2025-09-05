#pragma once
#include "SceneCamera.h"
#include "Model.h"
#include "Kaidel/Core/UUID.h"
#include "Kaidel/Animation/Animation.h"

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

#define GLM_ENABLE_EXPERIMENTAL
#include <glm/gtx/quaternion.hpp>


namespace Kaidel {

	struct IDComponent {
		UUID ID;
		bool IsActive = true;
		bool IsVisible = true;
		bool VisibilityResult = false;
		IDComponent() = default;
		IDComponent(const IDComponent&) = default;
	};
	struct ParentComponent {
		std::vector<UUID> Children;
	};
	struct ChildComponent {
		UUID Parent;
		glm::vec3 LocalPosition = {0,0,0};
		glm::vec3 LocalRotation = {0,0,0};

		ChildComponent() = default;
		ChildComponent(UUID parent):Parent(parent) {
		}
	};
	struct TagComponent
	{
		std::string Tag;

		TagComponent() = default;
		TagComponent(const TagComponent&) = default;
		TagComponent(const std::string& tag)
			: Tag(tag) {}
	};
	glm::mat4 _GetTransform(const glm::vec3& pos, const glm::quat& rot, const glm::vec3& scl);

	struct TransformComponent
	{
		glm::vec3 Translation = { 0.0f, 0.0f, 0.0f };
		glm::quat Rotation = glm::quat();
		glm::vec3 Scale = { 1.0f, 1.0f, 1.0f };
		
		glm::mat4 GlobalTransform = glm::mat4(1.0f);

		TransformComponent() = default;
		TransformComponent(const TransformComponent&) = default;
		TransformComponent(const glm::vec3& translation)
			: Translation(translation) {}

		glm::mat4 GetTransform() const
		{
			return _GetTransform(Translation, Rotation, Scale);
		}
	};


	//2D
	struct SpriteRendererComponent
	{
		Ref<Texture> SpriteTexture;
		SpriteRendererComponent() = default;
		SpriteRendererComponent(const SpriteRendererComponent&) = default;
	};


	struct CircleRendererComponent {
		glm::vec4 Color{ 1.0f };
		float Thickness=1.0f;
		float Fade = 0.005f;

		CircleRendererComponent() = default;
		CircleRendererComponent(const CircleRendererComponent&) = default;
	};
	template<typename T, typename _Points>
	glm::vec3 Bezier(const std::vector<_Points>& controlPoints, T t) {
		uint32_t n = (uint32_t)controlPoints.size() - 1U;
		glm::vec3 result{ 0.0f };
		T oneMinusT = 1.0f - t;
		for (uint32_t i = 0; i <= n; ++i) {
			result += (CalcBinomialCoefficient(n, i) * std::pow(oneMinusT, (T)(n - i)) * std::pow(t, (T)i)) * controlPoints[i].Position;
		}
		return result;
	}
	struct LineRendererComponent {

		struct Point {
			glm::vec3 Position{};
		};
		uint64_t Tesselation{};
		std::vector<Point> Points;
		std::vector<Point> FinalPoints;
		glm::vec4 Color{};
		LineRendererComponent() = default;
		LineRendererComponent(const LineRendererComponent&) = default;
		LineRendererComponent(std::vector<Point>&& points) 
			: Points(std::move(points))
		{
			RecalculateFinalPoints();
		}
		void RecalculateFinalPoints() {
			float offset = 1.0f / (Tesselation + 1);
			std::vector<LineRendererComponent::Point> points;
			points.reserve(Tesselation + 1);
			for (uint32_t i = 0; i < Tesselation + 1; ++i) {
				points.push_back({ Bezier(Points, offset * i) });
			}
			FinalPoints = std::move(points);
		}

	};

	struct ModelComponent {
		Ref<Model> UsedModel;
		ModelComponent() = default;
		ModelComponent(const ModelComponent&) = default;
	};



	struct MeshComponent {
		Ref<Mesh> UsedMesh;
		std::vector<Ref<Material>> UsedMaterial;
		std::vector<bool> VisibilityResults;
		MeshComponent() = default;
		MeshComponent(const MeshComponent&) = default;
	};

	struct SkinnedMeshComponent {
		Ref<SkinnedMesh> UsedMesh;
		std::vector<Ref<Material>> UsedMaterial;
		std::vector<bool> VisibilityResults;
		UUID RootBone;
		SkinnedMeshComponent() = default;
		SkinnedMeshComponent(const SkinnedMeshComponent&) = default;
	};

	struct DirectionalLightComponent {
		glm::vec3 Color = glm::vec3(1.0);
		float SplitDistances[4] = { 0.1f, 0.3f, 0.5f, 1.0f };
		float MaxDistance = 1000.0f;
		float PancakeSize = 20.0f;
		float SplitLambda = 0.95f;
		float FadeStart = 1.0f;
		float LightSize = 0.0f;
	};


	struct CameraComponent
	{
		SceneCamera Camera;
		bool Primary = true; // TODO: think about moving to Scene
		bool FixedAspectRatio = false;

		CameraComponent() = default;
		CameraComponent(const CameraComponent&) = default;
	};

	class ScriptableEntity;

	struct ScriptComponent {
		std::vector<std::string> ScriptNames;
		ScriptComponent() = default;
		ScriptComponent(const ScriptComponent&) = default;
	};
	struct NativeScriptComponent
	{
		ScriptableEntity* Instance = nullptr;

		ScriptableEntity*(*InstantiateScript)();
		void (*DestroyScript)(NativeScriptComponent*);

		template<typename T>
		void Bind()
		{
			InstantiateScript = []() { return static_cast<ScriptableEntity*>(new T()); };
			DestroyScript = [](NativeScriptComponent* nsc) { delete nsc->Instance; nsc->Instance = nullptr; };
		}
	};

	struct AnimationPlayerComponent {
		enum class AnimationOnFinishAction {
			None = 0,
			Repeat
		};
		enum class PlayerState {
			Playing,
			Paused
		};
		
		float Time = 0.0f;
		
		float PlaybackSpeed = 1.0f;
		
		PlayerState State = PlayerState::Paused;
		AnimationOnFinishAction FinishAction = AnimationOnFinishAction::None;

		Ref<AnimationTree> Animation;

		AnimationPlayerComponent() = default;
		AnimationPlayerComponent(const AnimationPlayerComponent&) = default;
	};

	
	//Physics
	#pragma region Physics
	struct Rigidbody2DComponent {
		enum class BodyType {
			Static=0,Dynamic,Kinematic
		};
		BodyType Type = BodyType::Static;
		bool FixedRotation;
		void* RuntimeBody = nullptr;

		Rigidbody2DComponent() = default;
		Rigidbody2DComponent(const Rigidbody2DComponent&) = default;
	};

	struct BoxCollider2DComponent {
		glm::vec2 Offset = { 0.0f,0.0f };
		glm::vec2 Size = { 0.5f,0.5f };

		float Density = 1.0f;
		float Friction = .5f;
		float Restitution = 0.0f;
		float RestitutionThreshold = 0.5f;


		void* RuntimeBody = nullptr;

		BoxCollider2DComponent() = default;
		BoxCollider2DComponent(const BoxCollider2DComponent&) = default;
	};
	struct CircleCollider2DComponent {
		glm::vec2 Offset = { 0.0f,0.0f };
		float Radius = .5f;

		float Density = 1.0f;
		float Friction = .5f;
		float Restitution = 0.0f;
		float RestitutionThreshold = 0.5f;


		void* RuntimeBody = nullptr;

		CircleCollider2DComponent() = default;
		CircleCollider2DComponent(const CircleCollider2DComponent&) = default;
	};

	struct TextComponent {

		uint64_t RenderableCharacters = 0;
		std::string TextContent;
		glm::vec4 Color = glm::vec4(1.0f);

		Ref<Font> TextFont = RendererGlobals::GetDefaultFont();

		glm::vec4 BorderColor = glm::vec4(1.0f, 0.0f, 0.0f, 1.0f);
		float BorderThickness = 0.2f;
		float Kerning = 0.0f;

		TextComponent() = default;
		TextComponent(const TextComponent&) = default;
	};

#pragma endregion

}
