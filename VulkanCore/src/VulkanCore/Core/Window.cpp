#include "vcpch.h"
#include "Window.hpp"

#include "VulkanCore/Platforms/Windows/WindowsWindow.hpp"

namespace VkApp
{

	std::unique_ptr<Window> Window::Create(const WindowProperties properties)
	{
		#ifdef VKAPP_PLATFORM_WINDOWS
		return std::make_unique<WindowsWindow>(properties);
		#endif

		// TODO(Jorben): Add all the platforms
	}

}