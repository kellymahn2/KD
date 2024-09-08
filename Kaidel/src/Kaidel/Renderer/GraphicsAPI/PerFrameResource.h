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
	private:
		std::vector<T> m_Resources;
	};
}
