#include "KDpch.h"
#include "SceneRenderer.h"
#include "Components.h"
#include "Scene.h"
#include "Entity.h"
#include "Kaidel/Renderer/Text/Font.h"
#include "Kaidel/Renderer/Text/MSDFData.h"
#include "temp.h"

#include "Kaidel/Renderer/GraphicsAPI/PerFrameResource.h"

#include "Kaidel/Renderer/GraphicsAPI/UniformBuffer.h"
#include "Kaidel/Renderer/2D\Renderer2D.h"
#include "Kaidel/Core/JobSystem.h"
#include "Kaidel/Core/Timer.h"
#include "Kaidel/Renderer/RenderCommand.h"
#include "Kaidel/Renderer/GraphicsAPI/VertexBuffer.h"
#include "Kaidel/Renderer/GraphicsAPI/IndexBuffer.h"
#include "Kaidel/Renderer/GraphicsAPI/ShaderLibrary.h"
#include "Kaidel/Renderer/GraphicsAPI/TextureLibrary.h"
#include "Kaidel/Renderer/DescriptorSetPack.h"
#include "Kaidel/Scene/Material.h"
#include "Kaidel/Scene/Model.h"

#include <glm/glm.hpp>
#include <glm/gtx/compatibility.hpp>
#include <chrono>
#include <cmath>
#include <iostream>

#include "stb_image.h"
#define MAX_LIGHT_COUNT 100

#define SHADOW_MAP_SIZE 4096
#define SHADOW_MAPPING

namespace Kaidel {
	struct DeferredPassData {
		Ref<RenderPass> RenderPass;
		PerFrameResource<Ref<Framebuffer>> Output;
		//Ref<GraphicsPipeline> Pipeline;
	};
	
	struct SpriteData {
		static const constexpr uint64_t MaxTextureCount = 32;
		static const constexpr uint64_t MaxSpriteCount = 10000;
		static const constexpr uint64_t MaxVertexCount = MaxSpriteCount * 4;
		static const constexpr uint64_t MaxIndexCount = MaxSpriteCount * 6;

		Ref<IndexBuffer> SpriteIndexBuffer;
		Ref<GraphicsPipeline> SpriteGraphicsPipeline;
		std::vector<SpriteVertex> Vertices;
		uint64_t VertexCount;

		struct BatchDrawParams {
			uint64_t BufferIndex;
			uint64_t VertexCount;
			uint64_t TextureIndex;
			uint64_t BufferOffset;
		};

		struct BatchingData {
			BatchingData() {
				Buffers.push_back(VertexBuffer::Create(nullptr, 0));
				TextureSet.push_back(CreateTextureDescriptorForSprites());
			}

			void Reset() {
				BufferOffset = 0;
				BufferCount = 1;
				TextureSetCount = 1;
				TextureCount = 1;
			}

			std::vector<Ref<VertexBuffer>> Buffers;
			uint32_t BufferCount = 1;
			uint64_t BufferOffset = 0;

			std::vector<Ref<DescriptorSet>> TextureSet;
			uint32_t TextureSetCount = 1;
			uint32_t TextureCount = 1;

			Ref<DescriptorSet> CreateTextureDescriptorForSprites() {
				Ref<Shader> s = ShaderLibrary::LoadOrGetNamedShader("SpriteRender", "assets/_shaders/SpriteRender.glsl");
				Ref<DescriptorSet> set = DescriptorSet::Create(s, 0);

				//Set all the textures in the list to flat white texture.
				for (uint32_t i = 0; i < SpriteData::MaxTextureCount; ++i) {
					set->Update(RendererGlobals::GetSingleColorTexture(glm::vec4(1.0f)), {}, ImageLayout::ShaderReadOnlyOptimal, 0, i);
				}
				//Set a global sampler for all of them.
				set->Update({}, RendererGlobals::GetSamler(SamplerFilter::Linear, SamplerMipMapMode::Linear), {}, "u_Sampler");

				return set;
			}
		};
		std::vector<BatchDrawParams> DrawParams;
		PerFrameResource<BatchingData> Batches;
	};

	struct TextData {
		struct TextVertex {
			glm::vec3 Position;
			glm::vec4 Color;
			glm::vec2 TexCoord;
		};
		
		Ref<GraphicsPipeline> Pipeline;
		Ref<RenderPass> TextRenderPass;
		std::vector<TextVertex> Vertices;
		uint64_t VertexCount;

		PerFrameResource<Ref<VertexBuffer>> Buffer;
	};

#ifdef SHADOW_MAPPING
	struct ShadowData {
		Ref<RenderPass> RenderPass;
		Ref<GraphicsPipeline> Pipeline;
		PerFrameResource<Ref<Framebuffer>> Framebuffer;
		Ref<Sampler> ShadowSampler;
		PerFrameResource<DescriptorSetPack> Pack;
	};
#endif
	static struct Data {
		Ref<VertexBuffer> CubeVertexBuffer;
		Ref<IndexBuffer> CubeIndexBuffer;
		Ref<ComputePipeline> ClusterPipeline;
		PerFrameResource<DescriptorSetPack> ClusterPipelinePack;
		Ref<ComputePipeline> LightCullPipeline;
		PerFrameResource<DescriptorSetPack> LightCullPipelinePack;

		//PerFrameResource<Ref<UniformBuffer>> CameraBuffers;
		//PerFrameResource<Ref<DescriptorSet>> CameraBufferSets;

		PerFrameResource<Ref<StorageBuffer>> Clusters;
		PerFrameResource<Ref<StorageBuffer>> ClusterGrids;

		DeferredPassData DeferredPass;
		
		//Screen pass
		Ref<VertexBuffer> ScreenNDC;
		Ref<RenderPass> ScreenRenderPass;
		PerFrameResource<Ref<Framebuffer>> HDROutputs;
		Ref<GraphicsPipeline> ScreenPipeline;
		PerFrameResource<DescriptorSetPack> ScreenPipelinePack;

		//tonemap pass
		Ref<RenderPass> TonemapRenderPass;
		PerFrameResource<Ref<Framebuffer>> Outputs;
		Ref<GraphicsPipeline> TonemapPipeline;
		PerFrameResource<DescriptorSetPack> TonemapPipelinePack;

		PerFrameResource<Ref<UniformBuffer>> DirectionalLight;
		PerFrameResource<Ref<DescriptorSet>> DirectionalLightSet;

		SpriteData Sprites;
		TextData Texts;

		ShadowData Shadows;
		Ref<Sampler> GlobalSampler;
		uint32_t Width = 1280, Height = 720;
	}*s_Data;

	Ref<RenderPass> GetDeferredPassRenderPass() {
		SceneRenderer r(nullptr);
		return s_Data->DeferredPass.RenderPass;
	}

	TextureSamples samples = TextureSamples::x1;

	struct Light {
		glm::vec4 Position;
		glm::vec4 Color;
		float Radius;
	};

	std::vector<Light> lights;

	struct DirectionalLight {
		glm::mat4 GlobalShadowMatrix;
		glm::mat4 ViewProj[4];
		alignas(16) glm::vec3 Direction;
		alignas(16) glm::vec3 Color;
		alignas(16) float SplitDistances[4];
		alignas(16) glm::vec4 CascadeOffsets[4];
		alignas(16) glm::vec4 CascadeScales[4];
		float FadeStart;
	} DLight;

	#pragma region Clusters

	struct Cluster {
		glm::vec4 Min;
		glm::vec4 Max;
	};

	struct ClusterGrid {
		uint32_t Count;
		uint32_t Indices[50];
		uint32_t Padding;
	};

	static glm::uvec3 s_ClusterDimensions = { 16,9,24 };
	static uint64_t s_ClusterGridLength = s_ClusterDimensions.x * s_ClusterDimensions.y * s_ClusterDimensions.z;

