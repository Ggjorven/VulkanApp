#pragma once

#include <vector>
#include <optional>
#include <filesystem>

#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>
#include <vulkan/vulkan.h>

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
}

namespace VkApp
{

	class GraphicsContext
	{
	public:
		GraphicsContext(GLFWwindow* window);

		void Init();
		void Destroy();

		void SwapBuffers();

		VulkanInfo& GetVulkanInfo() { return m_Info; }

		void RecreateSwapChain();
		void RecordCommandBuffer(VkCommandBuffer& commandBuffer, uint32_t imageIndex);

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

	private:
		GLFWwindow* m_WindowHandle;

		VulkanInfo m_Info;
	};

}