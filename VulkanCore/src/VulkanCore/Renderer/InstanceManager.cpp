#include "vcpch.h"
#include "InstanceManager.hpp"

#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>

#include "VulkanCore/Core/Logging.hpp"

#include "VulkanCore/Core/Application.hpp"

#ifdef VKAPP_DIST
#define VKAPP_VALIDATION_LAYERS 0
#else
#define VKAPP_VALIDATION_LAYERS 1
#endif

namespace VkApp
{

	// ===================================
	// ------------ External -------------
	// ===================================
	static VkResult CreateDebugUtilsMessengerEXT(VkInstance instance, const VkDebugUtilsMessengerCreateInfoEXT* pCreateInfo,
		const VkAllocationCallbacks* pAllocator, VkDebugUtilsMessengerEXT* pDebugMessenger)
	{
		auto func = (PFN_vkCreateDebugUtilsMessengerEXT)vkGetInstanceProcAddr(instance, "vkCreateDebugUtilsMessengerEXT");

		if (func != nullptr)
			return func(instance, pCreateInfo, pAllocator, pDebugMessenger);

		return VK_ERROR_EXTENSION_NOT_PRESENT;
	}

	static void DestroyDebugUtilsMessengerEXT(VkInstance instance, VkDebugUtilsMessengerEXT debugMessenger, const VkAllocationCallbacks* pAllocator)
	{
		auto func = (PFN_vkDestroyDebugUtilsMessengerEXT)vkGetInstanceProcAddr(instance, "vkDestroyDebugUtilsMessengerEXT");

		if (func != nullptr)
			func(instance, debugMessenger, pAllocator);
	}

	// ===================================
	// ------------ Static ---------------
	// ===================================
	VKAPI_ATTR VkBool32 VKAPI_CALL VulkanDebugCallback(VkDebugUtilsMessageSeverityFlagBitsEXT messageSeverity, VkDebugUtilsMessageTypeFlagsEXT messageType, const VkDebugUtilsMessengerCallbackDataEXT* pCallbackData, void* pUserData)
	{
		if (messageSeverity >= VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT)
		{
			VKAPP_LOG_WARN("Validation layer: {0}", pCallbackData->pMessage);
			return VK_FALSE;
		}

		return VK_FALSE;
	}

	static void SetDebugInfo(VkDebugUtilsMessengerCreateInfoEXT& createInfo)
	{
		createInfo = {};
		createInfo.sType = VK_STRUCTURE_TYPE_DEBUG_UTILS_MESSENGER_CREATE_INFO_EXT;
		createInfo.messageSeverity = VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT;
		createInfo.messageType = VK_DEBUG_UTILS_MESSAGE_TYPE_GENERAL_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT;
		createInfo.pfnUserCallback = VulkanDebugCallback;
	}

	InstanceManager* InstanceManager::s_Instance = nullptr;

	// Requested Validation layers and extensions
	static const std::vector<const char*> s_RequestedValidationLayers = { "VK_LAYER_KHRONOS_validation" };
	static const std::vector<const char*> s_RequestedDeviceExtensions = { VK_KHR_SWAPCHAIN_EXTENSION_NAME };

	// ===================================
	// ------------ Public ---------------
	// ===================================
	InstanceManager::InstanceManager()
	{
		s_Instance = this;

		CreateInstance();
		CreateDebugger();
		CreateSurface();
		PickPhysicalDevice();
		CreateDevice();
	}

	void InstanceManager::Destroy()
	{
		s_Instance = nullptr;

		// Wait for the logical device to finish it's tasks
		vkDeviceWaitIdle(m_Device);

		vkDestroyDevice(m_Device, nullptr);

		#if VKAPP_VALIDATION_LAYERS
		DestroyDebugUtilsMessengerEXT(m_Instance, m_DebugMessenger, nullptr);
		#endif

		vkDestroySurfaceKHR(m_Instance, m_Surface, nullptr);
		vkDestroyInstance(m_Instance, nullptr);
	}

