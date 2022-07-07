#include "pch.h"
#include "Application.h"

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
	Shutdown();
}

void Application::Run()
{
	while (!glfwWindowShouldClose(m_window))
	{
		glfwPollEvents();
		DrawFrame();
	}

	vkDeviceWaitIdle(m_device.Get());
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
	glfwSetWindowUserPointer(m_window, this);
	glfwSetFramebufferSizeCallback(m_window, OnFramebufferResize);

	// And now the Vulkan journey begins...
	
	//////////////////////////////////////////////
	////	VULKAN INSTANCE					  ////
	//////////////////////////////////////////////
	CORE_ASSERT(ENABLE_VALIDATION_LAYERS && HasValidationSupport(), "Validation layers requested, but not available!");

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
	if (ENABLE_VALIDATION_LAYERS)
	{
		createInfo.enabledLayerCount = static_cast<uint32_t>(VALIDATION_LAYERS.size());
		createInfo.ppEnabledLayerNames = VALIDATION_LAYERS.data();

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
	if (ENABLE_VALIDATION_LAYERS)
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
	VK_CHECK(glfwCreateWindowSurface(m_instance, m_window, nullptr, &m_physicalDevice.GetSurface()), "Failed to create window surface!");

	//////////////////////////////////////////////
	////	PHYSICAL DEVICE					  ////
	//////////////////////////////////////////////
	m_physicalDevice.ChooseDevice();

	//////////////////////////////////////////////
	////	LOGICAL DEVICE					  ////
	//////////////////////////////////////////////
	m_device.Create();

	//////////////////////////////////////////////
	////	SWAPCHAIN						  ////
	//////////////////////////////////////////////
	m_physicalDevice.GetSwapchain().Create();

	//////////////////////////////////////////////
	////	SWAPCHAIN IMAGE VIEWS			  ////
	//////////////////////////////////////////////
	m_physicalDevice.GetSwapchain().CreateImageViews();

	//////////////////////////////////////////////
	////	PIPELINE						  ////
	//////////////////////////////////////////////
	m_pipeline.Create();

	//////////////////////////////////////////////
	////	FRAMEBUFFERS					  ////
	//////////////////////////////////////////////
	m_physicalDevice.GetSwapchain().CreateFramebuffers();

	//////////////////////////////////////////////
	////	COMMAND POOL					  ////
	//////////////////////////////////////////////
	QueueFamilyIndices indices = FindQueueFamilies(GetPhysicalDevice().Get(), GetPhysicalDevice().GetSurface());

	VkCommandPoolCreateInfo cmdPoolInfo{};
	cmdPoolInfo.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
	cmdPoolInfo.flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;
	cmdPoolInfo.queueFamilyIndex = indices.GraphicsFamily.value();

	VK_CHECK(vkCreateCommandPool(m_device.Get(), &cmdPoolInfo, nullptr, &m_commandPool), "Failed to create command pool!");

	//////////////////////////////////////////////
	////	COMMAND BUFFER					  ////
	//////////////////////////////////////////////
	m_commandBuffers.resize(MAX_FRAMES_IN_FLIGHT);

	VkCommandBufferAllocateInfo cmdBufferInfo{};
	cmdBufferInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
	cmdBufferInfo.commandPool = m_commandPool;
	cmdBufferInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
	cmdBufferInfo.commandBufferCount = (uint32_t) m_commandBuffers.size();

	VK_CHECK(vkAllocateCommandBuffers(m_device.Get(), &cmdBufferInfo, m_commandBuffers.data()), "Failed to create command buffer!");

	//////////////////////////////////////////////
	////	SYNCING							  ////
	//////////////////////////////////////////////
	m_imageAvailableSemaphores.resize(MAX_FRAMES_IN_FLIGHT);
	m_renderFinishedSemaphores.resize(MAX_FRAMES_IN_FLIGHT);
	m_inFlightFences.resize(MAX_FRAMES_IN_FLIGHT);

	VkSemaphoreCreateInfo semaphoreInfo{};
	semaphoreInfo.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;

	VkFenceCreateInfo fenceInfo{};
	fenceInfo.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
	fenceInfo.flags = VK_FENCE_CREATE_SIGNALED_BIT;

	for (size_t i = 0; i < MAX_FRAMES_IN_FLIGHT; i++)
	{
		VK_CHECK(vkCreateSemaphore(m_device.Get(), &semaphoreInfo, nullptr, &m_imageAvailableSemaphores[i]), "Failed to create image available semaphore!");
		VK_CHECK(vkCreateSemaphore(m_device.Get(), &semaphoreInfo, nullptr, &m_renderFinishedSemaphores[i]), "Failed to create image available semaphore!");
		VK_CHECK(vkCreateFence(m_device.Get(), &fenceInfo, nullptr, &m_inFlightFences[i]), "Failed to create in flight fence!");
	}
}

void Application::Shutdown()
{
	m_physicalDevice.GetSwapchain().Shutdown();

	m_pipeline.Shutdown();

	for (size_t i = 0; i < MAX_FRAMES_IN_FLIGHT; i++)
	{
		vkDestroySemaphore(GetDevice().Get(), m_renderFinishedSemaphores[i], nullptr);
		vkDestroySemaphore(GetDevice().Get(), m_imageAvailableSemaphores[i], nullptr);
		vkDestroyFence(GetDevice().Get(), m_inFlightFences[i], nullptr);
	}
	
	vkDestroyCommandPool(GetDevice().Get(), m_commandPool, nullptr);
	
	m_device.Shutdown();

	if (ENABLE_VALIDATION_LAYERS)
		DestroyDebugUtilsMessengerEXT(m_instance, m_debugMessenger, nullptr);

	m_physicalDevice.Shutdown();

	vkDestroyInstance(m_instance, nullptr);
	glfwDestroyWindow(m_window);
	glfwTerminate();
}

void Application::OnFramebufferResize(GLFWwindow* window, int width, int height)
{
	auto app = reinterpret_cast<Application*>(glfwGetWindowUserPointer(window));
		
	app->m_framebufferResized = true;
}

std::vector<const char*> Application::GetRequiredExtensions()
{
	uint32_t glfwExtensionCount{ 0 };
	const char** glfwExtensions;
	glfwExtensions = glfwGetRequiredInstanceExtensions(&glfwExtensionCount);

	std::vector<const char*> extensions(glfwExtensions, glfwExtensions + glfwExtensionCount);

	if (ENABLE_VALIDATION_LAYERS)
		extensions.push_back(VK_EXT_DEBUG_UTILS_EXTENSION_NAME);

	return extensions;
}

bool Application::HasValidationSupport()
{
	uint32_t layerCount;
	vkEnumerateInstanceLayerProperties(&layerCount, nullptr);

	std::vector<VkLayerProperties> availableLayers(layerCount);
	vkEnumerateInstanceLayerProperties(&layerCount, availableLayers.data());

	for (const char* layerName : VALIDATION_LAYERS)
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
	renderPassInfo.renderPass = m_pipeline.GetRenderPass();
	renderPassInfo.framebuffer = m_physicalDevice.GetSwapchain().GetFramebuffers()[imageIndex];
	renderPassInfo.renderArea.offset = { 0, 0 };
	renderPassInfo.renderArea.extent = m_physicalDevice.GetSwapchain().GetExtent();
	renderPassInfo.clearValueCount = 1;
	renderPassInfo.pClearValues = &clearValue;

	vkCmdBeginRenderPass(commandBuffer, &renderPassInfo, VK_SUBPASS_CONTENTS_INLINE);

	vkCmdBindPipeline(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, m_pipeline.GetPipeline());
	vkCmdDraw(commandBuffer, 3, 1, 0, 0);

	vkCmdEndRenderPass(commandBuffer);

	VK_CHECK(vkEndCommandBuffer(commandBuffer), "Failed to record command buffer!");
}

void Application::DrawFrame()
{
	vkWaitForFences(GetDevice().Get(), 1, &m_inFlightFences[m_currentFrame], VK_TRUE, UINT64_MAX);

	uint32_t imageIndex;
	VkResult result = vkAcquireNextImageKHR(GetDevice().Get(), m_physicalDevice.GetSwapchain().Get(), UINT64_MAX, m_imageAvailableSemaphores[m_currentFrame], VK_NULL_HANDLE, &imageIndex);

	if (result == VK_ERROR_OUT_OF_DATE_KHR)
	{
		GetPhysicalDevice().GetSwapchain().Recreate();
		return;
	}
	else if (result != VK_SUCCESS && result != VK_SUBOPTIMAL_KHR) {
		CORE_ERROR("Failed to acquire swapchain image!");
	}
	
	vkResetFences(GetDevice().Get(), 1, &m_inFlightFences[m_currentFrame]);

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

	VK_CHECK(vkQueueSubmit(GetDevice().GetGraphicsQueue(), 1, &submitInfo, m_inFlightFences[m_currentFrame]), "Failed to submit draw command buffer!");

	VkPresentInfoKHR presentInfo{};
	presentInfo.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;

	presentInfo.waitSemaphoreCount = 1;
	presentInfo.pWaitSemaphores = signalSemaphores;

	VkSwapchainKHR swapchains[] = { GetPhysicalDevice().GetSwapchain().Get() };
	presentInfo.swapchainCount = 1;
	presentInfo.pSwapchains = swapchains;

	presentInfo.pImageIndices = &imageIndex;

	result = vkQueuePresentKHR(GetDevice().GetPresentQueue(), &presentInfo);

	if (result == VK_ERROR_OUT_OF_DATE_KHR || result == VK_SUBOPTIMAL_KHR && m_framebufferResized)
	{
		m_framebufferResized = false;
		m_physicalDevice.GetSwapchain().Recreate();
	}
	else if (result != VK_SUCCESS)
		CORE_ERROR("Failed to present swapchain image!");

	m_currentFrame = (m_currentFrame + 1) % MAX_FRAMES_IN_FLIGHT;
}