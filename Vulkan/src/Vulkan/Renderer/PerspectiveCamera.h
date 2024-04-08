#pragma once

#include "Abstract/Camera.h"

class PerspectiveCamera : public Camera
{
public:
	PerspectiveCamera() = default;
	PerspectiveCamera(float width, float height, VulkanCore* core);
	
	virtual void OnUpdate(float deltaTime) override;
	virtual void OnUIUpdate(float deltaTime) override;

	static bool OnMouseScrollEvent(MouseScrollEvent& e);
	
	void SetWindowSize(float width, float height);
private:

	float m_ScreenWidth;
	float m_ScreenHeight;
	float m_AspectRatio;

	float m_FOV = 60.0f;
	float m_TempFOV = 60.0f;

	static float m_yScroll;
	float m_MovementSpeed = 1.0f;
	float m_RotationSpeed = 90.0f;

	inline static bool bIsInCameraMode = false;
	struct
	{
		double X;
		double Y;
	} m_PrevMousePos;

	glm::vec3 m_Front = glm::vec3(0.0f, 0.0f, -1.0f);
	glm::vec3 m_Target   = glm::vec3(0.0f);
	glm::vec3 m_CameraUp = glm::vec3(0.0f, 1.0f, 0.0f);	
	glm::vec3 m_CameraRight = glm::vec3(1.0f, 0.0f, 0.0f);

	glm::vec3 m_WorldUp = { 0.0f, 1.0f, 0.0f };
};