	// ===================================
	// -------- Initialization -----------
	// ===================================
	void InstanceManager::CreateInstance()
	{
		//First run a check for validation layer support
		if (VKAPP_VALIDATION_LAYERS && !ValidationLayersSupported())
			VKAPP_LOG_ERROR("Validation layers requested, but not supported.");

		VkApplicationInfo appInfo = {};
		appInfo.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
		appInfo.pApplicationName = "Vulkan Application";
		appInfo.applicationVersion = VK_MAKE_VERSION(1, 0, 0);
		appInfo.pEngineName = "No Engine";
		appInfo.engineVersion = VK_MAKE_VERSION(1, 0, 0);
		appInfo.apiVersion = VK_API_VERSION_1_0;

		auto extensions = GetRequiredExtensions();

		VkInstanceCreateInfo createInfo = {};
		createInfo.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
		createInfo.pApplicationInfo = &appInfo;

		//createInfo.flags |= VK_INSTANCE_CREATE_ENUMERATE_PORTABILITY_BIT_KHR;			// Note(Jorben): This line is needed for MacOS

		createInfo.enabledExtensionCount = static_cast<uint32_t>(extensions.size());
		createInfo.ppEnabledExtensionNames = extensions.data();

		#if VKAPP_VALIDATION_LAYERS
		createInfo.enabledLayerCount = static_cast<uint32_t>(s_RequestedValidationLayers.size());
		createInfo.ppEnabledLayerNames = s_RequestedValidationLayers.data();
		#else
		createInfo.enabledLayerCount = 0;
		#endif

		// Note(Jorben): Setup the debug messenger also for the create instance 
		VkDebugUtilsMessengerCreateInfoEXT debugCreateInfo = {};
		#if VKAPP_VALIDATION_LAYERS
		createInfo.enabledLayerCount = static_cast<uint32_t>(s_RequestedValidationLayers.size());
		createInfo.ppEnabledLayerNames = s_RequestedValidationLayers.data();

		SetDebugInfo(debugCreateInfo);
		createInfo.pNext = (VkDebugUtilsMessengerCreateInfoEXT*)&debugCreateInfo;
		#else
		createInfo.enabledLayerCount = 0;
		createInfo.pNext = nullptr;
		#endif

		// Create our vulkan instance
		if (vkCreateInstance(&createInfo, nullptr, &m_Instance) != VK_SUCCESS)
			VKAPP_LOG_FATAL("Failed to create Vulkan Instance.");
	}

	void InstanceManager::CreateDebugger()
	{
		#if !(VKAPP_VALIDATION_LAYERS)
		return;
		#else
		VkDebugUtilsMessengerCreateInfoEXT createInfo = {};
		SetDebugInfo(createInfo);

		if (CreateDebugUtilsMessengerEXT(m_Instance, &createInfo, nullptr, &m_DebugMessenger) != VK_SUCCESS)
			VKAPP_LOG_ERROR("Failed to set up debug messenger!");
		#endif
	}

	void InstanceManager::CreateSurface()
	{
		GLFWwindow* handle = reinterpret_cast<GLFWwindow*>(Application::Get().GetWindow().GetNativeWindow());

		if (glfwCreateWindowSurface(m_Instance, handle, nullptr, &m_Surface) != VK_SUCCESS)
			VKAPP_LOG_ERROR("Failed to create window surface!");
	}

	void InstanceManager::PickPhysicalDevice()
	{
		uint32_t deviceCount = 0;
		vkEnumeratePhysicalDevices(m_Instance, &deviceCount, nullptr);

		if (deviceCount == 0)
			VKAPP_LOG_ERROR("Failed to find GPUs with Vulkan support!");

		std::vector<VkPhysicalDevice> devices(deviceCount);
		vkEnumeratePhysicalDevices(m_Instance, &deviceCount, devices.data());

		for (const auto& device : devices)
		{
			if (PhysicalDeviceSuitable(device))
			{
				m_PhysicalDevice = device;
				break;
			}
		}

		// Note(Jorben): Check if no device was selected
		if (m_PhysicalDevice == VK_NULL_HANDLE)
			VKAPP_LOG_ERROR("Failed to find a suitable GPU!");
	}