	static void CreateClusterResources() {
		s_Data->ClusterPipeline = ComputePipeline::Create(ShaderLibrary::LoadShader("Cluster", "assets/_shaders/ClusterComp.shader"));
		for (auto& cluster : s_Data->Clusters) {
			cluster =
				StorageBuffer::Create(s_ClusterGridLength * sizeof(Cluster));
		}

		uint32_t i = 0;
		for (auto& pack : s_Data->ClusterPipelinePack) {
			pack =
				DescriptorSetPack(ShaderLibrary::GetNamedShader("Cluster"), {});
			pack.GetSet(0)->Update(s_Data->Clusters.GetResources()[i], 0);
			++i;
		}
	}

	void SceneRenderer::MakeClusters(const glm::mat4& invProj, float zNear, float zFar, const glm::vec2& screenSize) {
		RenderCommand::BindComputePipeline(s_Data->ClusterPipeline);
		s_Data->ClusterPipelinePack->Bind();
		RenderCommand::BindPushConstants(ShaderLibrary::GetNamedShader("Cluster"), 0,
			invProj, zNear, zFar, screenSize);
		RenderCommand::Dispatch(s_ClusterDimensions.x, s_ClusterDimensions.y, s_ClusterDimensions.z);
	}

	void SceneRenderer::InsertClusterBarrier() {
		BufferMemoryBarrier barrier{};
		barrier.Buffer = *s_Data->Clusters;
		barrier.Offset = 0;
		barrier.Size = -1;
		barrier.Src = AccessFlags_ShaderWrite;
		barrier.Dst = AccessFlags_ShaderRead;
		RenderCommand::PipelineBarrier(
			PipelineStages_ComputeShader,
			PipelineStages_ComputeShader,
			{},
			{ barrier },
			{}
		);
	}
	#pragma endregion

	#pragma region Light Culling
	static void CreateLightCullResources() {

		s_Data->LightCullPipeline = ComputePipeline::Create(ShaderLibrary::LoadShader("LightCull", "assets/_shaders/ClusterCullComp.shader"));

		for (auto& grid : s_Data->ClusterGrids) {
			grid = StorageBuffer::Create(s_ClusterGridLength * sizeof(ClusterGrid));
		}

		uint32_t i = 0;
		for (auto& pack : s_Data->LightCullPipelinePack) {
			pack =
				DescriptorSetPack(ShaderLibrary::GetNamedShader("LightCull"), 
					{ {0,s_Data->ClusterPipelinePack.GetResources()[i].GetSet(0)} });
			pack.GetSet(1)->Update(StorageBuffer::Create(MAX_LIGHT_COUNT * sizeof(Light)),0)
				.Update(s_Data->ClusterGrids.GetResources()[i],1);
			++i;
		}
	}
	
	void SceneRenderer::MakeLightGrids(const glm::mat4& view) {
		s_Data->LightCullPipelinePack->GetSet(1)->GetStorageBufferAtBinding(0)->SetData(lights.data(), lights.size() * sizeof(Light));

		RenderCommand::BindComputePipeline(s_Data->LightCullPipeline);
		s_Data->LightCullPipelinePack->Bind();
		RenderCommand::BindPushConstants(ShaderLibrary::GetNamedShader("LightCull"), 0,
			view, lights.size());
		RenderCommand::Dispatch(1, 1, 6);
	}
	
	void SceneRenderer::InsertLightGridBarrier() {
		BufferMemoryBarrier barrier{};
		barrier.Buffer = s_Data->LightCullPipelinePack->GetSet(1)->GetStorageBufferAtBinding(1);
		barrier.Offset = 0;
		barrier.Size = -1;
		barrier.Src = AccessFlags_ShaderWrite;
		barrier.Dst = AccessFlags_ShaderRead;
		RenderCommand::PipelineBarrier(
			PipelineStages_ComputeShader,
			PipelineStages_FragmentShader,
			{},
			{ barrier },
			{}
		);
	}
	#pragma endregion

	#pragma region 2D

	static std::array<SpriteVertex, 4> MakeSpriteVertices(const glm::mat4& transform, const glm::vec4& tint, uint32_t textureIndex, const SamplingRegion& region) {
		glm::vec2 positions[4] = {
			{-.5,-.5},
			{.5,-.5},
			{.5,.5},
			{-.5,.5}
		};

		glm::vec2 uv[4] = {
			{region.UV0.x,region.UV1.y},
			region.UV1,
			{region.UV1.x,region.UV0.y},
			region.UV0
		};

		std::array<SpriteVertex, 4> vertices{};

		for (uint32_t i = 0; i < 4; ++i) {
			SpriteVertex& vertex = vertices[i];
			vertex.Position = transform * glm::vec4(positions[i], 0, 1);
			vertex.Color = tint;
			vertex.UV = { uv[i], (float)textureIndex };
		}

		return vertices;
	}

	static void AddDraw() {
		auto& sprites = s_Data->Sprites;

		bool textureFull = sprites.Batches->TextureCount == SpriteData::MaxTextureCount;
		bool vertexFull = sprites.VertexCount + sprites.Batches->BufferOffset == SpriteData::MaxVertexCount;

		SpriteData::BatchDrawParams params;
		params.BufferIndex = sprites.Batches->BufferCount - 1;
		params.TextureIndex = sprites.Batches->TextureSetCount - 1;
		params.BufferOffset = sprites.Batches->BufferOffset;
		params.VertexCount = sprites.VertexCount;
		sprites.DrawParams.push_back(params);

		sprites.Batches->Buffers[sprites.Batches->BufferCount - 1]
			->SetData(sprites.Vertices.data(), sprites.VertexCount * sizeof(SpriteVertex), sprites.Batches->BufferOffset * sizeof(SpriteVertex));

		if (textureFull && vertexFull) {
			sprites.Batches->BufferCount++;
			sprites.Batches->BufferOffset = 0;
			sprites.Batches->TextureSetCount++;
			sprites.Batches->TextureCount = 1;
		}
		else if (textureFull) {
			sprites.Batches->BufferOffset += sprites.VertexCount;
			sprites.Batches->TextureSetCount++;
			sprites.Batches->TextureCount = 1;
		}
		else if (vertexFull) {
			sprites.Batches->BufferCount++;
			sprites.Batches->BufferOffset = 0;
		}

		if (sprites.Batches->BufferCount > sprites.Batches->Buffers.size()) {
			sprites.Batches->Buffers.push_back(VertexBuffer::Create(nullptr, 0));
		}

		if (sprites.Batches->TextureSetCount > sprites.Batches->TextureSet.size()) {
			sprites.Batches->TextureSet.push_back(sprites.Batches->CreateTextureDescriptorForSprites());
		}
		sprites.VertexCount = 0;
	}

	static void AddSpriteToBatch(const glm::mat4& transform, const glm::vec4& tint, Ref<TextureReference> texture, const SamplingRegion& region) {

		auto& sprites = s_Data->Sprites;

		uint32_t textureIndex = -1;
		for (uint32_t i = 0; i < sprites.Batches->TextureCount; ++i) {
			uint32_t setIndex = sprites.Batches->TextureSetCount - 1;
			if (sprites.Batches->TextureSet[setIndex]->GetTextureAtBinding(0, i) == texture) {
				textureIndex = i;
				break;
			}
		}

		if (textureIndex == -1) {
			if (sprites.Batches->TextureCount != SpriteData::MaxTextureCount) {
				uint32_t setIndex = sprites.Batches->TextureSetCount - 1;
				sprites.Batches->TextureSet[setIndex]->Update(texture, {}, ImageLayout::ShaderReadOnlyOptimal, 0, sprites.Batches->TextureCount);
				textureIndex = sprites.Batches->TextureCount++;
			}
			else {
				AddDraw();
				uint32_t setIndex = sprites.Batches->TextureSetCount - 1;
				sprites.Batches->TextureSet[setIndex]->Update(texture, {}, ImageLayout::ShaderReadOnlyOptimal, 0, sprites.Batches->TextureCount);
				textureIndex = sprites.Batches->TextureCount++;
			}
		}

		std::array<SpriteVertex, 4> vertices = MakeSpriteVertices(transform, tint, textureIndex, region);


		if (sprites.VertexCount + sprites.Batches->BufferOffset == SpriteData::MaxVertexCount) {
			AddDraw();
		}

		if (sprites.VertexCount + 4 > sprites.Vertices.size()) {
			uint64_t newSize = sprites.VertexCount * 1.5f + 4;
			if (newSize > SpriteData::MaxVertexCount) {
				newSize = SpriteData::MaxVertexCount;
			}
			sprites.Vertices.resize(newSize);
		}
		sprites.Vertices[sprites.VertexCount + 0] = vertices[0];
		sprites.Vertices[sprites.VertexCount + 1] = vertices[1];
		sprites.Vertices[sprites.VertexCount + 2] = vertices[2];
		sprites.Vertices[sprites.VertexCount + 3] = vertices[3];
		sprites.VertexCount += 4;
	}

