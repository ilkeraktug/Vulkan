#pragma once

#include <GLFW/glfw3.h>

class Time
{
public:
	static void OnUpdate()
	{
		float currentTime = (float)glfwGetTime();
		deltaTime = currentTime - m_LastTime;
		m_LastTime = currentTime;
	}

	static float deltaTime;
private:
	static float m_LastTime;
};