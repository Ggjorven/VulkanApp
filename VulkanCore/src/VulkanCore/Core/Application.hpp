#pragma once

#include "VulkanCore/Core/Events.hpp"
#include "VulkanCore/Core/Layer.hpp"

#include "VulkanCore/Core/Window.hpp"
#include "VulkanCore/Core/API.hpp"

#include "VulkanCore/ImGui/BaseImGuiLayer.hpp"

#include <filesystem>

namespace VkApp
{

	struct AppInfo
	{
	public:
		WindowProperties WindowProperties;
		int ArgCount = 0;
		char** Args = nullptr;

		AppInfo() = default;
	};

	class Application
	{
	public:
		Application(const AppInfo& appInfo);
		virtual ~Application();

		void OnEvent(Event& e);

		void Run();
		inline void Close() { m_Running = false; }

		void AddLayer(Layer* layer);
		void AddOverlay(Layer* layer);

		inline Window& GetWindow() { return *m_Window; }

		template<typename TEvent>
		inline void DispatchEvent(TEvent e = TEvent()) { static_assert(std::is_base_of<Event, TEvent>::value); OnEvent(e); }

		inline std::shared_ptr<API> GetAPIInstance() { return m_API; }

		inline static Application& Get() { return *s_Instance; }
		inline static std::filesystem::path GetWorkingDirectory() { return std::filesystem::path(s_Instance->m_AppInfo.Args[0]).parent_path(); }

	private:
		void Init(const AppInfo& appInfo);

		bool OnWindowClose(WindowCloseEvent& e);
		bool OnWindowResize(WindowResizeEvent& e);

	private:
		AppInfo m_AppInfo;

		std::unique_ptr<Window> m_Window = nullptr;
		std::shared_ptr<API> m_API = nullptr;

		bool m_Running = true;
		bool m_Minimized = false;

		LayerStack m_LayerStack;

	private:
		static Application* s_Instance;

		BaseImGuiLayer* m_ImGuiLayer = nullptr;
	};


	// Implemented by USER/CLIENT
	Application* CreateApplication(int argc, char* argv[]);

}