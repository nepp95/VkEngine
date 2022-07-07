#include "pch.h"
#include "Application.h"

// To validate or not to validate
const bool EnableValidationLayers = VKE_DEBUG;
// Validation layer
const std::vector<const char*> ValidationLayers = { "VK_LAYER_KHRONOS_validation", "VK_LAYER_LUNARG_monitor" };
// Device extensions
const std::vector<const char*> DeviceExtensions = { VK_KHR_SWAPCHAIN_EXTENSION_NAME };
// Max frames in flight
const int MaxFramesInFlight = 2;

struct QueueFamilyIndices
{
	std::optional<uint32_t> GraphicsFamily;
	std::optional<uint32_t> PresentFamily;

	bool IsComplete()
	{
		return GraphicsFamily.has_value() && PresentFamily.has_value();
	}
};

struct SwapChainSupportDetails
{
	VkSurfaceCapabilitiesKHR Capabilities;
	std::vector<VkSurfaceFormatKHR> Formats;
	std::vector<VkPresentModeKHR> PresentModes;
};

static QueueFamilyIndices FindQueueFamilies(VkPhysicalDevice device)
{
	QueueFamilyIndices indices;

	uint32_t queueFamilyCount{ 0 };
	vkGetPhysicalDeviceQueueFamilyProperties(device, &queueFamilyCount, nullptr);

	std::vector<VkQueueFamilyProperties> queueFamilies(queueFamilyCount);
	vkGetPhysicalDeviceQueueFamilyProperties(device, &queueFamilyCount, queueFamilies.data());

	int i{ 0 };
	for (const auto& queueFamily : queueFamilies)
	{
		// Graphics family
		if (queueFamily.queueFlags & VK_QUEUE_GRAPHICS_BIT)
			indices.GraphicsFamily = i;

		// Present family
		VkBool32 presentSupport{ false };
		vkGetPhysicalDeviceSurfaceSupportKHR(device, i, Application::Get().GetSurface(), &presentSupport);

		if (presentSupport)
			indices.PresentFamily = i;

		// If complete, stop this loop
		if (indices.IsComplete())
			break;

		i++;
	}

	return indices;
}

static std::vector<const char*> GetRequiredExtensions()
{
	uint32_t glfwExtensionCount{ 0 };
	const char** glfwExtensions;
	glfwExtensions = glfwGetRequiredInstanceExtensions(&glfwExtensionCount);

	std::vector<const char*> extensions(glfwExtensions, glfwExtensions + glfwExtensionCount);

	if (EnableValidationLayers)
		extensions.push_back(VK_EXT_DEBUG_UTILS_EXTENSION_NAME);

	return extensions;
}

static bool HasValidationSupport()
{
	uint32_t layerCount;
	vkEnumerateInstanceLayerProperties(&layerCount, nullptr);

	std::vector<VkLayerProperties> availableLayers(layerCount);
	vkEnumerateInstanceLayerProperties(&layerCount, availableLayers.data());
	
	for (const char* layerName : ValidationLayers)
	{
		bool layerFound{ false };

		for (const auto& layerProperties : availableLayers)
		{
			if (strcmp(layerName, layerProperties.layerName) == 0)
			{
				layerFound = true;
				break;
			}
		}

		if (!layerFound)
			return false;
	}

	return true;
}

static bool CheckDeviceExtensionSupport(VkPhysicalDevice device)
{
	uint32_t extensionCount;
	vkEnumerateDeviceExtensionProperties(device, nullptr, &extensionCount, nullptr);

	std::vector<VkExtensionProperties> availableExtensions(extensionCount);
	vkEnumerateDeviceExtensionProperties(device, nullptr, &extensionCount, availableExtensions.data());

	std::set<std::string> requiredExtensions(DeviceExtensions.begin(), DeviceExtensions.end());

	for (const auto& extension : availableExtensions)
	{
		requiredExtensions.erase(extension.extensionName);
	}

	return requiredExtensions.empty();
}

static SwapChainSupportDetails QuerySwapChainSupport(VkPhysicalDevice device, VkSurfaceKHR surface)
{
	SwapChainSupportDetails details;

	vkGetPhysicalDeviceSurfaceCapabilitiesKHR(device, surface, &details.Capabilities);

	uint32_t formatCount;
	vkGetPhysicalDeviceSurfaceFormatsKHR(device, surface, &formatCount, nullptr);

	if (formatCount != 0)
	{
		details.Formats.resize(formatCount);
		vkGetPhysicalDeviceSurfaceFormatsKHR(device, surface, &formatCount, details.Formats.data());
	}

	uint32_t presentModeCount;
	vkGetPhysicalDeviceSurfacePresentModesKHR(device, surface, &presentModeCount, nullptr);

	if (presentModeCount != 0)
	{
		details.PresentModes.resize(presentModeCount);
		vkGetPhysicalDeviceSurfacePresentModesKHR(device, surface, &presentModeCount, details.PresentModes.data());
	}

	return details;
}

