#pragma once

#include "VulkanCore/Core/Window.hpp"

#include <GLFW/glfw3.h>

namespace VkApp
{

	class WindowsWindow : public Window
	{
	public:
		WindowsWindow(const WindowProperties properties);
		virtual ~WindowsWindow();

		void SetEventCallBack(EventCallBackFunction func) override { m_Data.CallBack = func; }

		void OnUpdate() override;
		void OnRender() override;

		uint32_t GetWidth() const override { return m_Data.Width; }
		uint32_t GetHeight() const override { return m_Data.Height; }

		void SetVSync(bool enabled) override;
		bool IsVSync() const override { return m_Data.Vsync; }

		void* GetNativeWindow() const override { return (void*)m_Window; }

	private:
		bool Init(WindowProperties properties);
		void Shutdown();

		static void ErrorCallBack(int errorCode, const char* description);

	private:
		static bool s_GLFWinitialized;
		static uint32_t s_Instances;

		GLFWwindow* m_Window;
		WindowData m_Data;

	};
}