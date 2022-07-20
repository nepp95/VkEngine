include "Dependencies.lua"

workspace "VkEngine"
	architecture "x86_64"
	startproject "VkEngine"
	
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
	
	group "Dependencies"
		include "VkEngine/vendor/glfw"
		include "VkEngine/vendor/vulkan-memory-allocator"
	group ""

	include "VkEngine"