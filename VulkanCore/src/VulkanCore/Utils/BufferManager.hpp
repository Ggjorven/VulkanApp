#pragma once

#include <vector>
#include <filesystem>

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

		// TODO(Jorben): Improve
		static void CreateTexture(const std::filesystem::path& path, VkImage& dstImage, VkDeviceMemory& dstImageMemory);

	private:
		static void CreateImage(uint32_t width, uint32_t height, VkFormat format, VkImageTiling tiling, VkImageUsageFlags usage, VkMemoryPropertyFlags properties, VkImage& image, VkDeviceMemory& imageMemory);
		static void TransitionImageToLayout(VkImage& image, VkFormat format, VkImageLayout oldLayout, VkImageLayout newLayout);
		static void CopyBufferToImage(VkBuffer& buffer, VkImage& image, uint32_t width, uint32_t height);

	public:
		static VkCommandBuffer BeginSingleTimeCommands();
		static void EndSingleTimeCommands(VkCommandBuffer commandBuffer);
	};

}