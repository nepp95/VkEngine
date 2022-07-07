#include "pch.h"
#include "PhysicalDevice.h"

#include "Core/Application.h"

PhysicalDevice::PhysicalDevice()
{}

PhysicalDevice::~PhysicalDevice()
{}

void PhysicalDevice::Shutdown()
{
	vkDestroySurfaceKHR(Application::Get().GetInstance(), m_surface, nullptr);
}

void PhysicalDevice::ChooseDevice()
{
	uint32_t deviceCount{ 0 };
	vkEnumeratePhysicalDevices(Application::Get().GetInstance(), &deviceCount, nullptr);

	// If there is no Vulkan supported GPU, there is no point in continuing
	CORE_ASSERT(deviceCount, "Failed to find any GPU with Vulkan support!");

	std::vector<VkPhysicalDevice> devices(deviceCount);
	vkEnumeratePhysicalDevices(Application::Get().GetInstance(), &deviceCount, devices.data());

	for (const auto& device : devices)
	{
		if (IsDeviceSuitable(device, m_surface))
		{
			m_device = device;
			break;
		}
	}

	// Although there was atleast 1 device with Vulkan support, it/they did not meet our requirements
	CORE_ASSERT(m_device, "Failed to find a suitable GPU!");
}