	static void AddStringToBatch(const std::string& string, Ref<Font> font, const glm::mat4& transform, const glm::vec4& color) {

		auto& texts = s_Data->Texts;

		const MSDFData* data = font->GetMSDFData();
		
		Ref<Texture2D> fontAtlas = font->GetAtlasTexture();

		const auto& fontGeometry = data->FontGeometry;
		const auto& metrics = fontGeometry.getMetrics();

		double x = 0.0;
		double fsScale = 1.0 / (metrics.ascenderY - metrics.descenderY);
		double y = 0.0;
		double lineHeightOffset = 0.0;

		uint32_t tabSize = 1;

		for (uint32_t i = 0; i < string.length(); ++i) {

			if (string[i] == '\n') {
				x = 0.0;
				y -= fsScale * metrics.lineHeight + lineHeightOffset;
				continue;
			}
			if (string[i] == '\r') {
				continue;
			}
			
			auto glyph = fontGeometry.getGlyph(string[i]);

			if (string[i] == '\t') {
				glyph = fontGeometry.getGlyph(' ');
			}

			if (!glyph)
				glyph = fontGeometry.getGlyph('?');
			if (!glyph)
				return;

			


			double pl, pb, pr, pt;
			glyph->getQuadPlaneBounds(pl, pb, pr, pt);

			pl *= fsScale; pb *= fsScale; pr *= fsScale; pt *= fsScale;
			pl += x; pb += y; pr += x; pt += y;
			glm::vec2 quadMin((float)pl, (float)pb), quadMax((float)pr, (float)pt);

			double al, ab, ar, at;
			glyph->getQuadAtlasBounds(al, ab, ar, at);
			float texelWidth = 1.0f / (float)fontAtlas->GetTextureSpecification().Width;
			float texelHeight = 1.0f / (float)fontAtlas->GetTextureSpecification().Height;
			al *= texelWidth; ab *= texelHeight; ar *= texelWidth; at *= texelHeight;
			glm::vec2 coordTopLeft((float)al, (float)at), coordBottomRight((float)ar, (float)ab);

			if (texts.VertexCount + 4 > texts.Vertices.size()) {
				uint64_t newSize = texts.VertexCount * 1.5f + 4;
				texts.Vertices.resize(newSize);
			}

			texts.Vertices[texts.VertexCount + 0].Position = transform * glm::vec4((float)pl, (float)pb, 0.0f, 1.0f);
			texts.Vertices[texts.VertexCount + 0].Color = color;
			texts.Vertices[texts.VertexCount + 0].TexCoord = glm::vec2((float)al, (float)ab);

			texts.Vertices[texts.VertexCount + 1].Position = transform * glm::vec4((float)pr, (float)pb, 0.0f, 1.0f);
			texts.Vertices[texts.VertexCount + 1].Color = color;
			texts.Vertices[texts.VertexCount + 1].TexCoord = glm::vec2((float)ar, (float)ab);

			texts.Vertices[texts.VertexCount + 2].Position = transform * glm::vec4((float)pr, (float)pt, 0.0f, 1.0f);
			texts.Vertices[texts.VertexCount + 2].Color = color;
			texts.Vertices[texts.VertexCount + 2].TexCoord = glm::vec2((float)ar, (float)at);

			texts.Vertices[texts.VertexCount + 3].Position = transform * glm::vec4((float)pl, (float)pt, 0.0f, 1.0f);
			texts.Vertices[texts.VertexCount + 3].Color = color;
			texts.Vertices[texts.VertexCount + 3].TexCoord = glm::vec2((float)al, (float)at);
			
			texts.VertexCount += 4;

			if (i < string.length() - 1) {
				double advance = glyph->getAdvance();
				char nextChar = 'K';
				fontGeometry.getAdvance(advance, string[i], string[i + 1]);

				float kerningOffset = 0.0f;
				x += fsScale * advance + kerningOffset;
			}
		}
	}

	#pragma endregion

	#pragma region Screen Pass
	int ShowDebugShadow = false;

	static void CreateScreenPassResources() {
		{
			RenderPassSpecification specs{};
			specs.Colors =
			{
				RenderPassAttachment(Format::RGBA16F,ImageLayout::None,ImageLayout::ColorAttachmentOptimal,TextureSamples::x1,
					AttachmentLoadOp::Clear, AttachmentStoreOp::Store)
			};
			s_Data->ScreenRenderPass = RenderPass::Create(specs);
		}
		{
			FramebufferSpecification specs;
			specs.RenderPass = s_Data->ScreenRenderPass;
			specs.Width = s_Data->Width;
			specs.Height = s_Data->Height;
			for (auto& fb : s_Data->HDROutputs) {
				fb = Framebuffer::Create(specs);
			}
		}

		{
			GraphicsPipelineSpecification specs{};
			specs.Input.Bindings = {
				{
					{"a_NDC",Format::RG32F},
					{"a_TexCoords",Format::RG32F}
				}
			};
			specs.Multisample.Samples = TextureSamples::x1;
			specs.Primitive = PrimitiveTopology::TriangleList;
			specs.RenderPass = s_Data->ScreenRenderPass;
			specs.Subpass = 0;
			specs.Rasterization.CullMode = PipelineCullMode::None;
			specs.Shader = ShaderLibrary::LoadShader("LightPass", "assets/_shaders/CubeLightPass.shader");
			s_Data->ScreenPipeline = GraphicsPipeline::Create(specs);
		}

		{
			SamplerState state;
			state.AddressModeU = SamplerAddressMode::ClampToEdge;
			state.AddressModeV = SamplerAddressMode::ClampToEdge;
			state.AddressModeW = SamplerAddressMode::ClampToEdge;
			state.BorderColor = SamplerBorderColor::FloatOpaqueWhite;
			state.MagFilter = SamplerFilter::Nearest;
			state.MinFilter = SamplerFilter::Nearest;
			state.MipFilter = SamplerMipMapMode::Nearest;
			state.Compare = true;
			state.CompareOp = CompareOp::LessOrEqual;

			s_Data->Shadows.ShadowSampler = Sampler::Create(state);
		}

		s_Data->ScreenPipelinePack.Construct([](uint32_t i) {
			Ref<Texture2D> pos, norm, albedo, metalRough, depth;
			
			if (samples == TextureSamples::x1) {
				pos = s_Data->DeferredPass.Output[i]->GetColorAttachment(0);
				norm = s_Data->DeferredPass.Output[i]->GetColorAttachment(1);
				albedo = s_Data->DeferredPass.Output[i]->GetColorAttachment(2);
				metalRough = s_Data->DeferredPass.Output[i]->GetColorAttachment(3);
			}
			else {
				pos = s_Data->DeferredPass.Output[i]->GetResolveAttachment(0);
				norm = s_Data->DeferredPass.Output[i]->GetResolveAttachment(1);
				albedo = s_Data->DeferredPass.Output[i]->GetResolveAttachment(2);
				metalRough = s_Data->DeferredPass.Output[i]->GetResolveAttachment(3);
			}
			depth = s_Data->DeferredPass.Output[i]->GetDepthAttachment();
			
			DescriptorSetPack pack = DescriptorSetPack(ShaderLibrary::GetNamedShader("LightPass"), {});
			pack[0]->Update(s_Data->Clusters[i], 0);
			pack[1]->Update(s_Data->LightCullPipelinePack[i].GetSet(1)->GetStorageBufferAtBinding(0), 0).
				Update(s_Data->ClusterGrids[i], 1);
			//sampler
			pack[2]->Update({}, s_Data->GlobalSampler, {}, 0).
				//positions
				Update(pos, {}, ImageLayout::ShaderReadOnlyOptimal, 1).
				//normals
				Update(norm, {}, ImageLayout::ShaderReadOnlyOptimal, 2).
				//albedo
				Update(albedo, {}, ImageLayout::ShaderReadOnlyOptimal, 3).
				//metallic/roughness
				Update(metalRough, {}, ImageLayout::ShaderReadOnlyOptimal, 4).
				//depths
				Update(depth, {}, ImageLayout::ShaderReadOnlyOptimal, 5);
			pack[3]->Update(s_Data->DirectionalLight[i], 0).
				Update({}, s_Data->Shadows.ShadowSampler, {}, "ShadowSampler").
#ifdef SHADOW_MAPPING
				Update(s_Data->Shadows.Framebuffer[i]->GetDepthAttachment(), {}, ImageLayout::ShaderReadOnlyOptimal, "DirectionalShadows");
				
#endif
			return pack;
		});
	}

