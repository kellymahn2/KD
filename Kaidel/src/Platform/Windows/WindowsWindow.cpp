#include "KDpch.h"
#include "Platform/Windows/WindowsWindow.h"

#include "Kaidel/Core/Input.h"

#include "Kaidel/Events/ApplicationEvent.h"
#include "Kaidel/Events/MouseEvent.h"
#include "Kaidel/Events/KeyEvent.h"
#include "Kaidel/ImGui/ImGuiLayer.h"

#include "Kaidel/Renderer/Renderer.h"
#include "Kaidel/Renderer/RendererAPI.h"

#include <imgui.h>




namespace Kaidel {
	
	static uint8_t s_GLFWWindowCount = 0;

	static void GLFWErrorCallback(int error, const char* description)
	{
		KD_CORE_ERROR("GLFW Error ({0}): {1}", error, description);
	}

	WindowsWindow::WindowsWindow(const WindowProps& props)
	{
		Init(props);
	}

	WindowsWindow::~WindowsWindow()
	{

		Shutdown();
	}

	void WindowsWindow::Init(const WindowProps& props)
	{

		m_Data.Title = props.Title;
		m_Data.Width = props.Width;
		m_Data.Height = props.Height;
		m_Data.Fullscreen = props.Fullscreen;

		if (s_GLFWWindowCount == 0)
		{
			int success = glfwInit();
			KD_CORE_ASSERT(success, "Could not initialize GLFW!");
			glfwSetErrorCallback(GLFWErrorCallback);
		}
		if (m_Data.Fullscreen) {
			GLFWmonitor* monitor = glfwGetPrimaryMonitor();
			const GLFWvidmode* vidMode = glfwGetVideoMode(monitor);
			m_Data.Width = vidMode->width;
			m_Data.Height = vidMode->height;
		}

		KD_CORE_INFO("Creating window {0} ({1}, {2})", m_Data.Title, m_Data.Width, m_Data.Height);
		{
		#ifdef KD_DEBUG
			if (Renderer::GetAPI() == RendererAPI::API::OpenGL)
				glfwWindowHint(GLFW_OPENGL_DEBUG_CONTEXT, GLFW_TRUE);
		#endif
			if(Renderer::GetAPI() == RendererAPI::API::DirectX || Renderer::GetAPI() == RendererAPI::API::Vulkan)
				glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
			m_Window = glfwCreateWindow((int)m_Data.Width, (int)m_Data.Height, m_Data.Title.c_str(), nullptr, nullptr);
			++s_GLFWWindowCount;
		}
		m_Context = GraphicsContext::Create(this);
		m_Context->Init();

		RenderCommand::Get() = RendererAPI::Create();

		glfwSetWindowUserPointer(m_Window, &m_Data);

		// Set GLFW callbacks
		glfwSetWindowSizeCallback(m_Window, [](GLFWwindow* window, int width, int height)
		{
			WindowData& data = *(WindowData*)glfwGetWindowUserPointer(window);
			data.Width = width;
			data.Height = height;

			WindowResizeEvent event(width, height);
			data.EventCallback(event);
		});

		glfwSetWindowCloseCallback(m_Window, [](GLFWwindow* window)
		{
			WindowData& data = *(WindowData*)glfwGetWindowUserPointer(window);
			WindowCloseEvent event;
			data.EventCallback(event);
		});

		glfwSetKeyCallback(m_Window, [](GLFWwindow* window, int key, int scancode, int action, int mods)
		{
			WindowData& data = *(WindowData*)glfwGetWindowUserPointer(window);

			switch (action)
			{
				case GLFW_PRESS:
				{
					KeyPressedEvent event(key, 0);
					data.EventCallback(event);
					break;
				}
				case GLFW_RELEASE:
				{
					KeyReleasedEvent event(key);
					data.EventCallback(event);
					break;
				}
				case GLFW_REPEAT:
				{
					KeyPressedEvent event(key, 1);
					data.EventCallback(event);
					break;
				}
			}
		});

		glfwSetCharCallback(m_Window, [](GLFWwindow* window, unsigned int keycode)
		{
			WindowData& data = *(WindowData*)glfwGetWindowUserPointer(window);

			KeyTypedEvent event(keycode);
			data.EventCallback(event);
		});

		glfwSetMouseButtonCallback(m_Window, [](GLFWwindow* window, int button, int action, int mods)
		{
			WindowData& data = *(WindowData*)glfwGetWindowUserPointer(window);

			switch (action)
			{
				case GLFW_PRESS:
				{
					MouseButtonPressedEvent event(button);
					data.EventCallback(event);
					break;
				}
				case GLFW_RELEASE:
				{
					MouseButtonReleasedEvent event(button);
					data.EventCallback(event);
					break;
				}
			}
		});

		glfwSetScrollCallback(m_Window, [](GLFWwindow* window, double xOffset, double yOffset)
		{
			WindowData& data = *(WindowData*)glfwGetWindowUserPointer(window);

			MouseScrolledEvent event((float)xOffset, (float)yOffset);
			data.EventCallback(event);
		});

		glfwSetCursorPosCallback(m_Window, [](GLFWwindow* window, double xPos, double yPos)
		{
			WindowData& data = *(WindowData*)glfwGetWindowUserPointer(window);

			MouseMovedEvent event((float)xPos, (float)yPos);
			data.EventCallback(event);
		});


		glfwSetWindowRefreshCallback(m_Window, [](GLFWwindow* window) {
			WindowData& data = *(WindowData*)glfwGetWindowUserPointer(window);
			WindowRedrawEvent event;
			data.EventCallback(event);
			});

		SetVSync(false);
	}

