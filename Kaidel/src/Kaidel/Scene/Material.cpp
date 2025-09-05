#include "KDpch.h"
#include "Material.h"

#include "Kaidel/Renderer/RenderCommand.h"

namespace Kaidel {

	Ref<Material> Material::CreateFrom(Ref<Material> src)
	{
		Ref<Material> mat = CreateRef<Material>();
		mat->m_Pipeline = s_Pipeline;
		mat->m_TextureSet = DescriptorSet::Create(ShaderLibrary::GetNamedShader("DeferredGBufferGen"), 1);

		for (uint32_t i = 0; i < (uint32_t)MaterialTextureType::Max; ++i) {
			mat->m_Textures[i] = src->m_Textures[i];
			
			Ref<Texture> t = mat->m_Textures[i];
			if (!t)
				t = RendererGlobals::GetSingleColorTexture(glm::vec4(1.0f));

			mat->m_TextureSet->Update(t, {}, ImageLayout::ShaderReadOnlyOptimal, i);
		}

		mat->m_UniformData = src->m_UniformData;

		mat->m_MaterialBuffer = UniformBuffer::Create(sizeof(MaterialUniformData));
		mat->m_MaterialBuffer->SetData(&mat->m_UniformData, sizeof(MaterialUniformData));

		mat->m_DataSet =
			DescriptorSet::Create(ShaderLibrary::GetNamedShader("DeferredGBufferGen"), 2);

		mat->m_DataSet->Update(mat->m_MaterialBuffer, 0);

		return mat;
	}

	Material::Material(Ref<RenderPass> renderPass)
	{

		if (!s_Pipeline) {
			{
				GraphicsPipelineSpecification specs;

				specs.Input.Bindings.push_back(VertexInputBinding({
					{"a_Position",Format::RGB32F},
					{"a_TexCoords",Format::RG32F},
					{"a_Normal",Format::RGB32F},
					{"a_Tangent",Format::RGB32F},
					{"a_BiTangent",Format::RGB32F},
					}));
				specs.Multisample.Samples = TextureSamples::x1;
				specs.Primitive = PrimitiveTopology::TriangleList;
				specs.Rasterization.FrontCCW = true;
				specs.Rasterization.CullMode = PipelineCullMode::Front;
				specs.Shader = ShaderLibrary::LoadShader("DeferredGBufferGen", "assets/_shaders/DeferredGBufferGen.glsl");
				specs.RenderPass = renderPass;
				specs.Subpass = 0;
				specs.DepthStencil.DepthTest = true;
				specs.DepthStencil.DepthWrite = true;
				specs.DepthStencil.DepthCompareOperator = CompareOp::Less;
				s_Pipeline = GraphicsPipeline::Create(specs);
			}
		}

		m_Pipeline = s_Pipeline;
		m_TextureSet = DescriptorSet::Create(ShaderLibrary::GetNamedShader("DeferredGBufferGen"), 1);

		Ref<Texture> defaultWhite = RendererGlobals::GetSingleColorTexture(glm::vec4(1.0f));

		for (uint32_t i = 0; i < (uint32_t)MaterialTextureType::Max; ++i) {
			m_TextureSet->Update(defaultWhite, {}, ImageLayout::ShaderReadOnlyOptimal, i);
		}

		m_MaterialBuffer = UniformBuffer::Create(sizeof(MaterialUniformData));


		m_DataSet =
			DescriptorSet::Create(ShaderLibrary::GetNamedShader("DeferredGBufferGen"), 2);

		m_DataSet->Update(m_MaterialBuffer, 0);
	}

	void Material::BindValues()
	{
		RenderCommand::BindDescriptorSet(ShaderLibrary::GetNamedShader("DeferredGBufferGen"), RendererGlobals::GetSamplerSet(), 0);
		RenderCommand::BindDescriptorSet(ShaderLibrary::GetNamedShader("DeferredGBufferGen"), m_TextureSet, 1);
		RenderCommand::BindDescriptorSet(ShaderLibrary::GetNamedShader("DeferredGBufferGen"), m_DataSet, 2);
	}


