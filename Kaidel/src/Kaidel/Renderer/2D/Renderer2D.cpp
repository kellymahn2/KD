#include "KDpch.h"

#include "Kaidel/Core/BoundedVector.h"
#include "Kaidel/Renderer/RenderCommand.h"
#include "Kaidel/Renderer/GraphicsAPI/VertexArray.h"
#include "Kaidel/Renderer/GraphicsAPI/Buffer.h"
#include "Kaidel/Renderer/GraphicsAPI/Shader.h"
#include "Kaidel/Renderer/GraphicsAPI/UniformBuffer.h"
#include "Kaidel/Renderer/GraphicsAPI/GraphicsPipeline.h"
#include "Kaidel/Renderer/GraphicsAPI/RenderPass.h"

#include "Renderer2D.h"

namespace Kaidel{
	struct Renderer2DData {

		static constexpr const uint32_t MaxSpriteCount = 10000;
		static constexpr const uint32_t MaxSpriteVertexCount = MaxSpriteCount * 4;
		static constexpr const uint32_t MaxSpriteIndexCount = MaxSpriteCount * 6;

		SpriteVertex PresetSpriteVertices[4];

		uint32_t SpritesWaitingForRender = 0;
		uint32_t SpritesRendered = 0;

		BoundedVector<SpriteVertex> BakedSpriteVertices = BoundedVector<SpriteVertex>(0, MaxSpriteVertexCount, [](SpriteVertex*, uint64_t) {});

		Ref<VertexBuffer> SpriteVertexBuffer;
		Ref<IndexBuffer> SpriteIndexBuffer;
		Ref<GraphicsPipeline> SpritePipeline;
		Ref<RenderPass> SpriteRenderPass;

		Ref<Framebuffer> OutputBuffer;
		Ref<RenderPass> OutputRenderPass;
		
		Ref<UniformBuffer> CameraUnifomBuffer;
		struct CameraUnifomData {
			glm::mat4 ViewProjection;
		};
		CameraUnifomData Camera;
    };

    static Renderer2DData* s_RendererData;

	static uint32_t* CreateSpiteIndices(uint32_t count) {
		uint32_t actualCount = count - (count % 6);
		uint32_t* indices = new uint32_t[actualCount];

		uint32_t vertex = 0;
		for (uint32_t i = 0; i < actualCount; i += 6) {
			indices[i + 0] = vertex + 0;
			indices[i + 1] = vertex + 1;
			indices[i + 2] = vertex + 2;
			
			indices[i + 3] = vertex + 2;
			indices[i + 4] = vertex + 3;
			indices[i + 5] = vertex + 0;
			vertex += 4;
		}
		return indices;
	}



