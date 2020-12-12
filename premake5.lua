workspace "Vulkan"
	startproject "Vulkan"
	
	configurations { "Debug", "Release" }
	architecture "x64"

outputdir = "%{cfg.buildcfg}-%{cfg.architecture}-%{cfg.platform}"

IncludeDir = {}
IncludeDir["GLFW"] = "Vulkan/vendor/GLFW/include"
IncludeDir["glm"] = "Vulkan/vendor/glm"
IncludeDir["spdlog"] = "Vulkan/vendor/spdlog/include"

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

	pchheader("pch.h")
	pchsource("Vulkan/src/pch.cpp")

	files
	{
		"%{prj.name}/src/**.h",
		"%{prj.name}/src/**.cpp",
	}

	defines
	{	
		"GLFW_INCLUDE_VULKAN",
		"GLM_FORCE_RADIANS",
		"GLM_FORCE_DEPTH_ZERO_TO_ONE"
	}

	includedirs
	{
		"%{prj.name}/src",
		"%{IncludeDir.GLFW}",
		"%{IncludeDir.glm}",
		"%{IncludeDir.vulkan}",
		"%{IncludeDir.spdlog}"
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
		defines "ENABLE_ASSERT"
		runtime "Debug"
		symbols "on"

	filter { "configurations:Release" }
		runtime "Release"
		optimize "On"