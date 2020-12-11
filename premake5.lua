workspace "Vulkan"
	startproject "Vulkan"
	
	configurations { "Debug", "Release" }
	architecture "x64"

outputdir = "%{cfg.buildcfg}-%{cfg.architecture}-%{cfg.platform}"

IncludeDir = {}
IncludeDir["GLFW"] = "Vulkan/vendor/GLFW/include"
IncludeDir["glm"] = "Vulkan/vendor/glm"
--vulkan include default location
IncludeDir["vulkan"] = "C:/VulkanSDK/1.2.154.1/Include"

include "Vulkan/vendor/GLFW"

	project "Vulkan"
		location "Vulkan"
		kind "ConsoleApp"
		language "C++"
		cppdialect "C++17"
	
	targetdir ("bin/" ..outputdir.. "/%{prj.name}")
	objdir ("bin-int/" ..outputdir.. "/%{prj.name}")

	files
	{
		"%{prj.name}/src/**.h",
		"%{prj.name}/src/**.cpp",
	}

	includedirs
	{
		"%{IncludeDir.GLFW}",
		"%{IncludeDir.glm}",
		"%{IncludeDir.vulkan}"
	}

	libdirs
	{
	--vulkan lib default location
		"C:/VulkanSDK/1.2.154.1/Lib"
	}

	links
	{
		"GLFW",
		"vulkan-1.lib"
	}

	filter { "system:windows" }
		systemversion "latest"

	filter { "configurations:Debug" }
		runtime "Debug"
		symbols "on"

	filter { "configurations:Release" }
		runtime "Release"
		optimize "On"