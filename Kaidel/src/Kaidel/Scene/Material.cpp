#include "KDpch.h"
#include "Material.h"

#include "Kaidel/Renderer/RenderCommand.h"

#include "Kaidel/Renderer/Renderer3D.h"

namespace Kaidel {

	/*Ref<Material> Material::CreateFrom(Ref<Material> src)
	{
		Ref<Material> mat = CreateRef<Material>();
		mat->m_Pipeline = s_StandardPipeline;
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

	*/


	static Ref<GraphicsPipeline> s_StandardPipeline;

	bool StandardMaterial::IsLoaded() const
	{
		if (!AssetBase::IsLoaded())
			return false;

		//Make sure every texture is loaded.
		//for(uint32_t i = 0; i < )

		return true;
	}

	StandardMaterial::StandardMaterial(const Initializer& initializer)
	{
		if (!s_StandardPipeline) {
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
				specs.Rasterization.CullMode = PipelineCullMode::None;
				specs.Shader = ShaderLibrary::LoadShader("DeferredGBufferGen", "assets/_shaders/DeferredGBufferGen.glsl");
				specs.RenderPass = Renderer3D::GetDeferredPassRenderPass();
				specs.Subpass = 0;
				specs.DepthStencil.DepthTest = true;
				specs.DepthStencil.DepthWrite = true;
				specs.DepthStencil.DepthCompareOperator = CompareOp::Less;
				s_StandardPipeline = GraphicsPipeline::Create(specs);
			}
		}