static bool IsDeviceSuitable(VkPhysicalDevice device)
{
	QueueFamilyIndices indices = FindQueueFamilies(device);

	bool extensionsSupported = CheckDeviceExtensionSupport(device);

	bool swapChainAdequate{ false };
	if (extensionsSupported)
	{
		SwapChainSupportDetails swapChainSupport = QuerySwapChainSupport(device, Application::Get().GetSurface());
		swapChainAdequate = !swapChainSupport.Formats.empty() && !swapChainSupport.PresentModes.empty();
	}

	bool isDiscrete{ false };
	VkPhysicalDeviceProperties deviceProperties;
	vkGetPhysicalDeviceProperties(device, &deviceProperties);
	if (deviceProperties.deviceType == VK_PHYSICAL_DEVICE_TYPE_DISCRETE_GPU)
		isDiscrete = true;
	
	return indices.IsComplete() && extensionsSupported && swapChainAdequate && isDiscrete;
 }

static VkSurfaceFormatKHR ChooseSwapSurfaceFormat(const std::vector<VkSurfaceFormatKHR>& availableFormats)
{
	for (const auto& format : availableFormats)
	{
		if (format.format == VK_FORMAT_B8G8R8A8_SRGB && format.colorSpace == VK_COLOR_SPACE_SRGB_NONLINEAR_KHR)
			return format;
	}

	return availableFormats[0];
}

static VkPresentModeKHR ChooseSwapPresentMode(const std::vector<VkPresentModeKHR>& availablePresentModes)
{
	for (const auto& presentMode : availablePresentModes)
	{
		//if (presentMode == VK_PRESENT_MODE_MAILBOX_KHR)
		if (presentMode == VK_PRESENT_MODE_IMMEDIATE_KHR)
			return presentMode;
	}

	return VK_PRESENT_MODE_FIFO_KHR;
}

static VkExtent2D ChooseSwapExtent(const VkSurfaceCapabilitiesKHR& capabilities)
{
	if (capabilities.currentExtent.width != std::numeric_limits<uint32_t>::max())
		return capabilities.currentExtent;
	else
	{
		int width, height;
		glfwGetFramebufferSize(&Application::Get().GetWindow(), &width, &height);

		VkExtent2D actualExtent = {
			static_cast<uint32_t>(width),
			static_cast<uint32_t>(height)
		};

		// Bound values between min and max allowed extents supported by the implementation
		actualExtent.width = std::clamp(actualExtent.width, capabilities.maxImageExtent.width, capabilities.maxImageExtent.width);
		actualExtent.height = std::clamp(actualExtent.height, capabilities.maxImageExtent.height, capabilities.maxImageExtent.height);

		return actualExtent;
	}
}

static std::vector<char> ReadFile(const std::string& filepath)
{
	std::ifstream file(filepath, std::ios::ate | std::ios::binary);

	CORE_ASSERT(file.is_open(), "Failed to open shader file!");

	size_t fileSize{ (size_t)file.tellg() };
	std::vector<char> buffer(fileSize);

	file.seekg(0);
	file.read(buffer.data(), fileSize);

	file.close();

	return buffer;
}

static VkShaderModule CreateShaderModule(const std::vector<char>& bytecode)
{
	VkShaderModuleCreateInfo createInfo{};
	createInfo.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
	createInfo.codeSize = bytecode.size();
	createInfo.pCode = reinterpret_cast<const uint32_t*>(bytecode.data());

	VkShaderModule shaderModule;
	VK_CHECK(vkCreateShaderModule(Application::Get().GetDevice(), &createInfo, nullptr, &shaderModule), "Failed to create shader module!");

	return shaderModule;
}

static VKAPI_ATTR VkBool32 VKAPI_CALL DebugCallback(
	VkDebugUtilsMessageSeverityFlagBitsEXT messageSeverity,
	VkDebugUtilsMessageTypeFlagsEXT messageType,
	const VkDebugUtilsMessengerCallbackDataEXT* pCallbackData,
	void* pUserData)
{
	CORE_INFO("Validation layer: {0}", pCallbackData->pMessage);
	return VK_FALSE;
}

VkResult CreateDebugUtilsMessengerEXT(VkInstance instance, const VkDebugUtilsMessengerCreateInfoEXT* pCreateInfo, const VkAllocationCallbacks* pAllocator, VkDebugUtilsMessengerEXT* pDebugMessenger)
{
	auto func = (PFN_vkCreateDebugUtilsMessengerEXT)vkGetInstanceProcAddr(instance, "vkCreateDebugUtilsMessengerEXT");

	if (func != nullptr)
		return func(instance, pCreateInfo, pAllocator, pDebugMessenger);
	else
		return VK_ERROR_EXTENSION_NOT_PRESENT;
}

