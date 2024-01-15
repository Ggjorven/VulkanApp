#pragma once

#include <vector>
#include <array>
#include <filesystem>

#include <vulkan/vulkan.h>

#include <glm/glm.hpp>

namespace VkApp
{

	class Renderer;
	class IntanceManager;

	struct DescriptorInfo
	{
	public:
		uint32_t Binding = 0;
		VkDescriptorType DescriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
		uint32_t DescriptorCount = 1;
		VkShaderStageFlagBits StageFlags = VK_SHADER_STAGE_VERTEX_BIT;
	};

	struct PipelineInfo
	{
	public:
		std::vector<char> VertexShader = { };
		std::vector<char> FragmentShader = { };

		VkVertexInputBindingDescription VertexBindingDescription = {};
		std::array<VkVertexInputAttributeDescription, 2> VertexAttributeDescriptions = { };

		// Note(Jorben): Only supports 1 descriptor so far. // TODO(Jorben)
		std::vector<DescriptorInfo> Descriptors = { };
	};

	class GraphicsPipelineManager
	{
	public: // Public functions
		static GraphicsPipelineManager* Get() { return s_Instance; }

		GraphicsPipelineManager();
		void Destroy();
		void DestroyCurrentPipeline();

		static std::vector<char> ReadFile(const std::filesystem::path& path);
		static VkShaderModule CreateShaderModule(const std::vector<char>& data);
		
		void CreatePipeline(const PipelineInfo& info);

		inline std::vector<VkDescriptorSet>& GetDescriptorSets() { return m_DescriptorSets; }
		inline VkPipelineLayout& GetPipelineLayout() { return m_PipelineLayout; }

	private: // Initialization functions

	private: // Helper functions
		void CreateDescriptorSetLayout(const PipelineInfo& info);
		void CreateGraphicsPipeline(const PipelineInfo& info);
		void CreateDescriptorPool(const PipelineInfo& info);
		void CreateDescriptorSets(const PipelineInfo& info);

	private: // Static things
		static GraphicsPipelineManager* s_Instance;

	private: // Vulkan Data
		VkPipeline m_GraphicsPipeline = VK_NULL_HANDLE;
		VkPipelineLayout m_PipelineLayout = VK_NULL_HANDLE;

		VkDescriptorSetLayout m_DescriptorLayout = VK_NULL_HANDLE;

		VkDescriptorPool m_DescriptorPool = VK_NULL_HANDLE;
		std::vector<VkDescriptorSet> m_DescriptorSets = { };

		friend class Renderer;
		friend class InstanceManager;
	};

}