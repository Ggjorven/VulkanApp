#pragma once

#include <vector>
#include <optional>

#include <vulkan/vulkan.h>

namespace VkApp
{

	class Renderer;
	class SwapChainManager;
	class GraphicsPipeline;
	class GraphicsPipelineManager;

	class BaseImGuiLayer;

	class InstanceManager
	{
	public: // Public functions
		static InstanceManager* Get() { return s_Instance; }

		InstanceManager();
		void Destroy();

		inline VkInstance& GetInstance() { return m_Instance; }
		inline VkPhysicalDevice& GetPhysicalDevice() { return m_PhysicalDevice; }
		inline VkDevice& GetLogicalDevice() { return m_Device; }

		inline VkQueue& GetGraphicsQueue() { return m_GraphicsQueue; }

	private: // Initialization functions
		void CreateInstance();
		void CreateDebugger();
		void CreateSurface();
		void PickPhysicalDevice();
		void CreateDevice();

	private: // Helper functions
		bool ValidationLayersSupported();
		std::vector<const char*> GetRequiredExtensions();
		bool PhysicalDeviceSuitable(const VkPhysicalDevice& device);
		bool ExtensionsSupported(const VkPhysicalDevice& device);

		struct QueueFamilyIndices
		{
		public:
			std::optional<uint32_t> GraphicsFamily;
			std::optional<uint32_t> PresentFamily;

		public:
			bool IsComplete() const
			{
				return GraphicsFamily.has_value() && PresentFamily.has_value();
			}
		};
		QueueFamilyIndices FindQueueFamilies(const VkPhysicalDevice& device);

		struct SwapChainSupportDetails
		{
		public:
			VkSurfaceCapabilitiesKHR Capabilities;
			std::vector<VkSurfaceFormatKHR> Formats;
			std::vector<VkPresentModeKHR> PresentModes;
		};
		SwapChainSupportDetails QuerySwapChainSupport(const VkPhysicalDevice& device);

	private: // Static things
		static InstanceManager* s_Instance;

	private: // Vulkan Data
		VkInstance m_Instance = VK_NULL_HANDLE;
		VkDebugUtilsMessengerEXT m_DebugMessenger = VK_NULL_HANDLE;
		VkSurfaceKHR m_Surface = VK_NULL_HANDLE;

		VkPhysicalDevice m_PhysicalDevice = VK_NULL_HANDLE;
		VkDevice m_Device = VK_NULL_HANDLE;

		VkQueue m_GraphicsQueue = VK_NULL_HANDLE;
		VkQueue m_PresentQueue = VK_NULL_HANDLE;

		friend class Renderer;
		friend class SwapChainManager;
		friend class GraphicsPipeline;
		friend class GraphicsPipelineManager;

		friend class BaseImGuiLayer;
	};
}