	extern Ref<Font> font;

	void SceneRenderer::ScreenPass(float zNear, float zFar, const glm::vec3& cameraPos, const glm::mat4& viewMatrix, const glm::mat4& viewProjection) {
		float scale = 24.0f / (std::log2f(zFar / zNear));
		float bias = -24.0f * (std::log2f(zNear)) / std::log2f(zFar / zNear);
		
		RenderCommand::SetViewport(s_Data->Width, s_Data->Height, 0, 0);
		RenderCommand::SetScissor(s_Data->Width, s_Data->Height, 0, 0);

		//2D setup
		{
			auto& sprites = s_Data->Sprites;

			sprites.Batches->Reset();
			sprites.VertexCount = 0;
			sprites.DrawParams.clear();

			auto& sceneReg = ((Scene*)m_Context)->m_Registry;
			auto view = sceneReg.view<TransformComponent, SpriteRendererComponent>();

			for (auto& e : view) {
				auto& [tc, src] = view.get(e);

				SamplingRegion region{};
				region.Layer = 0;
				region.UV0 = { 0,0 };
				region.UV1 = { 1,1 };

				if (src.SpriteTexture)
					AddSpriteToBatch(tc.GetTransform(), glm::vec4(1.0f), src.SpriteTexture, region);
				else
					AddSpriteToBatch(tc.GetTransform(), glm::vec4(1.0f), RendererGlobals::GetSingleColorTexture(glm::vec4(1.0f)), region);
			}

			if (sprites.VertexCount) {
				SpriteData::BatchDrawParams params{};
				params.BufferIndex = sprites.Batches->BufferCount - 1;
				params.TextureIndex = sprites.Batches->TextureSetCount - 1;
				params.BufferOffset = sprites.Batches->BufferOffset;
				params.VertexCount = sprites.VertexCount;
				sprites.DrawParams.push_back(params);

				sprites.Batches->Buffers[sprites.Batches->BufferCount - 1]
					->SetData(sprites.Vertices.data(), sprites.VertexCount * sizeof(SpriteVertex), sprites.Batches->BufferOffset * sizeof(SpriteVertex));
			}
		}

		RenderCommand::BeginRenderPass(s_Data->ScreenRenderPass, *s_Data->HDROutputs,
			{ AttachmentColorClearValue(glm::vec4{0.0f,0.0f,0.0f,0.0f}) });

		//3D
		{
			RenderCommand::BindGraphicsPipeline(s_Data->ScreenPipeline);
			RenderCommand::BindVertexBuffers({ s_Data->ScreenNDC }, { 0 });
			s_Data->ScreenPipelinePack->Bind();
			RenderCommand::BindPushConstants(ShaderLibrary::GetNamedShader("LightPass"), 0, viewMatrix,
				glm::vec4(cameraPos, 0.0f), glm::vec2{ s_Data->Width,s_Data->Height }, zNear, zFar, scale, bias, ShowDebugShadow);
			RenderCommand::Draw(6, 1, 0, 0);
		}

		//2D
		{
			auto& sprites = s_Data->Sprites;
			
			if (sprites.DrawParams.size()) {
				uint64_t lastBuffer = -1;
				uint64_t lastTexture = -1;
				RenderCommand::BindGraphicsPipeline(sprites.SpriteGraphicsPipeline);
				RenderCommand::BindIndexBuffer(sprites.SpriteIndexBuffer, 0);
				RenderCommand::BindPushConstants(ShaderLibrary::GetNamedShader("SpriteRender"), 0, viewProjection);

				for (auto& draw : sprites.DrawParams) {
					if (lastBuffer != draw.BufferIndex) {
						lastBuffer = draw.BufferIndex;
						RenderCommand::BindVertexBuffers({ sprites.Batches->Buffers[draw.BufferIndex] }, { draw.BufferOffset });
					}
					if (lastTexture != draw.TextureIndex) {
						lastTexture = draw.TextureIndex;
						RenderCommand::BindDescriptorSet(ShaderLibrary::GetNamedShader("SpriteRender"), sprites.Batches->TextureSet[draw.TextureIndex], 0);
					}
					RenderCommand::DrawIndexed((draw.VertexCount / 4) * 6, 0, 1, 0, 0, 0);
				}
			}
		}

		RenderCommand::EndRenderPass();
	}
	void SceneRenderer::InsertScreenPassBarrier()
	{
		ImageMemoryBarrier hdrBarrier(
			s_Data->HDROutputs->Get()->GetColorAttachment(0),
			ImageLayout::ShaderReadOnlyOptimal,
			AccessFlags_ColorAttachmentWrite,
			AccessFlags_ShaderRead
		);

		RenderCommand::PipelineBarrier(
			PipelineStages_ColorAttachmentOutput,
			PipelineStages_FragmentShader,
			{},
			{},
			{ hdrBarrier }
		);
	}
	#pragma endregion

	#pragma region Tonemap Pass
	static void CreateTonemapPassResources() {
		{
			RenderPassSpecification specs{};
			specs.Colors =
			{
				RenderPassAttachment(Format::RGBA8SRGB,ImageLayout::None,ImageLayout::ColorAttachmentOptimal,TextureSamples::x1,
					AttachmentLoadOp::Clear, AttachmentStoreOp::Store)
			};
			s_Data->TonemapRenderPass = RenderPass::Create(specs);
		}
		{
			FramebufferSpecification specs;
			specs.RenderPass = s_Data->TonemapRenderPass;
			specs.Width = s_Data->Width;
			specs.Height = s_Data->Height;
			for (auto& fb : s_Data->Outputs) {
				fb = Framebuffer::Create(specs);
			}
		}

		{
			GraphicsPipelineSpecification specs{};
			specs.Input.Bindings = {
				{
					{"a_NDC",Format::RG32F},
					{"a_TexCoords",Format::RG32F}
				}
			};
			specs.Multisample.Samples = TextureSamples::x1;
			specs.Primitive = PrimitiveTopology::TriangleList;
			specs.RenderPass = s_Data->TonemapRenderPass;
			specs.Subpass = 0;
			specs.Rasterization.CullMode = PipelineCullMode::None;
			specs.Shader = ShaderLibrary::LoadShader("TonemapPass", "assets/_shaders/TonemapPass.shader");
			s_Data->TonemapPipeline = GraphicsPipeline::Create(specs);
		}
		uint32_t i = 0;
		for (auto& pack : s_Data->TonemapPipelinePack) {
			pack = DescriptorSetPack(ShaderLibrary::GetNamedShader("TonemapPass"), {});
			//sampler
			pack[0]->Update({}, s_Data->GlobalSampler, {}, 0).
				//hdr
				Update(s_Data->HDROutputs[i]->GetColorAttachment(0), {}, ImageLayout::ShaderReadOnlyOptimal, 1);
			++i;
		}
	}
	
