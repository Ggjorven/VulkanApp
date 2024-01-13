#include <VulkanCore/Core/Layer.hpp>

#include <vulkan/vulkan.h>

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

private:
	VkBuffer m_VertexBuffer = VK_NULL_HANDLE;
	VkDeviceMemory m_VertexBufferMemory = VK_NULL_HANDLE;
};