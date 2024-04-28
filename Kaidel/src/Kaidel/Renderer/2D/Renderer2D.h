#pragma once
#include "Kaidel/Renderer/GraphicsAPI/Framebuffer.h"
#include "Kaidel/Renderer/2D/Material2D.h"
#include "Kaidel/Core/BoundedVector.h"

#include "Kaidel/Renderer/GraphicsAPI/VertexArray.h"
#include "Kaidel/Renderer/GraphicsAPI/Buffer.h"
#include "Kaidel/Renderer/GraphicsAPI/Shader.h"
#include "Kaidel/Renderer/GraphicsAPI/UniformBuffer.h"


#include <glm/glm.hpp>

namespace Kaidel {

	struct _SpriteVertexBase {
		glm::vec3 Position;
		glm::vec2 TexCoords;
		int32_t MaterialID;
	};

	template<uint32_t Size>
	struct _SpriteVertex : public _SpriteVertexBase {
		uint8_t AdditionalDataBlock[Size];
	};

	template<>
	struct _SpriteVertex<0> : public _SpriteVertexBase {};

	struct _LineVertexBase {
		glm::vec3 Position;
		glm::vec4 Color;
	};

	template<uint32_t Size>
	struct _LineVertex : public _LineVertexBase {
		uint8_t AdditionalDataBlock[Size];
	};

	template<>
	struct _LineVertex<0> : public _LineVertexBase {};


	struct _BezierVertexBase {
		glm::vec3 Position;
	};


	template<uint32_t Size>
	struct _BezierVertex : public _BezierVertexBase {
		uint8_t AdditionalDataBlock[Size];
	};

	template<>
	struct _BezierVertex<0> : public _BezierVertexBase {};


	struct _PointVertexBase {
		glm::vec3 Position;
		glm::vec4 Color;
	};

	template<uint32_t Size>
	struct _PointVertex : public _PointVertexBase {
		uint8_t AdditionalDataBlock[Size];
	};

	template<>
	struct _PointVertex<0> : public _PointVertexBase {};


	using SpriteVertex = _SpriteVertex<0>;
	using LineVertex = _LineVertex<0>;
	using BezierVertex = _BezierVertex<0>;
	using PointVertex = _PointVertex<0>;

	struct Renderer2DBeginData {
		glm::mat4 CameraVP;
		Ref<Framebuffer> OutputBuffer;
	};

	/*struct CustomRenderer2D {
		Ref<VertexArray> VAO = {};
		Ref<VertexBuffer> VBO = {};
		Ref<IndexBuffer> IBO = {};
		Ref<Shader> Shader = {};
	};*/


	struct CustomRenderer {
		Ref<VertexArray> VAO;
		Ref<Shader> Shader;
	};




#pragma region RenedereringLimits
	static inline constexpr uint32_t MaxSprites = 2048;
	static inline constexpr uint32_t MaxSpriteVertices = MaxSprites * 4;
	static inline constexpr uint32_t MaxSpriteIndices = MaxSprites * 6;

	static inline constexpr uint32_t MaxLines = 2048;
	static inline constexpr uint32_t MaxLineVertices = MaxLines * 2;

	static inline constexpr uint32_t MaxPoints = 2048;
#pragma endregion
	class Renderer2D {
	public:


		static void Init();
		static void Shutdown();
		static void Begin(const Renderer2DBeginData& beginData);
		static void DrawSprite(const glm::mat4& transform, Ref<Material2D> material);
		static void DrawBezier(const glm::mat4& transform, const std::vector<glm::vec3>& points, const glm::vec4& color, float increment = 0.001);
		static void DrawLine(const glm::vec3& p0, const glm::vec3& p1, const glm::vec4& color);





		template<uint32_t Size = 0>
		static void DrawPoint(const glm::vec3& position, const glm::vec4& color, void* data = nullptr) {
			auto bvi = VerticesBuffer<_PointVertex<Size>, MaxPoints>::Vertices.Reserve(1);
			bvi[0] = { position,color };

			if constexpr (Size != 0) {
				if (data != nullptr) {
					std::memcpy(bvi[0].AdditionalDataBlock, data, Size);
				}
			}
			++s_Renderer2DData.PointRendererData.PointsWaitingForRender;
		}


		static void FlushSprites();
		static void FlushLines();

