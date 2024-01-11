#include "vcpch.h"
#include "Application.hpp"

#include <GLFW/glfw3.h>

namespace VkApp
{

	Application* Application::s_Instance = nullptr;

	Application::Application(const AppInfo& appInfo)
		: m_AppInfo(appInfo)
	{
		Init(appInfo);
	}

	Application::~Application()
	{
		for (Layer* layer : m_LayerStack)
		{
			layer->OnDetach();
			delete layer;
		}

		m_API->Destroy();
	}

	void Application::OnEvent(Event& e)
	{
		EventHandler handler(e);

		handler.Handle<WindowCloseEvent>(VKAPP_BIND_EVENT_FN(Application::OnWindowClose));
		handler.Handle<WindowResizeEvent>(VKAPP_BIND_EVENT_FN(Application::OnWindowResize));

		for (auto it = m_LayerStack.rbegin(); it != m_LayerStack.rend(); ++it)
		{
			if (e.Handled)
				break;
			(*it)->OnEvent(e);
		}
	}

	void Application::Run()
	{
		while (m_Running)
		{
			//Delta Time
			float currentTime = (float)glfwGetTime();
			static float lastTime = 0.0f;

			float deltaTime = currentTime - lastTime;
			lastTime = currentTime;

			//Update & Render
			m_Window->OnUpdate();

			for (Layer* layer : m_LayerStack)
			{
				layer->OnUpdate(deltaTime);
				layer->OnRender();
			}

			m_ImGuiLayer->Begin();
			for (Layer* layer : m_LayerStack)
				layer->OnImGuiRender();
			m_ImGuiLayer->End();

			m_Window->OnRender();
		}
	}

	void Application::AddLayer(Layer* layer)
	{
		m_LayerStack.AddLayer(layer);
	}

	void Application::AddOverlay(Layer* layer)
	{
		m_LayerStack.AddOverlay(layer);
	}

	void Application::Init(const AppInfo& appInfo)
	{
		s_Instance = this;

		m_Window = Window::Create(appInfo.WindowProperties);
		m_Window->SetEventCallBack(VKAPP_BIND_EVENT_FN(Application::OnEvent));

		m_API = API::Create();
		m_API->Init();

		//Add ImGui
		m_ImGuiLayer = new BaseImGuiLayer();
		AddOverlay(m_ImGuiLayer);
	}

	bool Application::OnWindowClose(WindowCloseEvent& e)
	{
		m_Running = false;

		return false;
	}

	bool Application::OnWindowResize(WindowResizeEvent& e)
	{
		if (e.GetWidth() == 0 || e.GetHeight() == 0)
		{
			m_Minimized = true;
			return true;
		}

		m_Minimized = false;

		return false;
	}

}