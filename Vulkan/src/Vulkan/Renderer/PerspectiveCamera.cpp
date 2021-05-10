#include "pch.h"
#include "PerspectiveCamera.h"

PerspectiveCamera::PerspectiveCamera(float width, float height, VulkanCore* core)
	:m_ScreenWidth(width), m_ScreenHeight(height)
{
	m_Position = { 0.0f, 0.0f, -3.0f };

	m_AspectRatio = m_ScreenWidth / m_ScreenHeight;

	m_ProjectionMatrix = glm::perspective(glm::radians(m_FOV), m_AspectRatio, 0.1f, 100.0f);
	m_ViewMatrix = glm::lookAt(m_Position, m_Target, m_WorldUp);


	MatricesBuffer.reset(new VulkanUniformBuffer(sizeof(glm::mat4), core));
	updateUniformBuffers();
}

void PerspectiveCamera::SetWindowSize(float width, float height)
{
	m_AspectRatio = m_ScreenWidth / m_ScreenHeight;
	m_ProjectionMatrix = glm::perspective(glm::radians(m_FOV), m_AspectRatio, 0.1f, 100.0f);

	updateUniformBuffers();
}

void PerspectiveCamera::RecalculateViewMatrix()
{
	m_CameraRight = glm::normalize(glm::cross(m_Front, m_CameraUp));

	m_Front.x = cos(glm::radians(m_Rotation.y)) * cos(glm::radians(m_Rotation.x));
	m_Front.y = sin(glm::radians(m_Rotation.x));
	m_Front.z = sin(glm::radians(m_Rotation.y)) * cos(glm::radians(m_Rotation.x));

	m_Front = glm::normalize(m_Front);

	m_ViewMatrix = glm::lookAt(m_Position, glm::normalize(m_Position + m_Front), m_WorldUp);

	updateUniformBuffers();
}