		template<uint32_t Size = 0>
		static void FlushPoints() {
			auto& vertices = VerticesBuffer<_PointVertex<Size>, MaxPoints>::Vertices;
			auto& renderer = s_Renderer2DData.PointRendererData.CustomRenderers.top();
			if (s_Renderer2DData.PointRendererData.PointsWaitingForRender) {
				s_Renderer2DData.OutputBuffer->Bind();
				renderer.VAO->GetVertexBuffers().front()->SetData(vertices.Get(), (uint32_t)(vertices.Size() * sizeof(_PointVertex<Size>)));
				renderer.Shader->Bind();
				RenderCommand::SetCullMode(CullMode::None);
				RenderCommand::DrawPoints(renderer.VAO, s_Renderer2DData.PointRendererData.PointsWaitingForRender);
				s_Renderer2DData.PointRendererData.RenderedPointCount += s_Renderer2DData.PointRendererData.PointsWaitingForRender;
				s_Renderer2DData.PointRendererData.PointsWaitingForRender = 0;
				s_Renderer2DData.OutputBuffer->Unbind();
				vertices.Reset();
			}
		}

		static void End();

		
		CustomRenderer CreatePointRenderer(const std::initializer_list<BufferElement>& additionalElements, Ref<Shader> shader);
		static void PushPointRenderer(const CustomRenderer& renderer);
		static void PopPointRenderer();

	private:


		/*template<typename T,uint64_t RendererID>
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
		}*/
		
		/*template<typename T, uint64_t RendererID>
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
		}*/
	private:
		

		static Ref<Material2D> GetDefaultMaterial();
		static Ref<Framebuffer> GetOutputFramebuffer();
		static void GetSegmentCount(float totalSegmentCount, float* lineCount, float* segmentPerLineCount);

		

	private:
		friend struct SpriteRendererData;
		friend struct LineRendererData;
		friend struct PointRendererData;
		/*template<typename T,uint64_t RendererID>
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
		};*/

		template<typename T,uint32_t MaxVertices>
		struct VerticesBuffer {
			static inline BoundedVector<T> Vertices = { 0,MaxVertices,[](auto data,uint64_t size) {} };
		};

		struct SpriteRendererData {


			SpriteVertex DefaultSpriteVertices[4];

			Ref<VertexArray> SpriteVAO;
			Ref<VertexBuffer> SpriteVBO;
			Ref<Shader> SpriteShader;

			uint32_t RenderedSpriteCount = 0;
			uint32_t SpritesWaitingForRender = 0;

			std::mutex SpriteRenderingMutex;
			uint32_t* SetupSpriteIndices() {
				uint32_t* quadIndices = new uint32_t[MaxSpriteIndices];

				uint32_t offset = 0;
				for (uint32_t i = 0; i < MaxSpriteIndices; i += 6)
				{
					quadIndices[i + 0] = offset + 0;
					quadIndices[i + 1] = offset + 1;
					quadIndices[i + 2] = offset + 2;

					quadIndices[i + 3] = offset + 2;
					quadIndices[i + 4] = offset + 3;
					quadIndices[i + 5] = offset + 0;

					offset += 4;
				}
				return quadIndices;
			}
			void Init();
		};

		struct LineRendererData {

			Ref<VertexArray> LineVAO;
			Ref<VertexBuffer> LineVBO;
			Ref<Shader> LineShader;

			uint32_t RenderedLineCount = 0;
			uint32_t LinesWaitingForRender = 0;

			void Init();
		};

		struct BezierRendererData {

			Ref<Shader> BezierShader;

			Ref<VertexArray> BezierVAO;
			Ref<VertexBuffer> BezierVBO;

			void Init();
		};

		struct PointRendererData {



			uint32_t RenderedPointCount = 0;
			uint32_t PointsWaitingForRender = 0;

			std::stack<CustomRenderer> CustomRenderers;

			void Init();
		};


		struct Renderer2DData {


			Ref<Framebuffer> OutputBuffer;

			struct Camera {
				glm::mat4 CameraViewProjection;
			};

			Camera CameraBuffer;

			Ref<UniformBuffer> CameraUniformBuffer;
			Ref<Material2D> DefaultMaterial;
			SpriteRendererData SpriteRendererData;
			LineRendererData LineRendererData;
			BezierRendererData BezierRendererData;
			PointRendererData PointRendererData;
		};
		static inline Renderer2DData s_Renderer2DData;
	};

	

}
