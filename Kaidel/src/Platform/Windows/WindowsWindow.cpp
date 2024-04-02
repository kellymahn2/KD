#include "KDpch.h"
#include "Platform/Windows/WindowsWindow.h"

#include "Kaidel/Core/Input.h"

#include "Kaidel/Events/ApplicationEvent.h"
#include "Kaidel/Events/MouseEvent.h"
#include "Kaidel/Events/KeyEvent.h"

#include "Kaidel/Renderer/Renderer.h"

#include "Platform/OpenGL/OpenGLContext.h"

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
		m_Context = GraphicsContext::Create(m_Window);
		m_Context->Init();

		RenderCommand::GetRendererAPI() = RendererAPI::Create();

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
		SetVSync(false);
	}

	void WindowsWindow::Shutdown()
	{
		glfwDestroyWindow(m_Window);
		--s_GLFWWindowCount;

		if (s_GLFWWindowCount == 0)
		{
			RenderCommand::GetRendererAPI()->Shutdown();
			m_Context->Shutdown();
			glfwTerminate();
		}
	}

	void WindowsWindow::OnUpdate()
	{
		glfwPollEvents();
		m_Context->SwapBuffers();
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

	static void CheckAndSetCursorPosition(GLFWwindow* window) {
			double rel_xpos, rel_ypos;
			glfwGetCursorPos(window, &rel_xpos, &rel_ypos);

			GLFWmonitor* monitor = glfwGetPrimaryMonitor();


			int window_pos_x, window_pos_y;
			glfwGetWindowPos(window, &window_pos_x, &window_pos_y);

			double xpos = rel_xpos + window_pos_x;
			double ypos = rel_ypos + window_pos_y;


			if (monitor != NULL) {
				int monitor_x, monitor_y;
				glfwGetMonitorPos(monitor, &monitor_x, &monitor_y);
				const GLFWvidmode* mode = glfwGetVideoMode(monitor);
				int monitor_width = mode->width;
				int monitor_height = mode->height;

				if (xpos <= monitor_x)
					xpos = monitor_x + monitor_width - 1;
				else if (xpos >= monitor_x + monitor_width -1)
					xpos = monitor_x;


				glfwSetCursorPos(window, xpos - window_pos_x, rel_ypos);
			}



	}


	void WindowsWindow::WrapCursor() const {
		CheckAndSetCursorPosition(m_Window);
	}
}
