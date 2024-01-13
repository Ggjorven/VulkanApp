#pragma once

#include <vector>
#include <array>
#include <optional>
#include <filesystem>

#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>
#include <vulkan/vulkan.h>

// We use this for the default graphics pipeline
#include <glm/glm.hpp>

namespace VkApp
{

	struct QueueFamilyIndices
	{
	public:
		std::optional<uint32_t> GraphicsFamily;
		std::optional<uint32_t> PresentFamily;

	public:
		bool IsComplete() const
		{
			return GraphicsFamily.has_value() && PresentFamily.has_value();
		}
	};

	struct SwapChainSupportDetails
	{
	public:
		VkSurfaceCapabilitiesKHR Capabilities;
		std::vector<VkSurfaceFormatKHR> Formats;
		std::vector<VkPresentModeKHR> PresentModes;
	};

	struct VulkanInfo
	{
	public:
		VkInstance Instance = VK_NULL_HANDLE;
		VkDebugUtilsMessengerEXT DebugMessenger = VK_NULL_HANDLE;
		VkSurfaceKHR Surface = VK_NULL_HANDLE;

		VkPhysicalDevice PhysicalDevice = VK_NULL_HANDLE;
		VkDevice LogicalDevice = VK_NULL_HANDLE;

		VkSwapchainKHR SwapChain = VK_NULL_HANDLE;
		VkFormat SwapChainImageFormat = VkFormat::VK_FORMAT_UNDEFINED;
		VkExtent2D SwapChainExtent = { };

		std::vector<VkImage> SwapChainImages = { };
		std::vector<VkImageView> SwapChainImageViews = { };
		std::vector<VkFramebuffer> SwapChainFramebuffers = { };

		VkQueue GraphicsQueue = VK_NULL_HANDLE;
		VkQueue PresentQueue = VK_NULL_HANDLE;

		VkRenderPass RenderPass = VK_NULL_HANDLE;
		VkPipeline GraphicsPipeline = VK_NULL_HANDLE;
		VkPipelineLayout PipelineLayout = VK_NULL_HANDLE;

		VkCommandPool CommandPool = VK_NULL_HANDLE;
		std::vector<VkCommandBuffer> CommandBuffers = { };

		// Used for synchronization
		std::vector<VkSemaphore> ImageAvailableSemaphores = { };
		std::vector<VkSemaphore> RenderFinishedSemaphores = { };
		std::vector<VkFence> InFlightFences = { };

		uint32_t CurrentFrame = 0;
	};

	struct RenderInfo
	{
	public:
		std::vector<VkBuffer> VertexBuffers = { };
		std::vector<VkDeviceSize> Offsets = { {0} };
		uint32_t VerticeCount = 0u;
	};

	class GraphicsContext
	{
	public:
		GraphicsContext(GLFWwindow* window);

		void Init();
		void Destroy();

		void SwapBuffers();

		VulkanInfo& GetVulkanInfo() { return m_Info; }

		void RecreateSwapChain();
		void RecordCommandBuffer(VkCommandBuffer& commandBuffer, uint32_t imageIndex, const RenderInfo& info);

		uint32_t FindMemoryType(uint32_t typeFilter, VkMemoryPropertyFlags properties);

		static constexpr const int32_t MAX_FRAMES_IN_FLIGHT = 2;

	public:
		static std::shared_ptr<GraphicsContext> Create(void* window) { return std::make_shared<GraphicsContext>(reinterpret_cast<GLFWwindow*>(window)); }

	private:
		void CreateInstance();
		void SetupDebugMessenger();
		void CreateSurface();
		void GetPhysicalDevice();
		void CreateLogicalDevice();
		void CreateSwapChain();
		void CreateImageViews();
		void CreateRenderPass();
		void CreateGraphicsPipeline();
		void CreateFramebuffers();
		void CreateCommandPool();
		void CreateCommandBuffers();
		void CreateSyncObjects();

		void CleanUpSwapChain();

		bool CheckValidationLayerSupport();
		std::vector<const char*> GetRequiredExtensions();
		bool IsPhysicalDeviceSuitable(const VkPhysicalDevice& device);
		bool CheckDeviceExtensionSupport(const VkPhysicalDevice& device);
		QueueFamilyIndices FindQueueFamilies(const VkPhysicalDevice& device);
		SwapChainSupportDetails QuerySwapChainSupport(const VkPhysicalDevice& device);
		VkShaderModule CreateShaderModule(const std::vector<char>& code);

		VkSurfaceFormatKHR ChooseSwapSurfaceFormat(const std::vector<VkSurfaceFormatKHR>& availableFormats);
		VkPresentModeKHR ChooseSwapPresentMode(const std::vector<VkPresentModeKHR>& availablePresentModes);
		VkExtent2D ChooseSwapExtent(const VkSurfaceCapabilitiesKHR& capabilities);

		static std::vector<char> ReadFile(const std::filesystem::path& filename);

	// Note(Jorben): The public specifier is set here on purpose so the functions are readable and this struct is not in the middle of it.
	public:
		// For the default graphics pipeline
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

	private:
		GLFWwindow* m_WindowHandle;

		VulkanInfo m_Info;
	};

}