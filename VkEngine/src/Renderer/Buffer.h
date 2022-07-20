#pragma once

#include "Vulkan/Base.h"

#include <glm/glm.hpp>

struct Vertex
{
	glm::vec2 Position;
	glm::vec3 Color;

	static VkVertexInputBindingDescription GetBindingDescription()
	{
		VkVertexInputBindingDescription bindingDescription{};
		bindingDescription.binding = 0;
		bindingDescription.stride = sizeof(Vertex);
		bindingDescription.inputRate = VK_VERTEX_INPUT_RATE_VERTEX;

		return bindingDescription;
	}

	static std::array<VkVertexInputAttributeDescription, 2> GetAttributeDescriptions()
	{
		std::array<VkVertexInputAttributeDescription, 2> attributeDescriptions{};

		attributeDescriptions[0].binding = 0;
		attributeDescriptions[0].location = 0;
		attributeDescriptions[0].format = VK_FORMAT_R32G32_SFLOAT;
		attributeDescriptions[0].offset = offsetof(Vertex, Position);

		attributeDescriptions[1].binding = 0;
		attributeDescriptions[1].location = 1;
		attributeDescriptions[1].format = VK_FORMAT_R32G32B32_SFLOAT;
		attributeDescriptions[1].offset = offsetof(Vertex, Color);

		return attributeDescriptions;
	}
};

class VertexBuffer
{
public:
	VertexBuffer(const void* vertices, uint32_t size);
	~VertexBuffer();

	void Shutdown();

	void SetData(const void* data, uint32_t size);

	VkBuffer& GetBuffer() { return m_buffer; }
	VkDeviceMemory& GetDeviceMemory() { return m_bufferMemory; }

private:
	VkBuffer m_buffer;
	VkDeviceMemory m_bufferMemory;
};