	void WindowsWindow::Shutdown()
	{
		glfwDestroyWindow(m_Window);
		--s_GLFWWindowCount;

		if (s_GLFWWindowCount == 0)
		{
			RenderCommand::Get()->Shutdown();
			m_Context->Shutdown();
			glfwTerminate();
		}
	}

	void WindowsWindow::PollEvents() const
	{
		glfwPollEvents();
	}

	void WindowsWindow::AcquireImage() const
	{
		m_Context->AcquireImage();
	}

	void WindowsWindow::PresentImage() const
	{
		m_Context->PresentImage();
	}

	void WindowsWindow::SwapBuffers() const
	{
		m_Context->SwapBuffers();
	}

	void WindowsWindow::OnUpdate()
	{
		PollEvents();
		SwapBuffers();
	}

	void WindowsWindow::SetVSync(bool enabled)
	{

		if (enabled)
			glfwSwapInterval(1);
		else
			glfwSwapInterval(0);

		m_Data.VSync = enabled;
	}

	bool WindowsWindow::IsVSync() const
	{
		return m_Data.VSync;
	}


	static void GetMonitorInformation(int* w, int* h) {
		GLFWmonitor* monitor = glfwGetPrimaryMonitor();
		int a, b;
		glfwGetMonitorWorkarea(monitor, &a, &b, w, h);
	}


	static void CheckAndSetCursorPosition(GLFWwindow* window) {

		//KD_CORE_INFO("-----------------------------");
		
		double currentPosRelWindowX, currentPosRelWindowY;
		glfwGetCursorPos(window, &currentPosRelWindowX, &currentPosRelWindowY);
		//KD_CORE_INFO("\t({},{})", currentPosRelWindowX, currentPosRelWindowY);

		int monitorWidth, monitorHeight;
		GetMonitorInformation(&monitorWidth, &monitorHeight);
		//KD_CORE_INFO("\t({},{})", monitorWidth, monitorHeight);

		int windowPosX, windowPosY;
		glfwGetWindowPos(window, &windowPosX, &windowPosY);
		//KD_CORE_INFO("\t({},{})", windowPosX, windowPosY);

		int globalPosX, globalPosY;
		globalPosX = currentPosRelWindowX + windowPosX;
		globalPosY = currentPosRelWindowY + windowPosY;
		//KD_CORE_INFO("\t({},{})", globalPosX, globalPosY);

		bool wrapped = false;

		if (globalPosX <= 0) {
			globalPosX = monitorWidth - 2;
			wrapped = true;
		}
		if (globalPosX >= monitorWidth - 1) {
			globalPosX = 1;
			wrapped = true;
		}
		
		double newPosRelWindowX, newPosRelWindowY;
		newPosRelWindowX = globalPosX - windowPosX;
		newPosRelWindowY = currentPosRelWindowY;
		glfwSetCursorPos(window, newPosRelWindowX, newPosRelWindowY);

		if (wrapped) {
			Application::Get().GetImGuiLayer()->OnMouseWrap(globalPosX);
		}

	}


	void WindowsWindow::WrapCursor() const {
		CheckAndSetCursorPosition(m_Window);
	}
}
