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
	glm::vec3 Position = { };
	glm::vec3 Colour = { };
	glm::vec2 TexCoord = { };

	static VkVertexInputBindingDescription GetBindingDescription()
	{
		VkVertexInputBindingDescription bindingDescription = {};
		bindingDescription.binding = 0;
		bindingDescription.stride = sizeof(Vertex);
		bindingDescription.inputRate = VK_VERTEX_INPUT_RATE_VERTEX;

		return bindingDescription;
	}

	static std::vector<VkVertexInputAttributeDescription> GetAttributeDescriptions()
	{
		std::vector<VkVertexInputAttributeDescription> attributeDescriptions = {};
		attributeDescriptions.resize((size_t)3);

		// Position
		attributeDescriptions[0].binding = 0;
		attributeDescriptions[0].location = 0;
		attributeDescriptions[0].format = VK_FORMAT_R32G32B32_SFLOAT;
		attributeDescriptions[0].offset = offsetof(Vertex, Position);

		// Colour
		attributeDescriptions[1].binding = 0;
		attributeDescriptions[1].location = 1;
		attributeDescriptions[1].format = VK_FORMAT_R32G32B32_SFLOAT;
		attributeDescriptions[1].offset = offsetof(Vertex, Colour);

		// TexCoord
		attributeDescriptions[2].binding = 0;
		attributeDescriptions[2].location = 2;
		attributeDescriptions[2].format = VK_FORMAT_R32G32_SFLOAT;
		attributeDescriptions[2].offset = offsetof(Vertex, TexCoord);

		return attributeDescriptions;
	}
};

const std::vector<Vertex> vertices = {
	{{-0.5f, -0.5f, -0.5f}, {1.0f, 0.0f, 0.0f}, {1.0f, 0.0f}},
	{{0.5f, -0.5f, -0.5f}, {0.0f, 1.0f, 0.0f}, {0.0f, 0.0f}},
	{{0.5f, 0.5f, -0.5f}, {0.0f, 0.0f, 1.0f}, {0.0f, 1.0f}},
	{{-0.5f, 0.5f, -0.5f}, {1.0f, 1.0f, 1.0f}, {1.0f, 1.0f}},

	{{-0.5f, -0.5f, 0.0f}, {1.0f, 0.0f, 0.0f}, {1.0f, 0.0f}},
	{{0.5f, -0.5f, 0.0f}, {0.0f, 1.0f, 0.0f}, {0.0f, 0.0f}},
	{{0.5f, 0.5f, 0.0f}, {0.0f, 0.0f, 1.0f}, {0.0f, 1.0f}},
	{{-0.5f, 0.5f, 0.0f}, {1.0f, 1.0f, 1.0f}, {1.0f, 1.0f}}
};

const std::vector<uint16_t> indices = {
	0, 1, 2, 2, 3, 0,
	4, 5, 6, 6, 7, 4
};

struct UniformBufferObject 
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

	DescriptorInfo imageDescriptor = {};
	imageDescriptor.Binding = 1;
	imageDescriptor.DescriptorCount = 1;
	imageDescriptor.DescriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
	imageDescriptor.StageFlags = VK_SHADER_STAGE_FRAGMENT_BIT;
	info.DescriptorSets.Set0.push_back(imageDescriptor);

	GraphicsPipelineManager::Get()->DestroyCurrentPipeline();
	GraphicsPipelineManager::Get()->CreatePipeline(info);

	BufferManager::CreateVertexBuffer(m_VertexBuffer, m_VertexBufferMemory, (void*)vertices.data(), sizeof(vertices[0]) * vertices.size());
	BufferManager::CreateIndexBuffer(m_IndexBuffer, m_IndexBufferMemory, (void*)indices.data(), sizeof(indices[0]) * indices.size());

	BufferManager::CreateUniformBuffer(m_UniformBuffers, sizeof(UniformBufferObject), m_UniformBuffersMemory, m_UniformBuffersMapped);

	BufferManager::CreateTexture("assets/textures/texture.jpg", m_TextureImage, m_TextureImageMemory);
	m_TextureView = BufferManager::CreateImageView(m_TextureImage, VK_FORMAT_R8G8B8A8_UNORM);
	m_Sampler = BufferManager::CreateSampler();

	// Initialize the descriptor sets/uniforms
	for (size_t i = 0; i < VKAPP_MAX_FRAMES_IN_FLIGHT; i++) 
	{
		// Uniform
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

		// Image
		VkDescriptorImageInfo imageInfo = {};
		imageInfo.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
		imageInfo.imageView = m_TextureView;
		imageInfo.sampler = m_Sampler;

		VkWriteDescriptorSet descriptorWrite2 = {};
		descriptorWrite2.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
		descriptorWrite2.dstSet = GraphicsPipelineManager::Get()->GetDescriptorSets()[0][i];
		descriptorWrite2.dstBinding = 1;
		descriptorWrite2.dstArrayElement = 0;
		descriptorWrite2.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
		descriptorWrite2.descriptorCount = 1;
		descriptorWrite2.pImageInfo = &imageInfo;

		std::vector<VkWriteDescriptorSet> descriptorWrites = { descriptorWrite, descriptorWrite2 };

		vkUpdateDescriptorSets(InstanceManager::Get()->GetLogicalDevice(), static_cast<uint32_t>(descriptorWrites.size()), descriptorWrites.data(), 0, nullptr);
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

	vkDestroySampler(logicalDevice, m_Sampler, nullptr);
	vkDestroyImageView(logicalDevice, m_TextureView, nullptr);

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
	
			vkCmdDrawIndexed(buffer, (uint32_t)indices.size(), 1, 0, 0, 0);
		});
}

void CustomLayer::OnImGuiRender()
{
	ImGui::ShowMetricsWindow();
}

void CustomLayer::OnEvent(Event& e)
{
}

void CustomLayer::UpdateUniformBuffers(float deltaTime, uint32_t imageIndex)
{
	auto& window = Application::Get().GetWindow();

	if (!Application::Get().IsMinimized()) // Note(Jorben): Added this line because, glm::perspective doesn't work if the aspect ratio is 0
	{
		static float sum = 0.0f;
		if (Input::IsKeyPressed(Key::D))
			sum += deltaTime * 1.5f;
		if (Input::IsKeyPressed(Key::A))
			sum -= deltaTime * 1.5f;

		UniformBufferObject ubo = {};
		ubo.Model = glm::rotate(glm::mat4(1.0f), sum * glm::radians(90.0f), glm::vec3(0.0f, 0.0f, 1.0f));
		ubo.View = glm::lookAt(glm::vec3(2.0f, 2.0f, 2.0f), glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(0.0f, 0.0f, 1.0f));
		ubo.Proj = glm::perspective(glm::radians(45.0f), (float)window.GetWidth() / (float)window.GetHeight(), 0.1f, 10.0f);
		ubo.Proj[1][1] *= -1;

		BufferManager::SetUniformData(m_UniformBuffersMapped[imageIndex], (void*)(&ubo), sizeof(ubo));
	}
}
