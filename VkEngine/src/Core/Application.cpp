#include "pch.h"
#include "Application.h"

Application* Application::s_instance = nullptr;

Application::Application(const ApplicationSpecification& specification)
	: m_specification(specification)
{
	CORE_ASSERT(!s_instance, "Application already exists!");
	s_instance = this;

	Init();
}

Application::~Application()
{
	Cleanup();
}

void Application::Run()
{
	while (!glfwWindowShouldClose(m_window))
	{
		glfwPollEvents();
	}
}

void Application::Init()
{
	// Set working directory
	if (!m_specification.WorkingDirectory.empty())
		std::filesystem::current_path(m_specification.WorkingDirectory);

	// Glfw
	glfwInit();

	glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
	glfwWindowHint(GLFW_RESIZABLE, GLFW_FALSE);

	m_window = glfwCreateWindow(m_specification.WindowWidth, m_specification.WindowHeight, m_specification.Name.c_str(), nullptr, nullptr);
}

void Application::Cleanup()
{
	glfwDestroyWindow(m_window);
	glfwTerminate();
}