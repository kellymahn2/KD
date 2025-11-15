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
	}*/


	static Ref<GraphicsPipeline> s_StandardPipeline;

	StandardMaterialInstance::StandardMaterialInstance()
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
				specs.Rasterization.CullMode = PipelineCullMode::Front;
				specs.Shader = ShaderLibrary::LoadShader("DeferredGBufferGen", "assets/_shaders/DeferredGBufferGen.glsl");
				specs.RenderPass = Renderer3D::GetDeferredPassRenderPass();
				specs.Subpass = 0;
				specs.DepthStencil.DepthTest = true;
				specs.DepthStencil.DepthWrite = true;
				specs.DepthStencil.DepthCompareOperator = CompareOp::Less;
				s_StandardPipeline = GraphicsPipeline::Create(specs);
			}
		}

		m_TextureSet = DescriptorSet::Create(ShaderLibrary::GetNamedShader("DeferredGBufferGen"), 3);

		Ref<Texture> defaultWhite = RendererGlobals::GetSingleColorTexture(glm::vec4(1.0f));

		for (uint32_t i = 0; i < (uint32_t)MaterialTextureType::Max; ++i) {
			m_TextureSet->Update(defaultWhite, {}, ImageLayout::ShaderReadOnlyOptimal, i);
		}

		m_UniformBuffer = UniformBuffer::Create(sizeof(MaterialUniformData));

		m_UniformSet =
			DescriptorSet::Create(ShaderLibrary::GetNamedShader("DeferredGBufferGen"), 4);

		m_UniformSet->Update(m_UniformBuffer, 0);

		UploadUniforms();
	}

	void StandardMaterialInstance::BindDescriptors()
	{
		RenderCommand::BindDescriptorSet(ShaderLibrary::GetNamedShader("DeferredGBufferGen"), m_TextureSet, 3);
		RenderCommand::BindDescriptorSet(ShaderLibrary::GetNamedShader("DeferredGBufferGen"), m_UniformSet, 4);
	}


	void StandardMaterialInstance::BindInstanceData(Ref<DescriptorSet> instanceDataSet)
	{
		RenderCommand::BindDescriptorSet(ShaderLibrary::GetNamedShader("DeferredGBufferGen"), instanceDataSet, 2);
	}


	void StandardMaterialInstance::BindInstanceOffset(uint32_t instanceOffset)
	{
		RenderCommand::BindPushConstants(ShaderLibrary::GetNamedShader("DeferredGBufferGen"), 0, instanceOffset);
	}


	void StandardMaterialInstance::BindPipeline() const
	{
		RenderCommand::BindGraphicsPipeline(s_StandardPipeline);
	}


	void StandardMaterialInstance::BindSceneData(Ref<DescriptorSet> sceneDataSet)
	{
		RenderCommand::BindDescriptorSet(ShaderLibrary::GetNamedShader("DeferredGBufferGen"), sceneDataSet, 1);
	}


	Kaidel::Ref<Kaidel::Shader> StandardMaterialInstance::GetShader()
	{
		return ShaderLibrary::GetNamedShader("DeferredGBufferGen");
	}


	void StandardMaterialInstance::Recreate()
	{
		throw std::logic_error("The method or operation is not implemented.");
	}


	VisualMaterialInstance::VisualMaterialInstance(Ref<VisualMaterial> visualMaterial)
	{
		SetVisualMaterial(visualMaterial);
	}

	void VisualMaterialInstance::BindDescriptors()
	{
		if (m_LastVersion != m_VisualMaterial->Version)
		{
			CreateSets();

			m_LastVersion = m_VisualMaterial->Version;
		}

		for (auto& [setID, set] : m_Sets)
		{
			RenderCommand::BindDescriptorSet(m_VisualMaterial->Shader, set, setID);
		}
	}


	void VisualMaterialInstance::BindInstanceData(Ref<DescriptorSet> instanceDataSet)
	{
		RenderCommand::BindDescriptorSet(m_VisualMaterial->Shader, instanceDataSet, 2);
	}


	void VisualMaterialInstance::BindInstanceOffset(uint32_t instanceOffset)
	{
		throw std::logic_error("The method or operation is not implemented.");
	}


	void VisualMaterialInstance::BindPipeline() const
	{
		RenderCommand::BindGraphicsPipeline(m_VisualMaterial->Pipeline);
	}


	void VisualMaterialInstance::SetVisualMaterial(Ref<VisualMaterial> visualMaterial)
	{
		m_VisualMaterial = visualMaterial;

		CreateSets();

		m_LastVersion = visualMaterial->Version;
	}

	void VisualMaterialInstance::BindSceneData(Ref<DescriptorSet> sceneDataSet)
	{
		RenderCommand::BindDescriptorSet(m_VisualMaterial->Shader, sceneDataSet, 1);
	}


	Kaidel::Ref<Kaidel::Shader> VisualMaterialInstance::GetShader()
	{
		return m_VisualMaterial->Shader;
	}


	void VisualMaterialInstance::Recreate()
	{
		throw std::logic_error("The method or operation is not implemented.");
	}


	void VisualMaterialInstance::UploadUniformData()
	{
		RenderCommand::DeviceWaitIdle();
		m_UniformBuffer->SetData(m_UniformData.data(), m_VisualMaterial->UniformLayout.Size);
	}

	void VisualMaterialInstance::CreateSets()
	{
		Ref<Shader> shader = m_VisualMaterial->Shader;
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
				if (set.Set == m_VisualMaterial->Code.UniformSet && binding.Binding == m_VisualMaterial->Code.UniformBinding)
				{
					m_UniformBuffer = UniformBuffer::Create(m_VisualMaterial->UniformLayout.Size);

					m_Sets[set.Set]->Update(m_UniformBuffer, binding.Binding);
						
					m_UniformData.resize(m_VisualMaterial->UniformLayout.Size, 0);
					continue;
				}
				
				auto it = m_VisualMaterial->DefaultTextureValues.find(binding.Name);
				
				if (it == m_VisualMaterial->DefaultTextureValues.end() || !it->second)
				{ 
					m_Sets[set.Set]->Update(RendererGlobals::GetSingleColorTexture(glm::vec4(1.0f)), {}, ImageLayout::ShaderReadOnlyOptimal, bindingID);
					continue;
				}

				m_Sets[set.Set]->Update(it->second, {}, ImageLayout::ShaderReadOnlyOptimal, bindingID);
			}

		}
	}

}
