#pragma once

#include "Vulkan/Base.h"

class Swapchain
{
public:
	Swapchain();
	~Swapchain();

	void Recreate();

	void Create();
	void CreateImageViews();
	void CreateFramebuffers();
	void Shutdown();

	const VkFormat& GetFormat() const { return m_swapchainImageFormat; }
	const VkExtent2D GetExtent() const { return m_swapchainExtent; }
	const std::vector<VkFramebuffer>& GetFramebuffers() const { return m_swapChainFramebuffers; }
	VkSwapchainKHR& Get() { return m_swapchain; }

private:
	VkSurfaceFormatKHR ChooseSwapSurfaceFormat(const std::vector<VkSurfaceFormatKHR>& availableFormats);
	VkPresentModeKHR ChooseSwapPresentMode(const std::vector<VkPresentModeKHR>& availablePresentModes);
	VkExtent2D ChooseSwapExtent(const VkSurfaceCapabilitiesKHR& capabilities);

private:
	VkSwapchainKHR m_swapchain;
	VkExtent2D m_swapchainExtent;
	VkFormat m_swapchainImageFormat;
	std::vector<VkImage> m_swapchainImages;
	std::vector<VkImageView> m_swapchainImageViews;
	std::vector<VkFramebuffer> m_swapChainFramebuffers;
};