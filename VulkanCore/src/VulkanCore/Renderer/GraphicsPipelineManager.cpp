#include "vcpch.h"
#include "GraphicsPipelineManager.hpp"

#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>

#include "VulkanCore/Core/Logging.hpp"

#include "VulkanCore/Core/Application.hpp"

#include "VulkanCore/Renderer/Renderer.hpp"
#include "VulkanCore/Renderer/InstanceManager.hpp" // For the retrieval of the logical device
#include "VulkanCore/Renderer/SwapChainManager.hpp"

namespace VkApp
{

	// ===================================
	// ------------ Static ---------------
	// ===================================
	GraphicsPipelineManager* GraphicsPipelineManager::s_Instance = nullptr;

	static InstanceManager* s_InstanceManager = nullptr;

	// ===================================
	// ------------ Public ---------------
	// ===================================
	GraphicsPipelineManager::GraphicsPipelineManager()
	{
		// Retrieve the InstanceManager, so we can use it in our code
		s_Instance = this;
		s_InstanceManager = InstanceManager::Get();

		// Note(Jorben): There is not default graphics pipeline created, this has to be done manually.
	}

	void GraphicsPipelineManager::Destroy()
	{
		DestroyCurrentPipeline();

		s_InstanceManager = nullptr;
		s_Instance = nullptr;
	}

	void GraphicsPipelineManager::DestroyCurrentPipeline()
	{
		vkDeviceWaitIdle(s_InstanceManager->m_Device);

		vkDestroyPipeline(s_InstanceManager->m_Device, m_GraphicsPipeline, nullptr);
		vkDestroyPipelineLayout(s_InstanceManager->m_Device, m_PipelineLayout, nullptr);

		vkDestroyDescriptorPool(s_InstanceManager->m_Device, m_DescriptorPool, nullptr);
		vkDestroyDescriptorSetLayout(s_InstanceManager->m_Device, m_DescriptorLayout, nullptr);
	}

	std::vector<char> GraphicsPipelineManager::ReadFile(const std::filesystem::path& path)
	{
		std::ifstream file(path, std::ios::ate | std::ios::binary);

		if (!file.is_open() || !file.good())
			VKAPP_LOG_ERROR("Failed to open file!");

		size_t fileSize = (size_t)file.tellg();
		std::vector<char> buffer(fileSize);

		file.seekg(0);
		file.read(buffer.data(), fileSize);

		file.close();
		return buffer;
	}

	VkShaderModule GraphicsPipelineManager::CreateShaderModule(const std::vector<char>& data)
	{
		VkShaderModuleCreateInfo createInfo = {};
		createInfo.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
		createInfo.codeSize = data.size();
		createInfo.pCode = reinterpret_cast<const uint32_t*>(data.data());

		VkShaderModule shaderModule = {};
		if (vkCreateShaderModule(s_InstanceManager->m_Device, &createInfo, nullptr, &shaderModule) != VK_SUCCESS)
			VKAPP_LOG_ERROR("Failed to create shader module!");

		return shaderModule;
	}

	void GraphicsPipelineManager::CreatePipeline(const PipelineInfo& info)
	{
		CreateDescriptorSetLayout(info);
		CreateGraphicsPipeline(info);
		CreateDescriptorPool(info);
		CreateDescriptorSets(info);
	}

	// ===================================
	// -------- Initialization -----------
	// ===================================
	


	// ===================================
	// ------------ Helper ---------------
	// ===================================
	void GraphicsPipelineManager::CreateDescriptorSetLayout(const PipelineInfo& info)
	{
		std::vector<VkDescriptorSetLayoutBinding> layouts = { };

		for (auto& descriptor : info.Descriptors)
		{
			VkDescriptorSetLayoutBinding uboLayoutBinding = {};
			uboLayoutBinding.binding = descriptor.Binding;
			uboLayoutBinding.descriptorType = descriptor.DescriptorType;
			uboLayoutBinding.descriptorCount = descriptor.DescriptorCount;
			uboLayoutBinding.stageFlags = descriptor.StageFlags;
			uboLayoutBinding.pImmutableSamplers = nullptr; // Optional

			layouts.push_back(uboLayoutBinding);
		}

		VkDescriptorSetLayoutCreateInfo layoutInfo = {};
		layoutInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
		layoutInfo.bindingCount = static_cast<uint32_t>(info.Descriptors.size());
		layoutInfo.pBindings = layouts.data();

		if (vkCreateDescriptorSetLayout(s_InstanceManager->m_Device, &layoutInfo, nullptr, &m_DescriptorLayout) != VK_SUCCESS)
			VKAPP_LOG_ERROR("Failed to create descriptor set layout!");
	}

