#pragma once

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

#include <VulkanCore/Core/Application.hpp>

using namespace VkApp;

class Camera
{
public:
	Camera();

	void OnUpdate(float deltaTime);

	glm::mat4 GetViewMatrix() const { return m_View; }

private:
	bool m_Escaped = true;

	glm::vec3 m_Position = glm::vec3(1.0f, 1.0f, 1.0f);
	glm::vec3 m_Up = glm::vec3(0.0f, 0.0f, 1.0f);
	glm::vec3 m_Front = glm::vec3(0.0f, 0.0f, -1.0f);
	glm::vec3 m_Right = glm::vec3(1.0f);
	float m_Speed = 2.5f;

	float m_Yaw = -90.0f;
	float m_Pitch = 0.0f;
	float m_LastX = 0.0f;
	float m_LastY = 0.0f;
	bool m_FirstMouse = true;
	float m_MouseSensitivity = 0.1f;

	glm::mat4 m_View = {};
};