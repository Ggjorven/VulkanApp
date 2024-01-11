#include <VulkanCore/Core/Layer.hpp>

using namespace VkApp;

// Create a custom layer with custom logic
class CustomLayer : public Layer
{
public:
	void OnAttach() override;
	void OnDetach() override;

	void OnUpdate(float deltaTime) override;
	void OnRender() override;
	void OnImGuiRender() override;

	void OnEvent(Event& e) override;

private:
	bool HandleWindowResize(WindowResizeEvent& e);
};