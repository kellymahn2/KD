#pragma once
#include "Kaidel/Renderer/GraphicsAPI/Framebuffer.h"
#include "Kaidel/Renderer/2D/Material2D.h"
#include "Kaidel/Core/BoundedVector.h"
#include <glm/glm.hpp>

namespace Kaidel {

	struct SpriteVertex {
		glm::vec3 Position;
		glm::vec2 TexCoords;
		int32_t MaterialID;
	};

	struct LineVertex {
		glm::vec3 Position;
		glm::vec4 Color;
	};

	struct BezierVertex {
		glm::vec3 Position;
	};

	struct PointVertex {
		glm::vec3 Position;
		glm::vec4 Color;
	};

	struct Renderer2DBeginData {
		glm::mat4 CameraVP;
		Ref<Framebuffer> OutputBuffer;
	};

	struct CustomRenderer2D {
		Ref<VertexArray> VAO = {};
		Ref<VertexBuffer> VBO = {};
		Ref<IndexBuffer> IBO = {};
		Ref<Shader> Shader = {};
	};


	static inline constexpr uint32_t MaxSprites = 2048;
	static inline constexpr uint32_t MaxSpriteVertices = MaxSprites * 4;
	static inline constexpr uint32_t MaxSpriteIndices = MaxSprites * 6;

	static inline constexpr uint32_t MaxLines = 2048;
	static inline constexpr uint32_t MaxLineVertices = MaxLines * 2;

	static inline constexpr uint32_t MaxPoints = 2048;

	class Renderer2D {
	public:
		static void Init();
		static void Shutdown();
		static void Begin(const Renderer2DBeginData& beginData);
		static void DrawSprite(const glm::mat4& transform, Ref<Material2D> material);
		static void DrawBezier(const glm::mat4& transform,const std::vector<glm::vec3>& points, const glm::vec4& color, float increment = 0.001);
		static void DrawLine(const glm::vec3& p0, const glm::vec3& p1, const glm::vec4& color);
		static void DrawPoint(const glm::vec3& position, const glm::vec4& color);
		static void End();

		template<typename T,uint64_t RendererID>
		static void BeginCustomSprite(const CustomRenderer2D& customSpriteRenderer) {
			CustomSpriteRendererData<T, RendererID>::CustomRenderer = customSpriteRenderer;
			CustomSpriteRendererData<T, RendererID>::SpritesWaitingForRender = 0;
			CustomSpriteRendererData<T, RendererID>::RenderedSpriteCount = 0;
		}

		template<typename T,uint64_t RendererID>
		static void BeginCustomLine(const CustomRenderer2D& customLineRenderer) {
			CustomLineRendererData<T, RendererID>::CustomRenderer = customLineRenderer;
			CustomLineRendererData<T, RendererID>::LinesWaitingForRender = 0;
			CustomLineRendererData<T, RendererID>::RenderedLineCount = 0;
		}

		template<typename T,uint64_t RendererID>
		static void BeginCustomBezier(const CustomRenderer2D& customBezierRenderer) {
			CustomBezierRendererData<T, RendererID>::CustomRenderer = customBezierRenderer;
		}
		template<typename T,uint64_t RendererID>
		static void BeginCustomPoint(const CustomRenderer2D& customPointRenderer) {
			CustomPointRendererData<T, RendererID>::CustomRenderer = customPointRenderer;
			CustomPointRendererData<T, RendererID>::PointsWaitingForRender = 0;
			CustomPointRendererData<T, RendererID>::RenderedPointCount = 0;
		}


		template<typename T,uint64_t RendererID>
		static void DrawSprite(T spriteVertices[4]) {
			using CustomRenderer = CustomSpriteRendererData<T, RendererID>;
			BoundedVector<T>& vertices = CustomRenderer::Vertices;
			auto bvi = vertices.Reserve(4);
			for (uint32_t i = 0; i < 4; ++i) {
				bvi[i] = std::move(vertex[i]);
			}

			++CustomRenderer::SpritesWaitingForRender;
		}
		template<typename T,uint64_t RendererID>
		static void DrawBezier(const std::vector<T>& points,const glm::vec4& color,const std::string& colorUniformName = "u_Color", const std::string& numControlPointsUniformName = "u_NumControlPoints", float increment = 0.001f) {
			using CustomRenderer = CustomSpriteRendererData<T, RendererID>;
			CustomRenderer2D& customRenderer = CustomRenderer::CustomRenderer;

			Ref<Framebuffer> outputBuffer = GetOutputFramebuffer();

			customRenderer.VBO->SetData(points.data(), points.size() * sizeof(T));
			customRenderer.Shader->Bind();

			customRenderer.Shader->SetInt(numControlPointsUniformName, points.size());
			customRenderer.Shader->SetFloat4(colorUniformName, color);

			float totalNumSegments = ceilf(1.0f / (float)increment);
			float numLines;
			float numSegmentsPerLine;

			GetSegmentCount(totalNumSegments, &numLines, &numSegmentsPerLine);

			outputBuffer->Bind();
			RenderCommand::SetCullMode(CullMode::None);
			RenderCommand::SetPatchVertexCount(points.size());
			RenderCommand::SetDefaultTessellationLevels({ numLines,numSegmentsPerLine,1.0,1.0 });
			RenderCommand::DrawPatches(customRenderer.VAO, points.size());
			RenderCommand::SetPatchVertexCount(3);
			RenderCommand::SetDefaultTessellationLevels();
			outputBuffer->Unbind();
		}

		template<typename T,uint64_t RendererID>
		static void DrawLine(const T& p0, const T& p1) {
			using CustomRenderer = CustomLineRendererData<T,RendererID>;
			BoundedVector<T>& vertices = CustomRenderer::Vertices;
			auto bvi = vertices.Reserve(2);
			bvi[0] = p0;
			bvi[1] = p1;
			++CustomRenderer::LinesWaitingForRender;
		}

