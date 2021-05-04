#pragma once

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

#include "Object.h"

#include "Vulkan/Renderer/VulkanVertexBuffer.h"
#include "Vulkan/Renderer/VulkanIndexBuffer.h"
#include "Vulkan/Renderer/VulkanUniformBuffer.h"


class Drawable : public Object
{
public:

	virtual void OnUpdate(float deltaTime) {};

	virtual void draw(VkCommandBuffer commandBuffer, VkPipelineLayout pipelineLayout);
	virtual void allocDescriptorSet(VkDescriptorPool descriptorPool, VkDescriptorSetLayout* descriptorLayout) {};
	
	virtual void SetPosition(glm::vec3 position);
	virtual void SetPosition(float* position);
	virtual void SetPosition(float x, float y, float z);
	virtual void SetPosition(float xyz);

	std::unique_ptr<VulkanUniformBuffer> ModelBuffer;
	VkDescriptorSet DescriptorSet;
protected:
	std::unique_ptr<VulkanVertexBuffer> m_VertexBuffer;
	std::unique_ptr<VulkanIndexBuffer> m_IndexBuffer;


	glm::vec3 m_Position = glm::vec3(0.0f);
	glm::mat4 m_ModelMatrix = glm::mat4(1.0f);
};