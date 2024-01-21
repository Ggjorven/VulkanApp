project "VulkanSandbox"
	kind "ConsoleApp"
	language "C++"
	cppdialect "C++20"
	staticruntime "off"
	
	architecture "x86_64"
	
	-- debugdir ("%{wks.location}/bin/" .. outputdir .. "/%{prj.name}") 
	
	targetdir ("%{wks.location}/bin/" .. outputdir .. "/%{prj.name}")
	objdir ("%{wks.location}/bin-int/" .. outputdir .. "/%{prj.name}")

	files
	{
		"src/**.h",
		"src/**.hpp",
		"src/**.cpp"
	}

	includedirs
	{
		"src",

		"%{wks.location}/VulkanCore/src",
		"%{wks.location}vendor",

		"%{IncludeDir.GLFW}",
		"%{IncludeDir.ImGui}",
		"%{IncludeDir.spdlog}",
		"%{IncludeDir.glm}",
		"%{IncludeDir.stb_image}",
		"%{IncludeDir.VulkanSDK}",
		"%{IncludeDir.VMA}"
	}

	links
	{
		"VulkanCore"
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
			"VKAPP_PLATFORM_WINDOWS",
			"GLFW_INCLUDE_NONE"
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

	-- Dist filter for Windows for Windowed Applications
	filter { "system:windows", "configurations:Dist" }
		kind "WindowedApp"