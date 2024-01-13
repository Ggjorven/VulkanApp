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
	{{-0.5f, 0.5f}, {0.0f, 0.0f, 1.0f}}
};

void CustomLayer::OnAttach()
{
	// Vulkan stuff
	auto vgc = Application::Get().GetWindow().GetGraphicsContext();
	auto& vi = vgc->GetVulkanInfo();

	// Create buffer info
	VkBufferCreateInfo bufferInfo{};
	bufferInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
	bufferInfo.size = sizeof(vertices[0]) * vertices.size();
	bufferInfo.usage = VK_BUFFER_USAGE_VERTEX_BUFFER_BIT;
	bufferInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;

	if (vkCreateBuffer(vi.LogicalDevice, &bufferInfo, nullptr, &m_VertexBuffer) != VK_SUCCESS)
		throw std::runtime_error("Failed to create vertex buffer!");

	VkMemoryRequirements memRequirements = {};
	vkGetBufferMemoryRequirements(vi.LogicalDevice, m_VertexBuffer, &memRequirements);

	VkMemoryAllocateInfo allocInfo = {};
	allocInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
	allocInfo.allocationSize = memRequirements.size;
	allocInfo.memoryTypeIndex = vgc->FindMemoryType(memRequirements.memoryTypeBits, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT);

	if (vkAllocateMemory(vi.LogicalDevice, &allocInfo, nullptr, &m_VertexBufferMemory) != VK_SUCCESS)
		throw std::runtime_error("Failed to allocate vertex buffer memory!");

	// Bind the memory to vertex buffer
	vkBindBufferMemory(vi.LogicalDevice, m_VertexBuffer, m_VertexBufferMemory, 0);

	// Filling the vertex buffer
	void* data;
	vkMapMemory(vi.LogicalDevice, m_VertexBufferMemory, 0, bufferInfo.size, 0, &data);
	memcpy(data, vertices.data(), (size_t)bufferInfo.size);
	vkUnmapMemory(vi.LogicalDevice, m_VertexBufferMemory);
}

void CustomLayer::OnDetach()
{
	// Vulkan stuff
	auto vgc = Application::Get().GetWindow().GetGraphicsContext();
	auto& vi = vgc->GetVulkanInfo();

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
