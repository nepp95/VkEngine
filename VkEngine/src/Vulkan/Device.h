#pragma once

#include "PhysicalDevice.h"

class Device
{
public:
	Device();
	~Device();

	void Create();
	void Shutdown();

	VkQueue& GetGraphicsQueue() { return m_graphicsQueue; }
	VkQueue& GetPresentQueue() { return m_presentQueue; }
	const VkDevice& Get() const { return m_device; }

private:
	PhysicalDevice m_physicalDevice;

	VkDevice m_device;
	VkQueue m_graphicsQueue;
	VkQueue m_presentQueue;
};