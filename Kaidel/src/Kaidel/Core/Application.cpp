#include "KDpch.h"
#include "Kaidel/Core/Application.h"
#include "Kaidel/Core/Random.h"
#include "Kaidel/Events/SettingsEvent.h"
#include "Kaidel/Core/Log.h"

#include "Kaidel/Renderer/Renderer.h"

#include "Kaidel/Scripting/ScriptEngine.h"

#include "Kaidel/Core/Input.h"

#include <GLFW/glfw3.h>
namespace Kaidel {
	Application* Application::s_Instance = nullptr;

	Application::Application(const ApplicationSpecification& specification)
		: m_Specification(specification)
	{
		KD_CORE_ASSERT(!s_Instance, "Application already exists!");
		
		Random::m_RandomEngine = std::mt19937(Random::m_RandomDevice());

		s_Instance = this;
		if (!m_Specification.WorkingDirectory.empty())
			std::filesystem::current_path(m_Specification.WorkingDirectory);
		m_Window = Window::Create(WindowProps(m_Specification.Name));
		m_Window->SetEventCallback(KD_BIND_EVENT_FN(Application::OnEvent));

		Renderer::Init();
		ScriptEngine::Init();

		JobSystem::InitMainJobSystem();

		m_ImGuiLayer = new ImGuiLayer();
		PushOverlay(m_ImGuiLayer);
	}

	Application::~Application()
	{
		delete &JobSystem::GetMainJobSystem();
		ScriptEngine::Shutdown();
		Renderer::Shutdown();
	}

	void Application::PushLayer(Layer* layer)
	{

		m_LayerStack.PushLayer(layer);
		layer->OnAttach();
	}

	void Application::PushOverlay(Layer* layer)
	{

		m_LayerStack.PushOverlay(layer);
		layer->OnAttach();
	}

	void Application::Close()
	{
		m_Running = false;
	}

	void Application::SubmitToMainThread(const std::function<void()>& func)
	{
		std::scoped_lock<std::mutex> lock(m_AppThreadQueueMutex);
		
		m_AppThreadQueue.push_back(func);
	}

	void Application::OnUpdate()
	{
		float time = (float)glfwGetTime();
		Timestep timestep = time - m_LastFrameTime;
		m_LastFrameTime = time;

		{
			if (!m_Minimized)
			{
				m_Window->AcquireImage();
				ExecuteMainThreadQueue();

				{
					for (Layer* layer : m_LayerStack)
						layer->OnUpdate(timestep);
				}

				{
					m_ImGuiLayer->Begin();
					{

						for (Layer* layer : m_LayerStack)
							layer->OnImGuiRender();
					}
					m_ImGuiLayer->End();
				}
				m_Window->PresentImage();
			}
			
		}
	}

	void Application::OnEvent(Event& e)
	{

		EventDispatcher dispatcher(e);
		dispatcher.Dispatch<WindowCloseEvent>(KD_BIND_EVENT_FN(Application::OnWindowClose));
		dispatcher.Dispatch<WindowResizeEvent>(KD_BIND_EVENT_FN(Application::OnWindowResize));
		//dispatcher.Dispatch<WindowRedrawEvent>(KD_BIND_EVENT_FN(Application::OnWindowRedraw));

		for (auto it = m_LayerStack.rbegin(); it != m_LayerStack.rend(); ++it)
		{
			if (e.Handled)
				break;
			(*it)->OnEvent(e);
		}

		//dispatcher.Dispatch<RendererSettingsChangedEvent>(KD_BIND_EVENT_FN(Application::OnRendererSettingsChanged));
	}

	void Application::Run()
	{

		while (m_Running)
		{

			OnUpdate();
			//{
			//	//SCOPED_TIMER(Swap Buffers)
			//	m_Window->SwapBuffers();
			//	m_Window->PollEvents();
			//}

			m_Window->PollEvents();

			m_Window->WrapCursor();
		}
	}

	bool Application::OnWindowClose(WindowCloseEvent& e)
	{
		m_Running = false;
		return true;
	}

	bool Application::OnWindowResize(WindowResizeEvent& e)
	{

		if (e.GetWidth() == 0 || e.GetHeight() == 0)
		{
			m_Minimized = true;
			return false;
		}

		m_Minimized = false;
		Renderer::OnWindowResize(e.GetWidth(), e.GetHeight());

		return false;
	}
	bool Application::OnRendererSettingsChanged(RendererSettingsChangedEvent& e) {

		auto& settings = RendererAPI::GetSettings();


		return true;
	}

	bool Application::OnWindowRedraw(WindowRedrawEvent& e)
	{
		return false;
	}

	void Application::ExecuteMainThreadQueue()
	{
		std::scoped_lock<std::mutex> lock(m_AppThreadQueueMutex);

		for (auto& func : m_AppThreadQueue)
			func();
		m_AppThreadQueue.clear();
	}

}
