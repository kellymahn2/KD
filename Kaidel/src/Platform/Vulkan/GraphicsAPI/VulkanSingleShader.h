#pragma once

#include "VulkanBase.h"
#include "Kaidel/Renderer/GraphicsAPI/SingleShader.h"
#include "VulkanGraphicsContext.h"

namespace Kaidel {
	namespace Vulkan {

		class VulkanSingleShader : public SingleShader {
		public:
			VulkanSingleShader(const SingleShaderSpecification& specification);
			~VulkanSingleShader();

			const SingleShaderSpecification& GetSpecification() const {  return m_Specification;}

		private:
			VkShaderModule m_ShaderModule = VK_NULL_HANDLE;
			SingleShaderSpecification m_Specification = {};
		};


	}
}
