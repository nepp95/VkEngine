-- Vulkan Dependencies

vulkansdk = os.getenv("VULKAN_SDK")
shaders = "%{wks.location}/VkEngine/assets/shaders"

IncludeDir = {}
IncludeDir["glfw"] = "%{wks.location}/VkEngine/vendor/glfw/include"
IncludeDir["glm"] = "%{wks.location}/VkEngine/vendor/glm"
IncludeDir["spdlog"] = "%{wks.location}/VkEngine/vendor/spdlog/include"
IncludeDir["stb_image"] = "%{wks.location}/VkEngine/vendor/stb_image"
IncludeDir["vulkansdk"] = "%{vulkansdk}/Include"
IncludeDir["vulkan_memory_allocator"] = "%{wks.location}/VkEngine/vendor/vulkan-memory-allocator"

LibraryDir = {}
LibraryDir["vulkansdk"] = "%{vulkansdk}/Lib"

Library = {}
Library["glfw"] = "GLFW"
Library["vulkan"] = "%{LibraryDir.vulkansdk}/vulkan-1.lib"
Library["vulkanutils"] = "%{LibraryDir.vulkansdk}/VkLayer_utils.lib"