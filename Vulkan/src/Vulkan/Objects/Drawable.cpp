#include "pch.h"
#include "Drawable.h"

Drawable::~Drawable()
{
}

void Drawable::draw(VkCommandBuffer commandBuffer, VkPipelineLayout pipelineLayout, int i)
{

	vkCmdBindDescriptorSets(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, pipelineLayout, 0, 1, &DescriptorSets[i], 0, nullptr);

	VkDeviceSize offsets[] = { 0 };
	vkCmdBindVertexBuffers(commandBuffer, 0, 1, &VertexBuffer->GetBuffer(), offsets);
	vkCmdBindIndexBuffer(commandBuffer, m_IndexBuffer->GetBuffer(), 0, m_IndexBuffer->GetIndexType());

	vkCmdDrawIndexed(commandBuffer, m_IndexBuffer->GetCount(), 1, 0, 0, 0);
}

void Drawable::SetPosition(glm::vec3 position)
{
	Position = position;
	// The coordinates of OpenGL and Vulkan API are opposite to each other. glm library meant to use on OpenGL applications.Hence, Y axis is mirrored by X axis.
	Position.y *= -1.0f;

	m_PositionMatrix = glm::translate(glm::mat4(1.0f), Position);

	m_ModelMatrix = m_PositionMatrix * m_ScaleMatrix * m_RotationMatrix;
	updateUniformBuffers();
}

void Drawable::SetPosition(float* position)
{
	Position.x = position[0];
	Position.y = position[1];
	Position.z = position[2];

	// The coordinates of OpenGL and Vulkan API are opposite to each other. glm library meant to use on OpenGL applications.Hence, Y axis is mirrored by X axis.
	Position.y *= -1.0f;

	m_PositionMatrix = glm::translate(glm::mat4(1.0f), Position);

	m_ModelMatrix = m_PositionMatrix * m_ScaleMatrix * m_RotationMatrix;
	updateUniformBuffers();
}

void Drawable::SetPosition(float x, float y, float z)
{
	Position.x = x;
	Position.y = y;
	Position.z = z;

	// The coordinates of OpenGL and Vulkan API are opposite to each other. glm library meant to use on OpenGL applications.Hence, Y axis is mirrored by X axis.
	Position.y *= -1;

	m_PositionMatrix = glm::translate(glm::mat4(1.0f), Position);

	m_ModelMatrix = m_PositionMatrix * m_ScaleMatrix * m_RotationMatrix;
	updateUniformBuffers();
}

void Drawable::SetPosition(float xyz)
{
	Position.x = xyz;
	Position.y = xyz;
	Position.z = xyz;

	// The coordinates of OpenGL and Vulkan API are opposite to each other. glm library meant to use on OpenGL applications.Hence, Y axis is mirrored by X axis.
	Position.y *= -1;

	m_PositionMatrix = glm::translate(glm::mat4(1.0f), Position);

	m_ModelMatrix = m_PositionMatrix * m_ScaleMatrix * m_RotationMatrix;
	updateUniformBuffers();
}

void Drawable::SetScale(glm::vec3 scale)
{
	Scale = scale;

	m_ScaleMatrix = glm::scale(glm::mat4(1.0f), Scale);

	m_ModelMatrix = m_PositionMatrix * m_ScaleMatrix * m_RotationMatrix;
	updateUniformBuffers();
}

void Drawable::SetScale(float* scale)
{
	Scale.x = scale[0];
	Scale.y = scale[1];
	Scale.z = scale[2];

	m_ScaleMatrix = glm::scale(glm::mat4(1.0f), Scale);

	m_ModelMatrix = m_PositionMatrix * m_ScaleMatrix * m_RotationMatrix;
	updateUniformBuffers();
}

void Drawable::SetScale(float x, float y, float z)
{
	Scale.x = x;
	Scale.y = y;
	Scale.z = z;

	m_ScaleMatrix = glm::scale(glm::mat4(1.0f), Scale);

	m_ModelMatrix = m_PositionMatrix * m_ScaleMatrix * m_RotationMatrix;
	updateUniformBuffers();
}

void Drawable::SetScale(float xyz)
{
	Scale.x = xyz;
	Scale.y = xyz;
	Scale.z = xyz;

	m_ScaleMatrix = glm::scale(glm::mat4(1.0f), Scale);

	m_ModelMatrix = m_PositionMatrix * m_ScaleMatrix * m_RotationMatrix;
	updateUniformBuffers();
}

void Drawable::SetRotation(glm::vec3 rotation)
{
	Rotation = rotation;

	m_RotationMatrix = glm::rotate(glm::mat4(1.0f), glm::radians(Rotation.x), glm::vec3(1.0f, 0.0f, 0.0f));
	m_RotationMatrix *= glm::rotate(glm::mat4(1.0f), glm::radians(Rotation.y), glm::vec3(0.0f, 1.0f, 0.0f));
	m_RotationMatrix *= glm::rotate(glm::mat4(1.0f), glm::radians(Rotation.z), glm::vec3(0.0f, 0.0f, 1.0f));

	m_ModelMatrix = m_PositionMatrix * m_ScaleMatrix * m_RotationMatrix;
	updateUniformBuffers();
}

