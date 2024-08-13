#pragma once
#include "Kaidel/Core/Base.h"
#include "Kaidel/Renderer/RendererDefinitions.h"
#include "Kaidel/Renderer/GraphicsAPI/GraphicsPipeline.h"



namespace Kaidel {


	struct DescriptorSetUpdateBuffer {
		RendererID Buffer;
		uint64_t Offset;
		uint64_t Size;
	};

	struct DescriptorSetUpdateImage {
		RendererID Sampler;
		RendererID ImageView;
		ImageLayout Layout;
	};


	struct DescriptorSetUpdate {
		DescriptorType Type;
		uint32_t Binding;
		uint32_t ArrayIndex;
		union {
			DescriptorSetUpdateBuffer BufferUpdate;
			DescriptorSetUpdateImage ImageUpdate;
		};
	};



	class DescriptorSet : public IRCCounter<false> {
	public:

		virtual ~DescriptorSet() = default;

		virtual RendererID GetSetID()const = 0;
		virtual void Update(const DescriptorSetUpdate& update) = 0;
		virtual void UpdateAll(const DescriptorSetUpdate& update) = 0;

		static Ref<DescriptorSet> Create(Ref<GraphicsPipeline> pipeline,uint32_t setBinding);
		static Ref<DescriptorSet> Create(DescriptorType type, ShaderStages flags);
	};
}
