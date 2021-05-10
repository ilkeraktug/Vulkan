#pragma once

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

#include "Vulkan/Renderer/VulkanUniformBuffer.h"

class Camera
{
public:

	void SetPosition(const glm::vec3& position) { m_Position = position; RecalculateViewMatrix(); }
	const glm::vec3& GetPosition() const { return m_Position; }

	glm::mat4 GetViewMatrix() { return m_ViewMatrix; }
	glm::mat4 GetProjectionMatrix() { return m_ProjectionMatrix; }
	glm::mat4 GetViewProjectionMatrix() { return m_ViewProjectionMatrix; }

	VkDescriptorSet descriptorSet;
public:
	std::unique_ptr<VulkanUniformBuffer> MatricesBuffer;
protected:
	virtual void RecalculateViewMatrix() = 0;

	virtual void updateUniformBuffers()
	{
		m_ViewProjectionMatrix = m_ProjectionMatrix * m_ViewMatrix;
		MatricesBuffer->copyData(&m_ViewProjectionMatrix, sizeof(glm::mat4));
	}
protected:
	glm::mat4 m_ViewMatrix;
	glm::mat4 m_ProjectionMatrix;
	glm::mat4 m_ViewProjectionMatrix;

	glm::vec3 m_Position = { 0.0f, 0.0f, -0.5f };
private:
};