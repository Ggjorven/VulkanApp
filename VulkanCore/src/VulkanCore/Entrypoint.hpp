#pragma once

// To be defined by user.
extern VkApp::Application* VkApp::CreateApplication(int argc, char* argv[]);

int main(int argc, char* argv[])
{
	VkApp::Application* app = VkApp::CreateApplication(argc, argv);
	app->Run();
	delete app;
	return 0;
}