void DestroyDebugUtilsMessengerEXT(VkInstance instance, VkDebugUtilsMessengerEXT debugMessenger, const VkAllocationCallbacks* pAllocator)
{
	auto func = (PFN_vkDestroyDebugUtilsMessengerEXT)vkGetInstanceProcAddr(instance, "vkDestroyDebugUtilsMessengerEXT");

	if (func != nullptr)
		func(instance, debugMessenger, pAllocator);
}

Application* Application::s_instance = nullptr;

Application::Application(const ApplicationSpecification& specification)
	: m_specification(specification)
{
	CORE_ASSERT(!s_instance, "Application already exists!");
	s_instance = this;

	Init();
}

Application::~Application()
{
	Cleanup();
}

void Application::Run()
{
	while (!glfwWindowShouldClose(m_window))
	{
		glfwPollEvents();
		DrawFrame();
	}

	vkDeviceWaitIdle(m_device);
}

void Application::Init()
{
	// Set working directory
	if (!m_specification.WorkingDirectory.empty())
		std::filesystem::current_path(m_specification.WorkingDirectory);

	// Glfw
	glfwInit();

	glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
	glfwWindowHint(GLFW_RESIZABLE, GLFW_FALSE);

	m_window = glfwCreateWindow(m_specification.WindowWidth, m_specification.WindowHeight, m_specification.Name.c_str(), nullptr, nullptr);

	// And now the Vulkan journey begins...
	
	//////////////////////////////////////////////
	////	VULKAN INSTANCE					  ////
	//////////////////////////////////////////////
	CORE_ASSERT(EnableValidationLayers && HasValidationSupport(), "Validation layers requested, but not available!");

	VkApplicationInfo appInfo{};
	appInfo.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
	appInfo.pApplicationName = m_specification.Name.c_str();
	appInfo.applicationVersion = VK_MAKE_VERSION(1, 0, 0);
	appInfo.pEngineName = "VkEngine";
	appInfo.engineVersion = VK_MAKE_VERSION(1, 0, 0);
	appInfo.apiVersion = VK_API_VERSION_1_3;

	VkInstanceCreateInfo createInfo{};
	createInfo.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
	createInfo.pApplicationInfo = &appInfo;
	
	// Extensions
	auto extensions = GetRequiredExtensions();
	createInfo.enabledExtensionCount = static_cast<uint32_t>(extensions.size());
	createInfo.ppEnabledExtensionNames = extensions.data();
	//

	// Layers
	VkDebugUtilsMessengerCreateInfoEXT dmCreateInfo{};
	if (EnableValidationLayers)
	{
		createInfo.enabledLayerCount = static_cast<uint32_t>(ValidationLayers.size());
		createInfo.ppEnabledLayerNames = ValidationLayers.data();

		dmCreateInfo = {};
		dmCreateInfo.sType = VK_STRUCTURE_TYPE_DEBUG_UTILS_MESSENGER_CREATE_INFO_EXT;
		dmCreateInfo.messageSeverity = VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT;
		dmCreateInfo.messageType = VK_DEBUG_UTILS_MESSAGE_TYPE_GENERAL_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT;
		dmCreateInfo.pfnUserCallback = DebugCallback;

		createInfo.pNext = (VkDebugUtilsMessengerCreateInfoEXT*)&dmCreateInfo;
	}
	else {
		createInfo.enabledLayerCount = 0;
		createInfo.pNext = nullptr;
	}
	//

	VK_CHECK(vkCreateInstance(&createInfo, nullptr, &m_instance), "Failed to create instance!");

	//////////////////////////////////////////////
	////	DEBUG MESSENGER					  ////
	//////////////////////////////////////////////
	if (EnableValidationLayers)
	{
		VkDebugUtilsMessengerCreateInfoEXT createInfo{};
		createInfo.sType = VK_STRUCTURE_TYPE_DEBUG_UTILS_MESSENGER_CREATE_INFO_EXT;
		createInfo.messageSeverity = VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT;
		createInfo.messageType = VK_DEBUG_UTILS_MESSAGE_TYPE_GENERAL_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT;
		createInfo.pfnUserCallback = DebugCallback;

		VK_CHECK(CreateDebugUtilsMessengerEXT(m_instance, &dmCreateInfo, nullptr, &m_debugMessenger), "Failed to setup debug messenger!");
	}

	//////////////////////////////////////////////
	////	VULKAN SURFACE					  ////
	//////////////////////////////////////////////
	VK_CHECK(glfwCreateWindowSurface(m_instance, m_window, nullptr, &m_surface), "Failed to create window surface!");

	//////////////////////////////////////////////
	////	PHYSICAL DEVICE					  ////
	//////////////////////////////////////////////
	uint32_t deviceCount{ 0 };
	vkEnumeratePhysicalDevices(m_instance, &deviceCount, nullptr);

	// If there is no Vulkan supported GPU, there is no point in continuing
	CORE_ASSERT(deviceCount, "Failed to find any GPU with Vulkan support!");

	std::vector<VkPhysicalDevice> devices(deviceCount);
	vkEnumeratePhysicalDevices(m_instance, &deviceCount, devices.data());

	for (const auto& device : devices)
	{
		if (IsDeviceSuitable(device))
		{
			m_physicalDevice = device;
			break;
		}
	}

	// Although there was atleast 1 device with Vulkan support, it/they did not meet our requirements
	CORE_ASSERT(m_physicalDevice, "Failed to find a suitable GPU!");

	//////////////////////////////////////////////
	////	LOGICAL DEVICE					  ////
	//////////////////////////////////////////////
	QueueFamilyIndices indices = FindQueueFamilies(m_physicalDevice);

	std::vector<VkDeviceQueueCreateInfo> queueCreateInfos;
	std::set<uint32_t> uniqueQueueFamilies = { indices.GraphicsFamily.value(), indices.PresentFamily.value() };

	float queuePriority = 1.0f;
	for (uint32_t queueFamily : uniqueQueueFamilies)
	{
		VkDeviceQueueCreateInfo createInfo{};
		createInfo.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
		createInfo.queueFamilyIndex = queueFamily;
		createInfo.queueCount = 1;
		createInfo.pQueuePriorities = &queuePriority;
		queueCreateInfos.push_back(createInfo);
	}

	VkPhysicalDeviceFeatures deviceFeatures{};

	VkDeviceCreateInfo deviceCreateInfo{};
	deviceCreateInfo.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
	deviceCreateInfo.pQueueCreateInfos = queueCreateInfos.data();
	deviceCreateInfo.queueCreateInfoCount = static_cast<uint32_t>(queueCreateInfos.size());
	deviceCreateInfo.pEnabledFeatures = &deviceFeatures;
	deviceCreateInfo.enabledExtensionCount = DeviceExtensions.size();
	deviceCreateInfo.ppEnabledExtensionNames = DeviceExtensions.data();

	if (EnableValidationLayers)
	{
		createInfo.enabledLayerCount = static_cast<uint32_t>(ValidationLayers.size());
		createInfo.ppEnabledLayerNames = ValidationLayers.data();
	}
	else {
		createInfo.enabledLayerCount = 0;
	}

	VK_CHECK(vkCreateDevice(m_physicalDevice, &deviceCreateInfo, nullptr, &m_device), "Failed to create logical device!");

	vkGetDeviceQueue(m_device, indices.GraphicsFamily.value(), 0, &m_graphicsQueue);
	vkGetDeviceQueue(m_device, indices.PresentFamily.value(), 0, &m_presentQueue);

	//////////////////////////////////////////////
	////	SWAPCHAIN						  ////
	//////////////////////////////////////////////
	SwapChainSupportDetails swapChainSupport = QuerySwapChainSupport(m_physicalDevice, m_surface);

	VkSurfaceFormatKHR surfaceFormat = ChooseSwapSurfaceFormat(swapChainSupport.Formats);
	m_swapchainImageFormat = surfaceFormat.format;
	VkPresentModeKHR presentMode = ChooseSwapPresentMode(swapChainSupport.PresentModes);
	m_swapchainExtent = ChooseSwapExtent(swapChainSupport.Capabilities);

	uint32_t imageCount = swapChainSupport.Capabilities.minImageCount + 1;
	if (swapChainSupport.Capabilities.maxImageCount > 0 && imageCount > swapChainSupport.Capabilities.maxImageCount)
		imageCount = swapChainSupport.Capabilities.maxImageCount;

	VkSwapchainCreateInfoKHR swapchainCreateInfo{};
	swapchainCreateInfo.sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR;
	swapchainCreateInfo.surface = m_surface;
	swapchainCreateInfo.minImageCount = imageCount;
	swapchainCreateInfo.imageFormat = surfaceFormat.format;
	swapchainCreateInfo.imageColorSpace = surfaceFormat.colorSpace;
	swapchainCreateInfo.imageExtent = m_swapchainExtent;
	swapchainCreateInfo.imageArrayLayers = 1;
	swapchainCreateInfo.imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;

	uint32_t queueFamilyIndices[] = { indices.GraphicsFamily.value(), indices.PresentFamily.value() };

	if (indices.GraphicsFamily != indices.PresentFamily)
	{
		swapchainCreateInfo.imageSharingMode = VK_SHARING_MODE_CONCURRENT;
		swapchainCreateInfo.queueFamilyIndexCount = 2;
		swapchainCreateInfo.pQueueFamilyIndices = queueFamilyIndices;
	}
	else {
		swapchainCreateInfo.imageSharingMode = VK_SHARING_MODE_EXCLUSIVE;
		swapchainCreateInfo.queueFamilyIndexCount = 0;
		swapchainCreateInfo.pQueueFamilyIndices = nullptr;
	}

	swapchainCreateInfo.preTransform = swapChainSupport.Capabilities.currentTransform;
	swapchainCreateInfo.compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR;
	swapchainCreateInfo.presentMode = presentMode;
	swapchainCreateInfo.clipped = VK_TRUE;
	swapchainCreateInfo.oldSwapchain = VK_NULL_HANDLE;

	VK_CHECK(vkCreateSwapchainKHR(m_device, &swapchainCreateInfo, nullptr, &m_swapchain), "Failed to create swapchain!");

	vkGetSwapchainImagesKHR(m_device, m_swapchain, &imageCount, nullptr);
	m_swapchainImages.resize(imageCount);
	vkGetSwapchainImagesKHR(m_device, m_swapchain, &imageCount, m_swapchainImages.data());

	//////////////////////////////////////////////
	////	SWAPCHAIN IMAGE VIEWS			  ////
	//////////////////////////////////////////////
	m_swapchainImageViews.resize(m_swapchainImages.size());

	for (size_t i = 0; i < m_swapchainImages.size(); i++)
	{
		VkImageViewCreateInfo ImageViewCreateInfo{};
		ImageViewCreateInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
		ImageViewCreateInfo.image = m_swapchainImages[i];
		ImageViewCreateInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;
		ImageViewCreateInfo.format = m_swapchainImageFormat;
		ImageViewCreateInfo.components.r = VK_COMPONENT_SWIZZLE_IDENTITY;
		ImageViewCreateInfo.components.g = VK_COMPONENT_SWIZZLE_IDENTITY;
		ImageViewCreateInfo.components.b = VK_COMPONENT_SWIZZLE_IDENTITY;
		ImageViewCreateInfo.components.a = VK_COMPONENT_SWIZZLE_IDENTITY;
		ImageViewCreateInfo.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
		ImageViewCreateInfo.subresourceRange.baseMipLevel = 0;
		ImageViewCreateInfo.subresourceRange.levelCount = 1;
		ImageViewCreateInfo.subresourceRange.baseArrayLayer = 0;
		ImageViewCreateInfo.subresourceRange.layerCount = 1;

		VK_CHECK(vkCreateImageView(m_device, &ImageViewCreateInfo, nullptr, &m_swapchainImageViews[i]), "Failed to create image views!");
	}

	//////////////////////////////////////////////
	////	PIPELINE RENDER PASS			  ////
	//////////////////////////////////////////////
	VkAttachmentDescription colorAttachment{};
	colorAttachment.format = m_swapchainImageFormat;
	colorAttachment.samples = VK_SAMPLE_COUNT_1_BIT;
	colorAttachment.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
	colorAttachment.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
	colorAttachment.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
	colorAttachment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
	colorAttachment.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
	colorAttachment.finalLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;

	VkAttachmentReference colorAttachmentRef{};
	colorAttachmentRef.attachment = 0;
	colorAttachmentRef.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

	VkSubpassDescription subpass{};
	subpass.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
	subpass.colorAttachmentCount = 1;
	subpass.pColorAttachments = &colorAttachmentRef;

	VkSubpassDependency dependency{};
	dependency.srcSubpass = VK_SUBPASS_EXTERNAL;
	dependency.dstSubpass = 0;
	dependency.srcStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
	dependency.srcAccessMask = 0;
	dependency.dstStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
	dependency.dstAccessMask = 0;

	VkRenderPassCreateInfo renderPassInfo{};
	renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
	renderPassInfo.attachmentCount = 1;
	renderPassInfo.pAttachments = &colorAttachment;
	renderPassInfo.subpassCount = 1;
	renderPassInfo.pSubpasses = &subpass;
	renderPassInfo.dependencyCount = 1;
	renderPassInfo.pDependencies = &dependency;

	VK_CHECK(vkCreateRenderPass(m_device, &renderPassInfo, nullptr, &m_renderPass), "Failed to create render pass!");

	//////////////////////////////////////////////
	////	PIPELINE						  ////
	//////////////////////////////////////////////
	// Shader modules
	auto vertexShaderCode = ReadFile("assets/shaders/vert.spv");
	auto fragmentShaderCode = ReadFile("assets/shaders/frag.spv");

	VkShaderModule vertexShaderModule = CreateShaderModule(vertexShaderCode);
	VkShaderModule fragmentShaderModule = CreateShaderModule(fragmentShaderCode);

	// Pipeline Shader Stages
	VkPipelineShaderStageCreateInfo vertexShaderCreateInfo{};
	vertexShaderCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
	vertexShaderCreateInfo.stage = VK_SHADER_STAGE_VERTEX_BIT;
	vertexShaderCreateInfo.module = vertexShaderModule;
	vertexShaderCreateInfo.pName = "main";

	VkPipelineShaderStageCreateInfo fragmentShaderCreateInfo{};
	fragmentShaderCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
	fragmentShaderCreateInfo.stage = VK_SHADER_STAGE_FRAGMENT_BIT;
	fragmentShaderCreateInfo.module = fragmentShaderModule;
	fragmentShaderCreateInfo.pName = "main";

	VkPipelineShaderStageCreateInfo shaderStages[] = { vertexShaderCreateInfo, fragmentShaderCreateInfo };

	// Pipeline Vertex Input State
	VkPipelineVertexInputStateCreateInfo vertexInputCreateInfo{};
	vertexInputCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;
	vertexInputCreateInfo.vertexBindingDescriptionCount = 0;
	vertexInputCreateInfo.pVertexBindingDescriptions = nullptr;
	vertexInputCreateInfo.vertexAttributeDescriptionCount = 0;
	vertexInputCreateInfo.pVertexAttributeDescriptions = nullptr;

	// Pipeline Input Assembly State
	VkPipelineInputAssemblyStateCreateInfo assemblyCreateInfo{};
	assemblyCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO;
	assemblyCreateInfo.topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;
	assemblyCreateInfo.primitiveRestartEnable = VK_FALSE;

	// Pipeline Viewport
	VkViewport viewport{};
	viewport.x = 0.0f;
	viewport.y = 0.0f;
	viewport.width = (float)m_swapchainExtent.width;
	viewport.height = (float)m_swapchainExtent.height;
	viewport.minDepth = 0.0f;
	viewport.maxDepth = 1.0f;

	VkRect2D scissor{};
	scissor.offset = { 0, 0 };
	scissor.extent = m_swapchainExtent;

	VkPipelineViewportStateCreateInfo viewportState{};
	viewportState.sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO;
	viewportState.viewportCount = 1;
	viewportState.pViewports = &viewport;
	viewportState.scissorCount = 1;
	viewportState.pScissors = &scissor;

	// Rasterizer
	VkPipelineRasterizationStateCreateInfo rasterizer{};
	rasterizer.sType = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO;
	rasterizer.depthClampEnable = VK_FALSE;
	rasterizer.rasterizerDiscardEnable = VK_FALSE;
	rasterizer.polygonMode = VK_POLYGON_MODE_FILL;
	rasterizer.lineWidth = 1.0f;
	rasterizer.cullMode = VK_CULL_MODE_BACK_BIT;
	rasterizer.frontFace = VK_FRONT_FACE_CLOCKWISE;
	rasterizer.depthBiasEnable = VK_FALSE;

	// Multisampling
	VkPipelineMultisampleStateCreateInfo multisampling{};
	multisampling.sType = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO;
	multisampling.sampleShadingEnable = VK_FALSE;
	multisampling.rasterizationSamples = VK_SAMPLE_COUNT_1_BIT;

	// Color blending
	VkPipelineColorBlendAttachmentState colorBlendAttachment{};
	colorBlendAttachment.colorWriteMask = VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT | VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT;
	colorBlendAttachment.blendEnable = VK_FALSE;

	VkPipelineColorBlendStateCreateInfo colorBlending{};
	colorBlending.sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO;
	colorBlending.logicOpEnable = VK_FALSE;
	colorBlending.attachmentCount = 1;
	colorBlending.pAttachments = &colorBlendAttachment;

	// Pipeline layout
	VkPipelineLayoutCreateInfo pipelineLayoutInfo{};
	pipelineLayoutInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;

	VK_CHECK(vkCreatePipelineLayout(m_device, &pipelineLayoutInfo, nullptr, &m_pipelineLayout), "Failed to create pipeline layout!");

	// Finally, the pipeline...
	VkGraphicsPipelineCreateInfo pipelineInfo{};
	pipelineInfo.sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO;
	pipelineInfo.stageCount = 2;
	pipelineInfo.pStages = shaderStages;
	pipelineInfo.pVertexInputState = &vertexInputCreateInfo;
	pipelineInfo.pInputAssemblyState = &assemblyCreateInfo;
	pipelineInfo.pViewportState = &viewportState;
	pipelineInfo.pRasterizationState = &rasterizer;
	pipelineInfo.pMultisampleState = &multisampling;
	pipelineInfo.pDepthStencilState = nullptr; // Optional
	pipelineInfo.pColorBlendState = &colorBlending;
	pipelineInfo.pDynamicState = nullptr; // Optional
	pipelineInfo.layout = m_pipelineLayout;
	pipelineInfo.renderPass = m_renderPass;
	pipelineInfo.subpass = 0;
	pipelineInfo.basePipelineHandle = VK_NULL_HANDLE; // Optional
	pipelineInfo.basePipelineIndex = -1; // Optional

	VK_CHECK(vkCreateGraphicsPipelines(m_device, VK_NULL_HANDLE, 1, &pipelineInfo, nullptr, &m_pipeline), "Failed to create graphics pipeline!");

	vkDestroyShaderModule(m_device, vertexShaderModule, nullptr);
	vkDestroyShaderModule(m_device, fragmentShaderModule, nullptr);

	//////////////////////////////////////////////
	////	FRAMEBUFFERS					  ////
	//////////////////////////////////////////////
	m_swapChainFramebuffers.resize(m_swapchainImageViews.size());

	for (size_t i = 0; i < m_swapchainImageViews.size(); i++)
	{
		VkImageView attachments[] = {
			m_swapchainImageViews[i]
		};

		VkFramebufferCreateInfo framebufferCreateInfo{};
		framebufferCreateInfo.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
		framebufferCreateInfo.renderPass = m_renderPass;
		framebufferCreateInfo.attachmentCount = 1;
		framebufferCreateInfo.pAttachments = attachments;
		framebufferCreateInfo.width = m_swapchainExtent.width;
		framebufferCreateInfo.height = m_swapchainExtent.height;
		framebufferCreateInfo.layers = 1;

		VK_CHECK(vkCreateFramebuffer(m_device, &framebufferCreateInfo, nullptr, &m_swapChainFramebuffers[i]), "Failed to create framebuffer!");
	}

	//////////////////////////////////////////////
	////	COMMAND POOL					  ////
	//////////////////////////////////////////////
	VkCommandPoolCreateInfo cmdPoolInfo{};
	cmdPoolInfo.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
	cmdPoolInfo.flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;
	cmdPoolInfo.queueFamilyIndex = indices.GraphicsFamily.value();

	VK_CHECK(vkCreateCommandPool(m_device, &cmdPoolInfo, nullptr, &m_commandPool), "Failed to create command pool!");

	//////////////////////////////////////////////
	////	COMMAND BUFFER					  ////
	//////////////////////////////////////////////
	m_commandBuffers.resize(MaxFramesInFlight);

	VkCommandBufferAllocateInfo cmdBufferInfo{};
	cmdBufferInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
	cmdBufferInfo.commandPool = m_commandPool;
	cmdBufferInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
	cmdBufferInfo.commandBufferCount = (uint32_t) m_commandBuffers.size();

	VK_CHECK(vkAllocateCommandBuffers(m_device, &cmdBufferInfo, m_commandBuffers.data()), "Failed to create command buffer!");

	//////////////////////////////////////////////
	////	SYNCING							  ////
	//////////////////////////////////////////////
	m_imageAvailableSemaphores.resize(MaxFramesInFlight);
	m_renderFinishedSemaphores.resize(MaxFramesInFlight);
	m_inFlightFences.resize(MaxFramesInFlight);

	VkSemaphoreCreateInfo semaphoreInfo{};
	semaphoreInfo.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;

	VkFenceCreateInfo fenceInfo{};
	fenceInfo.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
	fenceInfo.flags = VK_FENCE_CREATE_SIGNALED_BIT;

	for (size_t i = 0; i < MaxFramesInFlight; i++)
	{
		VK_CHECK(vkCreateSemaphore(m_device, &semaphoreInfo, nullptr, &m_imageAvailableSemaphores[i]), "Failed to create image available semaphore!");
		VK_CHECK(vkCreateSemaphore(m_device, &semaphoreInfo, nullptr, &m_renderFinishedSemaphores[i]), "Failed to create image available semaphore!");
		VK_CHECK(vkCreateFence(m_device, &fenceInfo, nullptr, &m_inFlightFences[i]), "Failed to create in flight fence!");
	}
}

