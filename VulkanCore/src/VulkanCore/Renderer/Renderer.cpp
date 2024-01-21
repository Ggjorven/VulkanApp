#include "vcpch.h"
#include "Renderer.hpp"

#include "VulkanCore/Core/Logging.hpp"

namespace VkApp
{
	// ===================================
	// ------------ Static ---------------
	// ===================================
	Renderer* Renderer::s_Instance = nullptr;

	void Renderer::Init()
	{
		s_Instance = new Renderer();

		// Create our own rendering specific things.
		s_Instance->CreateCommandPool();
		s_Instance->CreateCommandBuffers();
		s_Instance->CreateSyncObjects();
	}

	void Renderer::Destroy()
	{
		vkDeviceWaitIdle(s_Instance->m_InstanceManager.m_Device);

		s_Instance->m_GraphicsPipelineManager.Destroy();
		s_Instance->m_SwapChainManager.Destroy();

		// Destoy our rendering specific stuff before destroying the instance
		for (size_t i = 0; i < VKAPP_MAX_FRAMES_IN_FLIGHT; i++)
		{
			vkDestroySemaphore(s_Instance->m_InstanceManager.m_Device, s_Instance->m_RenderFinishedSemaphores[i], nullptr);
			vkDestroySemaphore(s_Instance->m_InstanceManager.m_Device, s_Instance->m_ImageAvailableSemaphores[i], nullptr);
			vkDestroyFence(s_Instance->m_InstanceManager.m_Device, s_Instance->m_InFlightFences[i], nullptr);
		}

		vkDestroyCommandPool(s_Instance->m_InstanceManager.m_Device, s_Instance->m_CommandPool, nullptr);

		s_Instance->m_InstanceManager.Destroy(); // Note(Jorben): Destroy InstanceManager last.

		delete s_Instance;
		s_Instance = nullptr;
	}

	void Renderer::AddToQueue(RenderFunction func)
	{
		s_Instance->m_RenderQueue.push_back(func);
	}

	void Renderer::AddToUIQueue(UIFunction func)
	{
		s_Instance->m_UIQueue.push_back(func);
	}

	void Renderer::Display()
	{
		s_Instance->QueuePresent();

		s_Instance->m_RenderQueue.clear();
		s_Instance->m_UIQueue.clear();
	}

	void Renderer::OnResize(uint32_t width, uint32_t height)
	{
		s_Instance->m_SwapChainManager.RecreateSwapChain();
	}

	// ===================================
	// ------------ Public ---------------
	// ===================================

	// ===================================
	// --------- Initialization ----------
	// ===================================
	void Renderer::CreateCommandPool()
	{
		InstanceManager::QueueFamilyIndices queueFamilyIndices = m_InstanceManager.FindQueueFamilies(m_InstanceManager.m_PhysicalDevice);

		VkCommandPoolCreateInfo poolInfo = {};
		poolInfo.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
		poolInfo.flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;
		poolInfo.queueFamilyIndex = queueFamilyIndices.GraphicsFamily.value();

		if (vkCreateCommandPool(m_InstanceManager.m_Device, &poolInfo, nullptr, &m_CommandPool) != VK_SUCCESS)
			VKAPP_LOG_ERROR("Failed to create command pool!");
	}

	void Renderer::CreateCommandBuffers()
	{
		m_CommandBuffers.resize(VKAPP_MAX_FRAMES_IN_FLIGHT);

		VkCommandBufferAllocateInfo allocInfo = {};
		allocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
		allocInfo.commandPool = m_CommandPool;
		allocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
		allocInfo.commandBufferCount = (uint32_t)m_CommandBuffers.size();

		if (vkAllocateCommandBuffers(m_InstanceManager.m_Device, &allocInfo, m_CommandBuffers.data()) != VK_SUCCESS)
			VKAPP_LOG_ERROR("Failed to allocate command buffers!");
	}

	void Renderer::CreateSyncObjects()
	{
		m_ImageAvailableSemaphores.resize(VKAPP_MAX_FRAMES_IN_FLIGHT);
		m_RenderFinishedSemaphores.resize(VKAPP_MAX_FRAMES_IN_FLIGHT);
		m_InFlightFences.resize(VKAPP_MAX_FRAMES_IN_FLIGHT);

		VkSemaphoreCreateInfo semaphoreInfo = {};
		semaphoreInfo.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;

		VkFenceCreateInfo fenceInfo = {};
		fenceInfo.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
		fenceInfo.flags = VK_FENCE_CREATE_SIGNALED_BIT;

		// Create our objects
		for (size_t i = 0; i < VKAPP_MAX_FRAMES_IN_FLIGHT; i++)
		{
			if (vkCreateSemaphore(m_InstanceManager.m_Device, &semaphoreInfo, nullptr, &m_ImageAvailableSemaphores[i]) != VK_SUCCESS ||
				vkCreateSemaphore(m_InstanceManager.m_Device, &semaphoreInfo, nullptr, &m_RenderFinishedSemaphores[i]) != VK_SUCCESS ||
				vkCreateFence(m_InstanceManager.m_Device, &fenceInfo, nullptr, &m_InFlightFences[i]) != VK_SUCCESS)
			{
				VKAPP_LOG_ERROR("Failed to create synchronization objects for a frame!");
			}
		}
	}

