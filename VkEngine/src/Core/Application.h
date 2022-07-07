#pragma once

#include "Vulkan/Base.h"
#include "Vulkan/Device.h"
#include "Vulkan/PhysicalDevice.h"
#include "Vulkan/Pipeline.h"

#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>

struct ApplicationCommandLineArgs
{
	int Count = 0;
	char** Args = nullptr;

	const char* operator[](int index) const
	{
		CORE_ASSERT(index < Count, "Command line argument out of bounds!");
		return Args[index];
	}
};

struct ApplicationSpecification
{
	std::string Name = "Application";
	std::string WorkingDirectory;

	uint32_t WindowWidth = 1280;
	uint32_t WindowHeight = 720;

	ApplicationCommandLineArgs CommandLineArgs;
};

class Application
{
public:
	Application(const ApplicationSpecification& specification);
	~Application();

	void Run();

	// Gets
	VkInstance& GetInstance() { return m_instance; }
	PhysicalDevice& GetPhysicalDevice() { return m_physicalDevice; }
	Device& GetDevice() { return m_device; }
	Pipeline& GetPipeline() { return m_pipeline; }

	const ApplicationSpecification& GetSpecification() const { return m_specification; }
	const GLFWwindow& GetWindow() const { return *m_window; }
	GLFWwindow& GetWindow() { return *m_window; }
	static Application& Get() { return *s_instance; }

private:
	void Init();
	void Shutdown();

	// VkInstance
	std::vector<const char*> GetRequiredExtensions();
	bool HasValidationSupport();
	//

	void RecordCommandBuffer(VkCommandBuffer commandBuffer, uint32_t imageIndex);
	void DrawFrame();

private:
	ApplicationSpecification m_specification;
	GLFWwindow* m_window;

	// Vulkan base
	VkInstance m_instance;
	VkDebugUtilsMessengerEXT m_debugMessenger;

	PhysicalDevice m_physicalDevice;
	Device m_device;

	Pipeline m_pipeline;

	// Command pool/buffers
	VkCommandPool m_commandPool;
	std::vector<VkCommandBuffer> m_commandBuffers;

	// Syncing
	std::vector<VkSemaphore> m_imageAvailableSemaphores; // image has been acquired from swapchain and ready for render
	std::vector<VkSemaphore> m_renderFinishedSemaphores; // finished render and ready for presentation
	std::vector<VkFence> m_inFlightFences; // fence to make sure only one frame renders at a time
	uint32_t m_currentFrame{ 0 };

	// Static instance
	static Application* s_instance;
};