	void InstanceManager::CreateDevice()
	{
		QueueFamilyIndices indices = FindQueueFamilies(m_PhysicalDevice);

		std::vector<VkDeviceQueueCreateInfo> queueCreateInfos;
		std::set<uint32_t> uniqueQueueFamilies = { indices.GraphicsFamily.value(), indices.PresentFamily.value() };

		float queuePriority = 1.0f;
		for (uint32_t queueFamily : uniqueQueueFamilies)
		{
			VkDeviceQueueCreateInfo queueCreateInfo = {};
			queueCreateInfo.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
			queueCreateInfo.queueFamilyIndex = queueFamily;
			queueCreateInfo.queueCount = 1;
			queueCreateInfo.pQueuePriorities = &queuePriority;
			queueCreateInfos.push_back(queueCreateInfo);
		}

		VkPhysicalDeviceFeatures deviceFeatures = {}; // TODO/NOTE(Jorben): More to add when start doing interesting things with Vulkan

		VkDeviceCreateInfo createInfo = {};
		createInfo.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
		createInfo.queueCreateInfoCount = static_cast<uint32_t>(queueCreateInfos.size());
		createInfo.pQueueCreateInfos = queueCreateInfos.data();
		createInfo.pEnabledFeatures = &deviceFeatures;
		createInfo.enabledExtensionCount = static_cast<uint32_t>(s_RequestedDeviceExtensions.size());
		createInfo.ppEnabledExtensionNames = s_RequestedDeviceExtensions.data();

		#if VKAPP_VALIDATION_LAYERS
		createInfo.enabledLayerCount = static_cast<uint32_t>(s_RequestedValidationLayers.size());
		createInfo.ppEnabledLayerNames = s_RequestedValidationLayers.data();
		#else
		createInfo.enabledLayerCount = 0;
		#endif

		if (vkCreateDevice(m_PhysicalDevice, &createInfo, nullptr, &m_Device) != VK_SUCCESS)
			VKAPP_LOG_ERROR("Failed to create logical device!");

		// Retrieve the graphics & present queue handle
		vkGetDeviceQueue(m_Device, indices.GraphicsFamily.value(), 0, &m_GraphicsQueue);
		vkGetDeviceQueue(m_Device, indices.PresentFamily.value(), 0, &m_PresentQueue);
	}

	// ===================================
	// ------------ Helper ---------------
	// ===================================
	bool InstanceManager::ValidationLayersSupported()
	{
		uint32_t layerCount;
		vkEnumerateInstanceLayerProperties(&layerCount, nullptr);

		std::vector<VkLayerProperties> availableLayers(layerCount);
		vkEnumerateInstanceLayerProperties(&layerCount, availableLayers.data());

		// Check if all requested layers are actually accessible 
		for (const char* layerName : s_RequestedValidationLayers)
		{
			bool layerFound = false;

			for (const auto& layerProperties : availableLayers)
			{
				if (strcmp(layerName, layerProperties.layerName) == 0)
				{
					layerFound = true;
					break;
				}
			}

			if (!layerFound)
				return false;
		}

		return true;
	}

	std::vector<const char*> InstanceManager::GetRequiredExtensions()
	{
		uint32_t glfwExtensionCount = 0;
		const char** glfwExtensions;
		glfwExtensions = glfwGetRequiredInstanceExtensions(&glfwExtensionCount);

		std::vector<const char*> extensions(glfwExtensions, glfwExtensions + glfwExtensionCount);

		#if VKAPP_VALIDATION_LAYERS
		extensions.push_back(VK_EXT_DEBUG_UTILS_EXTENSION_NAME);
		#endif

		//extensions.push_back(VK_KHR_PORTABILITY_ENUMERATION_EXTENSION_NAME); // Note(Jorben): This line is needed for MacOS

		return extensions;
	}

