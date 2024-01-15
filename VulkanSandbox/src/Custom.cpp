#include "Custom.hpp"

#include <iostream>
#include <imgui.h>

#include <VulkanCore/Core/Application.hpp>
#include <VulkanCore/Core/Logging.hpp>
#include <VulkanCore/Renderer/Renderer.hpp>
#include <VulkanCore/Utils/BufferManager.hpp>

#include <vulkan/vulkan.h>

const std::vector<Vertex> vertices = {
	{{-0.5f, -0.5f}, {1.0f, 0.0f, 0.0f}},
	{{0.5f, -0.5f}, {0.0f, 1.0f, 0.0f}},
	{{0.5f, 0.5f}, {0.0f, 0.0f, 1.0f}},
	{{-0.5f, 0.5f}, {1.0f, 1.0f, 1.0f}}
};

const std::vector<uint16_t> indices = {
	0, 1, 2, 2, 3, 0
};

void CustomLayer::OnAttach()
{
	BufferManager::CreateVertexBuffer(m_VertexBuffer, m_VertexBufferMemory, (void*)vertices.data(), sizeof(vertices[0]) * vertices.size());
	BufferManager::CreateIndexBuffer(m_IndexBuffer, m_IndexBufferMemory, (void*)indices.data(), sizeof(indices[0]) * indices.size());
}

void CustomLayer::OnDetach()
{
	auto logicalDevice = InstanceManager::Get()->GetLogicalDevice();

	vkDeviceWaitIdle(logicalDevice);

	vkDestroyBuffer(logicalDevice, m_VertexBuffer, nullptr);
	vkFreeMemory(logicalDevice, m_VertexBufferMemory, nullptr);

	vkDestroyBuffer(logicalDevice, m_IndexBuffer, nullptr);
	vkFreeMemory(logicalDevice, m_IndexBufferMemory, nullptr);
}

void CustomLayer::OnUpdate(float deltaTime)
{
}

void CustomLayer::OnRender()
{
	Renderer::AddToQueue([this](VkCommandBuffer& buffer)
		{
			std::vector<VkDeviceSize> offsets = { {0} };
	
			vkCmdBindVertexBuffers(buffer, 0, 1, &m_VertexBuffer, offsets.data());
			vkCmdBindIndexBuffer(buffer, m_IndexBuffer, 0, VK_INDEX_TYPE_UINT16);
	
			vkCmdDrawIndexed(buffer, indices.size(), 1, 0, 0, 0);
		});
}

void CustomLayer::OnImGuiRender()
{
}

void CustomLayer::OnEvent(Event& e)
{
}