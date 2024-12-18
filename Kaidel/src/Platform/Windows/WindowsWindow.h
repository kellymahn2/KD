#pragma once

#include "Kaidel/Core/Window.h"
#include "Kaidel/Renderer/GraphicsAPI/GraphicsContext.h"

#include <GLFW/glfw3.h>

namespace Kaidel {

	class WindowsWindow : public Window
	{
	public:
		WindowsWindow(const WindowProps& props);
		virtual ~WindowsWindow();

		void OnUpdate() override;

		unsigned int GetWidth() const override { return m_Data.Width; }
		unsigned int GetHeight() const override { return m_Data.Height; }

		// Window attributes
		void SetEventCallback(const EventCallbackFn& callback) override { m_Data.EventCallback = callback; }
		void SetVSync(bool enabled) override;
		bool IsVSync() const override;
		virtual const Scope<GraphicsContext>& GetContext()const override{return m_Context;}

		void SwapBuffers() const override;
		void PollEvents() const override;

		void AcquireImage()const override;
		void PresentImage()const override;

		virtual void WrapCursor() const override;

		virtual std::vector<const char*> GetRequiredInstanceExtensions()const override 
		{
			uint32_t extensionCount = 0;
			const char** extensions = glfwGetRequiredInstanceExtensions(&extensionCount);

			return { extensions,extensions + extensionCount };
		}


		virtual void* GetNativeWindow() const { return m_Window; }
	private:
		virtual void Init(const WindowProps& props);
		virtual void Shutdown();
	private:
		GLFWwindow* m_Window;

		Scope<GraphicsContext> m_Context;

		struct WindowData
		{
			std::string Title;
			unsigned int Width, Height;
			bool VSync = false;
			bool Fullscreen = true;
			EventCallbackFn EventCallback;
		};

		WindowData m_Data;
	};

}
