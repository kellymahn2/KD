#pragma once

#include "Kaidel/Renderer/OrthographicCamera.h"
#include "KDpch.h"
#include "Kaidel/Renderer/Texture.h"

#include "Kaidel/Renderer/Camera.h"
#include "Kaidel/Renderer/EditorCamera.h"

#include "Kaidel/Scene/Components.h"
#include <future>
namespace Kaidel {

	struct QuadVertex;
	struct CircleVertex;
	struct LineVertex;
	class Renderer2D
	{
	public:
		static void Init();
		static void Shutdown();

		static void BeginScene(const Camera& camera, const glm::mat4& transform);
		static void BeginScene(const EditorCamera& camera);
		static void BeginScene(const OrthographicCamera& camera); // TODO: Remove
		static void EndScene();
		static void Flush();
		static void FlushQuads(QuadVertex* start,uint64_t size);
		static void FlushCircles(CircleVertex* start, uint64_t size);
		static void FlushLines(LineVertex* start, uint64_t size);
		// Primitives


		static void DrawQuad(const glm::mat4& transform, const glm::vec4& color, uint64_t QuadIndex, int entityID=-1);
		static void DrawCircle(const glm::mat4& transform, const glm::vec4& color, uint64_t insertIndex, float Thickness = 1.0f, float fade = .005f, int entityID = -1);
		static void DrawLine(const glm::vec3& p0,const glm::vec3& p1, const glm::vec4& color,uint64_t insertIndex, int entityID = -1);




		static void DrawQuad(const glm::mat4& transform, const glm::vec4& color, int entityID = -1);
		static void DrawQuad(const glm::mat4& transform, const Ref<Texture2D>& texture, float tilingFactor = 1.0f, const glm::vec4& tintColor = glm::vec4(1.0f), int entityID = -1);

		static void DrawCircle(const glm::mat4& transform, const glm::vec4& color, float Thickness = 1.0f,float fade = .005f,int entityID = -1);

		static void DrawSprite(const glm::mat4& transform, SpriteRendererComponent& src, int entityID);
		
		static void DrawLine(const glm::vec3& p0, const glm::vec3& p1, const glm::vec4& color,int entityID=-1);
		static void DrawRect(const glm::vec3& position, const glm::vec2& size, const glm::vec4& color, int entityID = -1);
		static void DrawRect(const glm::mat4& transform, const glm::vec4& color, int entityID = -1);

		// Stats
		struct Statistics
		{
			uint32_t DrawCalls = 0;
			uint32_t QuadCount = 0;
			uint32_t CircleCount = 0;
			uint32_t GetTotalVertexCount() const { return QuadCount * 4; }
			uint32_t GetTotalIndexCount() const { return QuadCount * 6; }
		};
		static void ResetStats();
		static Statistics GetStats();
		static float GetLineWidth();
		static void SetLineWidth(float width);
		static Ref<Texture2D> GetWhite();
	private:
		static void SetVertexBufferValues(uint32_t vertexCount, const glm::mat4& transform, const glm::vec4& tintColor, const glm::vec2* textureCoords, float textureIndex, float tilingFactor, int entityID);
		static void StartBatch();
		static void StartQuadBatch();
		static void StartCircleBatch();
		static void StartLineBatch();
		static void NextBatch();
		enum class _LaunchType {
			MT = 1,ST = 2
		};
		
		friend struct Renderer2DData;
	};

}
