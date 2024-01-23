#pragma once

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

#include <VulkanCore/Core/Application.hpp>
#include <VulkanCore/Core/Events.hpp>

using namespace VkApp;

struct MovementArea
{
public:
	glm::vec3 Front;
	glm::vec3 Up;
	glm::vec3 Right;

	MovementArea(const glm::vec3& front, const glm::vec3& up, const glm::vec3& right)
		: Front(front), Up(up), Right(right)
	{
	}
};

struct CameraSettings
{
public:
	float Yaw;
	float Pitch;
	float FOV;

	CameraSettings(float yaw, float pitch, float fov)
		: Yaw(yaw), Pitch(pitch), FOV(fov)
	{
	}
};

class Camera
{
public:
	Camera() = default;
	Camera(float aspectRatio);
	virtual ~Camera();

	void OnUpdate(float deltaTime);
	void OnEvent(Event& e);

	inline glm::mat4 GetViewMatrix() const { return m_ViewMatrix; }
	inline glm::mat4 GetProjectionMatrix() const { return m_ProjectionMatrix; }

	inline void SetAspectRatio(float aspectRatio) { m_AspectRatio = aspectRatio; UpdateMatrices(); UpdateArea(); }

private:
	void UpdateMatrices();
	void UpdateArea();

	bool Resize(WindowResizeEvent& e);

private:
	glm::vec3 m_Position = { 0.0f, 0.0f, 0.0f };

	glm::mat4 m_ViewMatrix = { };
	glm::mat4 m_ProjectionMatrix = { };
	glm::mat4 m_ViewProjectionMatrix = { };

	MovementArea m_Area = { glm::vec3(0.0f, 0.0f, -1.0f), glm::vec3(0.0f, 1.0f, 0.0f), glm::vec3(1.0f, 0.0f, 0.0f) };
	CameraSettings m_Properties = { 0.0f, 0.0f, 45.0f };

	glm::vec2 m_LastMousePosition = { 0.0f, 0.0f };
	bool m_FirstUpdate = true;

	float m_AspectRatio = 0.0f;

	float m_MovementSpeed = 2.5f;
	float m_MouseSensitivity = 0.1f;
};