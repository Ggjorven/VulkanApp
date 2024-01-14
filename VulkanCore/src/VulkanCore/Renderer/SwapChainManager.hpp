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

		void RecreateSwapChain();

		VkRenderPass& GetRenderPass() { return m_RenderPass; }

	private: // Initialization functions
		void CreateSwapChain();
		void CreateImageViews();
		void CreateRenderPass();
		void CreateFramebuffers();

	private: // Helper functions
		VkSurfaceFormatKHR ChooseSwapSurfaceFormat(const std::vector<VkSurfaceFormatKHR>& availableFormats);
		VkPresentModeKHR ChooseSwapPresentMode(const std::vector<VkPresentModeKHR>& availablePresentModes);
		VkExtent2D ChooseSwapExtent(const VkSurfaceCapabilitiesKHR& capabilities);

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

		VkRenderPass m_RenderPass = VK_NULL_HANDLE;

		friend class InstanceManager;
		friend class Renderer;
	};

}