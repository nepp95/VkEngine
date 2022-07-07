#include "pch.h"
#include "Swapchain.h"

#include "Core/Application.h"

Swapchain::Swapchain()
{}

Swapchain::~Swapchain()
{}

void Swapchain::Recreate()
{
	int width{ 0 }, height{ 0 };
	glfwGetFramebufferSize(&Application::Get().GetWindow(), &width, &height);

	while (width == 0 || height == 0)
	{
		glfwGetFramebufferSize(&Application::Get().GetWindow(), &width, &height);
		glfwWaitEvents();
	}

	vkDeviceWaitIdle(Application::Get().GetDevice().Get());

	Shutdown();

	Create();
	CreateImageViews();
	CreateFramebuffers();
}

void Swapchain::Create()
{
	SwapChainSupportDetails swapChainSupport = QuerySwapChainSupport(Application::Get().GetPhysicalDevice().Get(), Application::Get().GetPhysicalDevice().GetSurface());

	VkSurfaceFormatKHR surfaceFormat = ChooseSwapSurfaceFormat(swapChainSupport.Formats);
	m_swapchainImageFormat = surfaceFormat.format;
	VkPresentModeKHR presentMode = ChooseSwapPresentMode(swapChainSupport.PresentModes);
	m_swapchainExtent = ChooseSwapExtent(swapChainSupport.Capabilities);

	uint32_t imageCount = swapChainSupport.Capabilities.minImageCount + 1;
	if (swapChainSupport.Capabilities.maxImageCount > 0 && imageCount > swapChainSupport.Capabilities.maxImageCount)
		imageCount = swapChainSupport.Capabilities.maxImageCount;

	VkSwapchainCreateInfoKHR swapchainCreateInfo{};
	swapchainCreateInfo.sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR;
	swapchainCreateInfo.surface = Application::Get().GetPhysicalDevice().GetSurface();
	swapchainCreateInfo.minImageCount = imageCount;
	swapchainCreateInfo.imageFormat = surfaceFormat.format;
	swapchainCreateInfo.imageColorSpace = surfaceFormat.colorSpace;
	swapchainCreateInfo.imageExtent = m_swapchainExtent;
	swapchainCreateInfo.imageArrayLayers = 1;
	swapchainCreateInfo.imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;

	QueueFamilyIndices indices = FindQueueFamilies(Application::Get().GetPhysicalDevice().Get(), Application::Get().GetPhysicalDevice().GetSurface());
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

	VK_CHECK(vkCreateSwapchainKHR(Application::Get().GetDevice().Get(), &swapchainCreateInfo, nullptr, &m_swapchain), "Failed to create swapchain!");

	vkGetSwapchainImagesKHR(Application::Get().GetDevice().Get(), m_swapchain, &imageCount, nullptr);
	m_swapchainImages.resize(imageCount);
	vkGetSwapchainImagesKHR(Application::Get().GetDevice().Get(), m_swapchain, &imageCount, m_swapchainImages.data());
}

void Swapchain::CreateImageViews()
{
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

		VK_CHECK(vkCreateImageView(Application::Get().GetDevice().Get(), &ImageViewCreateInfo, nullptr, &m_swapchainImageViews[i]), "Failed to create image views!");
	}
}

void Swapchain::CreateFramebuffers()
{
	m_swapChainFramebuffers.resize(m_swapchainImageViews.size());

	for (size_t i = 0; i < m_swapchainImageViews.size(); i++)
	{
		VkImageView attachments[] = {
			m_swapchainImageViews[i]
		};

		VkFramebufferCreateInfo framebufferCreateInfo{};
		framebufferCreateInfo.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
		framebufferCreateInfo.renderPass = Application::Get().GetPipeline().GetRenderPass();
		framebufferCreateInfo.attachmentCount = 1;
		framebufferCreateInfo.pAttachments = attachments;
		framebufferCreateInfo.width = m_swapchainExtent.width;
		framebufferCreateInfo.height = m_swapchainExtent.height;
		framebufferCreateInfo.layers = 1;

		VK_CHECK(vkCreateFramebuffer(Application::Get().GetDevice().Get(), &framebufferCreateInfo, nullptr, &m_swapChainFramebuffers[i]), "Failed to create framebuffer!");
	}
}

void Swapchain::Shutdown()
{
	for (auto framebuffer : m_swapChainFramebuffers)
		vkDestroyFramebuffer(Application::Get().GetDevice().Get(), framebuffer, nullptr);

	for (auto imageView : m_swapchainImageViews)
		vkDestroyImageView(Application::Get().GetDevice().Get(), imageView, nullptr);

	vkDestroySwapchainKHR(Application::Get().GetDevice().Get(), m_swapchain, nullptr);
}

VkSurfaceFormatKHR Swapchain::ChooseSwapSurfaceFormat(const std::vector<VkSurfaceFormatKHR>& availableFormats)
{
	for (const auto& format : availableFormats)
	{
		if (format.format == VK_FORMAT_B8G8R8A8_SRGB && format.colorSpace == VK_COLOR_SPACE_SRGB_NONLINEAR_KHR)
			return format;
	}

	return availableFormats[0];
}

VkPresentModeKHR Swapchain::ChooseSwapPresentMode(const std::vector<VkPresentModeKHR>& availablePresentModes)
{
	for (const auto& presentMode : availablePresentModes)
	{
		//if (presentMode == VK_PRESENT_MODE_MAILBOX_KHR)
		if (presentMode == VK_PRESENT_MODE_IMMEDIATE_KHR)
			return presentMode;
	}

	return VK_PRESENT_MODE_FIFO_KHR;
}

VkExtent2D Swapchain::ChooseSwapExtent(const VkSurfaceCapabilitiesKHR& capabilities)
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