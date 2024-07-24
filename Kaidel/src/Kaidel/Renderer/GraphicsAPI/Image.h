#pragma once

#include "Core.h"
#include "Kaidel/Renderer/RendererDefinitions.h"
namespace Kaidel {
	struct Image {
		RendererID ShaderBindable;
		RendererID _InternalImageID;
		RendererID _DeviceMemory;

		Format ImageFormat;

		uint32_t Width;
		uint32_t Height;
		uint32_t Depth;
		uint32_t Layers;
		uint32_t Levels;
		ImageLayout Layout;
		ImageLayout IntendedLayout;

	};

}
