#include "Custom.hpp"

#include <iostream>
#include <imgui.h>

#include <VulkanCore/Core/Application.hpp>
#include <VulkanCore/Core/API.hpp>

#include <vulkan/vulkan.h>

void CustomLayer::OnAttach()
{
}

void CustomLayer::OnDetach()
{
}

void CustomLayer::OnUpdate(float deltaTime)
{
}

void CustomLayer::OnRender()
{
	// Vulkan stuff
	/*
	auto gc = Application::Get().GetWindow().GetGraphicsContext();
	if (!gc)
		std::cout << "Something went wrong" << std::endl;

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
	vgc->RecordCommandBuffer(vi.CommandBuffers[vi.CurrentFrame], imageIndex);

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

	vi.CurrentFrame = (vi.CurrentFrame + 1) & VulkanContext::MAX_FRAMES_IN_FLIGHT; // We use the & operator since MAX_FRAMES_IN_FLIGHT is a power of 2 and this is a lot cheaper, if it's not use the % operator
	*/
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
