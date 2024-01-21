#pragma once

#include <vector>

#include <vulkan/vulkan.h>

namespace VkApp
{

	class Renderer;
	class IntanceManager;

	class SwapChainManager
	{
	public: // Public functions
		static SwapChainManager* Get() { return s_Instance; }

		SwapChainManager();
		void Destroy();

		void InitCommandPoolRequiredFunctions();
		void RecreateSwapChain();

		inline VkRenderPass& GetRenderPass() { return m_RenderPass; }
		inline VkExtent2D& GetExtent() { return m_SwapChainExtent;  }

		inline std::vector<VkImage>& GetImages() { return m_SwapChainImages; }
		inline std::vector<VkImageView>& GetImageViews() { return m_SwapChainImageViews; }

	private: // Initialization functions
		void CreateSwapChain();
		void CreateImageViews();
		void CreateRenderPass();

		void CreateDepthResources();
		void CreateFramebuffers();

	private: // Helper functions
		VkSurfaceFormatKHR ChooseSwapSurfaceFormat(const std::vector<VkSurfaceFormatKHR>& availableFormats);
		VkPresentModeKHR ChooseSwapPresentMode(const std::vector<VkPresentModeKHR>& availablePresentModes);
		VkExtent2D ChooseSwapExtent(const VkSurfaceCapabilitiesKHR& capabilities);

		VkFormat FindDepthFormat();
		VkFormat FindSupportedFormat(const std::vector<VkFormat>& candidates, VkImageTiling tiling, VkFormatFeatureFlags features);

		void CleanUpSwapChain();

	private: // Static things
		static SwapChainManager* s_Instance;

	private: // Vulkan Data
		VkSwapchainKHR m_SwapChain = VK_NULL_HANDLE;
		VkFormat m_SwapChainImageFormat = VkFormat::VK_FORMAT_UNDEFINED;
		VkExtent2D m_SwapChainExtent = { };

		std::vector<VkImage> m_SwapChainImages = { };
		std::vector<VkImageView> m_SwapChainImageViews = { };
		std::vector<VkFramebuffer> m_SwapChainFramebuffers = { };

		VkImage m_DepthImage = VK_NULL_HANDLE;
		VkDeviceMemory m_DepthImageMemory = VK_NULL_HANDLE;
		VkImageView m_DepthImageView = VK_NULL_HANDLE;

		VkRenderPass m_RenderPass = VK_NULL_HANDLE;

		friend class InstanceManager;
		friend class Renderer;
	};

}