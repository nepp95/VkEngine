#pragma once

#include "Vulkan/Swapchain.h"

class PhysicalDevice
{
public:
	PhysicalDevice();
	~PhysicalDevice();

	void Shutdown();

	void ChooseDevice();

	Swapchain& GetSwapchain() { return m_swapchain; }
	VkSurfaceKHR& GetSurface() { return m_surface; }
	VkPhysicalDevice& Get() { return m_device; }

private:
	VkPhysicalDevice m_device;
	VkSurfaceKHR m_surface;
	
	Swapchain m_swapchain;
};