	static void TonemapPass() {
		RenderCommand::BeginRenderPass(s_Data->TonemapRenderPass, *s_Data->Outputs, {
			AttachmentClearValue(glm::vec4(0.0f))
			});
		RenderCommand::BindGraphicsPipeline(s_Data->TonemapPipeline);
		RenderCommand::BindVertexBuffers({ s_Data->ScreenNDC }, { 0 });
		//s_Data->TonemapPipelinePack->GetSet(0)->Update(
		//	s_Data->ShadowData.ShadowBuffers->Get()->GetDepthAttachment(), {}, ImageLayout::ShaderReadOnlyOptimal, 1
		//);
		s_Data->TonemapPipelinePack->Bind();
		//RenderCommand::BindPushConstants(ShaderLibrary::GetNamedShader("TonemapPass"));
		RenderCommand::Draw(6, 1, 0, 0);
		RenderCommand::EndRenderPass();
	}

	static void InsertTonemapPassBarrier() {
		ImageMemoryBarrier barrier(
			s_Data->Outputs->Get()->GetColorAttachment(0),
			ImageLayout::ColorAttachmentOptimal,
			AccessFlags_ColorAttachmentWrite, 
			AccessFlags_ColorAttachmentWrite
		);

		RenderCommand::PipelineBarrier(
			PipelineStages_ColorAttachmentOutput, 
			PipelineStages_ColorAttachmentOutput, 
			{}, 
			{},
			{ barrier });
	}
	#pragma endregion

	#pragma region Deferred Pass
	static void CreateGBufferResources() {
		{
			RenderPassSpecification specs{};
			specs.Colors = {
				//Position
				RenderPassAttachment(Format::RGBA32F,ImageLayout::None,ImageLayout::ColorAttachmentOptimal,samples,
					AttachmentLoadOp::Clear,AttachmentStoreOp::Store),
				//Normals
				RenderPassAttachment(Format::RGBA32F,ImageLayout::None,ImageLayout::ColorAttachmentOptimal,samples,
					AttachmentLoadOp::Clear,AttachmentStoreOp::Store),
				//Albedo
				RenderPassAttachment(Format::RGBA8UN,ImageLayout::None,ImageLayout::ColorAttachmentOptimal,samples,
					AttachmentLoadOp::Clear,AttachmentStoreOp::Store),
				//Mettalic/roughness
				RenderPassAttachment(Format::RG32F,ImageLayout::None,ImageLayout::ColorAttachmentOptimal,samples,
					AttachmentLoadOp::Clear,AttachmentStoreOp::Store)
			};
			specs.DepthStencil =
				RenderPassAttachment(Format::Depth32F, ImageLayout::None, ImageLayout::DepthAttachmentOptimal, samples,
					AttachmentLoadOp::Clear, AttachmentStoreOp::Store);
			specs.AutoResolve = samples != TextureSamples::x1;
			s_Data->DeferredPass.RenderPass = RenderPass::Create(specs);
		}

		{
			FramebufferSpecification specs{};
			specs.Width = s_Data->Width;
			specs.Height = s_Data->Height;
			specs.RenderPass = s_Data->DeferredPass.RenderPass;

			s_Data->DeferredPass.Output.Construct([&specs](auto) {
				return Framebuffer::Create(specs);
			});
		}
	}

	static void DeferredPass(const glm::mat4& viewProj, const entt::registry& sceneReg) {

		s_Data->DeferredPass.RenderPass->SetDepthClearValue(AttachmentDepthStencilClearValue(1.0f, 0));

		RenderCommand::BeginRenderPass(s_Data->DeferredPass.RenderPass, s_Data->DeferredPass.Output,
			{}
		);

		Ref<VertexBuffer> lastSetVertexBuffer = {};
		Ref<IndexBuffer> lastSetIndexBuffer = {};

		Ref<Material> lastSetMaterial = {};

		RenderCommand::SetViewport(s_Data->Width, s_Data->Height, 0, 0);
		RenderCommand::SetScissor(s_Data->Width, s_Data->Height, 0, 0);

		{
			auto view = sceneReg.view<TransformComponent, ModelComponent>();
			for (auto& e : view) {
				auto& [tc,mc] = view.get(e);

				if (!mc.UsedModel)
					continue;
				
				glm::mat4 model = tc.GetTransform();

				for (auto& mesh : mc.UsedModel->GetMeshes()) {
					Ref<VertexBuffer> vb = mesh->GetVertexBuffer();
					Ref<IndexBuffer> ib = mesh->GetIndexBuffer();
					Ref<Material> mat = mesh->GetDefaultMaterial();
					KD_CORE_ASSERT(mat);
					
					if (lastSetVertexBuffer != vb) {
						RenderCommand::BindVertexBuffers({ vb }, { 0 });
						lastSetVertexBuffer = vb;
					}
					if (lastSetIndexBuffer != ib) {
						RenderCommand::BindIndexBuffer(ib, 0);
						lastSetIndexBuffer = ib;
					}
					
					if (!lastSetMaterial) {
						mat->BindValues();
						mat->BindPipeline();
						mat->BindBaseValues(viewProj);
						lastSetMaterial = mat;
					}
					else if (lastSetMaterial != mat) {
						mat->BindValues();
					
						//RenderCommand::BindDescriptorSet(ShaderLibrary::GetNamedShader("DeferredGBufferGen"), mesh->GetDefaultMaterial()->GetTextureSet(), 1);
					
						if (lastSetMaterial->GetPipeline() != mat->GetPipeline()) {
							mat->BindPipeline();
							mat->BindBaseValues(viewProj);
						}
					
						lastSetMaterial = mat;
					}
					
					lastSetMaterial->BindTransform(model);
					
					RenderCommand::DrawIndexed(mesh->GetIndexCount(), mesh->GetVertexCount(), 1, 0, 0, 0);
				}
			}
		}
		RenderCommand::EndRenderPass();
		
	}
	
