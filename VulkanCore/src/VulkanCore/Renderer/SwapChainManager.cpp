#include "vcpch.h"
#include "SwapChainManager.hpp"

#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>

#include "VulkanCore/Core/Logging.hpp"

#include "VulkanCore/Core/Application.hpp"

#include "VulkanCore/Renderer/InstanceManager.hpp" // For the retrieval of the logical device

namespace VkApp
{

	// ===================================
	// ------------ Static ---------------
	// ===================================
	SwapChainManager* SwapChainManager::s_Instance = nullptr;

	static InstanceManager* s_InstanceManager = nullptr;

	// ===================================
	// ------------ Public ---------------
	// ===================================
	SwapChainManager::SwapChainManager()
	{
		// Retrieve the InstanceManager, so we can use it in our code
		s_Instance = this;
		s_InstanceManager = InstanceManager::Get();

		CreateSwapChain();
		CreateImageViews();
		CreateRenderPass();
		CreateFramebuffers();
	}

	void SwapChainManager::Destroy()
	{
		CleanUpSwapChain();
		
		vkDestroyRenderPass(s_InstanceManager->m_Device, m_RenderPass, nullptr);

		s_InstanceManager = nullptr;
		s_Instance = nullptr;
	}

	void SwapChainManager::RecreateSwapChain()
	{
		auto handle = reinterpret_cast<GLFWwindow*>(Application::Get().GetWindow().GetNativeWindow());

		// Pause if minimized
		int width = 0, height = 0;
		glfwGetFramebufferSize(handle, &width, &height);
		while (width == 0 || height == 0) // This loop only executes if it's minimized
		{
			glfwGetFramebufferSize(handle, &width, &height);
			glfwWaitEvents();
		}

		vkDeviceWaitIdle(s_InstanceManager->m_Device);

		CleanUpSwapChain();

		CreateSwapChain();
		CreateImageViews();
		CreateFramebuffers();
	}

	// ===================================
	// -------- Initialization -----------
	// ===================================
	void SwapChainManager::CreateSwapChain()
	{
		InstanceManager::SwapChainSupportDetails swapChainSupport = s_InstanceManager->QuerySwapChainSupport(s_InstanceManager->m_PhysicalDevice);

		VkSurfaceFormatKHR surfaceFormat = ChooseSwapSurfaceFormat(swapChainSupport.Formats);
		VkPresentModeKHR presentMode = ChooseSwapPresentMode(swapChainSupport.PresentModes);
		VkExtent2D extent = ChooseSwapExtent(swapChainSupport.Capabilities);

		// Note(Jorben): +1 because sticking to the minimum can cause us to wait on the driver sometimes
		uint32_t imageCount = swapChainSupport.Capabilities.minImageCount + 1;

		// Making sure we don't exceed the maximum
		if (swapChainSupport.Capabilities.maxImageCount > 0 && imageCount > swapChainSupport.Capabilities.maxImageCount)
			imageCount = swapChainSupport.Capabilities.maxImageCount;

		VkSwapchainCreateInfoKHR createInfo = {};
		createInfo.sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR;
		createInfo.surface = s_InstanceManager->m_Surface;
		createInfo.minImageCount = imageCount;
		createInfo.imageFormat = surfaceFormat.format;
		createInfo.imageColorSpace = surfaceFormat.colorSpace;
		createInfo.imageExtent = extent;
		createInfo.imageArrayLayers = 1;
		createInfo.imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;

		InstanceManager::QueueFamilyIndices indices = s_InstanceManager->FindQueueFamilies(s_InstanceManager->m_PhysicalDevice);
		uint32_t queueFamilyIndices[] = { indices.GraphicsFamily.value(), indices.PresentFamily.value() };

		if (indices.GraphicsFamily != indices.PresentFamily)
		{
			createInfo.imageSharingMode = VK_SHARING_MODE_CONCURRENT;
			createInfo.queueFamilyIndexCount = 2;
			createInfo.pQueueFamilyIndices = queueFamilyIndices;
		}
		else
		{
			createInfo.imageSharingMode = VK_SHARING_MODE_EXCLUSIVE;
			createInfo.queueFamilyIndexCount = 0; // Optional
			createInfo.pQueueFamilyIndices = nullptr; // Optional
		}

		createInfo.preTransform = swapChainSupport.Capabilities.currentTransform;
		createInfo.compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR;
		createInfo.presentMode = presentMode;
		createInfo.clipped = VK_TRUE;

		createInfo.oldSwapchain = VK_NULL_HANDLE;

		// Creation of the swapchain
		if (vkCreateSwapchainKHR(s_InstanceManager->m_Device, &createInfo, nullptr, &m_SwapChain) != VK_SUCCESS)
			VKAPP_LOG_ERROR("Failed to create swap chain!");

		// Note(Jorben): We query the amount of images again, because vulkan is allowed to create a swapchain with more images.
		vkGetSwapchainImagesKHR(s_InstanceManager->m_Device, m_SwapChain, &imageCount, nullptr);
		m_SwapChainImages.resize(imageCount);
		vkGetSwapchainImagesKHR(s_InstanceManager->m_Device, m_SwapChain, &imageCount, m_SwapChainImages.data());

		// Store the format and extent for the future
		m_SwapChainImageFormat = surfaceFormat.format;
		m_SwapChainExtent = extent;
	}

