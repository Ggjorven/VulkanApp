#include "Custom.hpp"

#include <imgui.h>

#include <VulkanCore/Core/Application.hpp>
#include <VulkanCore/Core/Input/Input.hpp>
#include <VulkanCore/Core/Logging.hpp>
#include <VulkanCore/Renderer/Renderer.hpp>
#include <VulkanCore/Utils/BufferManager.hpp>

#include <vulkan/vulkan.h>

#define GLM_FORCE_RADIANS
#define GLM_FORCE_DEPTH_ZERO_TO_ONE 
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

struct Vertex
{
public:
	glm::vec2 Position = { };
	glm::vec3 Colour = { };

	static VkVertexInputBindingDescription GetBindingDescription()
	{
		VkVertexInputBindingDescription bindingDescription = {};
		bindingDescription.binding = 0;
		bindingDescription.stride = sizeof(Vertex);
		bindingDescription.inputRate = VK_VERTEX_INPUT_RATE_VERTEX;

		return bindingDescription;
	}

	static std::array<VkVertexInputAttributeDescription, 2> GetAttributeDescriptions()
	{
		std::array<VkVertexInputAttributeDescription, 2> attributeDescriptions = {};

		// Position
		attributeDescriptions[0].binding = 0;
		attributeDescriptions[0].location = 0;
		attributeDescriptions[0].format = VK_FORMAT_R32G32_SFLOAT;
		attributeDescriptions[0].offset = offsetof(Vertex, Position);

		// Colour
		attributeDescriptions[1].binding = 0;
		attributeDescriptions[1].location = 1;
		attributeDescriptions[1].format = VK_FORMAT_R32G32B32_SFLOAT;
		attributeDescriptions[1].offset = offsetof(Vertex, Colour);

		return attributeDescriptions;
	}
};

const std::vector<Vertex> vertices = {
	{{-0.5f, -0.5f}, {1.0f, 0.0f, 0.0f}},
	{{0.5f, -0.5f}, {0.0f, 1.0f, 0.0f}},
	{{0.5f, 0.5f}, {0.0f, 0.0f, 1.0f}},
	{{-0.5f, 0.5f}, {1.0f, 1.0f, 1.0f}}
};

const std::vector<uint16_t> indices = {
	0, 1, 2, 2, 3, 0
};

struct UniformBufferObject 
{
	glm::mat4 Model;
	glm::mat4 View;
	glm::mat4 Proj;
};

struct UniformBufferObject2
{
	glm::mat4 Model;
	glm::mat4 View;
	glm::mat4 Proj;
};

void CustomLayer::OnAttach()
{
	PipelineInfo info = {};
	info.VertexShader = GraphicsPipelineManager::ReadFile("assets\\shaders\\vert.spv");
	info.FragmentShader = GraphicsPipelineManager::ReadFile("assets\\shaders\\frag.spv");
	info.VertexBindingDescription = Vertex::GetBindingDescription();
	info.VertexAttributeDescriptions = Vertex::GetAttributeDescriptions();

	DescriptorInfo defaultDescriptor = {};
	defaultDescriptor.Binding = 0;
	info.DescriptorSets.Set0.push_back(defaultDescriptor);

	DescriptorInfo newDescriptor = {};
	newDescriptor.Binding = 0;
	info.DescriptorSets.Set1.push_back(newDescriptor);

	GraphicsPipelineManager::Get()->DestroyCurrentPipeline();
	GraphicsPipelineManager::Get()->CreatePipeline(info);

	BufferManager::CreateVertexBuffer(m_VertexBuffer, m_VertexBufferMemory, (void*)vertices.data(), sizeof(vertices[0]) * vertices.size());
	BufferManager::CreateIndexBuffer(m_IndexBuffer, m_IndexBufferMemory, (void*)indices.data(), sizeof(indices[0]) * indices.size());

	BufferManager::CreateUniformBuffer(m_UniformBuffers, sizeof(UniformBufferObject), m_UniformBuffersMemory, m_UniformBuffersMapped);
	BufferManager::CreateUniformBuffer(m_UniformBuffers2, sizeof(UniformBufferObject2), m_UniformBuffersMemory2, m_UniformBuffersMapped2);

	// Initialize the descriptor sets/uniforms
	for (size_t i = 0; i < VKAPP_MAX_FRAMES_IN_FLIGHT; i++) 
	{
		VkDescriptorBufferInfo bufferInfo = {};
		bufferInfo.buffer = m_UniformBuffers[i];
		bufferInfo.offset = 0;
		bufferInfo.range = sizeof(UniformBufferObject);

		VkWriteDescriptorSet descriptorWrite = {};
		descriptorWrite.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
		descriptorWrite.dstSet = GraphicsPipelineManager::Get()->GetDescriptorSets()[0][i];
		descriptorWrite.dstBinding = 0;
		descriptorWrite.dstArrayElement = 0;
		descriptorWrite.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
		descriptorWrite.descriptorCount = 1;
		descriptorWrite.pBufferInfo = &bufferInfo;
		descriptorWrite.pImageInfo = nullptr; // Optional
		descriptorWrite.pTexelBufferView = nullptr; // Optional

		// TODO(Jorben): Remove. // Note(Jorben): We're doing this again for second descriptor
		VkDescriptorBufferInfo bufferInfo2 = {};
		bufferInfo2.buffer = m_UniformBuffers2[i];
		bufferInfo2.offset = 0;
		bufferInfo2.range = sizeof(UniformBufferObject2);

		VkWriteDescriptorSet descriptorWrite2 = {};
		descriptorWrite2.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
		descriptorWrite2.dstSet = GraphicsPipelineManager::Get()->GetDescriptorSets()[1][i];
		descriptorWrite2.dstBinding = 0;
		descriptorWrite2.dstArrayElement = 0;
		descriptorWrite2.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
		descriptorWrite2.descriptorCount = 1;
		descriptorWrite2.pBufferInfo = &bufferInfo2;
		descriptorWrite2.pImageInfo = nullptr; // Optional
		descriptorWrite2.pTexelBufferView = nullptr; // Optional

		vkUpdateDescriptorSets(InstanceManager::Get()->GetLogicalDevice(), 1, &descriptorWrite, 0, nullptr);
		vkUpdateDescriptorSets(InstanceManager::Get()->GetLogicalDevice(), 1, &descriptorWrite2, 0, nullptr);
	}

	// -------
	//  Textures
	// -------
	BufferManager::CreateTexture("assets/textures/texture.jpg", m_TextureImage, m_TextureImageMemory);
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

	// TODO(Jorben): Remove
	for (size_t i = 0; i < VKAPP_MAX_FRAMES_IN_FLIGHT; i++)
	{
		vkDestroyBuffer(logicalDevice, m_UniformBuffers2[i], nullptr);
		vkFreeMemory(logicalDevice, m_UniformBuffersMemory2[i], nullptr);
	}

	vkDestroyImage(logicalDevice, m_TextureImage, nullptr);
	vkFreeMemory(logicalDevice, m_TextureImageMemory, nullptr);
}

