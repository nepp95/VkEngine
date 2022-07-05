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
	const ApplicationSpecification& GetSpecification() const { return m_specification; }
	static Application& Get() { return *s_instance; }

private:
	void Init();
	void Cleanup();

private:
	ApplicationSpecification m_specification;
	GLFWwindow* m_window;

	static Application* s_instance;
};