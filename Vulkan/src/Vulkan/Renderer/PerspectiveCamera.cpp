#include "pch.h"
#include "PerspectiveCamera.h"

#include "Vulkan/Core/Input.h"

float PerspectiveCamera::m_yScroll = 1.0;

PerspectiveCamera::PerspectiveCamera(float width, float height, VulkanCore* core)
	:m_ScreenWidth(width), m_ScreenHeight(height)
{
	m_AspectRatio = m_ScreenWidth / m_ScreenHeight;

	m_ProjectionMatrix = glm::perspective(glm::radians(m_FOV), m_AspectRatio, 1.0f, 512.0f);
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

void PerspectiveCamera::OnUpdate(float deltaTime)
{
	if(Input::IsMouseButtonPressed(GLFW_MOUSE_BUTTON_2))
	{
		bIsInCameraMode = true;
		Input::SetCursorHidden(bIsInCameraMode);
		
		double currentMousePosX = Input::GetMousePosition().first;
		double currentMousePosY = Input::GetMousePosition().second;

		double deltaX = currentMousePosX - m_PrevMousePos.X;
		double deltaY = currentMousePosY - m_PrevMousePos.Y;
		
		if(Input::IsKeyPressed(GLFW_KEY_W))
		{
			addPosition(getForwardVector() * m_MovementSpeed * m_yScroll * deltaTime);
		}
		else if(Input::IsKeyPressed(GLFW_KEY_S))
		{
			addPosition(-getForwardVector() * m_MovementSpeed * m_yScroll * deltaTime);
		}

		if(Input::IsKeyPressed(GLFW_KEY_D))
		{
			addPosition(getRightVector() * m_MovementSpeed * m_yScroll * deltaTime);
		}
		else if(Input::IsKeyPressed(GLFW_KEY_A))
		{
			addPosition(-getRightVector() * m_MovementSpeed * m_yScroll * deltaTime);
		}
		
		if(Input::IsKeyPressed(GLFW_KEY_Q))
		{
			addPosition(m_WorldUp * m_MovementSpeed * m_yScroll * deltaTime);
		}
		else if(Input::IsKeyPressed(GLFW_KEY_E))
		{
			addPosition(-m_WorldUp * m_MovementSpeed * m_yScroll * deltaTime);
		}
		

		addRotation(glm::vec3(0.0f, 1.0f, 0.0f) * m_RotationSpeed * (float)deltaX * deltaTime);
		addRotation(glm::vec3(1.0f, 0.0f, 0.0f) * m_RotationSpeed * (float)deltaY * deltaTime);
		
	}
	else
	{
		bIsInCameraMode = false;

		Input::SetCursorHidden(bIsInCameraMode);
	}

	m_PrevMousePos.X = Input::GetMousePosition().first;
	m_PrevMousePos.Y = Input::GetMousePosition().second;

	if(Input::IsKeyPressed(GLFW_KEY_LEFT_CONTROL))
	{
		if(Input::IsKeyPressed(GLFW_KEY_C))
		{
			//VK_INFO("GLFW_KEY_Z FOV{0}", m_FOV);
			m_FOV = std::clamp(m_FOV + 30.0f * deltaTime, 5.0f, 150.0f);
		}
		else if(Input::IsKeyPressed(GLFW_KEY_Z))
		{
			//VK_INFO("GLFW_KEY_C FOV{0}", m_FOV);
			m_FOV = std::clamp(m_FOV - 30.0f * deltaTime, 5.0f, 150.0f);
		}

		m_ProjectionMatrix = glm::perspective(glm::radians(m_FOV), m_AspectRatio, 1.0f, 512.0f);
	}
	else
	{
		m_FOV = 60.0f;
		m_ProjectionMatrix = glm::perspective(glm::radians(m_FOV), m_AspectRatio, 1.0f, 512.0f);
	}
}

void PerspectiveCamera::OnUIUpdate(float deltaTime)
{
}

bool PerspectiveCamera::OnMouseScrollEvent(MouseScrollEvent& e)
{
	if(bIsInCameraMode)
	{
		m_yScroll += e.GetYScroll();
		m_yScroll = std::clamp(m_yScroll, 1.0f, 50.f);
		VK_INFO("{0}", m_yScroll);
	}

	return true;
}
