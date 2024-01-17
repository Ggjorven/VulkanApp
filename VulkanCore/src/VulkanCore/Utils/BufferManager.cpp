#include "vcpch.h"
#include "BufferManager.hpp"

#include "VulkanCore/Core/Logging.hpp"

#include "VulkanCore/Renderer/Renderer.hpp"
#include "VulkanCore/Renderer/InstanceManager.hpp"
#include "VulkanCore/Renderer/SwapChainManager.hpp"

namespace VkApp
{

	// ===================================
	// ------------ Static ---------------
	// ===================================
	void BufferManager::CreateBuffer(VkDeviceSize size, VkBufferUsageFlags usage, VkMemoryPropertyFlags properties, VkBuffer& dstBuffer, VkDeviceMemory& dstBufferMemory)
	{
		auto logicalDevice = InstanceManager::Get()->GetLogicalDevice();

		VkBufferCreateInfo bufferInfo = {};
		bufferInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
		bufferInfo.size = size;
		bufferInfo.usage = usage;
		bufferInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;

		if (vkCreateBuffer(logicalDevice, &bufferInfo, nullptr, &dstBuffer) != VK_SUCCESS)
			VKAPP_LOG_ERROR("Failed to create buffer!");

		VkMemoryRequirements memRequirements = {};
		vkGetBufferMemoryRequirements(logicalDevice, dstBuffer, &memRequirements);

		VkMemoryAllocateInfo allocInfo = {};
		allocInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
		allocInfo.allocationSize = memRequirements.size;
		allocInfo.memoryTypeIndex = FindMemoryType(memRequirements.memoryTypeBits, properties);

		if (vkAllocateMemory(logicalDevice, &allocInfo, nullptr, &dstBufferMemory) != VK_SUCCESS)
			VKAPP_LOG_ERROR("Failed to allocate buffer memory!");

		vkBindBufferMemory(logicalDevice, dstBuffer, dstBufferMemory, 0);
	}

	void BufferManager::CopyBuffer(VkBuffer& srcBuffer, VkBuffer& dstBuffer, VkDeviceSize& size)
	{
		auto instanceManager = InstanceManager::Get();

		VkCommandBufferAllocateInfo allocInfo = {};
		allocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
		allocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
		allocInfo.commandPool = Renderer::Get()->GetCommandPool();
		allocInfo.commandBufferCount = 1;

		// Create a new command buffer
		VkCommandBuffer commandBuffer = {};
		vkAllocateCommandBuffers(instanceManager->GetLogicalDevice(), &allocInfo, &commandBuffer);

		VkCommandBufferBeginInfo beginInfo = {};
		beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
		beginInfo.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;

		vkBeginCommandBuffer(commandBuffer, &beginInfo);

		VkBufferCopy copyRegion = {};
		copyRegion.srcOffset = 0; // Optional
		copyRegion.dstOffset = 0; // Optional
		copyRegion.size = size;
		vkCmdCopyBuffer(commandBuffer, srcBuffer, dstBuffer, 1, &copyRegion);

		// We recorded a copy command.
		vkEndCommandBuffer(commandBuffer);

		// Let's execute it
		VkSubmitInfo submitInfo = {};
		submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
		submitInfo.commandBufferCount = 1;
		submitInfo.pCommandBuffers = &commandBuffer;

		vkQueueSubmit(instanceManager->GetGraphicsQueue(), 1, &submitInfo, VK_NULL_HANDLE);
		vkQueueWaitIdle(instanceManager->GetGraphicsQueue());

		vkFreeCommandBuffers(instanceManager->GetLogicalDevice(), Renderer::Get()->GetCommandPool(), 1, &commandBuffer);
	}

	uint32_t BufferManager::FindMemoryType(uint32_t typeFilter, VkMemoryPropertyFlags properties)
	{
		VkPhysicalDeviceMemoryProperties memProperties = {};
		vkGetPhysicalDeviceMemoryProperties(InstanceManager::Get()->GetPhysicalDevice(), &memProperties);

		for (uint32_t i = 0; i < memProperties.memoryTypeCount; i++)
		{
			if ((typeFilter & (1 << i)) && (memProperties.memoryTypes[i].propertyFlags & properties) == properties)
				return i;
		}

		VKAPP_LOG_ERROR("Failed to find suitable memory type!");
		return -1;
	}

	void BufferManager::CreateVertexBuffer(VkBuffer& dstBuffer, VkDeviceMemory& dstMemory, void* vertices, VkDeviceSize size)
	{
		auto logicalDevice = InstanceManager::Get()->GetLogicalDevice();

		VkBuffer stagingBuffer;
		VkDeviceMemory stagingBufferMemory;

		CreateBuffer(size, VK_BUFFER_USAGE_TRANSFER_SRC_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, stagingBuffer, stagingBufferMemory);

		void* data;
		vkMapMemory(logicalDevice, stagingBufferMemory, 0, size, 0, &data);
		memcpy(data, vertices, (size_t)size);
		vkUnmapMemory(logicalDevice, stagingBufferMemory);

		CreateBuffer(size, VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_VERTEX_BUFFER_BIT, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, dstBuffer, dstMemory);
		CopyBuffer(stagingBuffer, dstBuffer, size);

		// Free the staging buffer
		vkDestroyBuffer(logicalDevice, stagingBuffer, nullptr);
		vkFreeMemory(logicalDevice, stagingBufferMemory, nullptr);
	}

	void BufferManager::CreateIndexBuffer(VkBuffer& dstBuffer, VkDeviceMemory& dstMemory, void* indices, VkDeviceSize size)
	{
		auto logicalDevice = InstanceManager::Get()->GetLogicalDevice();

		VkBuffer stagingBuffer;
		VkDeviceMemory stagingBufferMemory;

		CreateBuffer(size, VK_BUFFER_USAGE_TRANSFER_SRC_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, stagingBuffer, stagingBufferMemory);

		void* data;
		vkMapMemory(logicalDevice, stagingBufferMemory, 0, size, 0, &data);
		memcpy(data, indices, (size_t)size);
		vkUnmapMemory(logicalDevice, stagingBufferMemory);

		CreateBuffer(size, VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_INDEX_BUFFER_BIT, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, dstBuffer, dstMemory);
		CopyBuffer(stagingBuffer, dstBuffer, size);

		// Free the staging buffer
		vkDestroyBuffer(logicalDevice, stagingBuffer, nullptr);
		vkFreeMemory(logicalDevice, stagingBufferMemory, nullptr);
	}

	void BufferManager::CreateUniformBuffer(std::vector<VkBuffer>& buffers, VkDeviceSize size, std::vector<VkDeviceMemory>& buffersMemory, std::vector<void*>& mappedBuffers)
	{
		auto logicalDevice = InstanceManager::Get()->GetLogicalDevice();

		buffers.resize(VKAPP_MAX_FRAMES_IN_FLIGHT);
		buffersMemory.resize(VKAPP_MAX_FRAMES_IN_FLIGHT);
		mappedBuffers.resize(VKAPP_MAX_FRAMES_IN_FLIGHT);

		for (size_t i = 0; i < VKAPP_MAX_FRAMES_IN_FLIGHT; i++) {
			CreateBuffer(size, VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, buffers[i], buffersMemory[i]);

			vkMapMemory(logicalDevice, buffersMemory[i], 0, size, 0, &mappedBuffers[i]);
		}
	}

	void BufferManager::SetUniformData(void* mappedBuffer, void* data, uint32_t size)
	{
		memcpy(mappedBuffer, data, (size_t)size);
	}

}
