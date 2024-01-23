#pragma once

#include <string>
#include <functional>
#include <memory>

#include "VulkanCore/Core/Events.hpp"

namespace VkApp
{

	using EventCallBackFunction = std::function<void(Event&)>;

	struct WindowProperties
	{
		std::string Name;
		uint32_t Width;
		uint32_t Height;

		bool Titlebar = true;
		bool VSync = true;

		bool CustomPos = false;
		uint32_t X = 0u;
		uint32_t Y = 0u;

		WindowProperties(std::string name = "VulkanApp Window", uint32_t width = 1280u, uint32_t height = 720u)
			: Name(name), Width(width), Height(height)
		{
		}
	};

	struct WindowData
	{
		std::string Name;
		uint32_t Width;
		uint32_t Height;

		bool Vsync = false;
		EventCallBackFunction CallBack;

		WindowData(std::string name = "VulkanApp Window", uint32_t width = 1280, uint32_t height = 720)
			: Name(name), Width(width), Height(height)
		{
		}

		WindowData operator=(WindowProperties const& properties)
		{
			WindowData newData;
			newData.Name = properties.Name;
			newData.Width = properties.Width;
			newData.Height = properties.Height;
			newData.Vsync = properties.VSync;
			return newData;
		}
	};

	class Window
	{
	public:
		virtual ~Window() = default;

		virtual void SetEventCallBack(EventCallBackFunction func) = 0;

		virtual void OnUpdate() = 0;
		virtual void OnRender() = 0;

		// Actual window size
		virtual uint32_t GetWidth() const = 0;
		virtual uint32_t GetHeight() const = 0;

		// Extra
		virtual void SetVSync(bool enabled) = 0;
		virtual bool IsVSync() const = 0;

		virtual void SetTitle(const std::string& title) = 0;

		virtual void* GetNativeWindow() const = 0;

		static std::unique_ptr<Window> Create(const WindowProperties properties = WindowProperties());
	};

}