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
#include <glm/gtc/type_ptr.hpp>

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
	info.VertexBindingDescription = MeshVertex::GetBindingDescription();
	info.VertexAttributeDescriptions = MeshVertex::GetAttributeDescriptions();

	DescriptorInfo defaultDescriptor = {};
	defaultDescriptor.Binding = 0;
	info.DescriptorSets.Set0.push_back(defaultDescriptor);

	DescriptorInfo imageDescriptor = {};
	imageDescriptor.Binding = 1;
	imageDescriptor.DescriptorCount = 1;
	imageDescriptor.DescriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
	imageDescriptor.StageFlags = VK_SHADER_STAGE_FRAGMENT_BIT;
	info.DescriptorSets.Set0.push_back(imageDescriptor);

	m_Pipeline = GraphicsPipelineManager::Get()->CreatePipeline("My Pipeline", info);

	m_Mesh = Mesh("assets/objects/Cat.obj");

	BufferManager::CreateUniformBuffer(m_UniformBuffers, sizeof(UniformBufferObject), m_UniformBuffersMemory, m_UniformBuffersMapped);

	uint32_t mipLevels = 0;
	BufferManager::CreateTexture("assets/objects/Cat_diffuse.jpg", m_TextureImage, m_TextureImageMemory, mipLevels);
	m_TextureView = BufferManager::CreateImageView(m_TextureImage, VK_FORMAT_R8G8B8A8_UNORM, VK_IMAGE_ASPECT_COLOR_BIT, mipLevels);
	m_Sampler = BufferManager::CreateSampler(mipLevels);

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
		descriptorWrite.dstSet = m_Pipeline.GetDescriptorSets()[0][i];
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
		descriptorWrite2.dstSet = m_Pipeline.GetDescriptorSets()[0][i];
		descriptorWrite2.dstBinding = 1;
		descriptorWrite2.dstArrayElement = 0;
		descriptorWrite2.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
		descriptorWrite2.descriptorCount = 1;
		descriptorWrite2.pImageInfo = &imageInfo;

		std::vector<VkWriteDescriptorSet> descriptorWrites = { descriptorWrite, descriptorWrite2 };

		vkUpdateDescriptorSets(InstanceManager::Get()->GetLogicalDevice(), static_cast<uint32_t>(descriptorWrites.size()), descriptorWrites.data(), 0, nullptr);
	}

	auto& window = Application::Get().GetWindow();

	m_Camera.SetAspectRatio((float)window.GetWidth() / (float)window.GetHeight());
	m_Camera.SetPosition(glm::vec3(0.0f, 35.0f, 45.0f));

	m_Camera.GetCameraSettings().Yaw = 270.0f;
	m_Camera.GetCameraSettings().Pitch = -15.0f;
}

void CustomLayer::OnDetach()
{
	auto logicalDevice = InstanceManager::Get()->GetLogicalDevice();

	vkDeviceWaitIdle(logicalDevice);

	m_Mesh.Destroy();

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

	static float timer = 0.0f;
	timer += deltaTime;
	if (timer > 0.5f)
	{
		ImGuiIO& io = ImGui::GetIO();

		std::string title = fmt::format("[VulkanApp] Running at {} FPS", (int)io.Framerate);
		Application::Get().GetWindow().SetTitle(title.c_str());

		timer = 0.0f;
	}
}

void CustomLayer::OnRender()
{
	Renderer::AddToQueue([this](VkCommandBuffer& buffer, uint32_t imageIndex)
		{
			std::vector<VkDeviceSize> offsets = { {0} };
			uint32_t currentFrame = Renderer::Get()->GetCurrentImage();

			m_Pipeline.Bind(buffer, VK_PIPELINE_BIND_POINT_GRAPHICS);

			vkCmdBindVertexBuffers(buffer, 0, 1, &m_Mesh.GetVertexBuffer(), offsets.data());
			vkCmdBindIndexBuffer(buffer, m_Mesh.GetIndexBuffer(), 0, VK_INDEX_TYPE_UINT32);

			vkCmdBindDescriptorSets(buffer, VK_PIPELINE_BIND_POINT_GRAPHICS, m_Pipeline.GetPipelineLayout(), 0, 1, &m_Pipeline.GetDescriptorSets()[0][currentFrame], 0, nullptr);
	
			vkCmdDrawIndexed(buffer, m_Mesh.GetAmountOfIndices(), 1, 0, 0, 0);
		});
}

void CustomLayer::OnImGuiRender()
{
	ImGui::Begin("Camera Settings");

	ImGui::DragFloat("FOV", &m_Camera.GetCameraSettings().FOV);
	ImGui::Spacing();
	ImGui::DragFloat3("Position", glm::value_ptr(m_Camera.GetPosition()), 1.0f);
	ImGui::DragFloat("Yaw", &m_Camera.GetCameraSettings().Yaw);
	ImGui::DragFloat("Pitch", &m_Camera.GetCameraSettings().Pitch);
	ImGui::Spacing();
	ImGui::DragFloat("Speed", &m_Camera.GetSpeed(), 0.2f);

	ImGui::End();
}

void CustomLayer::OnEvent(Event& e)
{
	m_Camera.OnEvent(e);
}

void CustomLayer::UpdateUniformBuffers(float deltaTime, uint32_t imageIndex)
{
	auto& window = Application::Get().GetWindow();

	if (!Application::Get().IsMinimized()) // Note(Jorben): Added this line because, glm::perspective doesn't work if the aspect ratio is 0
	{
		m_Camera.OnUpdate(deltaTime);

		static float sum = 0.0f;
		sum += deltaTime;

		UniformBufferObject ubo = {};
		ubo.Model = glm::rotate(glm::mat4(1.0f), glm::radians(270.0f), glm::vec3(1.0f, 0.0f, 0.0f));

		ubo.View = m_Camera.GetViewMatrix();

		ubo.Proj = m_Camera.GetProjectionMatrix();
		ubo.Proj[1][1] *= -1;

		BufferManager::SetUniformData(m_UniformBuffersMapped[imageIndex], (void*)(&ubo), sizeof(ubo));
	}
}
