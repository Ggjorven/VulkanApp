#pragma once

#include <vector>

#include <VulkanCore/Core/Layer.hpp>

#include <vulkan/vulkan.h>

using namespace VkApp;

class CustomLayer : public Layer
{
public:
	void OnAttach() override;
	void OnDetach() override;

	void OnUpdate(float deltaTime) override;
	void OnRender() override;
	void OnImGuiRender() override;

	void OnEvent(Event& e) override;

private:
	void UpdateUniformBuffers(float deltaTime, uint32_t imageIndex);

private:
	VkBuffer m_VertexBuffer = VK_NULL_HANDLE;
	VkDeviceMemory m_VertexBufferMemory = VK_NULL_HANDLE;

	VkBuffer m_IndexBuffer = VK_NULL_HANDLE;
	VkDeviceMemory m_IndexBufferMemory = VK_NULL_HANDLE;

	std::vector<VkBuffer> m_UniformBuffers = { };
	std::vector<VkDeviceMemory> m_UniformBuffersMemory = { };
	std::vector<void*> m_UniformBuffersMapped = { };

	std::vector<VkBuffer> m_UniformBuffers2 = { };
	std::vector<VkDeviceMemory> m_UniformBuffersMemory2 = { };
	std::vector<void*> m_UniformBuffersMapped2 = { };
};