#pragma once

#include "Kaidel/Core/Application.h"
#include "Kaidel/Core/Window.h"
#include <vector>

namespace Kaidel {

	template<typename T>
	class PerFrameResource {
	public:
		PerFrameResource() {
			m_Resources.resize(Application::Get().GetWindow().GetContext()->GetMaxFramesInFlightCount());
		}

		void Construct(std::function<T(uint32_t)>&& func) {
			for (uint32_t i = 0; i < m_Resources.size(); ++i) {
				m_Resources[i] = func(i);
			}
		}

		void Map(std::function<T(uint32_t, const T&)>&& func) {
			for (uint32_t i = 0; i < m_Resources.size(); ++i) {
				m_Resources[i] = func(i, m_Resources[i]);
			}
		}

		void Set(const T& value) {
			for (uint32_t i = 0; i < m_Resources.size(); ++i) {
				m_Resources[i] = value;
			}
		}

		T* operator->() { return &m_Resources[Application::Get().GetWindow().GetContext()->GetCurrentFrameIndex()]; }
		T& operator*() { return m_Resources[Application::Get().GetWindow().GetContext()->GetCurrentFrameIndex()]; }

		const T* operator->() const { return &m_Resources[Application::Get().GetWindow().GetContext()->GetCurrentFrameIndex()]; }
		const T& operator*() const { return m_Resources[Application::Get().GetWindow().GetContext()->GetCurrentFrameIndex()]; }

		auto begin() { return m_Resources.begin(); }
		auto end() { return m_Resources.end(); }
		auto begin()const { return m_Resources.begin(); }
		auto end()const { return m_Resources.end(); }
		auto& GetResources() { return m_Resources; }
		const auto& GetResources()const { return m_Resources; }

		T& operator[](uint32_t i) { return m_Resources[i]; }
		const T& operator[](uint32_t i)const { return m_Resources[i]; }

		operator T& () { return m_Resources[Application::Get().GetWindow().GetContext()->GetCurrentFrameIndex()]; }
		operator const T& ()const { return m_Resources[Application::Get().GetWindow().GetContext()->GetCurrentFrameIndex()]; }

	private:
		std::vector<T> m_Resources;
	};
}
