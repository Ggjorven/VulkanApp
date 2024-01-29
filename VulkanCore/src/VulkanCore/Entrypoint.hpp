#pragma once

// To be defined by user.
extern VkApp::Application* VkApp::CreateApplication(int argc, char* argv[]);

#if !defined(VKAPP_DIST) // Non Dist build on all Platforms

int main(int argc, char* argv[])
{
	VkApp::Application* app = VkApp::CreateApplication(argc, argv);
	app->Run();
	delete app;
	return 0;
}

#elif defined(VKAPP_PLATFORM_WINDOWS) // Dist on Windows
#include <Windows.h>

int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, PSTR lpCmdLine, int nCmdShow)
{
    VkApp::Application* app = VkApp::CreateApplication(__argc, __argv);
    app->Run();
    delete app;
    return 0;
} 
#else // Dist on all other platforms // Maybe fix this? Or maybe this works? // TODO(Jorben): Test this on MacOS and Linux

int main(int argc, char* argv[])
{
	VkApp::Application* app = VkApp::CreateApplication(argc, argv);
	app->Run();
	delete app;
	return 0;
}

#endif
