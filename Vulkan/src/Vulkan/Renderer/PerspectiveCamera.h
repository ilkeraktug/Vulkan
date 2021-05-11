#pragma once

#include "Abstract/Camera.h"

class PerspectiveCamera : public Camera
{
public:
	PerspectiveCamera() = default;
	PerspectiveCamera(float width, float height, VulkanCore* core);

	void SetWindowSize(float width, float height);

	void SetRotation(glm::vec3 rotation) { m_Rotation; RecalculateViewMatrix(); }

private:
	void RecalculateViewMatrix() override;
private:

	float m_ScreenWidth;
	float m_ScreenHeight;
	float m_AspectRatio;

	float m_FOV = 45.0f;

	glm::vec3 m_Front = glm::vec3(0.0f, 0.0f, -1.0f);
	glm::vec3 m_Target   = glm::vec3(0.0f);
	glm::vec3 m_CameraUp = glm::vec3(0.0f, 1.0f, 0.0f);	
	glm::vec3 m_CameraRight = glm::vec3(1.0f, 0.0f, 0.0f);

	glm::vec3 m_WorldUp = { 0.0f, 1.0f, 0.0f };

	glm::vec3 m_Rotation = glm::vec3(0.0f);
};