	bool InstanceManager::PhysicalDeviceSuitable(const VkPhysicalDevice& device)
	{
		QueueFamilyIndices indices = FindQueueFamilies(device);

		bool extensionsSupported = ExtensionsSupported(device);
		bool swapChainAdequate = false;

		if (extensionsSupported)
		{
			SwapChainSupportDetails swapChainSupport = QuerySwapChainSupport(device);
			swapChainAdequate = !swapChainSupport.Formats.empty() && !swapChainSupport.PresentModes.empty();
		}

		return indices.IsComplete() && extensionsSupported && swapChainAdequate;
	}

	bool InstanceManager::ExtensionsSupported(const VkPhysicalDevice& device)
	{
		uint32_t extensionCount;
		vkEnumerateDeviceExtensionProperties(device, nullptr, &extensionCount, nullptr);

		std::vector<VkExtensionProperties> availableExtensions(extensionCount);
		vkEnumerateDeviceExtensionProperties(device, nullptr, &extensionCount, availableExtensions.data());

		std::set<std::string> requiredExtensions(s_RequestedDeviceExtensions.begin(), s_RequestedDeviceExtensions.end());

		for (const auto& extension : availableExtensions)
			requiredExtensions.erase(extension.extensionName);

		// Note(Jorben): It's empty if all the required extensions are available
		return requiredExtensions.empty();
	}

	InstanceManager::QueueFamilyIndices InstanceManager::FindQueueFamilies(const VkPhysicalDevice& device)
	{
		QueueFamilyIndices indices;

		uint32_t queueFamilyCount = 0;
		vkGetPhysicalDeviceQueueFamilyProperties(device, &queueFamilyCount, nullptr);

		std::vector<VkQueueFamilyProperties> queueFamilies(queueFamilyCount);
		vkGetPhysicalDeviceQueueFamilyProperties(device, &queueFamilyCount, queueFamilies.data());

		int32_t i = 0;
		for (const auto& queueFamily : queueFamilies)
		{
			// Early exit check
			if (indices.IsComplete())
				break;

			if (queueFamily.queueFlags & VK_QUEUE_GRAPHICS_BIT)
				indices.GraphicsFamily = i;

			VkBool32 presentSupport = false;
			vkGetPhysicalDeviceSurfaceSupportKHR(device, i, m_Surface, &presentSupport);
			if (presentSupport)
				indices.PresentFamily = i;

			i++;
		}

		return indices;
	}

	InstanceManager::SwapChainSupportDetails InstanceManager::QuerySwapChainSupport(const VkPhysicalDevice& device)
	{
		SwapChainSupportDetails details;

		// Capabilities
		vkGetPhysicalDeviceSurfaceCapabilitiesKHR(device, m_Surface, &details.Capabilities);

		// Formats
		uint32_t formatCount;
		vkGetPhysicalDeviceSurfaceFormatsKHR(device, m_Surface, &formatCount, nullptr);

		if (formatCount != 0)
		{
			details.Formats.resize((size_t)formatCount);
			vkGetPhysicalDeviceSurfaceFormatsKHR(device, m_Surface, &formatCount, details.Formats.data());
		}

		// Presentation modes
		uint32_t presentModeCount;
		vkGetPhysicalDeviceSurfacePresentModesKHR(device, m_Surface, &presentModeCount, nullptr);

		if (presentModeCount != 0)
		{
			details.PresentModes.resize((size_t)presentModeCount);
			vkGetPhysicalDeviceSurfacePresentModesKHR(device, m_Surface, &presentModeCount, details.PresentModes.data());
		}

		return details;
	}

}