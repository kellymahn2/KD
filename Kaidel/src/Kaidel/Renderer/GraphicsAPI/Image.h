#pragma once

#include "Core.h"
#include "Kaidel/Renderer/RendererDefinitions.h"
#include "Kaidel/Renderer/GraphicsAPI/DescriptorSet.h"
namespace Kaidel {
	struct Image {
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