	void SwapChainManager::CreateImageViews()
	{
		m_SwapChainImageViews.resize(m_SwapChainImages.size());

		for (size_t i = 0; i < m_SwapChainImages.size(); i++)
		{
			VkImageViewCreateInfo createInfo = {};
			createInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
			createInfo.image = m_SwapChainImages[i];
			createInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;
			createInfo.format = m_SwapChainImageFormat;
			createInfo.components.r = VK_COMPONENT_SWIZZLE_IDENTITY;
			createInfo.components.g = VK_COMPONENT_SWIZZLE_IDENTITY;
			createInfo.components.b = VK_COMPONENT_SWIZZLE_IDENTITY;
			createInfo.components.a = VK_COMPONENT_SWIZZLE_IDENTITY;
			createInfo.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
			createInfo.subresourceRange.baseMipLevel = 0;
			createInfo.subresourceRange.levelCount = 1;
			createInfo.subresourceRange.baseArrayLayer = 0;
			createInfo.subresourceRange.layerCount = 1;

			if (vkCreateImageView(s_InstanceManager->m_Device, &createInfo, nullptr, &m_SwapChainImageViews[i]) != VK_SUCCESS)
				VKAPP_LOG_ERROR("Failed to create image views!");
		}
	}

	void SwapChainManager::CreateRenderPass()
	{
		VkAttachmentDescription colorAttachment = {};
		colorAttachment.format = m_SwapChainImageFormat;
		colorAttachment.samples = VK_SAMPLE_COUNT_1_BIT;
		colorAttachment.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
		colorAttachment.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
		colorAttachment.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
		colorAttachment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
		colorAttachment.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
		colorAttachment.finalLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;

		VkAttachmentReference colorAttachmentRef = {};
		colorAttachmentRef.attachment = 0;
		colorAttachmentRef.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

		VkSubpassDescription subpass = {};
		subpass.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
		subpass.colorAttachmentCount = 1;
		subpass.pColorAttachments = &colorAttachmentRef;

		VkSubpassDependency dependency = {};
		dependency.srcSubpass = VK_SUBPASS_EXTERNAL;
		dependency.dstSubpass = 0;
		dependency.srcStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
		dependency.srcAccessMask = 0;
		dependency.dstStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
		dependency.dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;

		VkRenderPassCreateInfo renderPassInfo = {};
		renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
		renderPassInfo.attachmentCount = 1;
		renderPassInfo.pAttachments = &colorAttachment;
		renderPassInfo.subpassCount = 1;
		renderPassInfo.pSubpasses = &subpass;
		renderPassInfo.dependencyCount = 1;
		renderPassInfo.pDependencies = &dependency;

		if (vkCreateRenderPass(s_InstanceManager->m_Device, &renderPassInfo, nullptr, &m_RenderPass) != VK_SUCCESS)
			VKAPP_LOG_ERROR("Failed to create render pass!");
	}