void Application::Cleanup()
{
	for (size_t i = 0; i < MaxFramesInFlight; i++)
	{
		vkDestroySemaphore(m_device, m_renderFinishedSemaphores[i], nullptr);
		vkDestroySemaphore(m_device, m_imageAvailableSemaphores[i], nullptr);
		vkDestroyFence(m_device, m_inFlightFences[i], nullptr);
	}

	vkDestroyCommandPool(m_device, m_commandPool, nullptr);

	for (auto framebuffer : m_swapChainFramebuffers)
		vkDestroyFramebuffer(m_device, framebuffer, nullptr);

	vkDestroyPipeline(m_device, m_pipeline, nullptr);
	vkDestroyPipelineLayout(m_device, m_pipelineLayout, nullptr);
	vkDestroyRenderPass(m_device, m_renderPass, nullptr);

	for (auto imageView : m_swapchainImageViews)
		vkDestroyImageView(m_device, imageView, nullptr);

	vkDestroySwapchainKHR(m_device, m_swapchain, nullptr);
	vkDestroyDevice(m_device, nullptr);

	if (EnableValidationLayers)
		DestroyDebugUtilsMessengerEXT(m_instance, m_debugMessenger, nullptr);

	vkDestroySurfaceKHR(m_instance, m_surface, nullptr);
	vkDestroyInstance(m_instance, nullptr);
	glfwDestroyWindow(m_window);
	glfwTerminate();
}

