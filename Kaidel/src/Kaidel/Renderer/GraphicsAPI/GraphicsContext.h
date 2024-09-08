#pragma once

namespace Kaidel {

	class Window;
	class GraphicsContext 
	{
	public:
		virtual ~GraphicsContext() = default;

		virtual void Init() = 0;
		//TODO: deprecated, REMOVE.
		virtual void SwapBuffers() = 0;
		virtual void Shutdown() {};

		virtual void AcquireImage() = 0;
		virtual void PresentImage() = 0;

		//ImGui callbacks
		virtual void ImGuiInit()const = 0;
		virtual void ImGuiBegin()const = 0;
		virtual void ImGuiEnd()const = 0;
		virtual void ImGuiShutdown()const = 0;

		virtual uint32_t GetMaxFramesInFlightCount()const = 0;
		virtual uint32_t GetCurrentFrameIndex()const = 0;

		static Scope<GraphicsContext> Create(Window* window);
	};

}
