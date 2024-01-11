#include "vcpch.h"
#include "BaseImGuiLayer.hpp"

#include <imgui.h>
#include <backends/imgui_impl_glfw.h>
#include <backends/imgui_impl_opengl3.h>

#include "VulkanCore/Core/Application.hpp"
#include "VulkanCore/Core/API.hpp"

#include <GLFW/glfw3.h>

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

		// TODO(Jorben): Add Vulkan support
	}

	void BaseImGuiLayer::OnDetach()
	{
		// TODO(Jorben): Add Vulkan support
		return;

		ImGui_ImplGlfw_Shutdown();
		ImGui::DestroyContext();
	}

	void BaseImGuiLayer::Begin()
	{
		// TODO(Jorben): Add Vulkan support
		return;

		ImGui_ImplGlfw_NewFrame();
		ImGui::NewFrame();
	}

	void BaseImGuiLayer::End()
	{
		// Temporary // TODO(Jorben): Remove and properly implement ImGui
		return;

		ImGuiIO& io = ImGui::GetIO();

		io.DisplaySize = ImVec2((float)Application::Get().GetWindow().GetWidth(), (float)Application::Get().GetWindow().GetHeight());

		// Rendering
		ImGui::Render();

		// TODO(Jorben): Add Vulkan support

		if (io.ConfigFlags & ImGuiConfigFlags_ViewportsEnable)
		{
			GLFWwindow* backup_current_context = glfwGetCurrentContext();
			ImGui::UpdatePlatformWindows();
			ImGui::RenderPlatformWindowsDefault();
			glfwMakeContextCurrent(backup_current_context);
		}
	}

}