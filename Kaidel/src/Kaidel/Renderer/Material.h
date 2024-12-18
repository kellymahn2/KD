#pragma once
#include "Kaidel/Core/Base.h"
#include "Kaidel/Scene/Entity.h"
#include "Kaidel/Renderer/GraphicsAPI/UniformBuffer.h"
#include "Kaidel/Renderer/RenderCommand.h"

#include <glm/glm.hpp>


namespace Kaidel {

	static constexpr inline uint32_t MaterialValuesBinding = 0;
	static constexpr inline uint32_t MaterialDataSet = 3;

	enum class MaterialTextureType {
		Albedo = 0,
		Metallic = 1,
		Roughness = 2,
		Normal = 3,
		Max
	};
	class Material3D : public IRCCounter<false> {
	public:
		Material3D() = default;
		virtual ~Material3D() {};

		virtual void DrawRect() {};

		virtual void DrawMesh(uint32_t instanceCount, uint32_t indexCount) = 0;

		const std::set<UUID> GetEntitySet() const { return m_Entities; }
		void AddEntity(UUID id) { m_Entities.insert(id); }

		void RemoveEntity(UUID id) { m_Entities.erase(id); }

		bool EntityUses(UUID id) { return m_Entities.find(id) != m_Entities.end(); }
	protected:
		std::set<UUID> m_Entities;
	};

	class BaseMaterial3D : public Material3D{
	public:
		BaseMaterial3D() {
			m_Buffer = UniformBuffer::Create(sizeof(MaterialValues), MaterialValuesBinding);
			m_Buffer->SetData(&m_Values, sizeof(MaterialValues), 0);

			m_DescriptorSet = DescriptorSet::Create(s_Pipeline, MaterialDataSet);
		}

		const glm::vec3& GetAlbedoColor()const { return m_Values.AlbedoColor; }
		void SetAlbedoColor(const glm::vec3& color) { m_Values.AlbedoColor = color; }

		const glm::vec3& GetSpecular()const { return m_Values.Specular; }
		void SetSpecular(const glm::vec3& value) { m_Values.Specular = value; }

		const glm::vec3& GetMetallic()const { return m_Values.Metallic; }
		void SetMetallic(const glm::vec3& value) { m_Values.Metallic = value; }

		const glm::vec3& GetRoughness()const { return m_Values.Roughness; }
		void SetRoughness(const glm::vec3& value) { m_Values.Roughness = value; }


		void SetTexture(MaterialTextureType type, Ref<Image> image) {
			m_Textures[(uint32_t)type] = image;
			m_NeedsDescriptorWrite = true;
			DescriptorSetUpdate update{};
			update.ArrayIndex = 0;
			update.Binding = MaterialValuesBinding + 1 + (uint32_t)type;
			update.Type = DescriptorType::CombinedSampler;
			update.ImageUpdate.Sampler = s_Sampler->GetRendererID();
			update.ImageUpdate.Layout = ImageLayout::ShaderReadOnlyOptimal;
			update.ImageUpdate.ImageView = image->GetSpecification().ImageView;
			m_DescriptorSet->Update(update);
		}

		void DrawMesh(uint32_t instanceCount, uint32_t indexCount)
		{
			for (auto& texture : m_Textures) {
				RenderCommand::Transition(texture, ImageLayout::ShaderReadOnlyOptimal);
			}

			m_Buffer->SetData(&m_Values, sizeof(MaterialValues));

			RenderCommand::BindDescriptorSet(m_DescriptorSet, MaterialDataSet);
			RenderCommand::BindGraphicsPipeline(s_Pipeline);
			RenderCommand::DrawIndexedInstanced(indexCount, instanceCount, 0, 0);
		}

	private:
		struct MaterialValues {
			glm::vec3 AlbedoColor = {};
			glm::vec3 Specular = {};
			glm::vec3 Metallic = {};
			glm::vec3 Roughness = {};
		};

		MaterialValues m_Values;

		Ref<Image> m_Textures[(uint32_t)MaterialTextureType::Max] = {};
		Ref<DescriptorSet> m_DescriptorSet;
		bool m_NeedsDescriptorWrite = true;

		glm::vec2 m_TopLeftUV = { 0,1 };
		glm::vec2 m_BottomRightUV = { 1,0 };


		bool m_Changed = false;
		Ref<UniformBuffer> m_Buffer;
		static inline Ref<GraphicsPipeline> s_Pipeline;
		static inline Ref<SamplerState> s_Sampler;
	};
}