		Initialize(initializer);
	}

	void StandardMaterial::Initialize(const Initializer& initializer)
	{
		m_TextureSet = DescriptorSet::Create(ShaderLibrary::GetNamedShader("DeferredGBufferGen"), 3);

		Ref<SpriteSheet> defaultWhite = RendererGlobals::GetSingleColorTexture(glm::vec4(1.0f));

		for (uint32_t i = 0; i < (uint32_t)MaterialTextureType::Max; ++i) {
			Ref<SpriteSheet> t = defaultWhite;

			if (initializer.Textures[i])
			{
				t = initializer.Textures[i];
				m_UniformData.HasTexture[i] = true;
			}

			m_Textures[i] = t;

			m_TextureSet->Update(t->GetTexture(), {}, ImageLayout::ShaderReadOnlyOptimal, i);
		}


		m_UniformData.BaseColor = initializer.BaseColor;
		m_UniformData.Roughness = initializer.Roughness;
		m_UniformData.Metalness = initializer.Metalness;
		m_UniformData.MetalnessChannel = initializer.MetalnessChannel;
		m_UniformData.RoughnessChannel = initializer.RoughnessChannel;

		m_UniformBuffer = UniformBuffer::Create(sizeof(MaterialUniformData), &m_UniformData);

		m_UniformSet =
			DescriptorSet::Create(ShaderLibrary::GetNamedShader("DeferredGBufferGen"), 4);

		m_UniformSet->Update(m_UniformBuffer, 0);
	}

	void StandardMaterial::BindDescriptors()
	{
		RenderCommand::BindDescriptorSet(ShaderLibrary::GetNamedShader("DeferredGBufferGen"), m_TextureSet, 3);
		RenderCommand::BindDescriptorSet(ShaderLibrary::GetNamedShader("DeferredGBufferGen"), m_UniformSet, 4);
	}


	void StandardMaterial::BindInstanceData(Ref<DescriptorSet> instanceDataSet)
	{
		RenderCommand::BindDescriptorSet(ShaderLibrary::GetNamedShader("DeferredGBufferGen"), instanceDataSet, 2);
	}


	void StandardMaterial::BindInstanceOffset(uint32_t instanceOffset)
	{
		RenderCommand::BindPushConstants(ShaderLibrary::GetNamedShader("DeferredGBufferGen"), 0, instanceOffset);
	}


	void StandardMaterial::BindPipeline() const
	{
		RenderCommand::BindGraphicsPipeline(s_StandardPipeline);
	}


	void StandardMaterial::BindSceneData(Ref<DescriptorSet> sceneDataSet)
	{
		RenderCommand::BindDescriptorSet(ShaderLibrary::GetNamedShader("DeferredGBufferGen"), sceneDataSet, 1);
	}


	Kaidel::Ref<Kaidel::Shader> StandardMaterial::GetShader()
	{
		return ShaderLibrary::GetNamedShader("DeferredGBufferGen");
	}


	void StandardMaterial::Recreate()
	{
		throw std::logic_error("The method or operation is not implemented.");
	}


	void StandardMaterial::SetTexture(MaterialTextureType type, Ref<SpriteSheet> image)
	{
		if (image)
		{
			m_Textures[(uint32_t)type] = image;
			m_UniformData.HasTexture[(uint32_t)type] = 1; 
			//TODO: since the previews run on their own command buffer, it doesn't get updated.
			Application::Get().SubmitToMainThread([this, image, type]() {
				RenderCommand::DeviceWaitIdle();
				m_TextureSet->Update(image->GetTexture(), image->GetSampler(), ImageLayout::ShaderReadOnlyOptimal, (uint32_t)type);
			});
		}
		else
		{
			m_Textures[(uint32_t)type] = {};
			m_UniformData.HasTexture[(uint32_t)type] = 0;
			Application::Get().SubmitToMainThread([this, type]() {
				RenderCommand::DeviceWaitIdle();
				m_TextureSet->Update(
					RendererGlobals::GetSingleColorTexture(glm::vec4(1.0f))->GetTexture(), {},
					ImageLayout::ShaderReadOnlyOptimal, (uint32_t)type);
				});
		}
		UploadUniforms();
	}

	ShaderMaterial::ShaderMaterial(Ref<VisualShader> visualMaterial)
	{
		if (visualMaterial)
		{
			SetVisualShader(visualMaterial);
		}
	}

	void ShaderMaterial::BindDescriptors()
	{
		AssureLatest();

		for (auto& [setID, set] : m_Sets)
		{
			RenderCommand::BindDescriptorSet(m_VisualShader->Shader, set, setID);
		}
	}


	void ShaderMaterial::BindInstanceData(Ref<DescriptorSet> instanceDataSet)
	{
		RenderCommand::BindDescriptorSet(m_VisualShader->Shader, instanceDataSet, 2);
	}


	void ShaderMaterial::BindInstanceOffset(uint32_t instanceOffset)
	{
		RenderCommand::BindPushConstants(m_VisualShader->Shader, 0, instanceOffset);
	}


	void ShaderMaterial::BindPipeline() const
	{
		RenderCommand::BindGraphicsPipeline(m_VisualShader->Pipeline);
	}


	void ShaderMaterial::SetVisualShader(Ref<VisualShader> visualMaterial)
	{
		m_VisualShader = visualMaterial;

		CreateSets();

		m_LastVersion = visualMaterial->Version;
	}

	void ShaderMaterial::BindSceneData(Ref<DescriptorSet> sceneDataSet)
	{
		RenderCommand::BindDescriptorSet(m_VisualShader->Shader, sceneDataSet, 1);
	}


	Kaidel::Ref<Kaidel::Shader> ShaderMaterial::GetShader()
	{
		return m_VisualShader->Shader;
	}


	void ShaderMaterial::Recreate()
	{
		throw std::logic_error("The method or operation is not implemented.");
	}


	void ShaderMaterial::UploadUniformData()
	{
		m_UniformBuffer->SetData(m_UniformData.data(), m_VisualShader->UniformLayout.Size);
	}

	void ShaderMaterial::SetTexture(Ref<Texture2D> texture, uint32_t set, uint32_t binding)
	{
		if (texture)
		{
			Application::Get().SubmitToMainThread([this, texture, set, binding]() {
				RenderCommand::DeviceWaitIdle();
				m_Sets[set]->Update(texture, {}, ImageLayout::ShaderReadOnlyOptimal, binding);
				});
		}
		else
		{
			Application::Get().SubmitToMainThread([this, set, binding]() {
				RenderCommand::DeviceWaitIdle();
				m_Sets[set]->Update(RendererGlobals::GetSingleColorTexture(glm::vec4(1.0f))->GetTexture(), {}, ImageLayout::ShaderReadOnlyOptimal, binding);
				});
		}
	}

	void ShaderMaterial::AssureLatest()
	{
		if (m_LastVersion != m_VisualShader->Version)
		{
			CreateSets();

			m_LastVersion = m_VisualShader->Version;
		}
	}

	void ShaderMaterial::CreateSets()
	{
		Ref<Shader> shader = m_VisualShader->Shader;
		auto& reflection = shader->GetReflection();

		m_UniformData.clear();
		m_Sets.clear();

		for (auto& [setID, set] : reflection.Sets)
		{
			if (set.Set == 0 || set.Set == 1 || set.Set == 2)
				continue;
			m_Sets[set.Set] = DescriptorSet::Create(shader, set.Set);

			for (auto& [bindingID, binding] : set.Bindings)
			{
				if (set.Set == m_VisualShader->Code.UniformSet && binding.Binding == m_VisualShader->Code.UniformBinding)
				{
					m_UniformBuffer = UniformBuffer::Create(m_VisualShader->UniformLayout.Size);

					m_Sets[set.Set]->Update(m_UniformBuffer, binding.Binding);
						
					m_UniformData.resize(m_VisualShader->UniformLayout.Size, 0);
					continue;
				}
				
				auto it = m_VisualShader->DefaultTextureValues.find(binding.Name);
				
				if (it == m_VisualShader->DefaultTextureValues.end() || !it->second)
				{ 
					m_Sets[set.Set]->Update(RendererGlobals::GetSingleColorTexture(glm::vec4(1.0f))->GetTexture(), {}, ImageLayout::ShaderReadOnlyOptimal, bindingID);
					continue;
				}

				m_Sets[set.Set]->Update(it->second, {}, ImageLayout::ShaderReadOnlyOptimal, bindingID);
			}

		}
	}

}
