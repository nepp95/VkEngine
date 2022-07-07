#pragma once

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
	VkSurfaceKHR& GetSurface() { return m_surface; }
	VkDevice& GetDevice() { return m_device; }

	const ApplicationSpecification& GetSpecification() const { return m_specification; }
	const GLFWwindow& GetWindow() const { return *m_window; }
	GLFWwindow& GetWindow() { return *m_window; }
	static Application& Get() { return *s_instance; }

private:
	void Init();
	void Cleanup();

	void RecordCommandBuffer(VkCommandBuffer commandBuffer, uint32_t imageIndex);
	void DrawFrame();

private:
	ApplicationSpecification m_specification;
	GLFWwindow* m_window;

	// Vulkan base
	VkInstance m_instance;
	VkPhysicalDevice m_physicalDevice;
	VkSurfaceKHR m_surface;
	VkDebugUtilsMessengerEXT m_debugMessenger;

	// Logical device
	VkDevice m_device;
	VkQueue m_graphicsQueue;
	VkQueue m_presentQueue;
	
	// Swapchain
	VkSwapchainKHR m_swapchain;
	VkExtent2D m_swapchainExtent;
	VkFormat m_swapchainImageFormat;
	std::vector<VkImage> m_swapchainImages;
	std::vector<VkImageView> m_swapchainImageViews;
	std::vector<VkFramebuffer> m_swapChainFramebuffers;

	// Pipeline
	VkRenderPass m_renderPass;
	VkPipelineLayout m_pipelineLayout;
	VkPipeline m_pipeline;

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