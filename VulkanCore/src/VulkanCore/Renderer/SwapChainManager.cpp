#include "vcpch.h"
#include "SwapChainManager.hpp"

#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>

#include "VulkanCore/Core/Logging.hpp"

#include "VulkanCore/Core/Application.hpp"

#include "VulkanCore/Renderer/InstanceManager.hpp" // For the retrieval of the logical device
#include "VulkanCore/Utils/BufferManager.hpp" 

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
		//CreateDepthResources();
		//CreateFramebuffers();
	}

	void SwapChainManager::Destroy()
	{
		CleanUpSwapChain();
		
		vkDestroyRenderPass(s_InstanceManager->m_Device, m_RenderPass, nullptr);

		s_InstanceManager = nullptr;
		s_Instance = nullptr;
	}

	void SwapChainManager::InitCommandPoolRequiredFunctions()
	{
		CreateDepthResources();
		CreateFramebuffers();
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
		CreateDepthResources();
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
			m_SwapChainImageViews[i] = BufferManager::CreateImageView(m_SwapChainImages[i], m_SwapChainImageFormat, VK_IMAGE_ASPECT_COLOR_BIT);
		}
	}

	void SwapChainManager::CreateRenderPass()
	{
		// Colour
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

		// Depth
		VkAttachmentDescription depthAttachment = {};
		depthAttachment.format = FindDepthFormat();
		depthAttachment.samples = VK_SAMPLE_COUNT_1_BIT;
		depthAttachment.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
		depthAttachment.storeOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
		depthAttachment.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
		depthAttachment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
		depthAttachment.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
		depthAttachment.finalLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;

		VkAttachmentReference depthAttachmentRef = {};
		depthAttachmentRef.attachment = 1;
		depthAttachmentRef.layout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;

		// Subpass
		VkSubpassDescription subpass = {};
		subpass.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
		subpass.colorAttachmentCount = 1;
		subpass.pColorAttachments = &colorAttachmentRef;
		subpass.pDepthStencilAttachment = &depthAttachmentRef;

		VkSubpassDependency dependency = {};
		dependency.srcSubpass = VK_SUBPASS_EXTERNAL;
		dependency.dstSubpass = 0;
		dependency.srcStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT | VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT;
		dependency.srcAccessMask = 0;
		dependency.dstStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT | VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT;
		dependency.dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT | VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT;

		std::array<VkAttachmentDescription, 2> attachments = { colorAttachment, depthAttachment };

		VkRenderPassCreateInfo renderPassInfo = {};
		renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
		renderPassInfo.attachmentCount = static_cast<uint32_t>(attachments.size());
		renderPassInfo.pAttachments = attachments.data();
		renderPassInfo.subpassCount = 1;
		renderPassInfo.pSubpasses = &subpass;
		renderPassInfo.dependencyCount = 1;
		renderPassInfo.pDependencies = &dependency;

		if (vkCreateRenderPass(s_InstanceManager->m_Device, &renderPassInfo, nullptr, &m_RenderPass) != VK_SUCCESS)
			VKAPP_LOG_ERROR("Failed to create render pass!");
	}

	void SwapChainManager::CreateDepthResources()
	{
		VkFormat depthFormat = FindDepthFormat();

		BufferManager::CreateImage(m_SwapChainExtent.width, m_SwapChainExtent.height, depthFormat, VK_IMAGE_TILING_OPTIMAL, VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, m_DepthImage, m_DepthImageMemory);

		m_DepthImageView = BufferManager::CreateImageView(m_DepthImage, depthFormat, VK_IMAGE_ASPECT_DEPTH_BIT);

		BufferManager::TransitionImageToLayout(m_DepthImage, depthFormat, VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL);
	}

	void SwapChainManager::CreateFramebuffers()
	{
		m_SwapChainFramebuffers.resize(m_SwapChainImageViews.size());

		for (size_t i = 0; i < m_SwapChainImageViews.size(); i++)
		{
			std::array<VkImageView, 2> attachments = { m_SwapChainImageViews[i], m_DepthImageView };

			VkFramebufferCreateInfo framebufferInfo = {};
			framebufferInfo.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
			framebufferInfo.renderPass = m_RenderPass;
			framebufferInfo.attachmentCount = static_cast<uint32_t>(attachments.size());
			framebufferInfo.pAttachments = attachments.data();
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
			//if (availableFormat.format == VK_FORMAT_B8G8R8A8_SRGB && availableFormat.colorSpace == VK_COLOR_SPACE_SRGB_NONLINEAR_KHR)
			//	return availableFormat;

			if (availableFormat.format == VK_FORMAT_B8G8R8A8_UNORM && availableFormat.colorSpace == VK_COLOR_SPACE_SRGB_NONLINEAR_KHR)
				return availableFormat;
		}

		// Note(Jorben): If nothing 100% satisfies our needs it okay to just go with the first one.
		return availableFormats[0];
	}

	VkPresentModeKHR SwapChainManager::ChooseSwapPresentMode(const std::vector<VkPresentModeKHR>& availablePresentModes)
	{
		for (const auto& availablePresentMode : availablePresentModes)
		{
			if (availablePresentMode == VK_PRESENT_MODE_IMMEDIATE_KHR) // Note(Jorben): Maybe change this back to triple buffering: VK_PRESENT_MODE_MAILBOX_KHR
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

	VkFormat SwapChainManager::FindDepthFormat()
	{
		return FindSupportedFormat({ VK_FORMAT_D32_SFLOAT, VK_FORMAT_D32_SFLOAT_S8_UINT, VK_FORMAT_D24_UNORM_S8_UINT }, VK_IMAGE_TILING_OPTIMAL, VK_FORMAT_FEATURE_DEPTH_STENCIL_ATTACHMENT_BIT);
	}

	VkFormat SwapChainManager::FindSupportedFormat(const std::vector<VkFormat>& candidates, VkImageTiling tiling, VkFormatFeatureFlags features)
	{
		for (VkFormat format : candidates) 
		{
			VkFormatProperties props;
			vkGetPhysicalDeviceFormatProperties(s_InstanceManager->m_PhysicalDevice, format, &props);

			if (tiling == VK_IMAGE_TILING_LINEAR && (props.linearTilingFeatures & features) == features)
				return format;
			else if (tiling == VK_IMAGE_TILING_OPTIMAL && (props.optimalTilingFeatures & features) == features)
				return format;
		}

		VKAPP_LOG_ERROR("Failed to find supported format!");
		return VK_FORMAT_UNDEFINED;
	}

	void SwapChainManager::CleanUpSwapChain()
	{
		vkDestroyImageView(s_InstanceManager->m_Device, m_DepthImageView, nullptr);
		vkDestroyImage(s_InstanceManager->m_Device, m_DepthImage, nullptr);
		vkFreeMemory(s_InstanceManager->m_Device, m_DepthImageMemory, nullptr);

		for (size_t i = 0; i < m_SwapChainFramebuffers.size(); i++)
			vkDestroyFramebuffer(s_InstanceManager->m_Device, m_SwapChainFramebuffers[i], nullptr);

		for (size_t i = 0; i < m_SwapChainImageViews.size(); i++)
			vkDestroyImageView(s_InstanceManager->m_Device, m_SwapChainImageViews[i], nullptr);

		vkDestroySwapchainKHR(s_InstanceManager->m_Device, m_SwapChain, nullptr);
	}

}