	static void InsertDeferredBarrier() {
		
		if (samples == TextureSamples::x1) {
			ImageMemoryBarrier positionBarrier(
				s_Data->DeferredPass.Output->Get()->GetColorAttachment(0),
				ImageLayout::ShaderReadOnlyOptimal,
				AccessFlags_ColorAttachmentWrite,
				AccessFlags_ShaderRead);
			ImageMemoryBarrier normalBarrier(
				s_Data->DeferredPass.Output->Get()->GetColorAttachment(1),
				ImageLayout::ShaderReadOnlyOptimal,
				AccessFlags_ColorAttachmentWrite,
				AccessFlags_ShaderRead);
			ImageMemoryBarrier albedoBarrier(
				s_Data->DeferredPass.Output->Get()->GetColorAttachment(2),
				ImageLayout::ShaderReadOnlyOptimal,
				AccessFlags_ColorAttachmentWrite,
				AccessFlags_ShaderRead
			);
			ImageMemoryBarrier metallicRoughnessBarrier(
				s_Data->DeferredPass.Output->Get()->GetColorAttachment(3),
				ImageLayout::ShaderReadOnlyOptimal,
				AccessFlags_ColorAttachmentWrite,
				AccessFlags_ShaderRead
			);
			ImageMemoryBarrier depthBarrier(
				s_Data->DeferredPass.Output->Get()->GetDepthAttachment(),
				ImageLayout::ShaderReadOnlyOptimal,
				AccessFlags_DepthStencilWrite,
				AccessFlags_ShaderRead
			);
			RenderCommand::PipelineBarrier(
				PipelineStages_ColorAttachmentOutput | PipelineStages_LateFragmentTests | PipelineStages_EarlyFragmentTests,
				PipelineStages_VertexShader,
				{},
				{},
				{ positionBarrier,albedoBarrier,normalBarrier,metallicRoughnessBarrier,depthBarrier }
			);
		}
		else {
			ImageMemoryBarrier positionBarrier(
				s_Data->DeferredPass.Output->Get()->GetResolveAttachment(0),
				ImageLayout::ShaderReadOnlyOptimal,
				AccessFlags_ColorAttachmentWrite,
				AccessFlags_ShaderRead);
			ImageMemoryBarrier normalBarrier(
				s_Data->DeferredPass.Output->Get()->GetResolveAttachment(1),
				ImageLayout::ShaderReadOnlyOptimal,
				AccessFlags_ColorAttachmentWrite,
				AccessFlags_ShaderRead);
			ImageMemoryBarrier albedoBarrier(
				s_Data->DeferredPass.Output->Get()->GetResolveAttachment(2),
				ImageLayout::ShaderReadOnlyOptimal,
				AccessFlags_ColorAttachmentWrite,
				AccessFlags_ShaderRead
			);
			ImageMemoryBarrier metallicRoughnessBarrier(
				s_Data->DeferredPass.Output->Get()->GetResolveAttachment(3),
				ImageLayout::ShaderReadOnlyOptimal,
				AccessFlags_ColorAttachmentWrite,
				AccessFlags_ShaderRead
			);

			ImageMemoryBarrier depthBarrier(
				s_Data->DeferredPass.Output->Get()->GetDepthAttachment(),
				ImageLayout::ShaderReadOnlyOptimal,
				AccessFlags_DepthStencilWrite,
				AccessFlags_ShaderRead
			);

			RenderCommand::PipelineBarrier(
				PipelineStages_ColorAttachmentOutput | PipelineStages_LateFragmentTests | PipelineStages_EarlyFragmentTests,
				PipelineStages_VertexShader,
				{},
				{},
				{ positionBarrier,albedoBarrier,normalBarrier,metallicRoughnessBarrier,depthBarrier }
			);
		}
	}

	#pragma endregion
	
	std::array<PerFrameResource<Ref<Texture2D>>*, 4> DepthTexture;
	
	#pragma region ShadowPass

	static void CreateShadowPassResources() {
#ifdef SHADOW_MAPPING
		RenderPassSpecification rpSpecs;
		rpSpecs.DepthStencil =
			RenderPassAttachment(Format::Depth32F, ImageLayout::None, ImageLayout::DepthAttachmentOptimal, TextureSamples::x1,
				AttachmentLoadOp::Clear, AttachmentStoreOp::Store);
		s_Data->Shadows.RenderPass = RenderPass::Create(rpSpecs);


		FramebufferSpecification fbSpecs;
		fbSpecs.Width = SHADOW_MAP_SIZE;
		fbSpecs.Height = SHADOW_MAP_SIZE;
		fbSpecs.Layers = 4;
		fbSpecs.RenderPass = s_Data->Shadows.RenderPass;
		s_Data->Shadows.Framebuffer.Construct([&fbSpecs](uint32_t) {
			return Framebuffer::Create(fbSpecs);
		});
		/*for (int j = 0; j < 4; ++j) {
			DepthTexture[j] = new PerFrameResource<Ref<Texture2D>>;
			DepthTexture[j]->Construct([j](uint32_t i) {
				return s_Data->Shadows.Framebuffer[j][i]->GetDepthAttachment();
				});
		}*/
		
		GraphicsPipelineSpecification gpSpecs;
		gpSpecs.Shader = ShaderLibrary::LoadOrGetNamedShader("ShadowPass", "assets/_shaders/ShadowPass.glsl");
		gpSpecs.Input.Bindings.push_back(VertexInputBinding({
			{ "a_Position",Format::RGB32F },
			{ sizeof(MeshVertex) - sizeof(MeshVertex::Position) }
			}));
		gpSpecs.Multisample.Samples = TextureSamples::x1;
		gpSpecs.Primitive = PrimitiveTopology::TriangleList;
		gpSpecs.Rasterization.FrontCCW = true;
		gpSpecs.Rasterization.CullMode = PipelineCullMode::None;
		gpSpecs.RenderPass = s_Data->Shadows.RenderPass;
		gpSpecs.Subpass = 0;
		gpSpecs.DepthStencil.DepthTest = true;
		gpSpecs.DepthStencil.DepthWrite = true;
		gpSpecs.DepthStencil.DepthCompareOperator = CompareOp::LessOrEqual;
		
		s_Data->Shadows.Pipeline = GraphicsPipeline::Create(gpSpecs);

		s_Data->Shadows.Pack.Construct([&gpSpecs](uint32_t i) {
			auto pack = DescriptorSetPack(gpSpecs.Shader, {});
			pack[0]->Update(s_Data->DirectionalLight[i], "DLight");
			return pack;
		});

#endif
	}

#undef near
#undef far

	static std::vector<float> GetSplitDistances(uint32_t count, float near, float far, float splitLambda) {
		std::vector<float> cascadeSplits(count);

		float minZ = near;
		float maxZ = far;

		float range = maxZ - minZ;
		float ratio = maxZ / minZ;

		for (uint32_t i = 0; i < count; i++) {
			float p = (i + 1) / (float)count;
			float log = minZ * std::pow(ratio, p);
			float uniform = minZ + range * p;
			float d = splitLambda * (log - uniform) + uniform;
			cascadeSplits[i] = d;
		}
		
		return cascadeSplits;
	}