	void GraphicsPipelineManager::CreateGraphicsPipeline(const PipelineInfo& info)
	{
		VkShaderModule vertShaderModule = CreateShaderModule(info.VertexShader);
		VkShaderModule fragShaderModule = CreateShaderModule(info.FragmentShader);

		// Vertex shader info
		VkPipelineShaderStageCreateInfo vertShaderStageInfo = {};
		vertShaderStageInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
		vertShaderStageInfo.stage = VK_SHADER_STAGE_VERTEX_BIT;
		vertShaderStageInfo.module = vertShaderModule;
		vertShaderStageInfo.pName = "main";

		// Fragment shader info
		VkPipelineShaderStageCreateInfo fragShaderStageInfo = {};
		fragShaderStageInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
		fragShaderStageInfo.stage = VK_SHADER_STAGE_FRAGMENT_BIT;
		fragShaderStageInfo.module = fragShaderModule;
		fragShaderStageInfo.pName = "main";

		// PipelineShader info
		VkPipelineShaderStageCreateInfo shaderStages[2] = { vertShaderStageInfo, fragShaderStageInfo };

		auto bindingDescription = info.VertexBindingDescription;
		auto attributeDescriptions = info.VertexAttributeDescriptions;

		VkPipelineVertexInputStateCreateInfo vertexInputInfo = {};
		vertexInputInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;
		vertexInputInfo.vertexBindingDescriptionCount = 1;
		vertexInputInfo.vertexAttributeDescriptionCount = static_cast<uint32_t>(attributeDescriptions.size());
		vertexInputInfo.pVertexBindingDescriptions = &bindingDescription;
		vertexInputInfo.pVertexAttributeDescriptions = attributeDescriptions.data();

		VkPipelineInputAssemblyStateCreateInfo inputAssembly = {};
		inputAssembly.sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO;
		inputAssembly.topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;
		inputAssembly.primitiveRestartEnable = VK_FALSE;

		VkPipelineViewportStateCreateInfo viewportState = {};
		viewportState.sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO;
		viewportState.viewportCount = 1;
		viewportState.scissorCount = 1;

		VkPipelineRasterizationStateCreateInfo rasterizer = {};
		rasterizer.sType = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO;
		rasterizer.depthClampEnable = VK_FALSE;
		rasterizer.rasterizerDiscardEnable = VK_FALSE;
		rasterizer.polygonMode = VK_POLYGON_MODE_FILL;
		rasterizer.lineWidth = 1.0f;
		rasterizer.cullMode = VK_CULL_MODE_BACK_BIT;
		rasterizer.frontFace = VK_FRONT_FACE_COUNTER_CLOCKWISE;
		rasterizer.depthBiasEnable = VK_FALSE;

		VkPipelineMultisampleStateCreateInfo multisampling = {};
		multisampling.sType = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO;
		multisampling.sampleShadingEnable = VK_FALSE;
		multisampling.rasterizationSamples = VK_SAMPLE_COUNT_1_BIT;

		VkPipelineColorBlendAttachmentState colorBlendAttachment = {};
		colorBlendAttachment.colorWriteMask = VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT | VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT;
		colorBlendAttachment.blendEnable = VK_FALSE;

		VkPipelineColorBlendStateCreateInfo colorBlending = {};
		colorBlending.sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO;
		colorBlending.logicOpEnable = VK_FALSE;
		colorBlending.logicOp = VK_LOGIC_OP_COPY;
		colorBlending.attachmentCount = 1;
		colorBlending.pAttachments = &colorBlendAttachment;
		colorBlending.blendConstants[0] = 0.0f;
		colorBlending.blendConstants[1] = 0.0f;
		colorBlending.blendConstants[2] = 0.0f;
		colorBlending.blendConstants[3] = 0.0f;

		std::vector<VkDynamicState> dynamicStates = {
			VK_DYNAMIC_STATE_VIEWPORT,
			VK_DYNAMIC_STATE_SCISSOR
		};

		VkPipelineDynamicStateCreateInfo dynamicState = {};
		dynamicState.sType = VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO;
		dynamicState.dynamicStateCount = static_cast<uint32_t>(dynamicStates.size());
		dynamicState.pDynamicStates = dynamicStates.data();

		VkPipelineLayoutCreateInfo pipelineLayoutInfo = {};
		pipelineLayoutInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
		pipelineLayoutInfo.pushConstantRangeCount = 0;
		pipelineLayoutInfo.setLayoutCount = 1;					// TODO(Jorben): Remove?
		pipelineLayoutInfo.pSetLayouts = &m_DescriptorLayout;	// TODO(Jorben): Remove?

		if (vkCreatePipelineLayout(s_InstanceManager->m_Device, &pipelineLayoutInfo, nullptr, &m_PipelineLayout) != VK_SUCCESS)
			VKAPP_LOG_ERROR("Failed to create pipeline layout!");

		// Create the actual graphics pipeline (where we actually use the shaders and other info)
		VkGraphicsPipelineCreateInfo pipelineInfo = {};
		pipelineInfo.sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO;
		pipelineInfo.stageCount = 2;
		pipelineInfo.pStages = shaderStages;
		pipelineInfo.pVertexInputState = &vertexInputInfo;
		pipelineInfo.pInputAssemblyState = &inputAssembly;
		pipelineInfo.pViewportState = &viewportState;
		pipelineInfo.pRasterizationState = &rasterizer;
		pipelineInfo.pMultisampleState = &multisampling;
		pipelineInfo.pDepthStencilState = nullptr; // Optional
		pipelineInfo.pColorBlendState = &colorBlending;
		pipelineInfo.pDynamicState = &dynamicState;
		pipelineInfo.layout = m_PipelineLayout;
		pipelineInfo.renderPass = SwapChainManager::Get()->GetRenderPass();
		pipelineInfo.subpass = 0;
		pipelineInfo.basePipelineHandle = VK_NULL_HANDLE; // Optional
		pipelineInfo.basePipelineIndex = -1; // Optional

		if (vkCreateGraphicsPipelines(s_InstanceManager->m_Device, VK_NULL_HANDLE, 1, &pipelineInfo, nullptr, &m_GraphicsPipeline) != VK_SUCCESS)
			VKAPP_LOG_ERROR("Failed to create graphics pipeline!");

		// Destroy at the end
		vkDestroyShaderModule(s_InstanceManager->m_Device, fragShaderModule, nullptr);
		vkDestroyShaderModule(s_InstanceManager->m_Device, vertShaderModule, nullptr);
	}

