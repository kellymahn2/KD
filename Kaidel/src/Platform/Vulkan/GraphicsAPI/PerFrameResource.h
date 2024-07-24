#pragma once

#include <vector>

namespace Kaidel {
	uint32_t GetFramesInFlightCount();
	uint32_t GetCurrentFrameNumber();

	template<typename T>
	class PerFrameResource {
	public:
		PerFrameResource() {
			m_Resources.resize(GetFramesInFlightCount());
		}
		T* operator->() { return &m_Resources[GetCurrentFrameNumber()]; }
		T& operator*() { return m_Resources[GetCurrentFrameNumber()]; }

		const T* operator->() const { return &m_Resources[GetCurrentFrameNumber()]; }
		const T& operator*() const { return m_Resources[GetCurrentFrameNumber()]; }

		auto begin() { return m_Resources.begin(); }
		auto end() { return m_Resources.end(); }
		auto begin()const { return m_Resources.begin(); }
		auto end()const { return m_Resources.end(); }
		auto& GetResources() { return m_Resources; }
		const auto& GetResources()const { return m_Resources; }

	private:
		std::vector<T> m_Resources;
	};
}
