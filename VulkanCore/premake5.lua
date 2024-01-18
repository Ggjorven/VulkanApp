project "VulkanCore"
	kind "StaticLib"
	language "C++"
	cppdialect "C++20"
	staticruntime "On"

	architecture "x86_64"

	targetdir ("%{wks.location}/bin/" .. outputdir .. "/%{prj.name}")
	objdir ("%{wks.location}/bin-int/" .. outputdir .. "/%{prj.name}")

	pchheader "vcpch.h"
	pchsource "src/VulkanCore/vcpch.cpp"

	files
	{
		"src/**.h",
		"src/**.hpp",
		"src/**.cpp",
		
		"%{wks.location}/vendor/stb_image/src/stb_image.cpp"
	}

	defines
	{
		"_CRT_SECURE_NO_WARNINGS",
		"GLFW_INCLUDE_NONE",
		"GLEW_STATIC"
	}

	includedirs
	{
		"src",
		"src/VulkanCore",

		"%{IncludeDir.GLFW}",
		"%{IncludeDir.ImGui}",
		"%{IncludeDir.glm}",
		"%{IncludeDir.spdlog}",
		"%{IncludeDir.stb_image}",
		"%{IncludeDir.VulkanSDK}"
	}

	links
	{
		"%{Library.Vulkan}",

		"GLFW",
		"ImGui",
		"spdlog"
	}

	disablewarnings
	{
		"4005",
		"4996"
	}

	filter "system:windows"
		systemversion "latest"
		staticruntime "on"

		defines
		{
			"VKAPP_PLATFORM_WINDOWS"
		}

	filter "configurations:Debug"
		defines "VKAPP_DEBUG"
		runtime "Debug"
		symbols "on"

	filter "configurations:Release"
		defines "VKAPP_RELEASE"
		runtime "Release"
		optimize "on"

	filter "configurations:Dist"
		defines "VKAPP_DIST"
		runtime "Release"
		optimize "on"