#include "vcpch.h"
#include "BufferManager.hpp"

#include <stb_image.h>

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
		VkCommandBuffer commandBuffer = BeginSingleTimeCommands();

		VkBufferCopy copyRegion = {};
		copyRegion.size = size;
		vkCmdCopyBuffer(commandBuffer, srcBuffer, dstBuffer, 1, &copyRegion);

		EndSingleTimeCommands(commandBuffer);
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

	void BufferManager::CreateTexture(const std::filesystem::path& path, VkImage& dstImage, VkDeviceMemory& dstImageMemory)
	{
		VkBuffer stagingBuffer;
		VkDeviceMemory stagingMemory;

		int texWidth, texHeight, texChannels;
		
		stbi_uc* pixels = stbi_load(path.string().c_str(), &texWidth, &texHeight, &texChannels, STBI_rgb_alpha);
		VkDeviceSize imageSize = texWidth * texHeight * 4;

		if (!pixels)
			VKAPP_LOG_ERROR("Failed to load texture image!");

		CreateBuffer(imageSize, VK_BUFFER_USAGE_TRANSFER_SRC_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, stagingBuffer, stagingMemory);

		auto logicalDevice = InstanceManager::Get()->GetLogicalDevice();

		void* data;
		vkMapMemory(logicalDevice, stagingMemory, 0, imageSize, 0, &data);
		memcpy(data, pixels, static_cast<size_t>(imageSize));
		vkUnmapMemory(logicalDevice, stagingMemory);

		// Clean up data
		stbi_image_free((void*)pixels);

		CreateImage(texWidth, texHeight, VK_FORMAT_R8G8B8A8_UNORM, VK_IMAGE_TILING_OPTIMAL, VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_SAMPLED_BIT, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, dstImage, dstImageMemory);
		
		TransitionImageToLayout(dstImage, VK_FORMAT_R8G8B8A8_UNORM, VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL);
		CopyBufferToImage(stagingBuffer, dstImage, static_cast<uint32_t>(texWidth), static_cast<uint32_t>(texHeight));
		TransitionImageToLayout(dstImage, VK_FORMAT_R8G8B8A8_UNORM, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);

		// Cleanup
		vkDestroyBuffer(logicalDevice, stagingBuffer, nullptr);
		vkFreeMemory(logicalDevice, stagingMemory, nullptr);
	}

	VkImageView BufferManager::CreateImageView(VkImage& image, VkFormat format, VkImageAspectFlags aspectFlags)
	{
		VkImageViewCreateInfo viewInfo = {};
		viewInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
		viewInfo.image = image;
		viewInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;
		viewInfo.format = format;
		viewInfo.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
		viewInfo.subresourceRange.baseMipLevel = 0;
		viewInfo.subresourceRange.levelCount = 1;
		viewInfo.subresourceRange.baseArrayLayer = 0;
		viewInfo.subresourceRange.layerCount = 1;
		viewInfo.subresourceRange.aspectMask = aspectFlags;

		VkImageView imageView;
		if (vkCreateImageView(InstanceManager::Get()->GetLogicalDevice(), &viewInfo, nullptr, &imageView) != VK_SUCCESS)
			VKAPP_LOG_ERROR("Failed to create texture image view!");

		return imageView;
	}

	VkSampler BufferManager::CreateSampler()
	{
		VkSamplerCreateInfo samplerInfo = {};
		samplerInfo.sType = VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO;
		samplerInfo.magFilter = VK_FILTER_LINEAR;
		samplerInfo.minFilter = VK_FILTER_LINEAR;
		samplerInfo.addressModeU = VK_SAMPLER_ADDRESS_MODE_REPEAT;
		samplerInfo.addressModeV = VK_SAMPLER_ADDRESS_MODE_REPEAT;
		samplerInfo.addressModeW = VK_SAMPLER_ADDRESS_MODE_REPEAT;

		VkPhysicalDeviceProperties properties = {};
		vkGetPhysicalDeviceProperties(InstanceManager::Get()->GetPhysicalDevice(), &properties);
		
		samplerInfo.anisotropyEnable = VK_TRUE;								// Can be disabled: just set VK_FALSE
		samplerInfo.maxAnisotropy = properties.limits.maxSamplerAnisotropy; // And 1.0f

		samplerInfo.borderColor = VK_BORDER_COLOR_INT_OPAQUE_BLACK;
		samplerInfo.unnormalizedCoordinates = VK_FALSE;
		samplerInfo.compareEnable = VK_FALSE;
		samplerInfo.compareOp = VK_COMPARE_OP_ALWAYS;

		samplerInfo.mipmapMode = VK_SAMPLER_MIPMAP_MODE_LINEAR;
		samplerInfo.mipLodBias = 0.0f;
		samplerInfo.minLod = 0.0f;
		samplerInfo.maxLod = 0.0f;

		VkSampler sampler;
		if (vkCreateSampler(InstanceManager::Get()->GetLogicalDevice(), &samplerInfo, nullptr, &sampler) != VK_SUCCESS)
			VKAPP_LOG_ERROR("Failed to create texture sampler!");

		return sampler;
	}

	void BufferManager::CreateImage(uint32_t width, uint32_t height, VkFormat format, VkImageTiling tiling, VkImageUsageFlags usage, VkMemoryPropertyFlags properties, VkImage& image, VkDeviceMemory& imageMemory)
	{
		VkImageCreateInfo imageInfo = {};
		imageInfo.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
		imageInfo.imageType = VK_IMAGE_TYPE_2D;
		imageInfo.extent.width = width;
		imageInfo.extent.height = height;
		imageInfo.extent.depth = 1;
		imageInfo.mipLevels = 1;
		imageInfo.arrayLayers = 1;
		imageInfo.format = format;
		imageInfo.tiling = tiling;
		imageInfo.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
		imageInfo.usage = usage;
		imageInfo.samples = VK_SAMPLE_COUNT_1_BIT;
		imageInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;

		auto logicalDevice = InstanceManager::Get()->GetLogicalDevice();

		if (vkCreateImage(logicalDevice, &imageInfo, nullptr, &image) != VK_SUCCESS)
			VKAPP_LOG_ERROR("Failed to create image!");

		VkMemoryRequirements memRequirements = {};
		vkGetImageMemoryRequirements(logicalDevice, image, &memRequirements);

		VkMemoryAllocateInfo allocInfo = {};
		allocInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
		allocInfo.allocationSize = memRequirements.size;
		allocInfo.memoryTypeIndex = FindMemoryType(memRequirements.memoryTypeBits, properties);

		if (vkAllocateMemory(logicalDevice, &allocInfo, nullptr, &imageMemory) != VK_SUCCESS)
			VKAPP_LOG_ERROR("Failed to allocate image memory!");

		vkBindImageMemory(logicalDevice, image, imageMemory, 0);
	}

	void BufferManager::TransitionImageToLayout(VkImage& image, VkFormat format, VkImageLayout oldLayout, VkImageLayout newLayout)
	{
		VkCommandBuffer commandBuffer = BeginSingleTimeCommands();

		VkImageMemoryBarrier barrier = {};
		barrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
		barrier.oldLayout = oldLayout;
		barrier.newLayout = newLayout;
		barrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
		barrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;

		barrier.image = image;
		barrier.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
		barrier.subresourceRange.baseMipLevel = 0;
		barrier.subresourceRange.levelCount = 1;
		barrier.subresourceRange.baseArrayLayer = 0;
		barrier.subresourceRange.layerCount = 1;

		VkPipelineStageFlags sourceStage;
		VkPipelineStageFlags destinationStage;

		// New layout checks
		if (newLayout == VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL) 
		{
			barrier.subresourceRange.aspectMask = VK_IMAGE_ASPECT_DEPTH_BIT;

			if (BufferManager::HasStencilComponent(format)) 
				barrier.subresourceRange.aspectMask |= VK_IMAGE_ASPECT_STENCIL_BIT;
		}
		else
			barrier.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;

		// Old layout checks
		if (oldLayout == VK_IMAGE_LAYOUT_UNDEFINED && newLayout == VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL) 
		{
			barrier.srcAccessMask = 0;
			barrier.dstAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;

			sourceStage = VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT;
			destinationStage = VK_PIPELINE_STAGE_TRANSFER_BIT;
		}
		else if (oldLayout == VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL && newLayout == VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL) 
		{
			barrier.srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
			barrier.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;

			sourceStage = VK_PIPELINE_STAGE_TRANSFER_BIT;
			destinationStage = VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT;
		}
		else if (oldLayout == VK_IMAGE_LAYOUT_UNDEFINED && newLayout == VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL) 
		{
			barrier.srcAccessMask = 0;
			barrier.dstAccessMask = VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_READ_BIT | VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT;

			sourceStage = VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT;
			destinationStage = VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT;
		}
		else
			throw std::invalid_argument("Unsupported layout transition!");

		vkCmdPipelineBarrier(commandBuffer, sourceStage, destinationStage, 0, 0, nullptr, 0, nullptr, 1, &barrier);

		EndSingleTimeCommands(commandBuffer);
	}

	void BufferManager::CopyBufferToImage(VkBuffer& buffer, VkImage& image, uint32_t width, uint32_t height)
	{
		VkCommandBuffer commandBuffer = BeginSingleTimeCommands();

		VkBufferImageCopy region{};
		region.bufferOffset = 0;
		region.bufferRowLength = 0;
		region.bufferImageHeight = 0;

		region.imageSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
		region.imageSubresource.mipLevel = 0;
		region.imageSubresource.baseArrayLayer = 0;
		region.imageSubresource.layerCount = 1;

		region.imageOffset = { 0, 0, 0 };
		region.imageExtent = { width, height, 1};

		vkCmdCopyBufferToImage(commandBuffer, buffer, image, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, 1, &region);

		EndSingleTimeCommands(commandBuffer);
	}

	VkCommandBuffer BufferManager::BeginSingleTimeCommands()
	{
		VkCommandBufferAllocateInfo allocInfo = {};
		allocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
		allocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
		allocInfo.commandPool = Renderer::Get()->GetCommandPool();
		allocInfo.commandBufferCount = 1;

		VkCommandBuffer commandBuffer;
		if (vkAllocateCommandBuffers(InstanceManager::Get()->GetLogicalDevice(), &allocInfo, &commandBuffer))
			VKAPP_LOG_WARN("Failed to create single time command buffer!");

		VkCommandBufferBeginInfo beginInfo = {};
		beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
		beginInfo.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;

		vkBeginCommandBuffer(commandBuffer, &beginInfo);

		return commandBuffer;
	}

	void BufferManager::EndSingleTimeCommands(VkCommandBuffer commandBuffer)
	{
		vkEndCommandBuffer(commandBuffer);

		VkSubmitInfo submitInfo = {};
		submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
		submitInfo.commandBufferCount = 1;
		submitInfo.pCommandBuffers = &commandBuffer;

		vkQueueSubmit(InstanceManager::Get()->GetGraphicsQueue(), 1, &submitInfo, VK_NULL_HANDLE);
		vkQueueWaitIdle(InstanceManager::Get()->GetGraphicsQueue());

		vkFreeCommandBuffers(InstanceManager::Get()->GetLogicalDevice(), Renderer::Get()->GetCommandPool(), 1, &commandBuffer);
	}

}
