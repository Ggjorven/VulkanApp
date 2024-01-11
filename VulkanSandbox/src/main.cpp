#include <VulkanCore/Core/Application.hpp>
#include <VulkanCore/Entrypoint.hpp>

#include "Custom.hpp"

// Create your own application class
class Sandbox : public VkApp::Application
{
public:
	Sandbox(const VkApp::AppInfo& appInfo)
		: VkApp::Application(appInfo)
	{
		// Add your own custom layers/overlays
		AddLayer(new CustomLayer());
	}
};



// ----------------------------------------------------------------
//                    Set Application specs here...
// ----------------------------------------------------------------
VkApp::Application* VkApp::CreateApplication(int argc, char* argv[])
{
	AppInfo appInfo;
	appInfo.ArgCount = argc;
	appInfo.Args = argv;

	// Add more application specs...
	appInfo.WindowProperties.Name = "Custom";

	return new Sandbox(appInfo);
}