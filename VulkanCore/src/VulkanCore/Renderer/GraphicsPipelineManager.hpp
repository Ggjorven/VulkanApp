#pragma once

#include <vector>
#include <set>
#include <unordered_set>
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

	struct DescriptorSets
	{
	public:
		std::vector<DescriptorInfo> Set0 = { };
		std::vector<DescriptorInfo> Set1 = { };
		std::vector<DescriptorInfo> Set2 = { };
		std::vector<DescriptorInfo> Set3 = { };

		static std::unordered_set<VkDescriptorType> GetUniqueTypes(const std::vector<DescriptorInfo>& descriptors);
		static uint32_t AmountOf(VkDescriptorType type, const std::vector<DescriptorInfo>& descriptors);

		static VkDescriptorSetLayout GetDescriptorSetLayout(const std::vector<DescriptorInfo>& descriptors);
		static VkDescriptorPool CreatePool(const std::vector<DescriptorInfo>& descriptors);
		static std::vector<VkDescriptorSet> CreateDescriptorSets(VkDescriptorSetLayout& layout, VkDescriptorPool& pool, const std::vector<DescriptorInfo>& descriptors);
	};

	struct PipelineInfo
	{
	public:
		std::vector<char> VertexShader = { };
		std::vector<char> FragmentShader = { };

		VkVertexInputBindingDescription VertexBindingDescription = {};
		std::vector<VkVertexInputAttributeDescription> VertexAttributeDescriptions = { };

		// Note(Jorben): Only supports 1 descriptor so far. // TODO(Jorben)
		DescriptorSets DescriptorSets = {};
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

		inline std::vector<VkDescriptorPool>& GetDescriptorPools() { return m_DescriptorPools; }
		inline std::vector<std::vector<VkDescriptorSet>>& GetDescriptorSets() { return m_DescriptorSets; }
		inline VkPipelineLayout& GetPipelineLayout() { return m_PipelineLayout; }

		inline VkDescriptorPool& GetImGuiPool() { return m_ImGuiDescriptorPool; }

	private: // Initialization functions

	private: // Helper functions
		void CreateDescriptorSetLayout(const PipelineInfo& info);
		void CreateGraphicsPipeline(const PipelineInfo& info);
		void CreateDescriptorPool(const PipelineInfo& info);
		void CreateDescriptorSets(const PipelineInfo& info);

		void CreateImGuiDescriptorPool();

	private: // Static things
		static GraphicsPipelineManager* s_Instance;

	private: // Vulkan Data
		VkPipeline m_GraphicsPipeline = VK_NULL_HANDLE;
		VkPipelineLayout m_PipelineLayout = VK_NULL_HANDLE;

		std::vector<VkDescriptorSetLayout> m_DescriptorLayouts = { };
		std::vector<VkDescriptorPool> m_DescriptorPools = { };
		// Note(Jorben): The first index is the index of the descriptor and the second are VKAPP_MAX_FRAMES_INFLIGHT of sets.
		std::vector<std::vector<VkDescriptorSet>> m_DescriptorSets = { };

		VkDescriptorPool m_ImGuiDescriptorPool = VK_NULL_HANDLE;

		friend class Renderer;
		friend class InstanceManager;
	};

}