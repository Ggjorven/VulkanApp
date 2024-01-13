#include "Custom.hpp"

#include <iostream>
#include <imgui.h>

#include <VulkanCore/Core/Application.hpp>
#include <VulkanCore/Core/API.hpp>
#include <VulkanCore/Core/Logging.hpp>

#include <vulkan/vulkan.h>

const std::vector<GraphicsContext::Vertex> vertices = {
	{{0.0f, -0.5f}, {1.0f, 1.0f, 1.0f}},
	{{0.5f, 0.5f}, {0.0f, 1.0f, 0.0f}},
	{{-0.5f, 0.5f}, {0.0f, 0.0f, 1.0f}},

	{{0.0f, -0.3f}, {1.0f, 0.0f, 0.0f}},
	{{0.3f, 0.3f}, {0.0f, 1.0f, 0.0f}},
	{{-0.3f, 0.3f}, {0.0f, 0.0f, 1.0f}}
};

/*
const std::vector<GraphicsContext::Vertex> vertices = {
	{{-0.5f, 0.5f}, {1.0f, 1.0f, 1.0f}},
	{{-0.5f, -0.5f}, {0.0f, 1.0f, 0.0f}},
	{{0.5f, -0.5f}, {0.0f, 0.0f, 1.0f}},

	{{0.5f, 0.5f}, {1.0f, 0.0f, 0.0f}},
	{{0.5f, -0.5f}, {0.0f, 1.0f, 0.0f}},
	{{-0.5f, 0.5f}, {0.0f, 0.0f, 1.0f}}
};
*/

static void CreateBuffer(VkDeviceSize size, VkBufferUsageFlags usage, VkMemoryPropertyFlags properties, VkBuffer& buffer, VkDeviceMemory& bufferMemory) 
{
	// Vulkan stuff
	auto vgc = Application::Get().GetWindow().GetGraphicsContext();
	auto& vi = vgc->GetVulkanInfo();

	VkBufferCreateInfo bufferInfo = {};
	bufferInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
	bufferInfo.size = size;
	bufferInfo.usage = usage;
	bufferInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;

	if (vkCreateBuffer(vi.LogicalDevice, &bufferInfo, nullptr, &buffer) != VK_SUCCESS)
		throw std::runtime_error("failed to create buffer!");

	VkMemoryRequirements memRequirements = {};
	vkGetBufferMemoryRequirements(vi.LogicalDevice, buffer, &memRequirements);

	VkMemoryAllocateInfo allocInfo = {};
	allocInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
	allocInfo.allocationSize = memRequirements.size;
	allocInfo.memoryTypeIndex = vgc->FindMemoryType(memRequirements.memoryTypeBits, properties);

	if (vkAllocateMemory(vi.LogicalDevice, &allocInfo, nullptr, &bufferMemory) != VK_SUCCESS)
		throw std::runtime_error("failed to allocate buffer memory!");

	vkBindBufferMemory(vi.LogicalDevice, buffer, bufferMemory, 0);
}

static void CopyBuffer(VkBuffer& srcBuffer, VkBuffer& dstBuffer, VkDeviceSize& size)
{
	// Vulkan stuff
	auto vgc = Application::Get().GetWindow().GetGraphicsContext();
	auto& vi = vgc->GetVulkanInfo();

	VkCommandBufferAllocateInfo allocInfo = {};
	allocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
	allocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
	allocInfo.commandPool = vi.CommandPool;
	allocInfo.commandBufferCount = 1;

	// Create a new command buffer
	VkCommandBuffer commandBuffer = {};
	vkAllocateCommandBuffers(vi.LogicalDevice, &allocInfo, &commandBuffer);

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

	vkQueueSubmit(vi.GraphicsQueue, 1, &submitInfo, VK_NULL_HANDLE);
	vkQueueWaitIdle(vi.GraphicsQueue);

	vkFreeCommandBuffers(vi.LogicalDevice, vi.CommandPool, 1, &commandBuffer);
}

void CustomLayer::OnAttach()
{
	// Vulkan stuff
	auto vgc = Application::Get().GetWindow().GetGraphicsContext();
	auto& vi = vgc->GetVulkanInfo();

	VkDeviceSize bufferSize = sizeof(vertices[0]) * vertices.size();

	VkBuffer stagingBuffer;
	VkDeviceMemory stagingBufferMemory;
	CreateBuffer(bufferSize, VK_BUFFER_USAGE_TRANSFER_SRC_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, stagingBuffer, stagingBufferMemory);

	void* data;
	vkMapMemory(vi.LogicalDevice, stagingBufferMemory, 0, bufferSize, 0, &data);
	memcpy(data, vertices.data(), (size_t)bufferSize);
	vkUnmapMemory(vi.LogicalDevice, stagingBufferMemory);

	CreateBuffer(bufferSize, VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_VERTEX_BUFFER_BIT, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, m_VertexBuffer, m_VertexBufferMemory);
	CopyBuffer(stagingBuffer, m_VertexBuffer, bufferSize);

	// Free the staging buffer
	vkDestroyBuffer(vi.LogicalDevice, stagingBuffer, nullptr);
	vkFreeMemory(vi.LogicalDevice, stagingBufferMemory, nullptr);
}

