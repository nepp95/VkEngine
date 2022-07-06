project "VkEngine"
	kind "ConsoleApp"
	language "C++"
	cppdialect "C++17"

	targetdir ("%{wks.location}/bin/" .. outputdir .. "/%{prj.name}")
	objdir ("%{wks.location}/bin-int/" .. outputdir .. "/%{prj.name}")

	pchheader "pch.h"
	pchsource "src/pch.cpp"

	files
	{
		"src/**.h",
		"src/**.cpp",
		"vendor/stb_image/*.h",
		"vendor/stb_image/*.cpp",
	}

	includedirs
	{
		"src",
		"%{IncludeDir.glfw}",
		"%{IncludeDir.glm}",
		"%{IncludeDir.spdlog}",
		"%{IncludeDir.stb_image}",
		"%{IncludeDir.vulkansdk}"
	}

	links
	{
		"%{Library.glfw}",
		"%{Library.vulkan}"
	}

	filter "system:windows"
		systemversion "latest"

	filter "configurations:Debug"
		defines "VKE_DEBUG"
		runtime "Debug"
		symbols "on"

		postbuildcommands
		{
			"\"%{vulkansdk}/Bin/glslc.exe\" %{shaders}/shader.vert -o %{shaders}/vert.spv",
			"\"%{vulkansdk}/Bin/glslc.exe\" %{shaders}/shader.frag -o %{shaders}/frag.spv"
		}

	filter "configurations:Release"
		defines "VKE_RELEASE"
		runtime "Release"
		optimize "on"

	filter "configurations:Dist"
		defines "VKE_DIST"
		runtime "Release"
		optimize "on"