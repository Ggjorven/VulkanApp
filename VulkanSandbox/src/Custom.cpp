#include "Custom.hpp"

#include <imgui.h>

#include <VulkanCore/Core/Application.hpp>
#include <VulkanCore/Core/Logging.hpp>
#include <VulkanCore/Renderer/Renderer.hpp>
#include <VulkanCore/Utils/BufferManager.hpp>

#include <vulkan/vulkan.h>

#define GLM_FORCE_RADIANS
#define GLM_FORCE_DEPTH_ZERO_TO_ONE 
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

const std::vector<Vertex> vertices = {
	{{-0.5f, -0.5f}, {1.0f, 0.0f, 0.0f}},
	{{0.5f, -0.5f}, {0.0f, 1.0f, 0.0f}},
	{{0.5f, 0.5f}, {0.0f, 0.0f, 1.0f}},
	{{-0.5f, 0.5f}, {1.0f, 1.0f, 1.0f}}
};

const std::vector<uint16_t> indices = {
	0, 1, 2, 2, 3, 0
};

struct UniformBufferObject {
	glm::mat4 Model;
	glm::mat4 View;
	glm::mat4 Proj;
};

void CustomLayer::OnAttach()
{
	BufferManager::CreateVertexBuffer(m_VertexBuffer, m_VertexBufferMemory, (void*)vertices.data(), sizeof(vertices[0]) * vertices.size());
	BufferManager::CreateIndexBuffer(m_IndexBuffer, m_IndexBufferMemory, (void*)indices.data(), sizeof(indices[0]) * indices.size());

	BufferManager::CreateUniformBuffer(m_UniformBuffers, sizeof(UniformBufferObject), m_UniformBuffersMemory, m_UniformBuffersMapped);

	// Initialize the descriptor sets/uniforms
	for (size_t i = 0; i < VKAPP_MAX_FRAMES_IN_FLIGHT; i++) 
	{
		VkDescriptorBufferInfo bufferInfo = {};
		bufferInfo.buffer = m_UniformBuffers[i];
		bufferInfo.offset = 0;
		bufferInfo.range = sizeof(UniformBufferObject);

		VkWriteDescriptorSet descriptorWrite = {};
		descriptorWrite.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
		descriptorWrite.dstSet = GraphicsPipelineManager::Get()->GetDescriptorSets()[i];
		descriptorWrite.dstBinding = 0;
		descriptorWrite.dstArrayElement = 0;
		descriptorWrite.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
		descriptorWrite.descriptorCount = 1;
		descriptorWrite.pBufferInfo = &bufferInfo;
		descriptorWrite.pImageInfo = nullptr; // Optional
		descriptorWrite.pTexelBufferView = nullptr; // Optional

		vkUpdateDescriptorSets(InstanceManager::Get()->GetLogicalDevice(), 1, &descriptorWrite, 0, nullptr);
	}
}

void CustomLayer::OnDetach()
{
	auto logicalDevice = InstanceManager::Get()->GetLogicalDevice();

	vkDeviceWaitIdle(logicalDevice);

	vkDestroyBuffer(logicalDevice, m_VertexBuffer, nullptr);
	vkFreeMemory(logicalDevice, m_VertexBufferMemory, nullptr);

	vkDestroyBuffer(logicalDevice, m_IndexBuffer, nullptr);
	vkFreeMemory(logicalDevice, m_IndexBufferMemory, nullptr);

	for (size_t i = 0; i < VKAPP_MAX_FRAMES_IN_FLIGHT; i++) 
	{
		vkDestroyBuffer(logicalDevice, m_UniformBuffers[i], nullptr);
		vkFreeMemory(logicalDevice, m_UniformBuffersMemory[i], nullptr);
	}
}

void CustomLayer::OnUpdate(float deltaTime)
{
	m_SavedDeltaTime = deltaTime;
}

void CustomLayer::OnRender()
{
	Renderer::AddToQueue([this](VkCommandBuffer& buffer, uint32_t imageIndex)
		{
			std::vector<VkDeviceSize> offsets = { {0} };
			uint32_t currentFrame = Renderer::Get()->GetCurrentImage();

			UpdateUniformBuffers(currentFrame);
	
			vkCmdBindVertexBuffers(buffer, 0, 1, &m_VertexBuffer, offsets.data());
			vkCmdBindIndexBuffer(buffer, m_IndexBuffer, 0, VK_INDEX_TYPE_UINT16);

			vkCmdBindDescriptorSets(buffer, VK_PIPELINE_BIND_POINT_GRAPHICS, GraphicsPipelineManager::Get()->GetPipelineLayout(), 0, 1, &GraphicsPipelineManager::Get()->GetDescriptorSets()[currentFrame], 0, nullptr);
	
			vkCmdDrawIndexed(buffer, indices.size(), 1, 0, 0, 0);
		});
}

void CustomLayer::OnImGuiRender()
{
}

void CustomLayer::OnEvent(Event& e)
{
}

void CustomLayer::UpdateUniformBuffers(uint32_t imageIndex)
{
	auto& extent = SwapChainManager::Get()->GetExtent();

	UniformBufferObject ubo = {};
	static float lastTime = 0.0f;
	lastTime += m_SavedDeltaTime;

	ubo.Model = glm::rotate(glm::mat4(1.0f), lastTime * glm::radians(90.0f), glm::vec3(0.0f, 0.0f, 1.0f));
	ubo.View = glm::lookAt(glm::vec3(2.0f, 2.0f, 2.0f), glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(0.0f, 0.0f, 1.0f));
	ubo.Proj = glm::perspective(glm::radians(45.0f), (float)extent.width / (float)extent.height, 0.1f, 10.0f);
	ubo.Proj[1][1] *= -1;

	memcpy(m_UniformBuffersMapped[imageIndex], &ubo, sizeof(ubo));
}
