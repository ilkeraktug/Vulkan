#pragma once

#include "Abstract/Camera.h"

#include "VulkanCore.h"

class OrthographicCamera : public Camera
{
public:
	OrthographicCamera() = default;
	OrthographicCamera(float left, float right, float bottom, float top, VulkanCore* core);

	void SetOrthograhic(float left, float right, float bottom, float top) { RecalculateProjectionMatrix(left, right, bottom, top); }

	void SetRotation(float rotation) { m_Rotation = rotation; RecalculateViewMatrix(); }
	float GetRotation() const { return m_Rotation; }

private:
	void RecalculateProjectionMatrix(float left, float right, float bottom, float top);
private:

	float m_Rotation = 0.0f;
};