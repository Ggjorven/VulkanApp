#include "Camera.hpp"

#include <VulkanCore/Core/Input/Input.hpp>


Camera::Camera()
{
}

void Camera::OnUpdate(float deltaTime)
{
    const Window& window = Application::Get().GetWindow();

    if (Input::IsKeyPressed(Key::Escape))
        m_Escaped = !m_Escaped;

    if (!Application::Get().IsMinimized() && !m_Escaped)
    {
        // Keyboard input
        if (Input::IsKeyPressed(Key::W))
            m_Position += m_Speed * m_Front * deltaTime;
        if (Input::IsKeyPressed(Key::S))
            m_Position -= m_Speed * m_Front * deltaTime;
        if (Input::IsKeyPressed(Key::A))
            m_Position -= glm::normalize(glm::cross(m_Front, m_Up)) * m_Speed * deltaTime;
        if (Input::IsKeyPressed(Key::D))
            m_Position += glm::normalize(glm::cross(m_Front, m_Up)) * m_Speed * deltaTime;

        // Mouse input for camera
        glm::vec2 mousePos = Input::GetMousePosition();
        if (m_FirstMouse)
        {
            Input::SetCursorPosition({ (float)window.GetWidth() / 2.0f, (float)window.GetHeight() / 2.0f });
            m_LastX = mousePos.x;
            m_LastY = mousePos.y;
            m_FirstMouse = false;
        }

        float xOffset = mousePos.x - m_LastX;
        float yOffset = m_LastY - mousePos.y; // Reversed since y-coordinates range from bottom to top
        m_LastX = mousePos.x;
        m_LastY = mousePos.y;

        xOffset *= m_MouseSensitivity;
        yOffset *= m_MouseSensitivity;

        m_Yaw += xOffset;
        m_Pitch += yOffset;

        // Make sure that when pitch is out of bounds, screen doesn't get flipped
        if (m_Pitch > 89.0f)
            m_Pitch = 89.0f;
        if (m_Pitch < -89.0f)
            m_Pitch = -89.0f;

        // Calculate the new Front vector
        glm::vec3 front(1.0f);
        front.x = cos(glm::radians(m_Yaw)) * cos(glm::radians(m_Pitch));
        front.y = sin(glm::radians(m_Pitch));
        front.z = sin(glm::radians(m_Yaw)) * cos(glm::radians(m_Pitch));
        m_Front = glm::normalize(front);

        // Calculate the Right vector
        m_Right = glm::normalize(glm::cross(m_Front, m_Up)); // Normalize the vectors, because their length gets closer to 0 the more you look up or down which results in slower movement.

        // Update the View matrix
        m_View = glm::lookAt(m_Position, m_Position + m_Front, m_Up);

        Input::SetCursorPosition({ (float)window.GetWidth() / 2.0f, (float)window.GetHeight() / 2.0f });
    }
}