void Application::RecordCommandBuffer(VkCommandBuffer commandBuffer, uint32_t imageIndex)
{
	VkCommandBufferBeginInfo cmdBufferInfo{};
	cmdBufferInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
	cmdBufferInfo.flags = 0;
	cmdBufferInfo.pInheritanceInfo = nullptr;

	VK_CHECK(vkBeginCommandBuffer(commandBuffer, &cmdBufferInfo), "Failed to begin recording command buffer!");

	VkClearValue clearValue{ 0.0f, 0.0f, 0.0f, 1.0f };
	VkRenderPassBeginInfo renderPassInfo{};
	renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
	renderPassInfo.renderPass = m_renderPass;
	renderPassInfo.framebuffer = m_swapChainFramebuffers[imageIndex];
	renderPassInfo.renderArea.offset = { 0, 0 };
	renderPassInfo.renderArea.extent = m_swapchainExtent;
	renderPassInfo.clearValueCount = 1;
	renderPassInfo.pClearValues = &clearValue;

	vkCmdBeginRenderPass(commandBuffer, &renderPassInfo, VK_SUBPASS_CONTENTS_INLINE);

	vkCmdBindPipeline(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, m_pipeline);
	vkCmdDraw(commandBuffer, 3, 1, 0, 0);

	vkCmdEndRenderPass(commandBuffer);

	VK_CHECK(vkEndCommandBuffer(commandBuffer), "Failed to record command buffer!");
}

