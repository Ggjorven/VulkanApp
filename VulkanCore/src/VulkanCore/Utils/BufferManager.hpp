#pragma once

#include <vector>

#include <vulkan/vulkan.h>

namespace VkApp
{

	class BufferManager
	{
	public:
		static void CreateBuffer(VkDeviceSize size, VkBufferUsageFlags usage, VkMemoryPropertyFlags properties, VkBuffer& dstBuffer, VkDeviceMemory& dstBufferMemory);
		static void CopyBuffer(VkBuffer& srcBuffer, VkBuffer& dstBuffer, VkDeviceSize& size);

		static uint32_t FindMemoryType(uint32_t typeFilter, VkMemoryPropertyFlags properties);

		static void CreateVertexBuffer(VkBuffer& dstBuffer, VkDeviceMemory& dstMemory, void* vertices, VkDeviceSize size = { 0u });
		static void CreateIndexBuffer(VkBuffer& dstBuffer, VkDeviceMemory& dstMemory, void* indices, VkDeviceSize size = { 0u });

		static void CreateUniformBuffer(std::vector<VkBuffer>& buffers, VkDeviceSize size, std::vector<VkDeviceMemory>& buffersMemory, std::vector<void*>& mappedBuffers);
		static void SetUniformData(void* mappedBuffer, void* data, uint32_t size);

	private:
		
	};

}