		template<typename T,uint64_t RendererID>
		static void DrawPoint(const T& vertex) {
			using CustomRenderer = CustomPointRendererData<T,RendererID>;
			BoundedVector<T>& vertices = CustomRenderer::Vertices;
			auto bvi = vertices.Reserve(1);	
			bvi[0] = vertex;
			++CustomRenderer::PointsWaitingForRender;
		}

		template<typename T, uint64_t RendererID>
		static void EndCustomSprite() {
			FlushSprites<T, RendererID>();
		}

		template<typename T, uint64_t RendererID>
		static void EndCustomLine() {
			FlushLines<T, RendererID>();
		}

		template<typename T, uint64_t RendererID>
		static void EndCustomPoint() {
			FlushPoints<T, RendererID>();
		}
		static void FlushSprites();
		static void FlushLines();
		static void FlushPoints();
		template<typename T, uint64_t RendererID>
		static void FlushSprites() {
			using CustomRenderer = CustomSpriteRendererData<T, RendererID>;
			BoundedVector<T>& vertices = CustomRenderer::Vertices;

			CustomRenderer2D& customRenderer = CustomRenderer::CustomRenderer;

			Ref<Framebuffer> outputBuffer = GetOutputFramebuffer();
			if (CustomRenderer::SpritesWaitingForRender) {
				outputBuffer->Bind();
				customRenderer.VBO->SetData(vertices.Get(), vertices.Size() * sizeof(T));
				customRenderer.Shader->Bind();
				RenderCommand::SetCullMode(CullMode::None);
				RenderCommand::DrawIndexed(customRenderer.VAO, CustomRenderer::SpritesWaitingForRender * 6);
				CustomRenderer::RenderedSpriteCount += CustomRenderer::SpritesWaitingForRender;
				CustomRenderer::SpritesWaitingForRender = 0;
				outputBuffer->Unbind();
			}
			vertices.Reset();
		}

		template<typename T, uint64_t RendererID>
		static void FlushLines() {
			using CustomRenderer = CustomLineRendererData<T, RendererID>;

			CustomRenderer2D& customRenderer = CustomRenderer::CustomRenderer;

			BoundedVector<T>& vertices = CustomRenderer::Vertices;
			Ref<Framebuffer> outputBuffer = GetOutputFramebuffer();
			if (CustomRenderer::LinesWaitingForRender) {
				outputBuffer->Bind();
				customRenderer.VBO->SetData(vertices.Get(), vertices.Size() * sizeof(T));
				customRenderer.Shader->Bind();
				RenderCommand::DrawLines(customRenderer.VAO, CustomRenderer::LinesWaitingForRender * 2);
				CustomRenderer::RenderedLineCount += CustomRenderer::LinesWaitingForRender;
				CustomRenderer::LinesWaitingForRender = 0;
				outputBuffer->Unbind();
			}
			vertices.Reset();
		}
		template<typename T, uint64_t RendererID>
		static void FlushPoints() {
			using CustomRenderer = CustomPointRendererData<T, RendererID>;
			CustomRenderer2D& customRenderer = CustomRenderer::CustomRenderer;

			BoundedVector<T>& vertices = CustomRenderer::Vertices;
			Ref<Framebuffer> outputBuffer = GetOutputFramebuffer();
			if (CustomRenderer::PointsWaitingForRender) {
				outputBuffer->Bind();
				customRenderer.VBO->SetData(vertices.Get(), vertices.Size() * sizeof(T));
				customRenderer.Shader->Bind();
				RenderCommand::DrawPoints(customRenderer.VAO, vertices.Size());
				CustomRenderer::RenderedPointCount += CustomRenderer::PointsWaitingForRender;
				CustomRenderer::PointsWaitingForRender = 0;
				outputBuffer->Unbind();
			}
			vertices.Reset();
		}
	private:
		

		static Ref<Material2D> GetDefaultMaterial();
		static Ref<Framebuffer> GetOutputFramebuffer();
		static void GetSegmentCount(float totalSegmentCount, float* lineCount, float* segmentPerLineCount);

		

	private:
		friend struct SpriteRendererData;
		friend struct LineRendererData;
		friend struct PointRendererData;
		template<typename T,uint64_t RendererID>
		struct CustomSpriteRendererData {
			static inline BoundedVector<T> Vertices = { 0,MaxSpriteVertices,[](auto data,uint64_t size) {
				FlushSprites<T,RendererID>();
				} };
			static inline uint32_t RenderedSpriteCount = 0;
			static inline uint32_t SpritesWaitingForRender = 0;
			static inline CustomRenderer2D CustomRenderer;
		};

		template<typename T, uint64_t RendererID>
		struct CustomBezierRendererData {
			static inline CustomRenderer2D CustomRenderer;
		};

		template<typename T,uint64_t RendererID>
		struct CustomLineRendererData{

			static inline BoundedVector<T> Vertices = {0,MaxLineVertices,[](auto data,uint64_t size){
				FlushLines<T,RendererID>();
			}};
			static inline CustomRenderer2D CustomRenderer;
			static inline uint32_t LinesWaitingForRender = 0;
			static inline uint32_t RenderedLineCount = 0;
		};

		template<typename T,uint64_t RendererID>
		struct CustomPointRendererData{
			static inline BoundedVector<T> Vertices = {0,MaxPoints,[](auto data,uint64_t size){
				FlushPoints<T,RendererID>();
			}};

			static inline CustomRenderer2D CustomRenderer;
			static inline uint32_t PointsWaitingForRender = 0;
			static inline uint32_t RenderedPointCount = 0;
		};


	private:
	};

	

}