void Application::DrawFrame()
{
	vkWaitForFences(m_device, 1, &m_inFlightFences[m_currentFrame], VK_TRUE, UINT64_MAX);
	vkResetFences(m_device, 1, &m_inFlightFences[m_currentFrame]);

	uint32_t imageIndex;
	vkAcquireNextImageKHR(m_device, m_swapchain, UINT64_MAX, m_imageAvailableSemaphores[m_currentFrame], VK_NULL_HANDLE, &imageIndex);

	vkResetCommandBuffer(m_commandBuffers[m_currentFrame], 0);
	RecordCommandBuffer(m_commandBuffers[m_currentFrame], imageIndex);

	VkSubmitInfo submitInfo{};
	submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;

	VkSemaphore waitSemaphores[] = { m_imageAvailableSemaphores[m_currentFrame]};
	VkPipelineStageFlags waitStages[] = { VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT };
	submitInfo.waitSemaphoreCount = 1;
	submitInfo.pWaitSemaphores = waitSemaphores;
	submitInfo.pWaitDstStageMask = waitStages;

	submitInfo.commandBufferCount = 1;
	submitInfo.pCommandBuffers = &m_commandBuffers[m_currentFrame];

	VkSemaphore signalSemaphores[] = { m_renderFinishedSemaphores[m_currentFrame]};
	submitInfo.signalSemaphoreCount = 1;
	submitInfo.pSignalSemaphores = signalSemaphores;

	VK_CHECK(vkQueueSubmit(m_graphicsQueue, 1, &submitInfo, m_inFlightFences[m_currentFrame]), "Failed to submit draw command buffer!");

	VkPresentInfoKHR presentInfo{};
	presentInfo.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;

	presentInfo.waitSemaphoreCount = 1;
	presentInfo.pWaitSemaphores = signalSemaphores;

	VkSwapchainKHR swapchains[] = { m_swapchain };
	presentInfo.swapchainCount = 1;
	presentInfo.pSwapchains = swapchains;

	presentInfo.pImageIndices = &imageIndex;

	vkQueuePresentKHR(m_presentQueue, &presentInfo);

	m_currentFrame = (m_currentFrame + 1) % MaxFramesInFlight;
}