	void Material::BindInstanceData(Ref<DescriptorSet> instanceDataSet)
	{
		RenderCommand::BindDescriptorSet(ShaderLibrary::GetNamedShader("DeferredGBufferGen"), instanceDataSet, 4);
	}

	void Material::BindSceneData(Ref<DescriptorSet> sceneDataSet)
	{
		RenderCommand::BindDescriptorSet(ShaderLibrary::GetNamedShader("DeferredGBufferGen"), sceneDataSet, 3);
	}

	Kaidel::Ref<Kaidel::Shader> Material::GetShader()
	{
		return ShaderLibrary::GetNamedShader("DeferredGBufferGen");
	}

	void Material::BindTransform(const glm::mat4& transform)
	{
		RenderCommand::BindPushConstants(ShaderLibrary::GetNamedShader("DeferredGBufferGen"), 0, transform);
	}

	void Material::BindPipeline() const
	{
		RenderCommand::BindGraphicsPipeline(m_Pipeline);
	}

	Kaidel::Ref<Kaidel::GraphicsPipeline> Material::GetPipeline() const
	{
		return m_Pipeline;
	}

	Kaidel::Ref<Kaidel::DescriptorSet> Material::GetTextureSet() const
	{
		return m_TextureSet;
	}

	const Kaidel::MaterialUniformData& Material::GetUniformData() const
	{
		return m_UniformData;
	}

	void Material::SetBaseColor(const glm::vec4& color)
	{
		m_UniformData.BaseColor = color;
	}

	const glm::vec4& Material::GetBaseColor() const
	{
		return m_UniformData.BaseColor;
	}

	void Material::SetTexture(MaterialTextureType type, Ref<Texture2D> image)
	{
		if (image)
		{
			m_Textures[(uint32_t)type] = image;
			Application::Get().SubmitToMainThread([this, image, type]() {
				KD_INFO("Bound");
				RenderCommand::DeviceWaitIdle();
				m_TextureSet->Update(image, {}, ImageLayout::ShaderReadOnlyOptimal, (uint32_t)type);
				});
			m_UniformData.HasTexture[(uint32_t)type] = 1;
		}
		else
		{
			m_Textures[(uint32_t)type] = {};
			Application::Get().SubmitToMainThread([this, type]() {
				RenderCommand::DeviceWaitIdle();
				m_TextureSet->Update(
					RendererGlobals::GetSingleColorTexture(glm::vec4(1.0f)), {},
					ImageLayout::ShaderReadOnlyOptimal, (uint32_t)type);
				});
			m_UniformData.HasTexture[(uint32_t)type] = 0;
		}
		UploadUniforms();
	}

	void Material::SetMetalnessColorChannel(MaterialTextureChannel channel)
	{
		m_UniformData.MetalnessChannel = channel;
		UploadUniforms();
	}

	void Material::SetRoughnessColorChannel(MaterialTextureChannel channel)
	{
		m_UniformData.RoughnessChannel = channel;
		UploadUniforms();
	}

	void Material::SetMetalness(float metalness)
	{
		m_UniformData.Metalness = metalness;
		UploadUniforms();
	}

	void Material::SetRoughness(float roughness)
	{
		m_UniformData.Roughness = roughness;
		UploadUniforms();
	}

	Kaidel::Ref<Kaidel::Texture2D> Material::GetTexture(MaterialTextureType type)
	{
		return m_Textures[(uint32_t)type];
	}

	bool Material::IsTextureUsed(MaterialTextureType type)
	{
		return m_UniformData.HasTexture[(uint32_t)type];
	}

	void Material::SetTextureUsed(MaterialTextureType type, bool isUsed)
	{
		m_UniformData.HasTexture[(uint32_t)type] = isUsed;
		UploadUniforms();
	}

	void Material::UploadUniforms()
	{
		RenderCommand::DeviceWaitIdle();
		m_MaterialBuffer->SetData(&m_UniformData, sizeof(MaterialUniformData));
	}

}
