#include "pch.h"
#include "Device.h"

#include "Core/Application.h"

Device::Device()
{}

Device::~Device()
{}

void Device::Create()
{
	QueueFamilyIndices indices = FindQueueFamilies(Application::Get().GetPhysicalDevice().Get(), Application::Get().GetPhysicalDevice().GetSurface());

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
	deviceCreateInfo.enabledExtensionCount = DEVICE_EXTENSIONS.size();
	deviceCreateInfo.ppEnabledExtensionNames = DEVICE_EXTENSIONS.data();

	if (ENABLE_VALIDATION_LAYERS)
	{
		deviceCreateInfo.enabledLayerCount = static_cast<uint32_t>(VALIDATION_LAYERS.size());
		deviceCreateInfo.ppEnabledLayerNames = VALIDATION_LAYERS.data();
	}
	else {
		deviceCreateInfo.enabledLayerCount = 0;
	}

	VK_CHECK(vkCreateDevice(Application::Get().GetPhysicalDevice().Get(), &deviceCreateInfo, nullptr, &m_device), "Failed to create logical device!");

	vkGetDeviceQueue(m_device, indices.GraphicsFamily.value(), 0, &m_graphicsQueue);
	vkGetDeviceQueue(m_device, indices.PresentFamily.value(), 0, &m_presentQueue);
}

void Device::Shutdown()
{
	vkDestroyDevice(m_device, nullptr);
}