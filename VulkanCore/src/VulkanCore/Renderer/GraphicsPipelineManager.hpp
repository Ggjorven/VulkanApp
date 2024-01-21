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

		DescriptorSets DescriptorSets = {};
	};

	class GraphicsPipelineManager;

	class GraphicsPipeline
	{
	public:
		GraphicsPipeline() = default;
		GraphicsPipeline(const PipelineInfo& info);
		void Destroy();

		void Bind(VkCommandBuffer& buffer, VkPipelineBindPoint bindPoint);

		inline VkPipelineLayout& GetPipelineLayout() { return m_PipelineLayout; }
		inline std::vector<VkDescriptorPool>& GetDescriptorPools() { return m_DescriptorPools; }
		inline std::vector<std::vector<VkDescriptorSet>>& GetDescriptorSets() { return m_DescriptorSets; }

	private: // Helper functions
		void CreateDescriptorSetLayout(const PipelineInfo& info);
		void CreateGraphicsPipeline(const PipelineInfo& info);
		void CreateDescriptorPool(const PipelineInfo& info);
		void CreateDescriptorSets(const PipelineInfo& info);

	private:
		VkPipeline m_GraphicsPipeline = VK_NULL_HANDLE;
		VkPipelineLayout m_PipelineLayout = VK_NULL_HANDLE;

		std::vector<VkDescriptorSetLayout> m_DescriptorLayouts = { };
		std::vector<VkDescriptorPool> m_DescriptorPools = { };

		// Note(Jorben): The first index is the index of the descriptor and the second are VKAPP_MAX_FRAMES_INFLIGHT of sets.
		std::vector<std::vector<VkDescriptorSet>> m_DescriptorSets = { };

		friend class GraphicsPipelineManager;
	};

	class GraphicsPipelineManager
	{
	public: // Public functions
		static GraphicsPipelineManager* Get() { return s_Instance; }

		GraphicsPipelineManager();
		void Destroy();

		GraphicsPipeline& GetPipeline(const std::string& id);

		void DestroyPipeline(const std::string& id);
		void DestroyAllPipelines();

		GraphicsPipeline& CreatePipeline(const std::string& id, const PipelineInfo& info);

		inline VkDescriptorPool& GetImGuiPool() { return m_ImGuiDescriptorPool; }
		
		static std::vector<char> ReadFile(const std::filesystem::path& path);
		static VkShaderModule CreateShaderModule(const std::vector<char>& data);

	private: // Initialization functions
		void CreateImGuiDescriptorPool();

	private: // Static things
		static GraphicsPipelineManager* s_Instance;

	private: // Vulkan Data
		VkDescriptorPool m_ImGuiDescriptorPool = VK_NULL_HANDLE;

		std::unordered_map<std::string, GraphicsPipeline> m_GraphicsPipelines = {};

		friend class Renderer;
		friend class InstanceManager;
	};

}