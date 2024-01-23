#include "Camera.hpp"

#include <VulkanCore/Core/Input/Input.hpp>


Camera::Camera(float aspectRatio)
    : m_AspectRatio(aspectRatio)
{
    UpdateMatrices();
    UpdateArea();
}

Camera::~Camera()
{
}

void Camera::OnUpdate(float deltaTime)
{
    UpdateMatrices();

    if (Input::IsKeyPressed(Key::LeftAlt))
    {
        float velocity = m_MovementSpeed * deltaTime;
        glm::vec3 moveDirection = { 0.0f, 0.0f, 0.0f };

        auto& area = m_Area;

        // Calculate forward/backward and left/right movement.
        if (Input::IsKeyPressed(Key::W))
            moveDirection += area.Front;
        if (Input::IsKeyPressed(Key::S))
            moveDirection -= area.Front;
        if (Input::IsKeyPressed(Key::A))
            moveDirection -= area.Right;
        if (Input::IsKeyPressed(Key::D))
            moveDirection += area.Right;

        // Calculate up/down movement.
        if (Input::IsKeyPressed(Key::Space))
            moveDirection += area.Up;
        if (Input::IsKeyPressed(Key::LeftShift))
            moveDirection -= area.Up;

        if (glm::length(moveDirection) > 0.0f)
            moveDirection = glm::normalize(moveDirection);

        // Update the camera position.
        m_Position += moveDirection * velocity;

        if (m_FirstUpdate)
        {
            m_LastMousePosition = Input::GetMousePosition();
            m_FirstUpdate = false;
        }

        //Mouse movement
        auto mousePosition = Input::GetMousePosition();

        float xOffset = static_cast<float>(mousePosition.x - m_LastMousePosition.x);
        float yOffset = static_cast<float>(m_LastMousePosition.y - mousePosition.y);

        //Reset cursor
        Window& window = Application::Get().GetWindow();
        int width = window.GetWidth();
        int height = window.GetHeight();

        Input::SetCursorPosition({ width / 2.0f, height / 2.0f });

        m_LastMousePosition.x = static_cast<float>(width / 2.f);
        m_LastMousePosition.y = static_cast<float>(height / 2.f);

        xOffset *= m_MouseSensitivity;
        yOffset *= m_MouseSensitivity;

        //Set new settings
        auto& settings = m_Properties;

        settings.Yaw += xOffset;
        settings.Pitch += yOffset;

        // Cap movement
        if (settings.Pitch > 89.0f)
            settings.Pitch = 89.0f;
        if (settings.Pitch < -89.0f)
            settings.Pitch = -89.0f;

        UpdateArea();

        Input::SetCursorMode(CursorMode::Disabled);
    }
    else
    {
        Input::SetCursorMode(CursorMode::Shown);
        m_FirstUpdate = true;
    }
}

void Camera::OnEvent(Event& e)
{
    EventHandler handler(e);

    handler.Handle<WindowResizeEvent>(VKAPP_BIND_EVENT_FN(Camera::Resize));
}

void Camera::UpdateMatrices()
{
    m_ViewMatrix = glm::lookAt(m_Position, m_Position + m_Area.Front, m_Area.Up);
    m_ProjectionMatrix = glm::perspective(glm::radians(m_Properties.FOV), m_AspectRatio, 0.1f, 100.0f);
    m_ViewProjectionMatrix = m_ProjectionMatrix * m_ViewProjectionMatrix;
}

void Camera::UpdateArea()
{
    glm::vec3 newFront(1.0f);

    newFront.x = cos(glm::radians(m_Properties.Yaw)) * cos(glm::radians(m_Properties.Pitch));
    newFront.y = sin(glm::radians(m_Properties.Pitch));
    newFront.z = sin(glm::radians(m_Properties.Yaw)) * cos(glm::radians(m_Properties.Pitch));

    m_Area.Front = glm::normalize(newFront);
    m_Area.Right = glm::normalize(glm::cross(m_Area.Front, m_Area.Up));
}

bool Camera::Resize(WindowResizeEvent& e)
{
    m_AspectRatio = (float)e.GetWidth() / (float)e.GetHeight();

    return false;
}

