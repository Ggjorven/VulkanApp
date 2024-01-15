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

	// Note(Jorben): For the default graphics pipeline
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

	// TODO(Jorben): Create a nice way to create a new pipeline with descriptor sets and everything.
	class GraphicsPipelineManager
	{
	public: // Public functions
		static GraphicsPipelineManager* Get() { return s_Instance; }

		GraphicsPipelineManager();
		void Destroy();

		void DestroyCurrentPipeline();

		static std::vector<char> ReadFile(const std::filesystem::path& path);
		VkShaderModule CreateShaderModule(const std::vector<char>& data);

		inline std::vector<VkDescriptorSet>& GetDescriptorSets() { return m_DescriptorSets; }
		inline VkPipelineLayout& GetPipelineLayout() { return m_PipelineLayout; }

	private: // Initialization functions
		void CreateDescriptorSetLayout(); // TODO(Jorben): Remove this default descriptor set?
		void CreateGraphicsPipeline();
		void CreateDescriptorPool();
		void CreateDescriptorSets();

	private: // Helper functions

	private: // Static things
		static GraphicsPipelineManager* s_Instance;

	private: // Vulkan Data
		VkPipeline m_GraphicsPipeline = VK_NULL_HANDLE;
		VkPipelineLayout m_PipelineLayout = VK_NULL_HANDLE;

		// TODO(Jorben): Remove this
		VkDescriptorSetLayout m_DescriptorLayout = VK_NULL_HANDLE;

		VkDescriptorPool m_DescriptorPool;
		std::vector<VkDescriptorSet> m_DescriptorSets;

		friend class Renderer;
		friend class InstanceManager;
	};

}