	void SwapChainManager::CreateFramebuffers()
	{
		m_SwapChainFramebuffers.resize(m_SwapChainImageViews.size());

		for (size_t i = 0; i < m_SwapChainImageViews.size(); i++)
		{
			VkImageView attachments[] = { m_SwapChainImageViews[i] };

			VkFramebufferCreateInfo framebufferInfo = {};
			framebufferInfo.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
			framebufferInfo.renderPass = m_RenderPass;
			framebufferInfo.attachmentCount = 1;
			framebufferInfo.pAttachments = attachments;
			framebufferInfo.width = m_SwapChainExtent.width;
			framebufferInfo.height = m_SwapChainExtent.height;
			framebufferInfo.layers = 1;

			if (vkCreateFramebuffer(s_InstanceManager->m_Device, &framebufferInfo, nullptr, &m_SwapChainFramebuffers[i]) != VK_SUCCESS)
				throw std::runtime_error("Failed to create framebuffer!");
		}
	}

	// ===================================
	// ------------ Helper ---------------
	// ===================================
	VkSurfaceFormatKHR SwapChainManager::ChooseSwapSurfaceFormat(const std::vector<VkSurfaceFormatKHR>& availableFormats)
	{
		for (const auto& availableFormat : availableFormats)
		{
			if (availableFormat.format == VK_FORMAT_B8G8R8A8_SRGB && availableFormat.colorSpace == VK_COLOR_SPACE_SRGB_NONLINEAR_KHR)
				return availableFormat;
		}

		// Note(Jorben): If nothing 100% satisfies our needs it okay to just go with the first one.
		return availableFormats[0];
	}

	VkPresentModeKHR SwapChainManager::ChooseSwapPresentMode(const std::vector<VkPresentModeKHR>& availablePresentModes)
	{
		for (const auto& availablePresentMode : availablePresentModes)
		{
			if (availablePresentMode == VK_PRESENT_MODE_MAILBOX_KHR)
				return availablePresentMode;
		}

		return VK_PRESENT_MODE_FIFO_KHR;
	}

	VkExtent2D SwapChainManager::ChooseSwapExtent(const VkSurfaceCapabilitiesKHR& capabilities)
	{
		if (capabilities.currentExtent.width != std::numeric_limits<uint32_t>::max())
			return capabilities.currentExtent;
		else
		{
			auto handle = reinterpret_cast<GLFWwindow*>(Application::Get().GetWindow().GetNativeWindow());

			int width, height;
			glfwGetFramebufferSize(handle, &width, &height);

			VkExtent2D actualExtent =
			{
				static_cast<uint32_t>(width),
				static_cast<uint32_t>(height)
			};

			actualExtent.width = std::clamp(actualExtent.width, capabilities.minImageExtent.width, capabilities.maxImageExtent.width);
			actualExtent.height = std::clamp(actualExtent.height, capabilities.minImageExtent.height, capabilities.maxImageExtent.height);

			return actualExtent;
		}
	}

	void SwapChainManager::CleanUpSwapChain()
	{
		for (size_t i = 0; i < m_SwapChainFramebuffers.size(); i++)
			vkDestroyFramebuffer(s_InstanceManager->m_Device, m_SwapChainFramebuffers[i], nullptr);

		for (size_t i = 0; i < m_SwapChainImageViews.size(); i++)
			vkDestroyImageView(s_InstanceManager->m_Device, m_SwapChainImageViews[i], nullptr);

		vkDestroySwapchainKHR(s_InstanceManager->m_Device, m_SwapChain, nullptr);
	}

}