	void SceneRenderer::ShadowPass(const SceneData& sceneData) {

		Scene* scene = (Scene*)m_Context;
		{
			auto view = scene->m_Registry.view<TransformComponent, DirectionalLightComponent>();
			entt::entity e = *view.begin();
			Entity entity{ e, scene };

			auto& [tc, dlc] = entity.GetComponent<TransformComponent, DirectionalLightComponent>();

			float maxDistance = glm::min(sceneData.zFar, dlc.MaxDistance);
			maxDistance = glm::max(maxDistance, sceneData.zNear + 0.001f);

			float minDistance = glm::min(sceneData.zNear, maxDistance);

			DLight.Color = dlc.Color;

			glm::mat4 lightRotation = glm::toMat4(glm::quat(tc.Rotation));
			lightRotation = glm::transpose(glm::inverse(glm::mat3(lightRotation)));


			static const constexpr int cascadeSplitLambda = 0.95f;

			std::vector<float> splits = GetSplitDistances(4, minDistance, maxDistance, dlc.SplitLambda);

			//KD_CORE_INFO("{},{},{},{}", splits[0], splits[1], splits[2], splits[3]);

			splits[0] = (maxDistance - minDistance) * 0.3f + minDistance;
			splits[1] = (maxDistance - minDistance) * 0.5f + minDistance;
			splits[2] = (maxDistance - minDistance) * 0.7f + minDistance;
			splits[3] = (maxDistance - minDistance) * 1.0f + minDistance;

			auto viewProj = ComputeLightViewProjections(sceneData, {0.1f,0.3f,0.5f,1.0f}, lightRotation, SHADOW_MAP_SIZE);
			DLight.GlobalShadowMatrix = viewProj.GlobalShadowMatrix;

			DLight.ViewProj[0] = viewProj.Cascades[0].ShadowMatrix;
			DLight.ViewProj[1] = viewProj.Cascades[1].ShadowMatrix;
			DLight.ViewProj[2] = viewProj.Cascades[2].ShadowMatrix;
			DLight.ViewProj[3] = viewProj.Cascades[3].ShadowMatrix;

			DLight.CascadeOffsets[0] = viewProj.Cascades[0].CascadeOffsets;
			DLight.CascadeOffsets[1] = viewProj.Cascades[1].CascadeOffsets;
			DLight.CascadeOffsets[2] = viewProj.Cascades[2].CascadeOffsets;
			DLight.CascadeOffsets[3] = viewProj.Cascades[3].CascadeOffsets;

			DLight.CascadeScales[0] = viewProj.Cascades[0].CascadeScales;
			DLight.CascadeScales[1] = viewProj.Cascades[1].CascadeScales;
			DLight.CascadeScales[2] = viewProj.Cascades[2].CascadeScales;
			DLight.CascadeScales[3] = viewProj.Cascades[3].CascadeScales;

			DLight.SplitDistances[0] = splits[0];
			DLight.SplitDistances[1] = splits[1];
			DLight.SplitDistances[2] = splits[2];
			DLight.SplitDistances[3] = splits[3];

			DLight.Direction = -glm::normalize(lightRotation[2]);

			DLight.FadeStart = dlc.FadeStart * splits[3];
		}

		s_Data->DirectionalLight->Get()->SetData(&DLight, sizeof(DLight));

		Ref<VertexBuffer> lastSetVertexBuffer = {};
		Ref<IndexBuffer> lastSetIndexBuffer = {};

		s_Data->Shadows.RenderPass->SetDepthClearValue({ 1.0f,0 });
		RenderCommand::BindGraphicsPipeline(s_Data->Shadows.Pipeline);
		RenderCommand::SetViewport(SHADOW_MAP_SIZE, SHADOW_MAP_SIZE,0,0);
		RenderCommand::SetScissor(SHADOW_MAP_SIZE, SHADOW_MAP_SIZE,0,0);

		
		RenderCommand::BeginRenderPass(s_Data->Shadows.RenderPass, s_Data->Shadows.Framebuffer, {});

		s_Data->Shadows.Pack->Bind();
		{
			auto view = scene->m_Registry.view<TransformComponent, ModelComponent>();
			for (auto& e : view) {
				auto& [tc, mc] = view.get(e);

				if (!mc.UsedModel)
					continue;

				glm::mat4 model = tc.GetTransform();

				for (auto& mesh : mc.UsedModel->GetMeshes()) {
					Ref<VertexBuffer> vb = mesh->GetVertexBuffer();
					Ref<IndexBuffer> ib = mesh->GetIndexBuffer();
					if (lastSetVertexBuffer != vb) {
						RenderCommand::BindVertexBuffers({ vb }, { 0 });
						lastSetVertexBuffer = vb;
					}
					if (lastSetIndexBuffer != ib) {
						RenderCommand::BindIndexBuffer(ib, 0);
						lastSetIndexBuffer = ib;
					}
					RenderCommand::BindPushConstants(ShaderLibrary::GetNamedShader("ShadowPass"), 0, model);

					RenderCommand::DrawIndexed(mesh->GetIndexCount(), mesh->GetVertexCount(), 1, 0, 0, 0);
				}
			}
		}
		RenderCommand::EndRenderPass();
	}
	
	static void InsertShadowPassBarrier() {

#ifdef SHADOW_MAPPING
		ImageMemoryBarrier depthBarrier(
			s_Data->Shadows.Framebuffer->Get()->GetDepthAttachment(),
			ImageLayout::ShaderReadOnlyOptimal,
			AccessFlags_DepthStencilWrite,
			AccessFlags_ShaderRead
		);

		RenderCommand::PipelineBarrier(
			PipelineStages_ColorAttachmentOutput | PipelineStages_LateFragmentTests | PipelineStages_EarlyFragmentTests,
			PipelineStages_VertexShader,
			{},
			{},
			{ depthBarrier }
		);
#endif
	}

	#pragma endregion

	static void CreateTextPassResources() {
		auto& texts = s_Data->Texts;

		{
			RenderPassSpecification specs{};
			specs.Colors =
			{
				RenderPassAttachment(Format::RGBA8SRGB,ImageLayout::ColorAttachmentOptimal,ImageLayout::ColorAttachmentOptimal,TextureSamples::x1,
					AttachmentLoadOp::Load, AttachmentStoreOp::Store)
			};
			texts.TextRenderPass = RenderPass::Create(specs);
		}

		{
			GraphicsPipelineSpecification specs;

			PipelineColorBlend::Attachment attachment;
			attachment.WriteR = true;
			attachment.WriteG = true;
			attachment.WriteB = true;
			attachment.WriteA = true;
			attachment.Blend = true;
			attachment.AttachmentIndex = 0;
			
			attachment.SrcColorBlend = BlendFactor::SrcAlpha;
			attachment.DstColorBlend = BlendFactor::OneMinusSrcAlpha;
			attachment.ColorBlendOp = BlendOp::Add;
			
			attachment.SrcAlphaBlend = BlendFactor::One;
			attachment.DstAlphaBlend = BlendFactor::Zero;
			attachment.AlphaBlendOp = BlendOp::Add;

			specs.Blend.Attachments.push_back(attachment);

			

			specs.Input.Bindings.push_back(
				{
					{"a_Position", Format::RGB32F},
					{"a_Color", Format::RGBA32F},
					{"a_TexCoords", Format::RG32F}
				}
			);
			specs.Primitive = PrimitiveTopology::TriangleList;
			specs.Rasterization.CullMode = PipelineCullMode::None;
			
			specs.Shader = ShaderLibrary::LoadOrGetNamedShader("TextPass", "assets/_shaders/TextPass.glsl");
			specs.RenderPass = texts.TextRenderPass;
			specs.Subpass = 0;

			texts.Pipeline = GraphicsPipeline::Create(specs);
		}

		{
			texts.Buffer.Construct([](uint32_t i) {
				return VertexBuffer::Create(nullptr, 0);
			});
		}
	}

	static void TextPass(const SceneData& sceneData) 
	{
		//static std::string test = "Kaidel v0.1";
		static std::string test = R"(#type vertex
#version 460 core
layout(location = 0) in vec3 a_Position;
layout(location = 1) in vec4 a_Color;
layout(location = 2) in vec2 a_TexCoords;

layout(push_constant) uniform PushConstants{
	mat4 ViewProjection;
};

layout(location = 0) out vec4 v_Color;
layout(location = 1) out vec2 v_TexCoords;

void main(){
	v_Color = a_Color;
	v_TexCoords = a_TexCoords.xy;

	gl_Position = ViewProjection * vec4(a_Position, 1.0);
	gl_Position.y *= -1.0;
}

#type fragment
#version 460 core

layout(location = 0) in vec4 v_Color;
layout(location = 1) in vec2 v_TexCoords;

layout(location = 0) out vec4 o_Color;

layout(set = 0, binding = 0) uniform texture2D u_FontAtlas;
layout(set = 0, binding = 1) uniform sampler u_Sampler;

const float pxRange = 2.0;

float screenPxRange() {
    vec2 unitRange = vec2(pxRange)/vec2(textureSize(sampler2D(u_FontAtlas, u_Sampler), 0));
    vec2 screenTexSize = vec2(1.0)/fwidth(v_TexCoords);
    return max(0.5*dot(unitRange, screenTexSize), 1.0);
}

float median(float r, float g, float b) {
    return max(min(r, g), min(max(r, g), b));
}

void main() {
	vec3 msd = texture(sampler2D(u_FontAtlas, u_Sampler), v_TexCoords).rgb;
    float sd = median(msd.r, msd.g, msd.b);
    float screenPxDistance = screenPxRange()*(sd - 0.5);
    float opacity = clamp(screenPxDistance + 0.5, 0.0, 1.0);

	if(opacity == 0.0)
		discard;

	vec4 bgColor = vec4(0.0);
    o_Color = mix(bgColor, v_Color, opacity);
	if(o_Color.a == 0.0)
		discard;
}


)";
		auto& texts = s_Data->Texts;
		//Reset.
		texts.VertexCount = 0;

		//Go through all texts need to be drawn and add to vertex buffer.
		AddStringToBatch(test, font, glm::mat4(1.0f), glm::vec4(1.0f));

		if (texts.VertexCount == 0)
			return;

		texts.Buffer->Get()->SetData(texts.Vertices.data(), texts.VertexCount * sizeof(TextData::TextVertex), 0);
		
		//Go through all texts need to be drawn and draw them.
		uint64_t vertexOffset = 0;
		uint64_t indexOffset = 0;

