#pragma once

#include <functional>

#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>
#include <vulkan/vulkan.h>

#include "VulkanCore/Renderer/InstanceManager.hpp"
#include "VulkanCore/Renderer/SwapChainManager.hpp"
#include "VulkanCore/Renderer/GraphicsPipelineManager.hpp"

namespace VkApp
{

	#define VKAPP_MAX_FRAMES_IN_FLIGHT 2
	typedef std::function<void(VkCommandBuffer&, uint32_t)> RenderFunction;

	class Renderer
	{
	public:
		static Renderer* Get() { return s_Instance; }

		static void Init();
		static void Destroy();

		static void AddToQueue(RenderFunction func);
		static void Display();

		static void OnResize(uint32_t width, uint32_t height);

	public:
		inline VkCommandPool& GetCommandPool() { return m_CommandPool; }
		inline uint32_t GetCurrentImage() const { return m_CurrentFrame; }

	private:
		static Renderer* s_Instance;

	private:
		void CreateCommandPool();
		void CreateCommandBuffers();
		void CreateSyncObjects();

		void QueuePresent();
		void RecordCommandBuffer(VkCommandBuffer& commandBuffer, uint32_t imageIndex);

	private:
		InstanceManager m_InstanceManager = {};
		SwapChainManager m_SwapChainManager = {};
		GraphicsPipelineManager m_GraphicsPipelineManager = {};

		// Own rendering specific things.
		uint32_t m_CurrentFrame = 0;

		VkCommandPool m_CommandPool = VK_NULL_HANDLE;
		std::vector<VkCommandBuffer> m_CommandBuffers = { };

		// Used for synchronization
		std::vector<VkSemaphore> m_ImageAvailableSemaphores = { };
		std::vector<VkSemaphore> m_RenderFinishedSemaphores = { };
		std::vector<VkFence> m_InFlightFences = { };

		// Queue of functions
		std::vector<RenderFunction> m_RenderQueue = { };

		friend class InstanceManager;
		friend class SwapChainManager;
		friend class GraphicsPipelineManager;
	};

}