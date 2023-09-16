#include "pch.h"
#include "PerspectiveCamera.h"

PerspectiveCamera::PerspectiveCamera(float width, float height, VulkanCore* core)
	:m_ScreenWidth(width), m_ScreenHeight(height)
{
	m_AspectRatio = m_ScreenWidth / m_ScreenHeight;

	m_ProjectionMatrix = glm::perspective(glm::radians(m_FOV), m_AspectRatio, 1.0f, 96.0f);
	//m_ViewMatrix = glm::lookAt(m_Position, m_Target, m_WorldUp);


	MatricesBuffer.reset(new VulkanUniformBuffer(sizeof(glm::mat4), core));
	RecalculateViewMatrix();
}

void PerspectiveCamera::SetWindowSize(float width, float height)
{
	m_AspectRatio = m_ScreenWidth / m_ScreenHeight;
	m_ProjectionMatrix = glm::perspective(glm::radians(m_FOV), m_AspectRatio, 1.0f, 96.0f);

	updateUniformBuffers();
}