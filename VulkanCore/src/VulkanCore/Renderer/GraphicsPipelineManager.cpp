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

	std::unordered_set<VkDescriptorType> DescriptorSets::GetUniqueTypes(const std::vector<DescriptorInfo>& descriptors)
	{
		std::unordered_set<VkDescriptorType> unique = {};

		for (auto& descriptor : descriptors)
		{
			unique.insert(descriptor.DescriptorType);
		}

		return unique;
	}

	uint32_t DescriptorSets::AmountOf(VkDescriptorType type, const std::vector<DescriptorInfo>& descriptors)
	{
		uint32_t count = 0;

		for (auto& descriptor : descriptors)
		{
			if (descriptor.DescriptorType == type)
				count++;
		}

		return count;
	}

	VkDescriptorSetLayout DescriptorSets::GetDescriptorSetLayout(const std::vector<DescriptorInfo>& descriptors)
	{
		std::vector<VkDescriptorSetLayoutBinding> layouts = { };

		for (auto& descriptor : descriptors)
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
		layoutInfo.bindingCount = static_cast<uint32_t>(descriptors.size());
		layoutInfo.pBindings = layouts.data();

		VkDescriptorSetLayout descriptorSetLayout = VK_NULL_HANDLE;
		if (vkCreateDescriptorSetLayout(s_InstanceManager->GetLogicalDevice(), &layoutInfo, nullptr, &descriptorSetLayout) != VK_SUCCESS)
			VKAPP_LOG_ERROR("Failed to create descriptor set layout!");

		return descriptorSetLayout;
	}

	VkDescriptorPool DescriptorSets::CreatePool(const std::vector<DescriptorInfo>& descriptors)
	{
		std::vector<VkDescriptorPoolSize> poolSizes = { };
		poolSizes.resize(DescriptorSets::GetUniqueTypes(descriptors).size());
		poolSizes.clear(); // Note(Jorben): For some reason without this line there is a VK_SAMPLER or something in the list.

		for (auto& type : DescriptorSets::GetUniqueTypes(descriptors))
		{
			VkDescriptorPoolSize poolSize = {};
			poolSize.type = type;
			poolSize.descriptorCount = DescriptorSets::AmountOf(type, descriptors) * VKAPP_MAX_FRAMES_IN_FLIGHT;

			poolSizes.push_back(poolSize);
		}

		VkDescriptorPoolCreateInfo poolInfo = {};
		poolInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
		poolInfo.poolSizeCount = static_cast<uint32_t>(poolSizes.size());
		poolInfo.pPoolSizes = poolSizes.data();
		poolInfo.maxSets = static_cast<uint32_t>(VKAPP_MAX_FRAMES_IN_FLIGHT); // Amount of sets?

		VkDescriptorPool pool = VK_NULL_HANDLE;
		if (vkCreateDescriptorPool(s_InstanceManager->GetLogicalDevice(), &poolInfo, nullptr, &pool) != VK_SUCCESS)
			VKAPP_LOG_ERROR("Failed to create descriptor pool!");

		return pool;
	}

	std::vector<VkDescriptorSet> DescriptorSets::CreateDescriptorSets(VkDescriptorSetLayout& layout, VkDescriptorPool& pool, const std::vector<DescriptorInfo>& descriptors)
	{
		std::vector<VkDescriptorSetLayout> layouts(VKAPP_MAX_FRAMES_IN_FLIGHT, layout);

		VkDescriptorSetAllocateInfo allocInfo = {};
		allocInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
		allocInfo.descriptorPool = pool;
		allocInfo.descriptorSetCount = static_cast<uint32_t>(VKAPP_MAX_FRAMES_IN_FLIGHT);
		allocInfo.pSetLayouts = layouts.data();

		std::vector<VkDescriptorSet> descriptorSets = { };
		descriptorSets.resize(VKAPP_MAX_FRAMES_IN_FLIGHT);

		if (vkAllocateDescriptorSets(s_InstanceManager->GetLogicalDevice(), &allocInfo, descriptorSets.data()) != VK_SUCCESS)
			VKAPP_LOG_ERROR("Failed to allocate descriptor sets!");

		return descriptorSets;
	}

	// ===================================
	// ------------ Public ---------------
	// ===================================
	GraphicsPipelineManager::GraphicsPipelineManager()
	{
		// Retrieve the InstanceManager, so we can use it in our code
		s_Instance = this;
		s_InstanceManager = InstanceManager::Get();

		CreateImGuiDescriptorPool(); // For ImGui

		// Note(Jorben): There is not default graphics pipeline created, this has to be done manually.
	}

	void GraphicsPipelineManager::Destroy()
	{
		DestroyCurrentPipeline();

		vkDestroyDescriptorPool(s_InstanceManager->m_Device, m_ImGuiDescriptorPool, nullptr);

		s_InstanceManager = nullptr;
		s_Instance = nullptr;
	}

	void GraphicsPipelineManager::DestroyCurrentPipeline()
	{
		vkDeviceWaitIdle(s_InstanceManager->m_Device);

		vkDestroyPipeline(s_InstanceManager->m_Device, m_GraphicsPipeline, nullptr);
		vkDestroyPipelineLayout(s_InstanceManager->m_Device, m_PipelineLayout, nullptr);

		for (auto& pool : m_DescriptorPools)
		{
			vkDestroyDescriptorPool(s_InstanceManager->m_Device, pool, nullptr);
		}

		for (auto& layout : m_DescriptorLayouts)
		{
			vkDestroyDescriptorSetLayout(s_InstanceManager->m_Device, layout, nullptr);
		}
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
		if (!info.DescriptorSets.Set0.empty())
			m_DescriptorLayouts.push_back(DescriptorSets::GetDescriptorSetLayout(info.DescriptorSets.Set0));
		if (!info.DescriptorSets.Set1.empty())
			m_DescriptorLayouts.push_back(DescriptorSets::GetDescriptorSetLayout(info.DescriptorSets.Set1));
		if (!info.DescriptorSets.Set2.empty())
			m_DescriptorLayouts.push_back(DescriptorSets::GetDescriptorSetLayout(info.DescriptorSets.Set2));
		if (!info.DescriptorSets.Set3.empty())
			m_DescriptorLayouts.push_back(DescriptorSets::GetDescriptorSetLayout(info.DescriptorSets.Set3));
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
		colorBlendAttachment.blendEnable = VK_FALSE; // Note(Jorben): Set true for transparancy

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
		pipelineLayoutInfo.setLayoutCount = static_cast<uint32_t>(m_DescriptorLayouts.size());					// TODO(Jorben): Remove?
		pipelineLayoutInfo.pSetLayouts = m_DescriptorLayouts.data();	// TODO(Jorben): Remove?

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
		if (!info.DescriptorSets.Set0.empty())
			m_DescriptorPools.push_back(DescriptorSets::CreatePool(info.DescriptorSets.Set0));
		if (!info.DescriptorSets.Set1.empty())
			m_DescriptorPools.push_back(DescriptorSets::CreatePool(info.DescriptorSets.Set1));
		if (!info.DescriptorSets.Set2.empty())
			m_DescriptorPools.push_back(DescriptorSets::CreatePool(info.DescriptorSets.Set2));
		if (!info.DescriptorSets.Set3.empty())
			m_DescriptorPools.push_back(DescriptorSets::CreatePool(info.DescriptorSets.Set3));
	}

	void GraphicsPipelineManager::CreateDescriptorSets(const PipelineInfo& info)
	{
		if (!info.DescriptorSets.Set0.empty())
			m_DescriptorSets.push_back(DescriptorSets::CreateDescriptorSets(m_DescriptorLayouts[0], m_DescriptorPools[0], info.DescriptorSets.Set0));
		if (!info.DescriptorSets.Set1.empty())
			m_DescriptorSets.push_back(DescriptorSets::CreateDescriptorSets(m_DescriptorLayouts[1], m_DescriptorPools[1], info.DescriptorSets.Set1));
		if (!info.DescriptorSets.Set2.empty())
			m_DescriptorSets.push_back(DescriptorSets::CreateDescriptorSets(m_DescriptorLayouts[2], m_DescriptorPools[2], info.DescriptorSets.Set2));
		if (!info.DescriptorSets.Set3.empty())
			m_DescriptorSets.push_back(DescriptorSets::CreateDescriptorSets(m_DescriptorLayouts[3], m_DescriptorPools[3], info.DescriptorSets.Set3));
	}

	void GraphicsPipelineManager::CreateImGuiDescriptorPool()
	{
		std::vector<VkDescriptorPoolSize> poolSizes =
		{
			{ VK_DESCRIPTOR_TYPE_SAMPLER, 10 },
			{ VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, 50 }, // Important for ImGui
			{ VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE, 10 },
			{ VK_DESCRIPTOR_TYPE_STORAGE_IMAGE, 10 },
			{ VK_DESCRIPTOR_TYPE_UNIFORM_TEXEL_BUFFER, 10 },
			{ VK_DESCRIPTOR_TYPE_STORAGE_TEXEL_BUFFER, 10 },
			{ VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, 10 },
			{ VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, 10 },
			{ VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC, 10 },
			{ VK_DESCRIPTOR_TYPE_STORAGE_BUFFER_DYNAMIC, 10 },
			{ VK_DESCRIPTOR_TYPE_INPUT_ATTACHMENT, 10 }
		};

		VkDescriptorPoolCreateInfo poolInfo = {};
		poolInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
		poolInfo.poolSizeCount = static_cast<uint32_t>(poolSizes.size());
		poolInfo.pPoolSizes = poolSizes.data();
		poolInfo.maxSets = static_cast<uint32_t>(VKAPP_MAX_FRAMES_IN_FLIGHT); // Amount of sets?

		if (vkCreateDescriptorPool(s_InstanceManager->GetLogicalDevice(), &poolInfo, nullptr, &m_ImGuiDescriptorPool) != VK_SUCCESS)
			VKAPP_LOG_ERROR("Failed to create descriptor pool!");
	}

}