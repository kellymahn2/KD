#pragma once

#include "Core.h"
#include "Kaidel/Renderer/RendererDefinitions.h"
#include "Kaidel/Renderer/GraphicsAPI/DescriptorSet.h"
namespace Kaidel {
	struct ImageSpecification {
		RendererID _InternalImageID;
		RendererID _DeviceMemory;

		//Temp.
		RendererID ImageView;
		RendererID Sampler;
		//Temp.

		Format ImageFormat;

		uint32_t Width;
		uint32_t Height;
		uint32_t Depth;
		uint32_t Layers;
		uint32_t Levels;
		ImageLayout Layout;
		ImageLayout IntendedLayout;
	};

	class Image : public IRCCounter<false> {
	public:
		Image() = default;
		Image(const ImageSpecification& spec)
			:m_Specification(spec)
		{}

		void SetSpecification(const ImageSpecification& spec) { m_Specification = spec; }
		const ImageSpecification& GetSpecification()const { return m_Specification; }
		ImageSpecification& GetSpecification() { return m_Specification; }


	private:
		ImageSpecification m_Specification;
	};


	struct ImageSamplingRegion {
		Ref<Image> m_Image;
		
		//Normalized
		glm::vec2 TopLeft;
		//Normalized
		glm::vec2 BottomRight;
	};











	struct ImageSubresource {
		RendererID _InternalImageID;
		RendererID _DeviceMemory;

		RendererID ImageView;

		uint32_t Layer;

		//Normalized
		glm::vec2 TopLeft;
		//Normalized
		glm::vec2 BottomRight;
	};


}
