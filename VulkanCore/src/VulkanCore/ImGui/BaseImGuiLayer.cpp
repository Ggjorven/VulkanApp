#include "vcpch.h"
#include "BaseImGuiLayer.hpp"

#include <imgui.h>
#include <backends/imgui_impl_glfw.h>
#include <backends/imgui_impl_vulkan.h>

#include "VulkanCore/Core/Application.hpp"

#include <GLFW/glfw3.h>

// For imgui initialization
#include "VulkanCore/Renderer/Renderer.hpp"
#include "VulkanCore/Renderer/InstanceManager.hpp"
#include "VulkanCore/Renderer/SwapChainManager.hpp"
#include "VulkanCore/Renderer/GraphicsPipelineManager.hpp"

#include "VulkanCore/Utils/BufferManager.hpp"

namespace VkApp
{

	BaseImGuiLayer::BaseImGuiLayer()
		: Layer("BaseImGuiLayer")
	{
	}

	void BaseImGuiLayer::OnAttach()
	{
		//ImGui Setup
		IMGUI_CHECKVERSION();
		ImGui::CreateContext();

		ImGuiIO& io = ImGui::GetIO(); (void)io;

		io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;       // Enable Keyboard Controls
		//io.ConfigFlags |= ImGuiConfigFlags_NavEnableGamepad;      // Enable Gamepad Controls
		io.ConfigFlags |= ImGuiConfigFlags_DockingEnable;           // Enable Docking
		io.ConfigFlags |= ImGuiConfigFlags_ViewportsEnable;         // Enable Multi-Viewport / Platform Windows
		//io.ConfigFlags |= ImGuiConfigFlags_ViewportsNoTaskBarIcons;
		//io.ConfigFlags |= ImGuiConfigFlags_ViewportsNoMerge;

		ImGui::StyleColorsDark();

		// When viewports are enabled we tweak WindowRounding/WindowBg so platform windows can look identical to regular ones.
		ImGuiStyle& style = ImGui::GetStyle();

		if (io.ConfigFlags & ImGuiConfigFlags_ViewportsEnable)
		{
			style.WindowRounding = 0.0f;
			style.Colors[ImGuiCol_WindowBg].w = 1.0f;
		}

		GLFWwindow* window = static_cast<GLFWwindow*>(Application::Get().GetWindow().GetNativeWindow());
		ImGui_ImplGlfw_InitForVulkan(window, true);

		ImGui_ImplVulkan_InitInfo  initInfo = {};
		initInfo.Instance = InstanceManager::Get()->GetInstance();
		initInfo.PhysicalDevice = InstanceManager::Get()->GetPhysicalDevice();
		initInfo.Device = InstanceManager::Get()->GetLogicalDevice();
		initInfo.QueueFamily = InstanceManager::Get()->FindQueueFamilies(InstanceManager::Get()->GetPhysicalDevice()).GraphicsFamily.value();
		initInfo.Queue = InstanceManager::Get()->GetGraphicsQueue();
		//initInfo.PipelineCache = vkPipelineCache;
		initInfo.DescriptorPool = GraphicsPipelineManager::Get()->GetImGuiPool();
		initInfo.Allocator = nullptr; // Optional, use nullptr to use the default allocator
		initInfo.MinImageCount = static_cast<uint32_t>(SwapChainManager::Get()->GetImageViews().size());
		initInfo.ImageCount = static_cast<uint32_t>(SwapChainManager::Get()->GetImageViews().size()); 
		initInfo.CheckVkResultFn = nullptr; // Optional, a callback function for Vulkan errors
		//init_info.MSAASamples = vkMSAASamples; // The number of samples per pixel in case of MSAA
		//init_info.Subpass = 0; // The index of the subpass where ImGui will be drawn

		ImGui_ImplVulkan_Init(&initInfo, SwapChainManager::Get()->GetRenderPass());
		
		// Create fonts
		io.Fonts->AddFontDefault();
		{
			VkCommandBuffer commandBuffer = BufferManager::BeginSingleTimeCommands();
			ImGui_ImplVulkan_CreateFontsTexture(commandBuffer);
			BufferManager::EndSingleTimeCommands(commandBuffer);

			ImGui_ImplVulkan_DestroyFontUploadObjects();

		}
	}

	void BaseImGuiLayer::OnDetach()
	{
		ImGui_ImplVulkan_Shutdown();
		ImGui_ImplGlfw_Shutdown();
		ImGui::DestroyContext();
	}

	void BaseImGuiLayer::Begin()
	{
		ImGui_ImplVulkan_NewFrame();
		ImGui_ImplGlfw_NewFrame();
		ImGui::NewFrame();
	}

	void BaseImGuiLayer::End()
	{
		ImGuiIO& io = ImGui::GetIO();

		io.DisplaySize = ImVec2((float)Application::Get().GetWindow().GetWidth(), (float)Application::Get().GetWindow().GetHeight());

		// Rendering
		ImGui::Render();

		Renderer::AddToQueue([this](VkCommandBuffer& buffer, uint32_t imageIndex) {
				ImGui_ImplVulkan_RenderDrawData(ImGui::GetDrawData(), buffer);
			});

		if (io.ConfigFlags & ImGuiConfigFlags_ViewportsEnable)
		{
			GLFWwindow* backup_current_context = glfwGetCurrentContext();
			ImGui::UpdatePlatformWindows();
			ImGui::RenderPlatformWindowsDefault();
			glfwMakeContextCurrent(backup_current_context);
		}
	}

}