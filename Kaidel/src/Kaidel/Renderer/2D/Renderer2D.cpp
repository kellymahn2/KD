#include "KDpch.h"

#include "Kaidel/Core/BoundedVector.h"
#include "Kaidel/Renderer/RenderCommand.h"
#include "Kaidel/Renderer/GraphicsAPI/Shader.h"
#include "Kaidel/Renderer/GraphicsAPI/ShaderLibrary.h"
#include "Kaidel/Renderer/GraphicsAPI/UniformBuffer.h"
#include "Kaidel/Renderer/GraphicsAPI/GraphicsPipeline.h"
#include "Kaidel/Renderer/GraphicsAPI/RenderPass.h"
#include "Kaidel/Renderer/GraphicsAPI/DescriptorSet.h"
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
		Ref<RenderPass> SpriteRenderPass;
		Ref<GraphicsPipeline> SpritePipeline;


		Ref<Framebuffer> OutputBuffer;
		
		Ref<UniformBuffer> CameraUnifomBuffer;
		struct CameraUnifomData {
			glm::mat4 ViewProjection;
		};
		CameraUnifomData Camera;
		Ref<DescriptorSet> CameraDescriptorSet;

		Ref<DescriptorSet> TextureDescriptorSet;
		Ref<Sampler> TextureSampler;
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
		//s_RendererData = new Renderer2DData;
		//
		//{
		//	SCOPED_TIMER("Vertex buffer");
		//	s_RendererData->SpriteVertexBuffer = VertexBuffer::Create(4);
		//}

		//{
		//	SCOPED_TIMER("Index buffer");
		//	uint32_t* indices = CreateSpiteIndices(s_RendererData->MaxSpriteIndexCount);
		//	s_RendererData->SpriteIndexBuffer = IndexBuffer::Create(indices, s_RendererData->MaxSpriteIndexCount);
		//	delete[] indices;
		//}

		//{
		//	SCOPED_TIMER("Uniform buffer");
		//	s_RendererData->CameraUnifomBuffer = UniformBuffer::Create(sizeof(Renderer2DData::CameraUnifomData), 0);
		//}

		//{
		//	SCOPED_TIMER("Render pass");
		//	RenderPassSpecification spec{};
		//	{
		//		AttachmentSpecification attach{};
		//		attach.AttachmentFormat = Format::RGBA8UN;
		//		attach.InitialLayout = ImageLayout::General;
		//		attach.FinalLayout = ImageLayout::ShaderReadOnlyOptimal;
		//		attach.LoadOp = AttachmentLoadOp::Load;
		//		attach.StoreOp = AttachmentStoreOp::Store;
		//		spec.OutputColors.push_back(AttachmentSpecification{ attach });
		//	}
		//	//spec.OutputDepth = { Format::Depth32F };
		//	//spec.OutputDepth.FinalLayout = ImageLayout::DepthAttachmentOptimal;
		//	s_RendererData->SpriteRenderPass = RenderPass::Create(spec);
		//}

		//{
		//	SCOPED_TIMER("Graphics pipeline");
		//	Ref<ShaderModule> vs = ShaderLibrary::LoadShader("assets/_shaders/SpriteVS.glsl", ShaderType::VertexShader);
		//	Ref<ShaderModule> fs = ShaderLibrary::LoadShader("assets/_shaders/SpriteFS.glsl", ShaderType::FragmentShader);

		//	GraphicsPipelineSpecification spec{};
		//	spec.CullMode = PipelineCullMode::None;
		//	spec.FrontCCW = true;
		//	spec.Name = "Sprite pipeline";
		//	spec.UsedRenderPass = s_RendererData->SpriteRenderPass;
		//	spec.VertexShader = vs;
		//	spec.FragmentShader = fs;

		//	VertexInpuBinding binding{};
		//	binding.Elements = { {"a_Position",VertexInputType::Float3},{"a_Color",VertexInputType::Float4},{"a_UVCoords",VertexInputType::Float3} };
		//	binding.InputRate = VertexInputRate::Vertex;
		//	VertexInputSpecification input{};
		//	input.Bindings.push_back(binding);
		//	spec.InputSpecification = input;

		//	s_RendererData->SpritePipeline = GraphicsPipeline::Create(spec);
		//}

		//{
		//	SCOPED_TIMER("Camera Descriptor set");
		//	s_RendererData->CameraDescriptorSet = DescriptorSet::Create(s_RendererData->SpritePipeline, 0);
		//	
		//}

		//{
		//	SCOPED_TIMER("Texture Descriptor set");
		//	s_RendererData->TextureDescriptorSet = DescriptorSet::Create(s_RendererData->SpritePipeline, 1);
		//}

		//{
		//	SCOPED_TIMER("Texture Sampler");
		//	SamplerParameters params{};
		//	params.MipmapMode = SamplerMipMapMode::Linear;
		//	params.MinificationFilter = SamplerFilter::Linear;
		//	params.MagnificationFilter = SamplerFilter::Linear;
		//	params.BorderColor = SamplerBorderColor::None;
		//	params.AddressModeU = SamplerAddressMode::ClampToEdge;
		//	params.AddressModeV = SamplerAddressMode::ClampToEdge;
		//	params.AddressModeW = SamplerAddressMode::ClampToEdge;
		//	s_RendererData->TextureSampler = SamplerState::Create(params);
		//}

		//s_RendererData->PresetSpriteVertices[0] = { glm::vec3{-.5f,-.5f,.0f} ,glm::vec4{1.0f}};
		//s_RendererData->PresetSpriteVertices[1] = { glm::vec3{ .5f,-.5f,.0f} ,glm::vec4{1.0f}};
		//s_RendererData->PresetSpriteVertices[2] = { glm::vec3{ .5f, .5f,.0f} ,glm::vec4{1.0f}};
		//s_RendererData->PresetSpriteVertices[3] = { glm::vec3{-.5f, .5f,.0f} ,glm::vec4{1.0f}};
		//s_RendererData->SpriteRenderPass->SetClearValue(0, { AttachmentColorClearValue(glm::vec4{1.0f}) });
	}
    void Renderer2D::Shutdown() {
		delete s_RendererData;
	}
	//	//FlushSprites();
	//}
	//
	//void Renderer2D::DrawSprite(const glm::mat4& transform, const glm::vec4& color, const SamplingRegion& region)
	//{
	//	SpriteVertex vertices[4] = {};
	//
	//	glm::vec2 uvs[4] = {};
	//	//Bottom-Left
	//	uvs[0] = { region.UV0.x,region.UV1.y };
	//	//Bottom-Right
	//	uvs[1] = region.UV1;
	//	//Top-Right
	//	uvs[2] = { region.UV1.x,region.UV0.y };
	//	//Top-Left
	//	uvs[3] = region.UV0;
	//
	//	for (uint32_t i = 0; i < 4; ++i) {
	//		vertices[i].Position = transform * glm::vec4(s_RendererData->PresetSpriteVertices[i].Position, 1.0f);
	//		vertices[i].Color = color;
	//		vertices[i].UV = { uvs[i],region.Layer };
	//	}
	//}
}
