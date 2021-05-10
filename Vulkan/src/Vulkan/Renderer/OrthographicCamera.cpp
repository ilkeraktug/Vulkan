#include "pch.h"
#include "OrthographicCamera.h"

OrthographicCamera::OrthographicCamera(float left, float right, float bottom, float top, VulkanCore* core)
{
	m_ProjectionMatrix = glm::ortho(left, right, bottom, top, -1.0f, 1.0f);
	m_ViewMatrix = glm::mat4(1.0f);
	
	MatricesBuffer.reset(new VulkanUniformBuffer(sizeof(glm::mat4), core));
	updateUniformBuffers();
}

void OrthographicCamera::RecalculateViewMatrix()
{
	m_ViewMatrix = glm::translate(glm::mat4(1.0f), m_Position) * glm::rotate(glm::mat4(1.0f), glm::radians(m_Rotation), glm::vec3(0.0f, 0.0f, 1.0f));

	updateUniformBuffers();
}

void OrthographicCamera::RecalculateProjectionMatrix(float left, float right, float bottom, float top)
{
	m_ProjectionMatrix = glm::ortho(left, right, bottom, top, -1.0f, 1.0f);

	updateUniformBuffers();
}
