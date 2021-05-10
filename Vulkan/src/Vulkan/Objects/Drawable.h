#pragma once

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

#include "Object.h"

#include "Vulkan/Renderer/VulkanVertexBuffer.h"
#include "Vulkan/Renderer/VulkanIndexBuffer.h"
#include "Vulkan/Renderer/VulkanUniformBuffer.h"

enum class Space
{
	Local, Self, Global
};

class Drawable : public Object
{
public:

	virtual ~Drawable();

	virtual void OnUpdate(float deltaTime) {};

	virtual void draw(VkCommandBuffer commandBuffer, VkPipelineLayout pipelineLayout);
	
	virtual void SetPosition(glm::vec3 position);
	virtual void SetPosition(float* position);
	virtual void SetPosition(float x, float y, float z);
	virtual void SetPosition(float xyz);	
	
	virtual void SetScale(glm::vec3 scale);
	virtual void SetScale(float* scale);
	virtual void SetScale(float x, float y, float z);
	virtual void SetScale(float xyz);

	virtual void SetRotation(glm::vec3 rotation);
	virtual void SetRotation(float* rotation);
	virtual void SetRotation(float x, float y, float z);
	virtual void SetRotation(float xyz);
	
	virtual void Translate(glm::vec3 position);
	virtual void Rotate(float angle, glm::vec3 axis = glm::vec3(0.0f, 0.0f, 1.0f), Space space = Space::Local);

	std::unique_ptr<VulkanUniformBuffer> ModelBuffer;
	VkDescriptorSet DescriptorSet;

	glm::vec3 Position = glm::vec3(0.0f);
	glm::vec3 Scale    = glm::vec3(0.0f);
	glm::vec3 Rotation = glm::vec3(0.0f);

	std::unique_ptr<VulkanVertexBuffer> VertexBuffer;

protected:
	virtual void updateUniformBuffers();

protected:
	std::unique_ptr<VulkanIndexBuffer> m_IndexBuffer;

	glm::mat4 m_PositionMatrix = glm::mat4(1.0f);
	glm::mat4 m_ScaleMatrix = glm::mat4(1.0f);
	glm::mat4 m_RotationMatrix = glm::mat4(1.0f);

	glm::mat4 m_ModelMatrix = glm::mat4(1.0f);
};