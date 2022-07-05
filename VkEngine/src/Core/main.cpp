#include "pch.h"

#include "Core/Application.h"

int main(int argc, char* argv[])
{
	ApplicationSpecification specification;
	specification.Name = "VkEngine";
	specification.CommandLineArgs = { argc, argv };

	// Create application
	auto app = new Application(specification);

	// Run application
	app->Run();

	// Delete application
	delete app;

	// Return succesful
	return 0;
}