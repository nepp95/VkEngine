#include "pch.h"
#include "Buffer.h"

#include "Core/Application.h"

VertexBuffer::VertexBuffer(const void* vertices, uint32_t size)
{
	VkDeviceSize bufferSize = size;

	VkBuffer stagingBuffer;
	VkDeviceMemory stagingBufferMemory;

	Application::Get().CreateBuffer(bufferSize, VK_BUFFER_USAGE_TRANSFER_SRC_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, stagingBuffer, stagingBufferMemory);

	void* data;
	vkMapMemory(Application::Get().GetDevice().Get(), stagingBufferMemory, 0, bufferSize, 0, &data);
	memcpy(data, vertices, (size_t)bufferSize);
	vkUnmapMemory(Application::Get().GetDevice().Get(), stagingBufferMemory);

	Application::Get().CreateBuffer(bufferSize, VK_BUFFER_USAGE_VERTEX_BUFFER_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, m_buffer, m_bufferMemory);

	Application::Get().CopyBuffer(stagingBuffer, m_buffer, bufferSize);

	vkDestroyBuffer(Application::Get().GetDevice().Get(), stagingBuffer, nullptr);
	vkFreeMemory(Application::Get().GetDevice().Get(), stagingBufferMemory, nullptr);
}

VertexBuffer::~VertexBuffer()
{
	Shutdown();
}

void VertexBuffer::Shutdown()
{
	vkDestroyBuffer(Application::Get().GetDevice().Get(), m_buffer, nullptr);
	vkFreeMemory(Application::Get().GetDevice().Get(), m_bufferMemory, nullptr);
}

void VertexBuffer::SetData(const void* data, uint32_t size)
{
	VkDeviceSize bufferSize = size;

	VkBuffer stagingBuffer;
	VkDeviceMemory stagingBufferMemory;

	Application::Get().CreateBuffer(bufferSize, VK_BUFFER_USAGE_TRANSFER_SRC_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, stagingBuffer, stagingBufferMemory);

	void* tempData;
	vkMapMemory(Application::Get().GetDevice().Get(), stagingBufferMemory, 0, bufferSize, 0, &tempData);
	memcpy(tempData, data, (size_t)bufferSize);
	vkUnmapMemory(Application::Get().GetDevice().Get(), stagingBufferMemory);

	Application::Get().CreateBuffer(bufferSize, VK_BUFFER_USAGE_VERTEX_BUFFER_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, m_buffer, m_bufferMemory);

	Application::Get().CopyBuffer(stagingBuffer, m_buffer, bufferSize);

	vkDestroyBuffer(Application::Get().GetDevice().Get(), stagingBuffer, nullptr);
	vkFreeMemory(Application::Get().GetDevice().Get(), stagingBufferMemory, nullptr);
}