    void Renderer2D::Init() {
		s_RendererData = new Renderer2DData;
		
		{
			SCOPED_TIMER("Vertex buffer");
			VertexBufferSpecification spec{};
			spec.Data = nullptr;
			spec.Size = 0;
			spec.MemoryType = VertexBufferMemoryType::Dynamic;

			s_RendererData->SpriteVertexBuffer = VertexBuffer::Create(spec);
		}

		{
			SCOPED_TIMER("Index buffer");
			uint32_t* indices = CreateSpiteIndices(s_RendererData->MaxSpriteIndexCount);
			s_RendererData->SpriteIndexBuffer = IndexBuffer::Create(indices, s_RendererData->MaxSpriteIndexCount);
			delete[] indices;
		}

		{
			SCOPED_TIMER("Uniform buffer");
			s_RendererData->CameraUnifomBuffer = UniformBuffer::Create(sizeof(Renderer2DData::CameraUnifomData), 0);
		}


		{
			SCOPED_TIMER("Render pass");
			RenderPassSpecification spec{};
			spec.BindingPoint = RenderPassBindPoint::Graphics;
			spec.OutputImages = { RenderPassAttachmentSpecification(TextureFormat::RGBA8, RenderPassImageLoadOp::Clear,
									RenderPassImageStoreOp::Store, RenderPassImageLayout::Undefined, RenderPassImageLayout::Color,
										RenderPassImageLayout::Color) };
			spec.OutputDepthAttachment = RenderPassAttachmentSpecification(TextureFormat::Depth32F, RenderPassImageLoadOp::DontCare,
				RenderPassImageStoreOp::DontCare, RenderPassImageLayout::Undefined, RenderPassImageLayout::Depth, RenderPassImageLayout::Depth);
			s_RendererData->SpriteRenderPass = RenderPass::Create(spec);
		}

		{
			Ref<SingleShader> fs;
			Ref<SingleShader> vs;
			
			
			{

				SCOPED_TIMER("Vertex shader");

				SingleShaderSpecification vsSpec{};

				vsSpec.ControlString = "assets/shaders/GeometryPass/Geometry_Sprite_VS_2D.glsl";
				vsSpec.Type = ShaderType::VertexShader;

				vs = SingleShader::CreateShader(vsSpec);
			}



			{
				SCOPED_TIMER("Fragment shader");

				SingleShaderSpecification fsSpec{};

				fsSpec.ControlString = "assets/shaders/GeometryPass/Geometry_Sprite_FS_2D.glsl";
				fsSpec.Type = ShaderType::FragmentShader;

				fs = SingleShader::CreateShader(fsSpec);
			}

			SCOPED_TIMER("Graphics pipeline");

			GraphicsPipelineSpecification spec{};
			spec.Culling = CullMode::None;
			spec.FrontCCW = true;
			spec.LineWidth = 1.0f;
			spec.PrimitveTopology = GraphicsPrimitveTopology::TriangleList;
			spec.RenderPass = s_RendererData->SpriteRenderPass;
			spec.Viewport = { 1280,720,0,0,0,1.0f };
			spec.Stages = {
				{ShaderType::VertexShader,vs },
				{ ShaderType::FragmentShader,fs }
			};

			GraphicsPipelineInputBufferSpecification bufferSpec{};
			bufferSpec.Elements = {
				{"a_Position",GraphicsPipelineInputDataType::Float3},
				{"a_Color",GraphicsPipelineInputDataType::Float4},
			};
			bufferSpec.InputRate = GraphicsPipelineInputRate::PerVertex;
			spec.InputLayout = GraphicsPipelineInputLayout({ bufferSpec });
			spec.UsedUniformBuffers = { s_RendererData->CameraUnifomBuffer };
			s_RendererData->SpritePipeline = GraphicsPipeline::Create(spec);
			s_RendererData->SpritePipeline->Finalize();
		}

		s_RendererData->PresetSpriteVertices[0] = { glm::vec3{-.5f,-.5f,.0f} ,glm::vec4{1.0f}};
		s_RendererData->PresetSpriteVertices[1] = { glm::vec3{ .5f,-.5f,.0f} ,glm::vec4{1.0f}};
		s_RendererData->PresetSpriteVertices[2] = { glm::vec3{ .5f, .5f,.0f} ,glm::vec4{1.0f}};
		s_RendererData->PresetSpriteVertices[3] = { glm::vec3{-.5f, .5f,.0f} ,glm::vec4{1.0f}};
	}
    void Renderer2D::Shutdown() {
		delete s_RendererData;
	}
    void Renderer2D::Begin(const glm::mat4& cameraVP,Ref<Framebuffer> outputColorBuffer) {
		s_RendererData->SpritesRendered = 0;

		s_RendererData->Camera.ViewProjection = cameraVP;
		s_RendererData->CameraUnifomBuffer->SetData(&s_RendererData->Camera, sizeof(Renderer2DData::CameraUnifomData));
		s_RendererData->OutputBuffer = outputColorBuffer;
		StartSpriteBatch();
	}
    void Renderer2D::End() {
		FlushSprites();
	}
    void Renderer2D::DrawSprite(const glm::mat4& transform,const glm::vec4& materials) {
		SpriteVertex vertices[4] = {};
		for (uint32_t i = 0; i < 4; ++i) {
			vertices[i].Position = transform * glm::vec4(s_RendererData->PresetSpriteVertices[i].Position,1.0f);
			vertices[i].Color = materials;
		}
		AddSprite(vertices);
	}
	void Renderer2D::AddSprite(SpriteVertex vertices[4]) {
		if (!s_RendererData->BakedSpriteVertices.CanReserveWithoutOverflow(4)) {
			//TODO: this is wrong, instead flush contents of command buffer so far.
			FlushSprites();
		}
		auto bvi = s_RendererData->BakedSpriteVertices.Reserve(4);
		memcpy(&bvi[0], vertices, 4 * sizeof(SpriteVertex));
		s_RendererData->SpritesWaitingForRender++;
	}

	void Renderer2D::StartSpriteBatch()
	{
		s_RendererData->SpritesWaitingForRender = 0;
		s_RendererData->BakedSpriteVertices.Reset();
	}

	void Renderer2D::FlushSprites()
	{
		if (s_RendererData->SpritesWaitingForRender) {
			s_RendererData->SpriteVertexBuffer->SetData(s_RendererData->BakedSpriteVertices.Get(), s_RendererData->BakedSpriteVertices.Size() * sizeof(SpriteVertex));
			
			RenderCommand::BindVertexBuffers({ s_RendererData->SpriteVertexBuffer });
			RenderCommand::BindIndexBuffer(s_RendererData->SpriteIndexBuffer);
			RenderCommand::BeginRenderPass(s_RendererData->OutputBuffer,s_RendererData->SpriteRenderPass);
			RenderCommand::BindGraphicsPipeline(s_RendererData->SpritePipeline);
			RenderCommand::DrawIndexed(s_RendererData->SpritesWaitingForRender * 6);
			RenderCommand::EndRenderPass();
		}
	}



}