void Drawable::SetRotation(float* rotation)
{
	Rotation.x = rotation[0];
	Rotation.y = rotation[1];
	Rotation.z = rotation[2];

	m_RotationMatrix = glm::rotate(glm::mat4(1.0f), glm::radians(Rotation.x), glm::vec3(1.0f, 0.0f, 0.0f));
	m_RotationMatrix *= glm::rotate(glm::mat4(1.0f), glm::radians(Rotation.y), glm::vec3(0.0f, 1.0f, 0.0f));
	m_RotationMatrix *= glm::rotate(glm::mat4(1.0f), glm::radians(Rotation.z), glm::vec3(0.0f, 0.0f, 1.0f));

	m_ModelMatrix = m_PositionMatrix * m_ScaleMatrix * m_RotationMatrix;
	updateUniformBuffers();
}

void Drawable::SetRotation(float x, float y, float z)
{
	Rotation.x = x;
	Rotation.y = y;
	Rotation.z = z;

	m_RotationMatrix = glm::rotate(glm::mat4(1.0f), glm::radians(Rotation.x), glm::vec3(1.0f, 0.0f, 0.0f));
	m_RotationMatrix *= glm::rotate(glm::mat4(1.0f), glm::radians(Rotation.y), glm::vec3(0.0f, 1.0f, 0.0f));
	m_RotationMatrix *= glm::rotate(glm::mat4(1.0f), glm::radians(Rotation.z), glm::vec3(0.0f, 0.0f, 1.0f));

	m_ModelMatrix = m_PositionMatrix * m_ScaleMatrix * m_RotationMatrix;
	updateUniformBuffers();

}

void Drawable::SetRotation(float xyz)
{
	Rotation.x = xyz;
	Rotation.y = xyz;
	Rotation.z = xyz;

	m_RotationMatrix = glm::rotate(glm::mat4(1.0f), glm::radians(Rotation.x), glm::vec3(1.0f, 0.0f, 0.0f));
	m_RotationMatrix *= glm::rotate(glm::mat4(1.0f), glm::radians(Rotation.y), glm::vec3(0.0f, 1.0f, 0.0f));
	m_RotationMatrix *= glm::rotate(glm::mat4(1.0f), glm::radians(Rotation.z), glm::vec3(0.0f, 0.0f, 1.0f));

	m_ModelMatrix = m_PositionMatrix * m_ScaleMatrix * m_RotationMatrix;
	updateUniformBuffers();
}

void Drawable::Translate(glm::vec3 position)
{
	m_PositionMatrix = glm::translate(m_PositionMatrix, position);

	Position.x = m_PositionMatrix[3][0];
	Position.y = m_PositionMatrix[3][1];
	Position.z = m_PositionMatrix[3][2];

	m_ModelMatrix = m_PositionMatrix * m_ScaleMatrix * m_RotationMatrix;
	updateUniformBuffers();
}

void Drawable::Rotate(float angle, glm::vec3 axis, Space space)
{
	//TODO : Corret position and rotation
	if (space == Space::Local)
	{
		m_RotationMatrix = glm::rotate(m_RotationMatrix, glm::radians(angle), axis);
		m_ModelMatrix = m_PositionMatrix * m_ScaleMatrix * m_RotationMatrix;
	}
	else if (space == Space::Self)
	{
		m_RotationMatrix = glm::rotate(m_RotationMatrix, glm::radians(angle), axis);
		m_ModelMatrix = m_PositionMatrix * m_ScaleMatrix * glm::inverse(m_RotationMatrix);
		//m_ModelMatrix = glm::inverse(m_RotationMatrix) * m_PositionMatrix;

		//glm::vec3 x = glm::vec3(1, 0, 0) * glm::radians(angle);
		//glm::vec3 y = glm::vec3(0, 1, 0) * glm::radians(angle);
		//glm::vec3 z = glm::vec3(0, 0, 1) * glm::radians(angle);
		//glm::vec3 r = x + y + z;
		//m_RotationMatrix = glm::rotate(m_RotationMatrix, glm::length(r), glm::normalize(r));
		//
		//m_ModelMatrix = m_PositionMatrix * m_RotationMatrix;
	}
	else if (space == Space::Global)
	{
		m_RotationMatrix = glm::rotate(m_RotationMatrix, glm::radians(angle), axis);
		m_ModelMatrix = glm::inverse(m_RotationMatrix) * m_ScaleMatrix * m_PositionMatrix;
		
		//m_ModelMatrix = m_RotationMatrix * m_PositionMatrix;
	}

	updateUniformBuffers();
}

bool Drawable::CheckCollision(const Drawable& other, float screenTop)
{
	if (other.Position.x <= Position.x + Scale.x / 2.0f && other.Position.x >= Position.x - Scale.x / 2.0f)
	{
		if (Position.y <= 0.0f)
		{
			if (-other.Position.y >= ((-Scale.y / 2) + screenTop - (other.Scale.y / 2)))
				return true;
		}
		else
		{
			if (-other.Position.y <= ((Scale.y / 2) - screenTop + (other.Scale.y / 2)))
				return true;
		}

		return false;
	}
	else
		return false;
}

void Drawable::updateUniformBuffers()
{
	ModelBuffer->copyData(&m_ModelMatrix, sizeof(glm::mat4));
}