	void GraphicsPipelineManager::CreateDescriptorPool(const PipelineInfo& info)
	{
		VkDescriptorPoolSize poolSizeUniforms = {};
		poolSizeUniforms.type = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
		poolSizeUniforms.descriptorCount = info.Descriptors.size() * VKAPP_MAX_FRAMES_IN_FLIGHT; // Means we can have this many uniform buffers 

		//VkDescriptorPoolSize poolSize = {};
		//poolSize.type = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
		//poolSize.descriptorCount = 2;

		std::vector<VkDescriptorPoolSize> poolSizes = { poolSizeUniforms };

		VkDescriptorPoolCreateInfo poolInfo = {};
		poolInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
		poolInfo.poolSizeCount = static_cast<uint32_t>(poolSizes.size());
		poolInfo.pPoolSizes = poolSizes.data();
		poolInfo.maxSets = static_cast<uint32_t>(VKAPP_MAX_FRAMES_IN_FLIGHT); // Amount of sets?

		if (vkCreateDescriptorPool(s_InstanceManager->m_Device, &poolInfo, nullptr, &m_DescriptorPool) != VK_SUCCESS)
			VKAPP_LOG_ERROR("Failed to create descriptor pool!");
	}

	void GraphicsPipelineManager::CreateDescriptorSets(const PipelineInfo& info)
	{
		std::vector<VkDescriptorSetLayout> layouts(VKAPP_MAX_FRAMES_IN_FLIGHT, m_DescriptorLayout);

		VkDescriptorSetAllocateInfo allocInfo = {};
		allocInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
		allocInfo.descriptorPool = m_DescriptorPool;
		allocInfo.descriptorSetCount = static_cast<uint32_t>(VKAPP_MAX_FRAMES_IN_FLIGHT);
		allocInfo.pSetLayouts = layouts.data();

		m_DescriptorSets.resize(VKAPP_MAX_FRAMES_IN_FLIGHT);

		if (vkAllocateDescriptorSets(s_InstanceManager->m_Device, &allocInfo, m_DescriptorSets.data()) != VK_SUCCESS)
			VKAPP_LOG_ERROR("Failed to allocate descriptor sets!");
	}

}