		RenderCommand::BindVertexBuffers({ *texts.Buffer }, { 0 });
		RenderCommand::BindIndexBuffer(s_Data->Sprites.SpriteIndexBuffer, 0);

		RenderCommand::BindGraphicsPipeline(texts.Pipeline);
		RenderCommand::BindPushConstants(ShaderLibrary::GetNamedShader("TextPass"), 0, sceneData.ViewProj);

		RenderCommand::BeginRenderPass(texts.TextRenderPass, *s_Data->Outputs, {});
		
		RenderCommand::BindDescriptorSet(ShaderLibrary::GetNamedShader("TextPass"), font->GetSet(), 0);
		RenderCommand::DrawIndexed(texts.VertexCount / 4 * 6, 4, 1, 0, 0, 0);

		RenderCommand::EndRenderPass();
	}

	static void CreateSpriteResources() {
		auto& spriteData = s_Data->Sprites;

		{
			uint32_t* indices = new uint32_t[SpriteData::MaxIndexCount];

			uint64_t vertex = 0;
			for (uint64_t i = 0; i < SpriteData::MaxIndexCount; i += 6) {
				indices[i] = vertex + 0;
				indices[i + 1] = vertex + 1;
				indices[i + 2] = vertex + 2;
				indices[i + 3] = vertex + 2;
				indices[i + 4] = vertex + 3;
				indices[i + 5] = vertex + 0;
				vertex += 4;
			}

			spriteData.SpriteIndexBuffer = IndexBuffer::Create(indices, SpriteData::MaxIndexCount * sizeof(uint32_t), IndexType::Uint32);
		}

		//Pipeline
		{
			GraphicsPipelineSpecification specs;
			specs.Shader = ShaderLibrary::LoadOrGetNamedShader("SpriteRender", "assets/_shaders/SpriteRender.glsl");
			specs.Subpass = 0;
			specs.RenderPass = s_Data->ScreenRenderPass;
			specs.Primitive = PrimitiveTopology::TriangleList;
			specs.Rasterization.CullMode = PipelineCullMode::None;
			specs.Input.Bindings =
			{
				{
					{"a_Position", Format::RGB32F},
					{"a_Color", Format::RGBA32F},
					{"a_TexCoords", Format::RGB32F}
				}
			};
			specs.DepthStencil.DepthTest = false;
			//specs.DepthStencil.DepthCompareOperator = Comap;
			spriteData.SpriteGraphicsPipeline = GraphicsPipeline::Create(specs);
		}
	}

	bool SceneRenderer::NeedsRecreation(Ref<Texture2D> output) {
		const auto& specs = output->GetTextureSpecification();
		return specs.Width != s_Data->Width ||
			specs.Height != s_Data->Height;
	}
	
	SceneRenderer::SceneRenderer(void* scene)
		:m_Context(scene)
	{
		if (!s_Data) {
			s_Data = new Data;

			{
				glm::vec2 screenVertices[] =
				{
					//NDC, TexCoords
					{-1.0,-1.0},{0,1},
					{1.0,-1.0},{1,1},
					{1,1},{1,0},
					{1,1},{1,0},
					{-1,1},{0,0},
					{-1.0,-1.0},{0,1},
				};

				s_Data->ScreenNDC = VertexBuffer::Create(screenVertices, sizeof(screenVertices));
			}
			
			{
				SamplerState state;
				state.Aniso = false;
				state.MagFilter = SamplerFilter::Linear;
				state.MinFilter = SamplerFilter::Linear;
				state.MipFilter = SamplerMipMapMode::Linear;
				state.AddressModeU = SamplerAddressMode::Repeat;
				state.AddressModeV = SamplerAddressMode::Repeat;
				state.AddressModeW = SamplerAddressMode::Repeat;
				s_Data->GlobalSampler = Sampler::Create(state);
			}

			CreateClusterResources();
			CreateLightCullResources();
			CreateGBufferResources();

			s_Data->DirectionalLight.Construct([](uint32_t i) {return UniformBuffer::Create(sizeof(DirectionalLight)); });

			CreateShadowPassResources();
			
			CreateScreenPassResources();

			CreateTonemapPassResources();

			CreateTextPassResources();

			CreateSpriteResources();
			Light l{};
			l.Color = glm::vec4(1.0f, 0, 0, 1);
			l.Position = glm::vec4(0,5,0.0f,0.0f);
			l.Radius = 100.0f;

			lights.push_back(l);
		}
	}

	//TODO: add depth to 2D.

	void SceneRenderer::Render(Ref<Texture2D> outputBuffer, const SceneData& sceneData)
	{
		Scene* scene = (Scene*)m_Context;
		if (NeedsRecreation(outputBuffer)) {
			RenderCommand::DeviceWaitIdle();
			s_Data->Width = outputBuffer->GetTextureSpecification().Width;
			s_Data->Height = outputBuffer->GetTextureSpecification().Height;
			RecreateSizeOrSampleDependent();
		}

		glm::mat4 proj = sceneData.Proj;
		glm::mat4 view = sceneData.View;

		glm::mat4 viewProj = sceneData.ViewProj;

		glm::mat4 invProj = glm::inverse(proj);

		float zNear = sceneData.zNear;
		float zFar = sceneData.zFar;

		{
			MakeClusters(invProj, zNear, zFar, sceneData.ScreenSize);
			InsertClusterBarrier();
			MakeLightGrids(view);

			DeferredPass(viewProj, scene->m_Registry);
			InsertLightGridBarrier();
			InsertDeferredBarrier();

			ShadowPass(sceneData);
			InsertShadowPassBarrier();

			ScreenPass(zNear, zFar, sceneData.CameraPos, view, sceneData.ViewProj);
			InsertScreenPassBarrier();
		}

		
		TonemapPass();
		InsertTonemapPassBarrier();

		TextPass(sceneData);

		ResolveToOutput(outputBuffer);
	}

	void SceneRenderer::RecreateSizeOrSampleDependent()
	{
		CreateGBufferResources();
		CreateScreenPassResources();
		CreateTonemapPassResources();
	}

	void SceneRenderer::ResolveToOutput(Ref<Texture2D> outputBuffer)
	{
		//Screen pass barrier
		{
			ImageMemoryBarrier barrier =
				ImageMemoryBarrier(
					s_Data->Outputs->Get()->GetColorAttachment(0),
					ImageLayout::TransferSrcOptimal,
					AccessFlags_ColorAttachmentWrite,
					AccessFlags_TransferRead
				);
			RenderCommand::PipelineBarrier(
				PipelineStages_ColorAttachmentOutput,
				PipelineStages_Transfer,
				{},
				{},
				{ barrier }
			);
		}

		ImageLayout outputLayout = outputBuffer->GetTextureSpecification().Layout;

		//Transition Output to copy layout
		{
			ImageMemoryBarrier barrier =
				ImageMemoryBarrier(
					outputBuffer,
					ImageLayout::TransferDstOptimal,
					AccessFlags_None,
					AccessFlags_TransferWrite
				);

			barrier.OldLayout = ImageLayout::None;

			RenderCommand::PipelineBarrier(
				PipelineStages_TopOfPipe,
				PipelineStages_Transfer,
				{},
				{},
				{ barrier }
			);
		}

		RenderCommand::CopyTexture(s_Data->Outputs->Get()->GetColorAttachment(0), 0, 0, outputBuffer, 0, 0);

		{
			ImageMemoryBarrier barrier =
				ImageMemoryBarrier(
					outputBuffer,
					ImageLayout::ShaderReadOnlyOptimal,
					AccessFlags_TransferWrite,
					AccessFlags_ShaderRead
				);

			RenderCommand::PipelineBarrier(
				PipelineStages_Transfer,
				PipelineStages_VertexShader,
				{},
				{},
				{ barrier }
			);
		}
	}

	glm::vec3 col = glm::vec3(1.0), dir = glm::vec3(0,0,1),pos(0,0,-500);

}
