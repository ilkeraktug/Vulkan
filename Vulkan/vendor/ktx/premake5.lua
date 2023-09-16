project "KTX"
	kind "StaticLib"
	language "C"

	targetdir ("bin/" .. outputdir .. "/%{prj.name}")
	objdir ("bin-int/" .. outputdir .. "/%{prj.name}")
	
	IncludeDir1 = {}
    IncludeDir1["ktx"] = "include"
    IncludeDir1["ktxOther"] = "other_include"
	
	files
	{
		"lib/texture.c",
		"lib/hashlist.c",
		"lib/checkheader.c",
		"lib/swap.c",
		"lib/memstream.c",
		"lib/filestream.c"
	}
	
	includedirs
	{
		"%{IncludeDir1.ktx}",
		"%{IncludeDir1.ktxOther}"
	}

	filter "configurations:Debug"
		runtime "Debug"
		symbols "on"

	filter "configurations:Release"
		runtime "Release"
		optimize "on"