	// ===================================
	// ------------- Helper --------------
	// ===================================
	void Renderer::QueuePresent()
	{
		vkWaitForFences(m_InstanceManager.m_Device, 1, &m_InFlightFences[m_CurrentFrame], VK_TRUE, UINT64_MAX);

		uint32_t imageIndex;

		VkResult result = vkAcquireNextImageKHR(m_InstanceManager.m_Device, m_SwapChainManager.m_SwapChain, UINT64_MAX, m_ImageAvailableSemaphores[m_CurrentFrame], VK_NULL_HANDLE, &imageIndex);
		if (result == VK_ERROR_OUT_OF_DATE_KHR)
		{
			m_SwapChainManager.RecreateSwapChain();
			return;
		}
		else if (result != VK_SUCCESS && result != VK_SUBOPTIMAL_KHR)
			VKAPP_LOG_ERROR("Failed to acquire swap chain image!");

		// Only reset the fence if we actually submit the work
		vkResetFences(m_InstanceManager.m_Device, 1, &m_InFlightFences[m_CurrentFrame]);

		vkResetCommandBuffer(m_CommandBuffers[m_CurrentFrame], 0);

		// Note(Jorben): Record the command buffer with all items in the queue
		RecordCommandBuffer(m_CommandBuffers[m_CurrentFrame], imageIndex);

		VkSubmitInfo submitInfo = {};
		submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;

		VkSemaphore waitSemaphores[] = { m_ImageAvailableSemaphores[m_CurrentFrame] };
		VkPipelineStageFlags waitStages[] = { VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT };
		submitInfo.waitSemaphoreCount = 1;
		submitInfo.pWaitSemaphores = waitSemaphores;
		submitInfo.pWaitDstStageMask = waitStages;

		submitInfo.commandBufferCount = 1;
		submitInfo.pCommandBuffers = &m_CommandBuffers[m_CurrentFrame];

		VkSemaphore signalSemaphores[] = { m_RenderFinishedSemaphores[m_CurrentFrame] };
		submitInfo.signalSemaphoreCount = 1;
		submitInfo.pSignalSemaphores = signalSemaphores;

		if (vkQueueSubmit(m_InstanceManager.m_GraphicsQueue, 1, &submitInfo, m_InFlightFences[m_CurrentFrame]) != VK_SUCCESS)
			VKAPP_LOG_ERROR("Failed to submit draw command buffer!");

		VkPresentInfoKHR presentInfo = {};
		presentInfo.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;
		presentInfo.waitSemaphoreCount = 1;
		presentInfo.pWaitSemaphores = signalSemaphores;

		VkSwapchainKHR swapChains[] = { m_SwapChainManager.m_SwapChain };
		presentInfo.swapchainCount = 1;
		presentInfo.pSwapchains = swapChains;
		presentInfo.pImageIndices = &imageIndex;
		presentInfo.pResults = nullptr; // Optional

		// Check for the result on present again
		result = vkQueuePresentKHR(m_InstanceManager.m_PresentQueue, &presentInfo);

		if (result == VK_ERROR_OUT_OF_DATE_KHR || result == VK_SUBOPTIMAL_KHR)
			m_SwapChainManager.RecreateSwapChain();
		else if (result != VK_SUCCESS)
			VKAPP_LOG_ERROR("Failed to present swap chain image!");

		// Note(Jorben): We use the & operator since MAX_FRAMES_IN_FLIGHT is a power of 2 and this is a lot cheaper, if it's not use the % operator
		m_CurrentFrame = (m_CurrentFrame + 1) & VKAPP_MAX_FRAMES_IN_FLIGHT;
	}

	void Renderer::RecordCommandBuffer(VkCommandBuffer& commandBuffer, uint32_t imageIndex)
	{
		VkCommandBufferBeginInfo beginInfo = {};
		beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;

		if (vkBeginCommandBuffer(commandBuffer, &beginInfo) != VK_SUCCESS)
			VKAPP_LOG_ERROR("Failed to begin recording command buffer!");

		VkClearValue clearColor = { {{0.0f, 0.0f, 0.0f, 1.0f}} };

		VkRenderPassBeginInfo renderPassInfo = {};
		renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
		renderPassInfo.renderPass = m_SwapChainManager.m_RenderPass;
		renderPassInfo.framebuffer = m_SwapChainManager.m_SwapChainFramebuffers[imageIndex];
		renderPassInfo.renderArea.offset = { 0, 0 };
		renderPassInfo.renderArea.extent = m_SwapChainManager.m_SwapChainExtent;

		renderPassInfo.clearValueCount = 1;
		renderPassInfo.pClearValues = &clearColor;

		vkCmdBeginRenderPass(commandBuffer, &renderPassInfo, VK_SUBPASS_CONTENTS_INLINE);

		//vkCmdBindPipeline(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, m_GraphicsPipelineManager.m_GraphicsPipeline);

		VkViewport viewport = {};
		viewport.x = 0.0f;
		viewport.y = 0.0f;
		viewport.width = (float)m_SwapChainManager.m_SwapChainExtent.width;
		viewport.height = (float)m_SwapChainManager.m_SwapChainExtent.height;
		viewport.minDepth = 0.0f;
		viewport.maxDepth = 1.0f;
		vkCmdSetViewport(commandBuffer, 0, 1, &viewport);

		VkRect2D scissor = {};
		scissor.offset = { 0, 0 };
		scissor.extent = m_SwapChainManager.m_SwapChainExtent;
		vkCmdSetScissor(commandBuffer, 0, 1, &scissor);

		// Run the queue of commands
		for (auto& func : m_RenderQueue)
			func(commandBuffer, imageIndex);

		for (auto& func : m_UIQueue)
			func(commandBuffer);

		vkCmdEndRenderPass(commandBuffer);

		if (vkEndCommandBuffer(commandBuffer) != VK_SUCCESS)
			VKAPP_LOG_ERROR("Failed to record command buffer!");
	}

}