void CustomLayer::OnDetach()
{
	// Vulkan stuff
	auto vgc = Application::Get().GetWindow().GetGraphicsContext();
	auto& vi = vgc->GetVulkanInfo();

	vkDeviceWaitIdle(vi.LogicalDevice);

	vkDestroyBuffer(vi.LogicalDevice, m_VertexBuffer, nullptr);
	vkFreeMemory(vi.LogicalDevice, m_VertexBufferMemory, nullptr);
}

void CustomLayer::OnUpdate(float deltaTime)
{
}

void CustomLayer::OnRender()
{
	// Vulkan stuff
	auto vgc = Application::Get().GetWindow().GetGraphicsContext();
	auto& vi = vgc->GetVulkanInfo();

	// Run some vulkan code
	vkWaitForFences(vi.LogicalDevice, 1, &vi.InFlightFences[vi.CurrentFrame], VK_TRUE, UINT64_MAX);

	uint32_t imageIndex;

	VkResult result = vkAcquireNextImageKHR(vi.LogicalDevice, vi.SwapChain, UINT64_MAX, vi.ImageAvailableSemaphores[vi.CurrentFrame], VK_NULL_HANDLE, &imageIndex);
	if (result == VK_ERROR_OUT_OF_DATE_KHR) 
	{
		vgc->RecreateSwapChain();
		return;
	}
	else if (result != VK_SUCCESS && result != VK_SUBOPTIMAL_KHR)
		throw std::runtime_error("Failed to acquire swap chain image!");

	// Only reset the fence if we actually submit the work
	vkResetFences(vi.LogicalDevice, 1, &vi.InFlightFences[vi.CurrentFrame]);

	vkResetCommandBuffer(vi.CommandBuffers[vi.CurrentFrame], 0);

	// Custom struct, remove?
	RenderInfo info = {};
	info.VertexBuffers.push_back(m_VertexBuffer);
	info.VerticeCount = vertices.size();

	vgc->RecordCommandBuffer(vi.CommandBuffers[vi.CurrentFrame], imageIndex, info);

	VkSubmitInfo submitInfo = {};
	submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;

	VkSemaphore waitSemaphores[] = { vi.ImageAvailableSemaphores[vi.CurrentFrame] };
	VkPipelineStageFlags waitStages[] = { VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT };
	submitInfo.waitSemaphoreCount = 1;
	submitInfo.pWaitSemaphores = waitSemaphores;
	submitInfo.pWaitDstStageMask = waitStages;

	submitInfo.commandBufferCount = 1;
	submitInfo.pCommandBuffers = &vi.CommandBuffers[vi.CurrentFrame];

	VkSemaphore signalSemaphores[] = { vi.RenderFinishedSemaphores[vi.CurrentFrame] };
	submitInfo.signalSemaphoreCount = 1;
	submitInfo.pSignalSemaphores = signalSemaphores;

	if (vkQueueSubmit(vi.GraphicsQueue, 1, &submitInfo, vi.InFlightFences[vi.CurrentFrame]) != VK_SUCCESS)
		throw std::runtime_error("Failed to submit draw command buffer!");

	VkPresentInfoKHR presentInfo = {};
	presentInfo.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;
	presentInfo.waitSemaphoreCount = 1;
	presentInfo.pWaitSemaphores = signalSemaphores;

	VkSwapchainKHR swapChains[] = { vi.SwapChain };
	presentInfo.swapchainCount = 1;
	presentInfo.pSwapchains = swapChains;
	presentInfo.pImageIndices = &imageIndex;
	presentInfo.pResults = nullptr; // Optional

	// Check for the result on present again
	result = vkQueuePresentKHR(vi.PresentQueue, &presentInfo);

	if (result == VK_ERROR_OUT_OF_DATE_KHR || result == VK_SUBOPTIMAL_KHR)
		vgc->RecreateSwapChain();
	else if (result != VK_SUCCESS)
		throw std::runtime_error("Failed to present swap chain image!");

	vi.CurrentFrame = (vi.CurrentFrame + 1) & GraphicsContext::MAX_FRAMES_IN_FLIGHT; // We use the & operator since MAX_FRAMES_IN_FLIGHT is a power of 2 and this is a lot cheaper, if it's not use the % operator
}

void CustomLayer::OnImGuiRender()
{
}

void CustomLayer::OnEvent(Event& e)
{
	EventHandler handler(e);

	handler.Handle<WindowResizeEvent>(VKAPP_BIND_EVENT_FN(CustomLayer::HandleWindowResize));
}

bool CustomLayer::HandleWindowResize(WindowResizeEvent& e)
{
	// Run the actual command
	/*
	vgc->RecreateSwapChain();
	*/

	return false;
}
