#pragma once

#include "VulkanCore/Core/Input/Input.hpp"

namespace VkApp
{

	class WindowsInput : public Input
	{
	public:
		WindowsInput() = default;
		virtual ~WindowsInput() = default;

		bool IsKeyPressedImplementation(Key keycode) override;
		bool IsMousePressedImplementation(MouseButton button) override;

		glm::vec2 GetMousePositionImplementation() override;

		void SetCursorPositionImplementation(glm::vec2 position) override;
		void SetCursorModeImplementation(CursorMode mode) override;
	};

}
