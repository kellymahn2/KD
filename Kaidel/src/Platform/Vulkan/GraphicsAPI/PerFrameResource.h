#pragma once

#include "VulkanGraphicsContext.h"

namespace Kaidel {
	namespace Vulkan {
		template<typename T>
		class PerFrameResource {
		public:
			PerFrameResource() {
				m_Resources.resize(VK_CONTEXT.GetSwapchain()->GetSpecification().ImageCount);
			}
			T* operator->() { return &m_Resources[VK_CONTEXT.GetCurrentFrameIndex()]; }
			T& operator*() { return m_Resources[VK_CONTEXT.GetCurrentFrameIndex()]; }

			const T* operator->() const{ return &m_Resources[VK_CONTEXT.GetCurrentFrameIndex()]; }
			const T& operator*() const{ return m_Resources[VK_CONTEXT.GetCurrentFrameIndex()]; }

			auto begin() { return m_Resources.begin(); }
			auto end() { return m_Resources.end(); }
			auto begin()const { return m_Resources.begin(); }
			auto end()const { return m_Resources.end(); }

			const auto& GetResources()const { return m_Resources; }

		private:
			std::vector<T> m_Resources;
		};

	}
}
