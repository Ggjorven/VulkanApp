include "Dependencies.lua"

workspace "VulkanApp"
	architecture "x86_64"
	startproject "VulkanSandbox"

	configurations
	{
		"Debug",
		"Release",
		"Dist"
	}

	flags
	{
		"MultiProcessorCompile"
	}

outputdir = "%{cfg.buildcfg}-%{cfg.system}-%{cfg.architecture}"

group "Additional Dependencies"
	include "vendor/GLFW"
	include "vendor/ImGui"
	include "vendor/spdlog"
	include "vendor/VulkanMemoryAllocator"
group ""

group "Core"
	include "VulkanCore"
group ""

include "VulkanSandbox"