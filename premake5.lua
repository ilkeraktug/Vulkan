workspace "Vulkan"
	startproject "Vulkan"
	
	configurations { "Debug", "Release" }
	architecture "x64"

outputdir = "%{cfg.buildcfg}-%{cfg.system}-%{cfg.architecture}"
VulkanSDK = os.getenv("VULKAN_SDK")

IncludeDir = {}
IncludeDir["glfw"] = "Vulkan/vendor/glfw/include"
IncludeDir["glm"] = "Vulkan/vendor/glm"
IncludeDir["imgui"] = "Vulkan/vendor/imgui"
IncludeDir["spdlog"] = "Vulkan/vendor/spdlog/include"
IncludeDir["stb_image"] = "Vulkan/vendor/stb_image"
IncludeDir["tiny_obj_loader"] = "Vulkan/vendor/tiny_obj_loader"
IncludeDir["tinygltf"] = "Vulkan/vendor/tinygltf"
IncludeDir["ktx"] = "Vulkan/vendor/ktx/include"
IncludeDir["ktxOther"] = "Vulkan/vendor/ktx/other_include"

--vulkan include default location
IncludeDir["vulkan"] = VulkanSDK .. "/Include"


include "Vulkan/vendor/glfw"
include "Vulkan/vendor/imgui"
include "Vulkan/vendor/ktx"

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
		"%{IncludeDir.stb_image}/**.h",
		"%{IncludeDir.stb_image}/**.cpp",
		"%{IncludeDir.tiny_obj_loader}/**.h",
		"%{IncludeDir.tiny_obj_loader}/**.cpp",
	}

	defines
	{	
		"GLFW_INCLUDE_VULKAN",
		"GLM_FORCE_RADIANS",
		"GLM_FORCE_DEPTH_ZERO_TO_ONE",
		"TINYGLTF_NO_STB_IMAGE_WRITE"
	}

	includedirs
	{
		"%{prj.name}/src",
		"%{IncludeDir.glfw}",
		"%{IncludeDir.glm}",
		"%{IncludeDir.imgui}",
		"%{IncludeDir.vulkan}",
		"%{IncludeDir.spdlog}",
		"%{IncludeDir.stb_image}",
		"%{IncludeDir.tiny_obj_loader}",
		"%{IncludeDir.tinygltf}",
		"%{IncludeDir.ktx}",
		"%{IncludeDir.ktxOther}"
	}

	libdirs
	{
	--vulkan lib default location
		 VulkanSDK .. "/Lib"
	}

	links
	{
		"glfw",
		"imgui",
		"vulkan-1.lib",
		"KTX"
	}

	filter { "system:windows" }
		defines { "PLATFORM_WINDOWS" }
		systemversion "latest"

	filter { "configurations:Debug" }
		defines { "ENABLE_ASSERT", "ENABLE_VALIDATION_LAYERS" }
		runtime "Debug"
		symbols "on"

	filter { "configurations:Release" }
		runtime "Release"
		optimize "On"