void CustomLayer::OnUpdate(float deltaTime)
{
	UpdateUniformBuffers(deltaTime, Renderer::Get()->GetCurrentImage());
}

void CustomLayer::OnRender()
{
	Renderer::AddToQueue([this](VkCommandBuffer& buffer, uint32_t imageIndex)
		{
			std::vector<VkDeviceSize> offsets = { {0} };
			uint32_t currentFrame = Renderer::Get()->GetCurrentImage();

			vkCmdBindVertexBuffers(buffer, 0, 1, &m_VertexBuffer, offsets.data());
			vkCmdBindIndexBuffer(buffer, m_IndexBuffer, 0, VK_INDEX_TYPE_UINT16);

			vkCmdBindDescriptorSets(buffer, VK_PIPELINE_BIND_POINT_GRAPHICS, GraphicsPipelineManager::Get()->GetPipelineLayout(), 0, 1, &GraphicsPipelineManager::Get()->GetDescriptorSets()[0][currentFrame], 0, nullptr);
			vkCmdBindDescriptorSets(buffer, VK_PIPELINE_BIND_POINT_GRAPHICS, GraphicsPipelineManager::Get()->GetPipelineLayout(), 1, 1, &GraphicsPipelineManager::Get()->GetDescriptorSets()[1][currentFrame], 0, nullptr); // Note(Jorben): Here we specify set = x in the 4th argument.
	
			vkCmdDrawIndexed(buffer, (uint32_t)indices.size(), 1, 0, 0, 0);
		});
}

void CustomLayer::OnImGuiRender()
{
	ImGui::ShowDemoWindow();
}

void CustomLayer::OnEvent(Event& e)
{
}

void CustomLayer::UpdateUniformBuffers(float deltaTime, uint32_t imageIndex)
{
	auto& window = Application::Get().GetWindow();

	static float sum = 0.0f;
	if (Input::IsKeyPressed(Key::D))
	{
		sum += deltaTime * 1.5f;
	}
	if (Input::IsKeyPressed(Key::A))
	{
		sum -= deltaTime * 1.5f;
	}

	UniformBufferObject ubo = {};
	ubo.Model = glm::rotate(glm::mat4(1.0f), sum * glm::radians(90.0f), glm::vec3(0.0f, 0.0f, 1.0f));
	ubo.View = glm::lookAt(glm::vec3(2.0f, 2.0f, 2.0f), glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(0.0f, 0.0f, 1.0f));
	ubo.Proj = glm::perspective(glm::radians(45.0f), (float)window.GetWidth() / (float)window.GetHeight(), 0.1f, 10.0f);
	ubo.Proj[1][1] *= -1;

	UniformBufferObject2 ubo2 = {};
	ubo2.Model = glm::rotate(glm::mat4(1.0f), glm::radians(0.0f), glm::vec3(0.0f, 0.0f, 1.0f));
	ubo2.View = glm::lookAt(glm::vec3(sum, 2.0f, 2.0f), glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(0.0f, 0.0f, 1.0f));
	ubo2.Proj = glm::perspective(glm::radians(45.0f), (float)window.GetWidth() / (float)window.GetHeight(), 0.1f, 10.0f);
	ubo2.Proj[1][1] *= -1;

	BufferManager::SetUniformData(m_UniformBuffersMapped[imageIndex], (void*)(&ubo), sizeof(ubo));
	BufferManager::SetUniformData(m_UniformBuffersMapped2[imageIndex], (void*)(&ubo2), sizeof(ubo2));
}
